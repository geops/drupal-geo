<?php // $Id$

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

function geo_rss_item($wkb) {
  $data = geo_wkb_get_data($wkb, 'text');
  return array(
    'key' => 'georss:'. strtolower($data['type']),
    'value' => $data['value'],
    'namespace' => array('xmlns:georss' => 'http://www.georss.org/georss'),
  );
}

function geo_wkt_validate($wkt, $geo_type = NULL, $srid = GEO_SRID_DEFAULT) {

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
