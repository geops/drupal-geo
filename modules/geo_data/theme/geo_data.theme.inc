<?php
// $Id$

/**
 * Implementation of hook_theme().
 */
function geo_data_theme_theme() {
  $path = drupal_get_path('module', 'geo_data') . '/theme';

  return array(
    'geo_data_formatter_default' => array(
      'arguments' => array('element' => NULL),
      'file' => 'geo_data.theme.inc',
      'path' => $path,
    ),
    'geo_data_formatter_key' => array(
      'arguments' => array('element' => NULL),
      'file' => 'geo_data.theme.inc',
      'path' => $path,
    ),
    'geo_data_select' => array(
      'arguments' => array('element' => NULL),
      'file' => 'geo_data.theme.inc',
      'path' => $path,
    ),
    'geo_data_field_settings' => array(
      'arguments' => array('form' => array()),
      'file' => 'geo_data.theme.inc',
      'path' => $path,
    ),
  );
}

/**
 * Default field theme function: return value field.
 */
function theme_geo_data_formatter_default($element) {
  // TODO: Should this theme geo_data_formatter_default be declared in hook_theme()?
  return check_plain($element['#item']['label']);
}

/**
 * Default field theme function: return key field.
 */
function theme_geo_data_formatter_key($element) {
  // TODO: Should this theme geo_data_formatter_key be declared in hook_theme()?
  return check_plain($element['#item']['value']);
}

/**
 * Theme function for the geo_data select element.
 */
function theme_geo_data_select($element) {
  // TODO: Should this theme geo_data_select be declared in hook_theme()?
  return $element['#children'];
}

/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function theme_geo_data_field_settings(&$form) {
  // TODO: Should this theme geo_data_field_settings be declared in hook_theme()?
  drupal_add_js(drupal_get_path('module', 'geo_data') . '/theme/geo_data_field_settings.js');
  $hdrs = array(t('Data table'), t('Display value'), t('Key columns'));
  foreach (element_children($form) as $key) {
    $row = array();

    $row[] = drupal_render($form[$key]['status']);

    unset($form[$key]['label']['#title']);
    $row[] = drupal_render($form[$key]['label']);

    foreach (array('key1', 'key2', 'key3') as $col) {
      unset($form[$key][$col]['#title']);
      $row[] = drupal_render($form[$key][$col]);
    }

    $rows[] = $row;

    $output .= drupal_render($form[$key]['options']);
  }

  return theme('table', array('header' => $hdrs, 'rows' => $rows)) . $output;
}
