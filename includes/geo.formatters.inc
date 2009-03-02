<?php // $Id$
/**
 * @file
 * Field formatters for geo_field fields.
 */
function theme_geo_formatter_default($element) {
  if (isset($element['#item']['wkt'])) return $element['#item']['wkt'];
}

function theme_geo_formatter_lat($element) {
  if (isset($element['#item']['lat'])) return $element['#item']['lat'];
  // TODO polygons, lines, etc won't have lat/lon but they'll have a centroid.
}

function theme_geo_formatter_lon($element) {
  if (isset($element['#item']['lon'])) return $element['#item']['lon'];
  // TODO polygons, lines, etc won't have lat/lon but they'll have a centroid.
}

function theme_geo_formatter_georss($element) {
  module_load_include('inc', 'geo', 'includes/geo.wkb');
  $item = $element['#item'];
  $data = geo_wkb_get_data($item['wkb'], 'text');

  if (in_array($data['type'], array('point', 'line', 'polygon'))) {
    $tag = 'georss:'. strtolower($item['type']);
    $value = $data['value'];
  }
  elseif($item['bbox']) {
    $tag = 'georss:bbox';
    $value = $item['bbox'];
  }
  if (isset($tag)) {
    return '<'. $tag .'>' . $value .'</'. $tag .'>';
  }
}

function theme_geo_formatter_svg($element) {
  // TODO this should be moved to a geo() call so that the backend can handle
  // this on its own.
  module_load_include('inc', 'geo', 'includes/geo.wkb');

  $data = geo_wkb_get_data($element['#item']['wkb'], 'text');
  if ($data['type'] == 'point') {
    $data = ' cx="'. $data['x'] .'" cy="'. $data['y'] .'" ';
  }
  elseif($data['type'] == 'polygon' || $data['type'] == 'linestring') {
    $data = ''. $data['value'] .'';
  }

  $bbox = $element['#item']['bbox'];
  preg_match('|[.*\(]+(.*[^\)]+)|', $bbox, $matches);
  $bbox = explode(' ', str_replace(',' ,' ', $matches[1]));
  $set = FALSE;
  $lat = $lon = array();
  foreach($bbox as $box) {
    if ($set) {
      $lat[] = $box;
      $set = FALSE;
    }
    else {
      $lon[] = $box;
      $set = TRUE;
    }
  }
  $x = min($lat);
  $y = min($lon);
  $width = ((max($lat) - $x) * 1.05);
  $height = ((max($lon) - $y) * 1.05);
  $out = '<?xml version="1.0" standalone="no"?>
<!DOCTYPE svg PUBLIC "-//W3C//DTD SVG 1.1//EN"
  "http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd">';
  $out .='<svg id="mainmap" width="50px" height="50px"
  viewBox=" '. $y .' '. $x .' '. $width .' ' . $height .'"
  overflow="visible"
  xmlns="http://www.w3.org/2000/svg"
  preserveAspectRatio="xMinYMin meet">';
  $out .= '<g id="geo_shape">'."\n";
  $out .= "\t".'<path d="M '.$data.' z" fill="red" stroke="blue" stroke-width="1%"/>'."\n";
  $out .= '</g>'."\n";
  $out .= '</svg>';

  $dest = file_create_path() . '/svg.svg';
  $filename = file_save_data($out, $dest, FILE_EXISTS_REPLACE);

  return  '<embed src="'. $filename .'"
        pluginspage="http://www.adobe.com/svg/viewer/install/" />';
}
