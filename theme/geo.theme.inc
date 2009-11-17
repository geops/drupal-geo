<?php // $Id$

function geo_theme_theme() {
  $path = drupal_get_path('module', 'geo') .'/theme';

  return array(
    'geo_formatter_default' => array(
      'arguments' => array('element' => NULL),
      'file' => 'geo.theme.inc',
      'path' => $path,
      'gis input' => 'wkt',
    ),
    'geo_formatter_lat' => array(
      'arguments' => array('element' => NULL),
      'file' => 'geo.theme.inc',
      'path' => $path,
      'gis input' => 'array',
    ),
    'geo_formatter_lon' => array(
      'arguments' => array('element' => NULL),
      'file' => 'geo.theme.inc',
      'path' => $path,
      'gis input' => 'array',
    ),
    'geo_formatter_georss' => array(
      'arguments' => array('element' => NULL),
      'file' => 'geo.theme.inc',
      'path' => $path,
      'gis input' => 'array',
    ),
    'geo_formatter_svg' => array(
      'arguments' => array('element' => NULL),
      'file' => 'geo.theme.inc',
      'path' => $path,
    ),
    'geo_fields' => array(
      'arguments' => array('element' => NULL),
    ),
    'geo_latlon' => array(
      'arguments' => array('element' => NULL),
    ),
  );
}

/**
 * Field formatters for geo fields.
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
