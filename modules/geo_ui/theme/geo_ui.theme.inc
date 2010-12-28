<?php
// $Id$

/**
 * Implementation of hook_theme().
 */
function geo_ui_theme_theme() {
  $path = drupal_get_path('module', 'geo_ui') . '/theme';

  return array(
    'geo_ui_admin_tables' => array(
      'arguments' => array('form' => NULL),
      'file' => 'geo_ui.theme.inc',
      'path' => $path,
    ),
  );
}

/**
 * Theme function for the geo_data table listing.
 */
function theme_geo_ui_admin_tables($form) {
  // TODO: Should this theme geo_ui_admin_tables be declared in hook_theme()?
  $hdrs = array(
    t('Name'),
    t('Table'),
    t('Records'),
    '',
  );

  $rows = array();
  foreach (element_children($form['tables']) as $k => $table) {
    $row = array();
    $row[] = drupal_render($form['tables'][$k]['label']);
    $row[] = drupal_render($form['tables'][$k]['count']);
    $row[] = drupal_render($form['tables'][$k]);
    $rows[] = $row;
  }

  $output = theme('table', array('header' => $hdrs, 'rows' => $rows));
  $output .= drupal_render($form);

  return $output;
}
