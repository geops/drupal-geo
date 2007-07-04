<?php

/**
 * @file
 * The CCK hooks for the geo module. These are included on every page load so as to be available at any time but are in a separate file for developer purposes.
 */
 
 /*
 TODO API for getting fields out of the table with attached info
 TODO generalize most, if not all, SQL in this file
 TODO standardize wkt vs. geometry
 TODO move geo_views_field_query_handler over to the per db installations (and possibly implment php based handlers there too)
 TODO form element for getting WKT, with validators?
 BUG selecting the same field twice with different options, results in the second choice twice
 */

/**
 * Implementation of hook_field_info()
 */
function geo_field_info() {
  // XXX are multipoints, etc different types? or just the single types + the multiple option?
  return array(
    'point' => array('label' => 'Point'),
    'linestring' => array('label' => 'Linestring'),
    'polygon' => array('label' => 'Polygon'),
    //'geometrycollection' => array('label' => 'Spatial: Mixed Collection'),
  );
}

/**
 * Implementation of hook_field_settings()
 */
function geo_field_settings($op, $field) {
  $field_name = $field['field_name'];
  $table_name = _content_tablename($field_name, CONTENT_DB_STORAGE_PER_FIELD);
  
  switch ($op) {
    case 'form':
      // TODO pick the SRID for this column
      return $form;

    case 'save':
      return NULL; //array('text_processing', 'max_length', 'allowed_values');
    
    case 'database columns':
      $geo_field = _geo_get_cck_field($field_name);
      if ($geo_field) {
        // the field already has storage. good i don't think there's anything else to do
      } else {
        // the field does not have storage, so we need to create its field

        switch ($GLOBALS['db_type']) {
          case 'pgsql':
            // XXX this is pg specific
            db_query("CREATE TABLE {$table_name} (
                vid int_unsigned NOT NULL default '0',
                delta int_unsigned NOT NULL default '0',
                nid int_unsigned NOT NULL default '0',
                PRIMARY KEY (vid,delta)
              )");
            break;
          case 'mysql':
          case 'mysqli':
            db_query("CREATE TABLE {" . $table_name . "} (
              vid integer unsigned NOT NULL default '0',
              delta integer unsigned NOT NULL default '0',
              nid integer unsigned NOT NULL default '0',
              PRIMARY KEY (vid,delta)
            )/*!40100 ENGINE = MYISAM DEFAULT CHARACTER SET utf8 */");
            break;
        }
        
        $type = strtoupper($field['type']);
        geo_add_geometry_column($table_name, $field_name, $type); // all cck fields are WGS84 for now
        // XXX should this next line be handled by geo_add_geometry_column instead? would make sense.... as they are basically the same
        _geo_insert_cck_field($table_name, $field_name, $type, 4326); // TODO allow choosing SRID!
        
      }
      break; //return NULL;
    
    
    case 'tables':
      $info = geo_field_info();
      $label_prefix = t('@type: @field', array('@type' => $info[$field['type']]['label'], '@field' => $field['widget']['label'] .' ('. $field['field_name'] .')'));
      $view_table = geo_views_create_geometry_table($table_name, $field_name, $label_prefix, 'node', 'vid', 'vid', array('content_field' => $field), $join_extras = NULL);
      //return array('node_data_' . $field_name => $view_table);
      break;
    /*
    case 'arguments':
      $arguments = content_views_field_arguments($field);
      // whatever additions / modifications needed on the default definitions
      return $arguments;
    */
    case 'filters':
      return array();

  }
}

/**
 * Implementation of hook_field()
 */
function geo_field($op, &$node, $field, &$node_field, $teaser, $page) {
  // This next line could probably be cached in the CCK field settings....
  $geo_field = _geo_get_cck_field($field['field_name']);
  
  switch ($op) {
    case 'load':
    case 'insert':
    case 'update':
    case 'delete':
    case 'submit':
      return geo_backend_field($op, $node, $field, $node_field, $teaser, $page, $geo_field);
    case 'validate':
      /* Pseudo
      - get the opengis type and validate
      */
      break;
  }
}

/**
 * Implementation of hook_field_formatter_info
 */
function geo_field_formatter_info() {
  // TODO some other formatters: asKML, asGML, asGeoRSS, etc
  // TODO Can we do math on the field? area, length, etc? perhaps fetches should always return that data....
  return array(
    'default' => array(
      'label' => t('Well Known Text'),
      'field types' => array_keys(geo_field_info()), // easy way to get all fields
    ),
  );
}

/**
 * Implementation fo hook_field_formatter
 */
function geo_field_formatter($field, $item, $formatter, $node) {
  //dpr("In my field formatter");
  //dpr($item);
  switch($formatter) {
    
    default:
      return $item['wkt'];
  };
}

/**
 * Implementation of hook_widget_info()
 */
function geo_widget_info() {
  return array(
    'wkt' => array(
      'label' => t('Direct Text Entry'),
      'field types' => array_keys(geo_field_info()), // easy way to get all fields
    ),
  );
}

/**
 * Implementation of hook_widget_settings()
 */
function geo_widget_settings($op, $widget) {
  switch ($op) {
    case 'form':
      $form = array();
      return $form;

    case 'validate':
      break;

    case 'save':
      return NULL; // array('rows');

  }
}

/**
 * Implementation of hook_widget
 */
function geo_widget($op, &$node, $field, &$items) {
  //dpr("In geo widget $op");
  //dpr($items);
  //dpr($node);
  switch ($op) {
    case 'prepare form values':
      break;

    case 'form':
      $form = array();
      $fn = $field['field_name'];
      
      $form[$fn] = array('#tree' => TRUE);
      // for now, we just have one item
      $form[$fn][0]['wkt'] = array(
        '#type' => 'textfield',
        '#title' => t($field['widget']['label']),
        '#default_value' => $items[0]['wkt'],
        '#required' => $field['required'],
        '#description' => $field['widget']['description'],
      );
      
      return $form;

    case 'process form values':

      break;
  }
}

function geo_views_relation_field_validator($fielddata, $view, $form) {
  // until I have a WKT processor, this is not too useful. :-)
}

/*** Helper Functions ****/

/* 
 * Retrieve an entry from the geo_field table, for this particular field
 */
function _geo_get_cck_field($field_name) {
  $fields = geo_fields('content');
  return $fields[$field_name];
}

/*
 * Register a new CCK field in the geo_field table
 */
function _geo_insert_cck_field($table_name, $field_name, $type, $srid) {
  // we borrow the content.module namespace to show this is a CCK field, as compared to another geo field
  $sql =<<<EOF
INSERT INTO {geo_field} (table_title, field_title, module, table_name, field_name, data_type, geometry_type, dimensions, srid)
  VALUES ('%s', '%s', 'content', '%s', '%s', '%s', '%s', 2, %d)
EOF;
  $title = 'content_' . $table_name . '_' . $field_name;
  db_query($sql, $field_name, $title, $table_name, $field_name, $type, $type, $srid);
}

