<?php // $Id$

function geo_wkb_types($key = NULL) {
  $geo_types = array(
    0 => 'none',
    1 => 'point',
    3 => 'linestring',  // This is specified as 'polyline' in the standard.
    5 => 'polygon',
    8 => 'multipoint',
    11 => 'pointz',
    13 => 'linestringz',
    15 => 'polygonz',
    18 => 'multipointz',
    21 => 'pointm',
    23 => 'linestingm',
    25 => 'polygonm',
    28 => 'multipointm',
    31 => 'multipatch',
  );

  if ($key) return $geo_types[$key];
  return $geo_types;
}

function geo_wkb_get_data($wkb = NULL, $format = 'text', $fp = NULL, $type = NULL) {
  $data = array();

  if (!$fp) {
    // Put our WKB data into memory so we can navigate around it with fseek.
    $fp = fopen("php://memory", 'r+');
    fputs($fp, $wkb);
    fseek($fp, 0);

    // Fetch the byte order (0 = Big Endian, 1 = Little Endian) and geo type.
    $data = unpack('corder/Ltype', fread($fp, 5));

    // Set the geometry type
    $data['type'] = geo_wkb_types($data['type']);
  }

  $func = 'geo_wkb_get_'. $format;
 
  switch($data['type']) {
    case 'point': // Contains x, y decimal values.
      $xy = unpack('dx/dy', fread($fp, 16));
      $data['value'] = $func($xy);
      $data['value'] = $func($data);
      return $data;

    case 'pointz': // Contains x, y, z decimal values.
      $xyz = unpack('dx/dy', fread($fp, 24));
      $data['value'] = $func($xyz);
      $data['value'] = $func($data);
      return $data;

    case 'linestring': // Contains count * (x, y) decimal values.
    case 'multipoint':
      $data['count'] = current(unpack('L', fread($fp, 4)));
      for ($i = 0; $i <= $data['count']; $i++) {
        $data['value'][$i] = geo_wkb_get_data(NULL, $format, $fp, 'point');
      }
      $data['value'] = $func($data['value']);
      return $data;

    case 'linestringz': // Contains count * (x, y, z) decimal values.
    case 'multipointz':
      $data['count'] = current(unpack('L', fread($fp, 4)));
      for ($i = 0; $i <= $data['count']; $i++) {
        $data['value'][$i] = geo_wkb_get_data(NULL, $format, $fp, 'pointz');
      }
      $data['value'] = $func($data['value']);
      return $data;

    case 'polygon': // Contains count * (line) items.
      $data['count'] = unpack('L', fread($fp, 4));
      for ($i = 0; $i <= $data['count']; $i++) {
        $data['value'][$i] = geo_wkb_get_data(NULL, $format, $fp, 'linestring');
      }
      $data['value'] = $func($data['value']);
      break;

    case 'polylinez':
      $pack .= 'Lcount/d*'; // Expecting count records of X, Y (, Z) decimals.
      break;

    case 'polygonz':
    case 'pointm':
    case 'polylinem':
    case 'polygonm':
    case 'multipointm':
    case 'multipatch':
      break;
    default:
      return $data;
  }
 

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

/**
 * WKT format.
 */
function geo_wkb_get_wkt($data) {
  $out = '';
  if (isset($data['x']) && isset($data['y'])) {
    $out = $data['x'] .' '. $data['y'];
    if (isset($data['z'])) $out .= ' '. $data['z'];
  }
  elseif (isset($data['value'])) {
    if (isset($data['type'])) $out = strtoupper($data['type']);
    $out .= '('. $data['value'] .')';
  }
  elseif (is_array($data)) {
    $out = join(', ', $data);
  }
  return $out;
}

/**
 * Text format is suitable for many outputs, such as SVG, GeoRSS, etc.
 */
function geo_wkb_get_text($data, $type = NULL) {
  $out = '';
  if (isset($data['x']) && isset($data['y'])) {
    $out = $data['y'] .' '. $data['x'];
    if (isset($data['z'])) $out .= ' '. $data['z'];
  }
  elseif (isset($data['value'])) {
    $out = $data['value'];
  }
  elseif (is_array($data)) {
    $out = join(', ', $data);
  }
  return $out;
}

/**
 * Array, in case that's all you wanted.
 */
function geo_wkb_get_array($data) {
  return $data;
}
