<?php // $Id$

/**
 * Return the geo backend database type.
 *
 * @return string
 *  The name of the backend: postgis, cck, mysql_spatial
 */
function geo_backend_type() {
  static $backend;

  if (!isset($backend)) {
    switch ($GLOBALS['db_type']) {
      case 'pgsql':
        // use the postgis backend if installed
        if (db_result(db_query("SELECT 1 FROM pg_catalog.pg_proc 
          WHERE proname = 'postgis_version'"))) {
          $backend = 'postgis';
        }
        break;

      case 'mysql':
      case 'mysqli':
        // Capabilities check.
        // If it can convert between native and wkt, assume it's working.
        if (db_result(@db_query("SELECT AsText(GeomFromText('POINT(1 1)'))")) == 'POINT(1 1)') {
          $backend = 'mysql_spatial';
        }
    }
  }

  return $backend;
}

function geo_supported_types() {
  return array(
    'point' => t('Point'),
    'linestring' => t('Linestring'),
    'polygon' => t('Polygon'),
  );
}

// TODO seriously clean this up! And add data types!
function geo_wkt($data) {
  if (!is_scalar($data)) {
    $data = (object) $data;
    if (!isset($data->type)) $data->type = 'point';
    switch ($data->type) {
      case 'point':
        return 'POINT('. $data->lon .' '. $data->lat .')';
    }
  }
}

function geo_wkt_from_point($lat, $lon) {
  if (strlen($lat) || strlen($lon)) {
    return 'POINT('. $lon .' '. $lat .')';
  }
}

function geo_field_select($field) {
  if (!$field) return;
  $field = check_plain($field);
  $alias = str_replace('.', '_', $field);
  $alias = preg_replace('/_geo$/', '', $alias);

  return " SRID($field) as {$alias}_srid, 
    AsBinary($field) AS {$alias}_wkb,
    AsBinary(Envelope($field)) AS {$alias}_bbox,
    AsBinary(Centroid($field)) AS $alias}_centroid";
}

function geo_query_operators($geo_type) {
  return array(
    'Equals' => t('Equals'),
    'NOT Equals' => t('Does not equal'),
    'Intersects' => t('Intersects'),
    'NOT Intersects' => t('Does not intersect'),
    'Within' => t('Within'),
    'NOT Within' => t('Not within'),
  );
}

function geo_query_functions($geo_type) {
  $geo_type = strtolower($geo_type);
  $functions = array( 'wkt' => 'wkt', 'wkb' => 'wkb' );

  if ($geo_type == 'point') {
    $functions['lat'] = 'lat';
    $functions['lon'] = 'lon';
  }

  if ($geo_type == 'polygon' || $geo_type == 'linestring') {
    $functions['area'] = 'area';
    $functions['bbox'] = 'bbox';
    $functions['perimeter'] = 'perimeter';
  }

  if ($geo_type == 'linestring') {
    $functions['start'] = 'start';
    $functions['end'] = 'end';
  }

  // TODO distance? given that we have a starting point?

  return $functions;
}

function geo_query_handlers($geo_type) {
  // TODO are these all supported by default?  (and through which functions?)
  return array(
    'wkt' => t('Well Known Text'),
    'wkb' => t('Well Known Binary'),
    'svg' => t('SVG'),
    'kml' => t('KML'),
    'gml' => t('GML'),
  );
}

function geo_query_area($field, $srid = null) {
  return 'Area('. $field .')';
}

function geo_query_bbox($field, $srid = null) {
  return 'Envelope('. $field .')';
}

function geo_query_centroid($field, $srid = null) {
  return 'Centroid('. $field .')';
}

function geo_query_geo_type($field, $srid = null) {
  return 'GeometryType('. $field .')';
}

function geo_query_end($field, $srid = null) {
  return 'EndPoint('. $field .')';
}

function geo_query_lat($field, $srid = null) {
  return 'Y('. $field .')';
}

function geo_query_lon($field, $srid = null) {
  return 'X('. $field .')';
}

function geo_query_length($field, $srid = null) {
  return 'Length('. $field .')';
}

function geo_query_perimeter($field, $srid = null) {
  return 'Length(ConvexHull('. $field .')';
}

function geo_query_start($field, $srid = null) {
  return 'StartPoint('. $field .')';
}

function geo_query_wkt($field, $srid = null) {
  return 'asText('. $field .')';
}

function geo_query_wkb($field, $srid = null) {
  return 'asBinary('. $field .')';
}

function geo_query_distance($field, $srid = null, $target) {
  $target = "GeomFromText('POINT(". $target['lon'] .','. $target['lat'] .")')";
  return 'Distance('. $field .', '. $target .')';
}

function geo_query_within($field, $srid = null, $target) {
  return "Within($field, GeomFromText('". $target['wkt'] ."'))";
}

function geo_query_contains($field, $srid = null, $target) {
  return "Contains(GeomFromText('". $target['wkt'] ."'), $field)";
}

function geo_query_extent($field) {
  return "ST_Extent($field)";
}

function geo_make_bbox($target, $limit) {
  $x = $target['lon'];
  $y = $target['lat'];
  $offset_x = $limit / (GEO_DEGREE_M * cos($y));
  $offset_y = $limit / GEO_DEGREE_M;

  $offset_x = $limit / GEO_DEGREE_M; // TODO Miles
  $offset_y = $limit / round(GEO_DEGREE_M * cos($x / 92215));

  $bbox = array(
    'points' => array(
      array('lon' => $x - $offset_x, 'lat' => $y + $offset_y),
      array('lon' => $x + $offset_x, 'lat' => $y + $offset_y),
      array('lon' => $x + $offset_x, 'lat' => $y - $offset_y),
      array('lon' => $x - $offset_x, 'lat' => $y - $offset_y),
      array('lon' => $x - $offset_x, 'lat' => $y + $offset_y),
    ),
  );
  // TODO call a function?
  $wkt = '';
  foreach ($bbox['points'] as $p) $wkt .= $p['lon'] .' '. $p['lat'] .', ';
  $bbox['wkt'] = 'POLYGON(('. substr($wkt, 0, -2) .'))';
  return $bbox;
}

/**
 * Insert a geometry object into the database on the specified row
 *
 * @param $table
 *  The name of the table that currently holds the geometry information.
 * @param $field_name
 *  The name of the field in which to insert the data.
 * @return void
 */
function geo_geo_from_wkt($wkt, $srid = GEO_DEFAULT_SRID) {
  return " GeomFromText('". $wkt ."') ";
}

function geo_rss_item($wkb) {
  $data = geo_wkb_get_data($wkb, 'text');
  return array(
    'key' => 'georss:'. strtolower($data['type']),
    'value' => $data['value'],
    'namespace' => array('xmlns:georss' => 'http://www.georss.org/georss'),
  );
}

function geo_gis_input_selector($default_value) {
  // Provide a selection of input handlers.
  $input_types = array();
  foreach (module_invoke_all('gis_input_info') as $name => $info) {
    $input_types[$name] = $info['title'];
  }

  $form = array(
    '#type' => 'select',
    '#title' => 'Widget',
    '#options' => $input_types,
    '#default_value' => $default_value,
  );
  return $form;
}

function geo_wkt_validate($wkt, $geo_type = NULL, $srid = GEO_DEFAULT_SRID) {

  // TODO this should probabaly be moved into db-specific backend files.
  $type = db_result(@db_query("SELECT GeometryType(GeomFromText('%s'))", $wkt));

  if (empty($type)) {
    return t('Unable to parse WKT.');
  }

  if ($geo_type && strtolower($geo_type) != strtolower($type)) {
    return t('Wrong geometry type. Got %result, was expecting %type.', array('%result' => $type, '%type' => $geo_type));
  }

  // All's well that ends well!
  return TRUE;
}
