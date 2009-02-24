<?php // $Id$

function geo_wkb_types($key = null) {
  $geo_types = array(
    0 => 'none',
    1 => 'point',
    3 => 'linestring',  //TODO this is specified as 'polyline' in the standard.
    5 => 'polygon',
    8 => 'multipoint',
    11 => 'pointz',
    13 => 'polylinez',
    15 => 'polygonz',
    18 => 'multipointz',
    21 => 'pointm',
    23 => 'polylinem',
    25 => 'polygonm',
    28 => 'multipointm',
    31 => 'multipatch',
  );

  if ($key) return $geo_types[$key];
  return $geo_types;
}

function geo_wkb_get_data($wkb, $format = NULL, &$type = NULL) {
  $data = unpack('corder/Ltype/d*', $wkb);
  $order = array_shift($data);
  $type = geo_wkb_types(array_shift($data));

  if ($format == 'points') {
    $points = array();
    switch ($type) {
      case 'point':
      case 'linestring':
      case 'polygon':
        if ($type != 'point') {
          array_shift($points); // TODO I just made that up.
        }
        foreach ($data as $k => $val) {
          if (isset($lon)) {
            $lat = $val;
            $points[] = array('lat' => $lat, 'lon' => $lon);
            unset($lon);
          }
          else $lon = $val;
        }
        break;
    }
    if ($type == 'point') $points = $points[0];
    return $points;
  }

  return $data;
}
