<?php

/**
 * Administration tasks for the geo module, such as finding and adding new tables to views.
 *
 * @version $Id$
 * @package geo
 **/

function _geo_build_tables() {
  $form = array('#base' =>'_geo_build_tables');
  $form['tables']['#tree'] = true;
  if (module_exists('views')) {
    $known_fields = geo_fields();

    views_load_cache(); // get access to some useful views_cache.inc functions
    $views_fields = _views_get_fields();
    //dpr($views_tables);
    //dpr($views_fields);

    // construct an #options array out of the field names:
    $views_field_options['NONE'] = t('No link');
    foreach($views_fields as $id => $field) {
      if($field['notafield']) { continue; } // skip things we can't really link to in the db.

      $views_field_options[$id] = $field['name'];
    }
    foreach (_geo_enabled_tables() as $table => $cols) {      
      $form['tables'][$table] = array( '#tree' => 1 );
      foreach ( _geo_table_desc($table) as $col => $desc) {
        $form['tables'][$table][$col] = array(
          'view_field' => array('#type' => 'select', '#options' => $views_field_options),
          'exposed' => array( '#type' => 'checkbox', '#default_value' => isset($fields[$table][$col]) ),
          'name'    => array( '#type' => 'markup', '#value' => $desc['name'] ),
          'type'    => array( '#type' => 'markup', '#value' => $desc['type'] ),
          'type_hidden' => array( '#type' => 'value', '#value' => $desc['type']),
          'desc'    => array( '#type' => 'textfield', '#default_value' => $fields[$table][$col]['title'], '#size' => 30 ),
        );
      }
    }
  }
  $form['submit'] = array('#type' => 'submit', '#value' => t('Save values'));
  return $form;
}

function _geo_build_tables_submit($form_id, $values) {
  views_load_cache(); // get access to some useful views_cache.inc functions
  $views_fields = _views_get_fields();
  
  foreach($values['tables'] as $table => $fields) {
    foreach($fields as $col => $data) {
      if($link_field = $views_fields[$data['view_field']]) {
        // get the "column" we will join on and the table where that column exists.
        $link_column = substr($data['view_field'], strpos($data['view_field'], '.') + 1);
        $link_table = $link_field['table'];
        $additional_columns = array();
        // now we iterate through this table again, to find all the "exposed" fields
        foreach($fields as $ecol => $edata) {
          if($edata['exposed']) {
            $additional_columns[$ecol] = array(
              'label' => $edata['desc'],
              'type' => $edata['type_hidden'],
            );
          }
        }
        /* pseudo 
        find or insert the field in the geo_field table -- perhaps everything should have an entry by now!
        get the id for that row so we can put a corresponding row in the geo_link table
        insert the data into the geo_link table
        implement hook_views_tables that reads the link table and generates tables based on those entries
        */
      }
    } 
  }
  //dpr($values);
}

function theme__geo_build_tables($form) {
  $rows = array();
  foreach (element_children($form['tables']) as $k => $table) {
    $rows[] = array(array('data' => $table, 'colspan' => 5, 'header' => 1));
    foreach (element_children($form['tables'][$table]) as $k => $i) {
      $rows[] = array(
        drupal_render($form['tables'][$table][$i]['view_field']),
        drupal_render($form['tables'][$table][$i]['exposed']),
        drupal_render($form['tables'][$table][$i]['name']),
        drupal_render($form['tables'][$table][$i]['type']),
        drupal_render($form['tables'][$table][$i]['desc']),
      );
    }
  }
  $header = array (
    t('Link to Existing Field'),
    t('Expose as field'),
    t('Column Name'),
    t('Data Type'),
    t('Label')
  );
  $output = theme('table', $header, $rows);
  $output .= drupal_render($form);

  return $output;
}

