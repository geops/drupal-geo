<?php // $Id$

/**
 * Views related functionality for the geo module
 *
 * @package geo
 */

/**
 * Implementation of hook_views_data().
 */
function geo_views_data() {
  $data = array();

  // Get all of the datasets stored in Geo's API.
  foreach (geo_list() as $geo) {

    // If available, use the views_data provided by the handler and move on.
    if (method_exists($geo, 'views_data')) {
      $data = array_merge($geo->views_data(), $data);
      continue;
    }

    // Only the SQL backends are likely to have these methods.
    if (method_exists($geo, 'tableName')) $table = $geo->tableName();
    if (method_exists($geo, 'columnName')) $column = $geo->columnName();
    if (method_exists($geo, 'schema')) $schema = $geo->schema();

    // There's a geometry column,
    if ($table && $column) {
      $data['geo'][$table] = array(
        'field' => array(
          'handler' => 'views_handler_field_numeric',
          'float' => $float,
          'click sortable' => TRUE,
        ),
        'sort' => array(
          'handler' => 'views_handler_sort',
        ),
        'filter' => array(
          'handler' => 'views_handler_filter_numeric',
        ),
        'argument' => array(
          'handler' => 'views_handler_argument_numeric',
          'click sortable' => TRUE,
        ),
      );
    }

    // This dataset has additional metadata in a schema definition.
    if (isset($schema)) {
      foreach ($schema as $geo_table => $info) {

        // Field information
        foreach ($info['fields'] as $field_name => $field_info) {
          $views_field = array();

          // Use the field's predefined views stuff.
          if (isset($field['views'])) {
            $views_field = $field['views'];
          }

          // Make our best guess at what kind of views support to use.
          else {
            switch ($field_info['type']) {
              case 'serial':
              case 'int':
              case 'float':
              case 'numeric':
                $float = in_array($field_info['type'], array('float', 'numeric'));
                $views_field = array(
                  'field' => array(
                    'handler' => 'views_handler_field_numeric',
                    'float' => $float,
                    'click sortable' => TRUE,
                  ),
                  'sort' => array(
                    'handler' => 'views_handler_sort',
                  ),
                  'filter' => array(
                    'handler' => 'views_handler_filter_numeric',
                  ),
                  'argument' => array(
                    'handler' => 'views_handler_argument_numeric',
                    'click sortable' => TRUE,
                  ),
                );
                break;

              case 'varchar':
              case 'char':
                $views_field = array(
                  'field' => array(
                    'handler' => 'views_handler_field',
                  ),
                  'sort' => array(
                    'handler' => 'views_handler_sort',
                  ),
                  'filter' => array(
                    'handler' => 'views_handler_filter_string',
                  ),
                  'argument' => array(
                    'handler' => 'views_handler_argument_string',
                  ),
                );
                break;

              case 'text':
                $views_field = array(
                  'field' => array(
                    'handler' => 'views_handler_field',
                    'click sortable' => TRUE,
                  ),
                  'sort' => array(
                    'handler' => 'views_handler_sort',
                  ),
                  'filter' => array(
                    'handler' => 'views_handler_filter_string',
                  ),
                  'argument' => array(
                    'handler' => 'views_handler_argument_string',
                    'click sortable' => TRUE,
                  ),
                );
                break;

              case 'blob':
                // Don't deal with these.
                break;

              case 'datetime':
            }
          }

          if ($views_field) {
            $views_field['title'] = '';
            $views_field['help'] = '';
          }
        }
      }
    }
  }
}

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
        'parent' => 'content_handler_field',
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
