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
  $points = geo_wkb_get_data($element['#item']['wkb'], NULL, $type);

  $item = $element['#item'];
  if (in_array($item['type'], array('POINT', 'LINE', 'POLYGON'))) {
    $tag = 'georss:'. strtolower($item['type']);
    $data = join(' ', $points);
  }
  elseif($item['bbox']) {
    $tag = 'georss:bbox';
    $data = $item['bbox'];
  }
  if (isset($tag)) {
    return '<'. $tag .'>' . $data .'</'. $tag .'>';
  }
}

function theme_geo_formatter_svg($element) {
  // TODO this should be moved to a geo() call so that the backend can handle
  // this on its own.
  module_load_include('inc', 'geo', 'includes/geo.wkb');

  $points = geo_wkb_get_data($element['#item']['wkb'], NULL, $type);
  if ($type == 'point') {
    $data = ' cx="'. $points[0][0] .'" cy="'. $points[0][1] .'" ';
  }
  elseif($type == 'polygon' || $type == 'linestring') {
    $data = 'M'. join(' ', $points) .'z';
  /*
    foreach ($points as $point) {
      $data .= join(',', $point).' ';
    }
*/
  }

  $out = '<?xml version="1.0" standalone="no"?>
<!DOCTYPE svg PUBLIC "-//W3C//DTD SVG 1.1//EN" 
  "http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd">';
//  $out .= '<svg width="100%" height="100%" viewBox="244000 -4231000 64000 81000" >';
  $out .= '<svg id="mainMap" x="0" y="15" viewBox="244000 -4231000 64000 81000" width="553" height="700">';
 $out .= "\t".'<path d="'.$data.'" fill="red" stroke="blue" stroke-width="300" />'."\n";
  $out .= '</svg>';
  
  $dest = file_create_path() . '/svg.svg';
  $filename = file_save_data($out, $dest, FILE_EXISTS_REPLACE);

  return  '<embed src="'. $filename .'" width="200" height="200"
        pluginspage="http://www.adobe.com/svg/viewer/install/" />';
}
