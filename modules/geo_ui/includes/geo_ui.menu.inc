<?php // $Id$

function geo_ui_menu_menu() {
  return array(
    'admin/build/geo' => array(
      'path'        => 'admin/build/geo',
      'title'       => t('Geospatial data'),
      'description' => t('Manage geographic information'),
      'page callback' => 'drupal_get_form',
      'page arguments' => array('geo_ui_admin'),
      'access arguments' => array('administer geospatial data'),
      'file'        => 'geo_ui.admin.inc',
      'file path' => drupal_get_path('module', 'geo_ui') .'/includes'
    ),
  );
}
