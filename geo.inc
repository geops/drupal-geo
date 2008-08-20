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
  return array('point', 'linestring', 'polygon');
}

function geo_wkt_from_point($lat, $lon) {
  return 'POINT('. $lon .' '. $lat .')';
}


function geo_field_select($field) {
  return " SRID($field) as srid, 
    asText($field) AS wkt,
    asText(centroid($field)) AS centroid,
    asText(envelope($field)) as bbox,
    Dimension($field) as dimension,
    GeometryType($field) AS type ";
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
  $functions = array( 'wkt' => 'wkt' );

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

  return $fields;
}

function geo_query_handlers($geo_type) {
  // TODO are these all supported by default?  (and through which functions?)
  return array(
    'wkt' => t('Well Known Text'),
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
  //TODO return 'EndPoint('. $field .')';
}

function geo_query_lon($field, $srid = null) {
  //TODO return 'EndPoint('. $field .')';
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
