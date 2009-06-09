<?php // $Id$
/**
 * @file
 * Field formatters for geo_field fields.
 */
function theme_geo_formatter_default($element) {
  return $element['#item']['wkt'];
}

function theme_geo_formatter_lat($element) {
  return $element['#item']['array']['lat'];
}

function theme_geo_formatter_lon($element) {
  return $element['#item']['array']['lon'];
}

function theme_geo_formatter_georss($element) {
  $item = $element['#item'];

  if (in_array($item['gis type'], array('point', 'line', 'polygon'))) {
    $tag = 'georss:'. $item['gis type'];
    $value = $item['text'];
  }
  elseif($item['bbox']) {
    $tag = 'georss:bbox';
    $value = $item['bbox'];
  }
  if (isset($tag)) {
    return '<'. $tag .'>' . $value .'</'. $tag .'>';
  }
}
