<?php // $Id$

/**
 * Views related functionality for the geo module
 *
 * @package geo
 */

require_once dirname(__FILE__) . '/views_join_geo.inc';

/**
 * Implementation of hook_views_data().
 */
function geo_views_data() {
  $data = array();

  $geo_fields = geo_field_names();

  // Get all of the datasets stored in Geo's API.
  foreach (geo_list() as $geo) {

    // Do not process Geo CCK fields, these will be handled separately.
    if (in_array($geo->name(), $geo_fields)) continue;

    // Create an all-purpose Geo base table.
    if (!isset($data['geo'])) {

      $data['geo_data']['table']['base'] = array(
        'title' => t('Geospatial data'),
        'description' => t(''),
      );

      // Add to 'geo' group.
      $data['geo_data']['table']['group']  = t('Geo');
    }

    // If available, use the views_data provided by the handler and move on.
    if (method_exists($geo, 'views_data')) {
      $data = array_merge($geo->views_data(), $data);
      continue;
    }

    // Only the SQL backends are likely to have these methods.
    if (method_exists($geo, 'tableName')) $table = $geo->tableName();
    if (method_exists($geo, 'columnName')) $column = $geo->columnName();
    if (method_exists($geo, 'schema')) $schema = $geo->schema();

    // There's a geometry column. Set up fields relationships between geo data.
    if ($table && $column) {

      // Add to 'geo' group.
      $data[$table]['table']['group']  = t('Geo');

      // Establish a relationship to the geo_data base table.
      $data[$table]['table']['join']  = array(
        'geo_data' => array(
          'handler' => 'views_join_geo',
          'left_field' => 'geo',
          'field' => $column,
          'geo_gid' => $geo->gid(),
        ),
      );

      // Establish a relationship to the geo_data "base table".
      $data[$table][$column] = array(
        'title' => $geo->title(),
        'field' => array(
          'handler' => 'views_handler_field_geo',
          'geo_gid' => $geo->gid(),
        ),
        'argument' => array(
          'handler' => 'views_handler_argument_geo',
          'geo_gid' => $geo->gid(),
        ),
        'filter' => array(
          'handler' => 'views_handler_filter_geo',
          'geo_gid' => $geo->gid(),
        ),
        'sort' => array(
          'handler' => 'views_handler_sort_geo',
          'geo_gid' => $geo->gid(),
        ),
      );

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
                  // Don't bother with these.
                  break;

                case 'datetime':
              }
            }

            if ($views_field) {
              // Set some common values.
              $views_field['title'] = $geo->title() .' - '. $field_name;
              $views_field['help'] = '';

              // Specify the table and field names in the Views field definition,
              // because we're using generic name of 'geo' to group these.
              $views_field['table'] = $geo_table;

              // Add it to the geo base table.
              $data[$geo_table][$field_name] = $views_field;
            }
          }
        }
      }
    }
  }

  return $data;
}

/**
 * Implementation of CCK's hook_field_settings_alter().
 */
function geo_field_settings_alter(&$data, $op, $field) {
  if ($op == 'views data') {
    if (in_array($field['field_name'], geo_field_names())) {

      // No views data from the field module.  Start with CCK defaults
      if (!$data) $data = content_views_field_views_data($field);

      // Load the geo handler that controls this dataset.
      if ($geo = geo_load(array('name' => $field['field_name']))) {
        $geo->setTableName(content_views_tablename($field));

        $db_info = content_database_info($field);
        foreach ($db_info['columns'] as $column => $info) {
          $table = $geo->tableName();
          $field = $info['column'];

          if ($info['gis type']) {
            // Add a relationship to the Geo join handler.
            $data[$table][$field]['relationship'] = array(
              'handler' => 'views_handler_relationship_geo',
              'join_handler' => 'views_join_geo',
              'base' => 'geo_data',
              'field' => 'geo',
              'label' => t('Geo'),
              'geo_gid' => $geo->gid(),
            );

            // Set all field handlers to Geo's built-in versions.
            foreach (array('field', 'filter', 'argument', 'sort') as $item) {
              $handler = 'views_handler_'. $item .'_geo';
              $data[$table][$field][$item]['handler'] = $handler;
              $data[$table][$field][$item]['geo_gid'] = $geo->gid();
            }
          }
        }
      }
      // TODO this isn't picking up any of geo_data's fields!
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
      'views_join_geo' => array(
        'parent' => 'views_join',
      ),
      'views_handler_relationship_geo' => array(
        'parent' => 'views_handler_relationship',
        'join_handler' => 'views_join_geo',
      ),
      'views_handler_field_geo' => array(
       'parent' => 'views_handler_field',
       'gis input' => 'array',
      ),
      'views_handler_filter_geo' => array(
       'parent' => 'views_handler_filter_numeric',
       'gis input' => 'array',
      ),
      'views_handler_sort_geo' => array(
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

/**
 * A helper function used across views handlers to select a target geometry.
 */
function geo_views_target_form(&$form, &$form_state, &$handler, $group) {

  if (!isset($handler->geo)) {
    $handler->geo = geo_load(array('gid' => $handler->definition['geo_gid']));
  }

  $functions = $handler->geo->functionNames($group);

  // Select which function to use.
  $form['function'] = array(
    '#type' => 'radios',
    '#title' => t($group .' function to use'),
    '#default_value' => $handler->options['function'],
    '#options' => $functions,
  );

  // Some of these functions require a relationship to another value.
  // If so, show the target handling functionality here.
  $deps = array();
  foreach ($functions as $function => $label) {
    $info = $handler->geo->functionInfo($function);
    if ($info['requires target']) $deps[] = $function;
  }

  $form['value']['input'] = array(
    '#type' => 'select',
    '#title' => 'Widget',
    '#default_value' => $handler->options['input'],
    '#weight' => 10,
    '#parents' => array('options', 'input'),
  );

  $form['value']['target_div_prefix'] = array(
    '#type' => 'hidden',
    '#id' => 'geo-target-options',
    '#prefix' => '<div><fieldset id="geo-target-options"><legend>' . t('Provide a target for this function') . '</legend>',
    '#process' => array('views_process_dependency'),
    '#dependency' => array('radio:options[function]' => $deps),
    '#weight' => 9,
  );

  $input_options = array();
  foreach (geo_inputs() as $name => $info) {
    $element = $info['element'];

    $input_options[$name] = $info['label'];

    $element = array_merge($element, array(
      '#title' => $handler->options['label'],
      '#default_value' => $handler->value['target'][$name],
      '#gis_type' => $handler->geo->geometryType(),
      '#gis_output' => 'wkt', // The format expected by this handler.
      //'#tree' => TRUE,
    ));

    $element['#process'][] = 'views_process_dependency';
    $element['#dependency'] = array('edit-options-input' => array($name));
    $element['#weight'] = 11;

    $form['value']['target'][$name] = $element;
    $form['value']['target']['#weight'] = 11;
    $form['value']['target']['#tree'] = TRUE;
  }

  // Set the list of elements back to the target options.
  $form['value']['input']['#options'] = $input_options;

  $form['value']['target_div_suffix'] = array(
    '#value' => '</fieldset></div>',
    '#weight' => 12,
  );
}
