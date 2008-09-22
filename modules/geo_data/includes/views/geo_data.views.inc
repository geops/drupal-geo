<?php // $Id$

/**
 * Implementation of hook_views_data()
 */ 
function geo_data_views_data() {
  $data = array();
  return $data; // TODO we're not doing anything with this yet.

  $data['geo_data']['table']['group']  = t('Geospatial Data');

  foreach (geo('tables', '/^content_field/') as $table => $col) {
    $data[$table]['table'] = array(
      'group' => t('Geospatial Data'),
      'base' => array(
        'field' => 'state', // TODO use key column.
        'title' => $table, // TODO use key column.
      ),
    );
  }
  //return $data;
}

function geo_data_views_handlers() {
  return array(
    'info' => array(
      'path' => drupal_get_path('module', 'geo_data') .'/includes/views',
    ),
    'handlers' => array(
      'views_handler_field_geo_data' => array(
        'parent' => 'content_handler_field_multiple',
      ),
    ),
  );
}
