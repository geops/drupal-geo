<?php // $Id$

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
