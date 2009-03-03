<?php // $Id$

/**
 * Most of the views stuff is handled by CCK, but extend its field handler with
 * our own.
 */
function geo_field_views_handlers() {
  return array(
    'info' => array(
      'path' => drupal_get_path('module', 'geo_field') .'/includes/views',
    ),
    'handlers' => array(
      'views_handler_field_geo_field' => array(
        'parent' => 'content_handler_field_multiple',
      ),
    ),
  );
}
