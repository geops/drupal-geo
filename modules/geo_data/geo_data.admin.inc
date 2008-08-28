<?php // $Id$

/**
 * Put together a form of all the spatial columns.
 */
function geo_data_admin() {
  
  $form = array('#theme' => 'geo_data_admin_tables');

  // Get a list of all geometry-enabled tables, excluding field tables.
  $tables  = geo('tables', '/^content_field/');

  foreach ($tables as $table => $column) {
    $form['tables'][$table] = array( '#tree' => 1 );
    $form['tables'][$table]['table_label'] = array(
      '#type' => 'textfield',
      '#title' => t('Table Label'),
      '#default_value' => '', // TODO set as table label from link table
    );

/*
    foreach (geo('table_desc', $table) as $col => $desc) {
      $form['tables'][$table][$col] = array(
        //'view_field' => array('#type' => 'select', '#options' => $views_field_options),
        'exposed' => array( '#type' => 'checkbox', '#default_value' => isset($fields[$table][$col]) ),
        'name'    => array( '#type' => 'markup', '#value' => $desc['name'] ),
        'type'    => array( '#type' => 'markup', '#value' => $desc['type'] ),
        'type_hidden' => array( '#type' => 'value', '#value' => $desc['type']),
        'desc'    => array( '#type' => 'textfield', '#default_value' => $fields[$table][$col]['title'], '#size' => 30 ),
      );
    }
*/
  }

/*
  if (module_exists('views')) {
    $known_fields = geo_data_fields();

    views_load_cache(); // get access to some useful views_cache.inc functions
    $views_fields = _views_get_fields();

    // construct an #options array out of the field names:
    $views_field_options['NONE'] = t('No link');
    foreach ($views_fields as $id => $field) {
      if ($field['notafield']) { continue; } // skip things we can't really link to in the db.

      $views_field_options[$id] = $field['name'];
    }
  }
*/

  $form['#attributes']['enctype'] = 'multipart/form-data';
  $form['import'] = array(
    '#type' => 'fieldset',
    '#title' => t('Import shape data'),
    '#tree' => TRUE,
    '#description' => t('You can import geospatial data from any source, and its data will be available to your Drupal site.'),
  );
  $form['import']['file'] = array(
    '#type' => 'file',
    '#title' => t('Shape file'),
  );
  $form['import']['table_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Table name'),
    '#description' => t('You can import geospatial data from any source, and its data will be available to your Drupal site.'),
  );
  $form['import']['create'] = array(
    '#type' => 'checkbox',
    '#title' => t('Create a new table.'),
    '#default_value' => 1,
    '#description' => t('You can import geospatial data from any source, and its data will be available to your Drupal site.'),
  );
  
  $form['submit'] = array('#type' => 'submit', '#value' => t('Save values'));
  return $form;
}

function geo_data_admin_submit($form, &$form_state) {
  if (isset($_FILES['files']['name']['import'])) {
    module_load_include('inc', 'geo', 'includes/shp2sql');
    $file = $_FILES['files']['tmp_name']['import']; 
    $table_name = 'geo_data_'. $form_state['values']['import']['table_name'];
    $create = 'geo_data_'. $form_state['values']['import']['table_name'];
    geo_shp2sql($file, $table_name, $create);
  }
/*
  $views_fields = _views_get_fields();
  
  foreach ($values['tables'] as $table => $fields) {
    foreach ($fields as $col => $data) {
      if ($link_field = $views_fields[$data['view_field']]) {
        // get the "column" we will join on and the table where that column exists.
        $link_column = substr($data['view_field'], strpos($data['view_field'], '.') + 1);
        $link_table = $link_field['table'];
        $additional_columns = array();
        // now we iterate through this table again, to find all the "exposed" fields
        foreach ($fields as $ecol => $edata) {
          if ($edata['exposed']) {
            $additional_columns[$ecol] = array(
              'label' => $edata['desc'],
              'type' => $edata['type_hidden'],
            );
          }
        }
      }
    } 
  }
*/
}

function theme_geo_data_admin_tables($form) {
  $rows = array();
  foreach (element_children($form['tables']) as $k => $table) {
    unset($form['tables'][$table]['table_label']['#title']);
    //TODO perhaps put the table in one column and the title in the other 4
    $form['tables'][$table]['table_label']['#prefix'] = "$table:";
    $rows[] = array(array('data' => drupal_render($form['tables'][$table]['table_label']), 'colspan' => 5, 'header' => 1));
    foreach (element_children($form['tables'][$table]) as $k => $i) {
      $rows[] = array(
        drupal_render($form['tables'][$table][$i]['exposed']),
        drupal_render($form['tables'][$table][$i]['name']),
        drupal_render($form['tables'][$table][$i]['type']),
        drupal_render($form['tables'][$table][$i]['desc']),
      );
    }
  }
  $hdrs = array(
    t('Expose as field'),
    t('Column Name'),
    t('Data Type'),
    t('Label'),
  );
  $output = theme('table', $hdrs, $rows);
  $output .= drupal_render($form);

  return $output;
}

/*
function geo_data_theme($existing, $type, $theme, $path) {
  return array(
    'geo_data_admin_tables' => array(
      'arguments' => array('form' => array()),
    ),
  );
}
*/
