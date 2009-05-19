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
