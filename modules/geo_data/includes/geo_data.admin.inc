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
  }

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
  $form['import']['srid'] = array(
    '#type' => 'textfield',
    '#title' => t('SRID'),
    '#default_value' => GEO_SRID_DEFAULT,
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
    $srid = $form_state['values']['import']['srid'];
    geo_shp2sql($file, $table_name, $create, $srid);
  }
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
