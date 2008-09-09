<?php // $Id$

/**
 * Views related functionality for the geo module
 *
 * @package geo
 */

function geo_views_handlers() {
  return array(
    'info' => array(
      'path' => drupal_get_path('module', 'geo') .'/includes/views',
    ),
    'handlers' => array(
      'views_handler_field_geo' => array(
       'parent' => 'views_handler_field',
      ),
      'views_handler_filter_geo' => array(
       'parent' => 'views_handler_filter',
      ),
    ),
  );
}
