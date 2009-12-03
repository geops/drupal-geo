<?php // $Id$

/**
 * Views related functionality for the geo module
 *
 * @package geo
 */


/**
 * Implementation of hook_views_handlers().
 */
function geo_views_handlers() {
  return array(
    'info' => array(
      'path' => drupal_get_path('module', 'geo') .'/includes/views',
    ),
    'handlers' => array(
      // For any field that handles geo data.
      'views_handler_field_geo_field' => array(
        'parent' => 'content_handler_field_multiple',
      ),
      'views_handler_field_geo' => array(
       'parent' => 'views_handler_field',
       'gis input' => 'array',
      ),
      'views_handler_filter_geo' => array(
       'parent' => 'views_handler_filter',
       'gis input' => 'array',
      ),
      'views_handler_field_geo_distance' => array(
       'parent' => 'views_handler_field_numeric',
       'gis input' => 'array',
      ),
      'views_handler_filter_geo_distance' => array(
       'parent' => 'views_handler_filter',
       'gis input' => 'array',
      ),
      'views_handler_sort_geo_distance' => array(
       'parent' => 'views_handler_sort',
      ),
    ),
  );
}

/**
 * Implementation of hook_views_plugins().
 */
function geo_views_plugins() {
  return array(
    'module' => 'geo',
    'display' => array(
      'geo_xml' => array(
        'title' => t('Geo XML'),
        'help' => t('XML output formats from Geo'),
        'handler' => 'views_plugin_display_geo_xml',
        'parent' => 'feed',
        'path' => drupal_get_path('module', 'geo') .'/includes/views',
        'uses hook menu' => TRUE,
        'use ajax' => FALSE,
        'use pager' => FALSE,
        'use fields' => TRUE,
        'accept attachments' => FALSE,
        'admin' => t('Geo XML'),
      ),
    ),
    'style' => array(
      'geo_xml' => array(
        'title' => t('Simple XML'),
        'help' => t('Generates a basic XML feed from a view.'),
        'handler' => 'views_plugin_style_geo_xml',
        'uses row plugin' => FALSE,
        'uses fields' => TRUE,
        'uses options' => TRUE,
        'type' => 'geo_xml',
        'path' => drupal_get_path('module', 'geo') .'/includes/views',
      ),
      'geo_kml' => array(
        'title' => t('KML'),
        'help' => t('Generates a KML feed from a view.'),
        'handler' => 'views_plugin_style_geo_kml',
        'parent' => 'geo_xml',
        'uses fields' => TRUE,
        'uses row plugin' => FALSE,
        'uses options' => TRUE,
        'type' => 'geo_xml',
        'path' => drupal_get_path('module', 'geo') .'/includes/views',
      ),
      'geo_svg' => array(
        'title' => t('SVG'),
        'help' => t('Generates a SVG document from a view.'),
        'handler' => 'views_plugin_style_geo_svg',
        'parent' => 'geo_xml',
        'uses fields' => TRUE,
        'uses row plugin' => FALSE,
        'uses options' => TRUE,
        'type' => 'geo_xml',
        'path' => drupal_get_path('module', 'geo') .'/includes/views',
      ),
    ),
  );
}
