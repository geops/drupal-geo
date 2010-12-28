<?php
// $Id$

function geo_theme_theme() {
  $path = drupal_get_path('module', 'geo') . '/theme';

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
      'gis input' => 'text',
    ),
    'geo_formatter_svg' => array(
      'arguments' => array('element' => NULL),
      'file' => 'geo.theme.inc',
      'path' => $path,
    ),
    'geo_fields' => array(
      'arguments' => array('element' => NULL),
      'file' => 'geo.theme.inc',
      'path' => $path,
    ),
    'geo_latlon' => array(
      'arguments' => array('element' => NULL),
      'file' => 'geo.theme.inc',
      'path' => $path,
    ),
  );
}

/**
 * Field formatters for geo fields.
 */
function theme_geo_formatter_default($element) {
  // TODO: Should this theme geo_formatter_default be declared in hook_theme()?
  return $element['#item']['wkt'];
}

/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function theme_geo_formatter_lat($element) {
  // TODO: Should this theme geo_formatter_lat be declared in hook_theme()?
  if ($element['#item']['array']) {
    return $element['#item']['array']['lat'];
  }
}

/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function theme_geo_formatter_lon($element) {
  // TODO: Should this theme geo_formatter_lon be declared in hook_theme()?
  if ($element['#item']['array']) {
    return $element['#item']['array']['lon'];
  }
}

/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function theme_geo_formatter_georss($element) {
  // TODO: Should this theme geo_formatter_georss be declared in hook_theme()?
  $item = $element['#item'];

  if (in_array($item['gis type'], array('point', 'line', 'polygon'))) {
    $tag = 'georss:' . $item['gis type'];
    $value = $item['text'];
  }
  elseif ($item['bbox']) {
    $tag = 'georss:bbox';
    $value = $item['text'];
  }
  if (isset($tag)) {
    return '<' . $tag . '>' . $value . '</' . $tag . '>';
  }
}

/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function theme_geo_fields($element) {
  // TODO: Should this theme geo_fields be declared in hook_theme()?
  return isset($element['#children']) ? $element['#children'] : '';
}

/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function theme_geo_latlon($element) {
  // TODO: Should this theme geo_latlon be declared in hook_theme()?
  $output = '<div class="container-inline">' . $element['#children'] . '</div>';
  return theme('form_element', $element, $output);
}
