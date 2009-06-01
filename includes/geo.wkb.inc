<?php // $Id$

function geo_wkb_types($key = NULL) {
  $geo_types = array(
    1 => 'point',
    2 => 'linestring',
    3 => 'polygon',
    4 => 'multipoint',
    5 => 'multilinestring',
    6 => 'multipolygon',
  );

  if ($key) return $geo_types[$key];
  return $geo_types;
}

function _geo_wkb_get_data($wkb = NULL, $format = 'text', $fp = NULL, $type = NULL) {
  $data = array();

  if (!$fp) {
    if (empty($wkb)) return;
    // Put our WKB data into memory so we can cruise around with fread.
    $fp = fopen("php://memory", 'r+');
    fputs($fp, $wkb);
    fseek($fp, 0);

    // Fetch the byte order (0 = Big Endian, 1 = Little Endian) and geo type.
    $data = unpack('cbyte_order/Ltype', fread($fp, 5));

    // Set the geometry type
    $data['type'] = geo_wkb_types($data['type']);
  }

  $func = 'geo_wkb_get_'. $format;

  if (!isset($type)) $type = $data['type'];
 
  switch($type) {

    case 'point': // Contains x, y decimal values.
      $data = array_merge($data, unpack('dx/dy', fread($fp, 16)));
      $data['value'] = $func($data);
      if (function_exists($post = $func .'_post')) $data = $post($data);
      return $data;

    case 'linestring': // Contains count * (point) values.
      $data['count'] = current(unpack('L', fread($fp, 4)));
      for ($i = 1; $i <= $data['count']; $i++) {
        $point = geo_wkb_get_data(NULL, $format, $fp, 'point');
        $data['value'] = $func($point, $data['value']);
      }
      if (function_exists($post = $func .'_post')) $data = $post($data);
      return $data;

    case 'polygon': // Contains count * (linestring) items.
      $data['count'] = current(unpack('L', fread($fp, 4)));
      for ($i = 1; $i <= $data['count']; $i++) {
        $line = geo_wkb_get_data(NULL, $format, $fp, 'linestring');
        $data['value'] = $func($line, $data['value']);
      }
      if (function_exists($post = $func .'_post')) $data = $post($data);
      return $data;
  }
}

/**
 * WKT format.
 */
function geo_wkb_get_wkt($data, $items = NULL) {
  $out = '';
  if (isset($data['x']) && isset($data['y'])) {
    $out = $data['x'] .' '. $data['y'];
    if (isset($data['z'])) $out .= ' '. $data['z'];
  }
  elseif (isset($data['value'])) {
    $out = $data['value'];
  }

  if (isset($items)) {
    return $items .= ', '. $out;
  }
  return $out;
}

function geo_wkb_get_wkt_post($data) {
  if (isset($data['value'])) {
    $value = '';
    if (isset($data['type'])) $value = strtoupper($data['type']);
    $data['value'] = $value .'('. $data['value'] .')';
  }
  return $data;
}

/**
 * Text format is suitable for many outputs, such as SVG, GeoRSS, etc.
 */
function geo_wkb_get_text($data, $items = NULL) {
  $out = '';
  if (isset($data['x']) && isset($data['y'])) {
    $out = $data['y'] .' '. $data['x'];
    if (isset($data['z'])) $out .= ' '. $data['z'];
  }
  elseif (isset($data['value'])) {
    $out = $data['value'];
  }

  if (isset($items)) {
    return $items .= ' '. $out;
  }
  return $out;
}

/**
 * Array, in case that's all you wanted.
 */
function geo_wkb_get_array($data, $items = NULL) {
  if (isset($data['x']) && isset($data['y'])) {
    $ret = array('lon' => $data['x'], 'lat' => $data['y']);
    $ret = array('lon' => $data['x'], 'lat' => $data['y']);
  }
  elseif (isset($data['value'])) {
    $ret = $data['value'];
  }
  if (isset($items)) {
    return array_merge($items, array($ret));
  }
  return array($ret);
}

function geo_wkb_get_array_post($data, $items = NULL) {
  if ($data['type'] == 'point') $data['value'] = $data['value'][0];
  return $data;
}

/**
 * KML format.
 * It's actually the same thing as WKT with no post-process.
 */
function geo_wkb_get_kml($data, $items = NULL) {
  $out = '';
  if (isset($data['x']) && isset($data['y'])) {
    $out = $data['x'] .','. $data['y'];
    if (isset($data['z'])) $out .= ','. $data['z'];
  }
  elseif (isset($data['value'])) {
    $out = $data['value'];
  }

  if (isset($items)) {
    return $items .= "\n". $out;
  }
  return $out;
}
