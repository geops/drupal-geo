<?php

/**
 * @file
 * The CCK hooks for the geo module. These are included on every page load so as to be available at any time but are in a separate file for developer purposes.
 */

/**
 * Implementation of hook_field_info()
 */
function geo_field_info() {
  // XXX are multipoints, etc different types? or just the single types + the multiple option?
  return array(
    'geo_point' => array('label' => 'Spatial: Point'),
    'geo_linestring' => array('label' => 'Spatial: Linestring'),
    'geo_polygon' => array('label' => 'Spatial: Polygon'),
    'geo_geometrycollection' => array('label' => 'Spatial: Mixed Collection'),
  );
}

/**
 * Implementation of hook_field_settings()
 */
function geo_field_settings($op, $field) {
  switch ($op) {
    case 'form':
      return $form;

    case 'save':
      return NULL; //array('text_processing', 'max_length', 'allowed_values');
    
    case 'database columns':
      $field_name = $field['field_name'];

      // is the field in the table?
      $res = db_query("SELECT * FROM {geo_field} WHERE field_name = '%s'", $field_name);
      if (db_num_rows($res)) {
        // the field already has storage. good i don't think there's anything else to do
      } else {
        // the field does not have storage, so we need to create its field
        // XXX this is pg specific
        db_query("CREATE TABLE {geo_$field_name} (
          vid int_unsigned NOT NULL default '0',
          nid int_unsigned NOT NULL default '0',
          PRIMARY KEY (vid)
        )");
        
        $type = strtoupper(substr($field['type'], '4'));
        geo_add_geometry_column('geo_' . $field_name, $field_name, $type); // all cck fields are WGS84 for now
        
        db_query("INSERT INTO {geo_field} VALUES ('%s', '%s')", $field_name, 'geo_' . $field_name);
      }
      
      
      break; //return NULL;
    
    /* this might be helpful later
    case 'tables':
      $tables = content_views_field_tables($field);
      // whatever additions / modifications needed on the default definitions
      return $tables;
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
 *
 * @param $op
 *   What kind of action is being performed. Possible values:
 *   - "load": The node is about to be loaded from the database. This hook
 *     should be used to load the field.
 *   - "view": The node is about to be presented to the user. The module
 *     should prepare and return an HTML string containing a default
 *     representation of the field.
 *     It will be called only if 'view' was set to TRUE in hook_field_settings('callbacks')
 *   - "validate": The user has just finished editing the node and is
 *     trying to preview or submit it. This hook can be used to check or
 *     even modify the node. Errors should be set with form_set_error().
 *   - "submit": The user has just finished editing the node and the node has
 *     passed validation. This hook can be used to modify the node.
 *   - "insert": The node is being created (inserted in the database).
 *   - "update": The node is being updated.
 *   - "delete": The node is being deleted.
 * @param &$node
 *   The node the action is being performed on. This argument is passed by
 *   reference for performance only; do not modify it.
 * @param $field
 *   The field the action is being performed on.
 * @param &$node_field
 *   The contents of the field in this node. Changes to this variable will
 *   be saved back to the node object.
 * @return
 *   This varies depending on the operation.
 *   - The "load" operation should return an object containing extra values
 *     to be merged into the node object.
 *   - The "view" operation should return a string containing an HTML
 *     representation of the field data.
 *   - The "insert", "update", "delete", "validate", and "submit" operations
 *     have no return value.
 *
 * In most cases, only "validate" operations is relevant ; the rest
 * have default implementations in content_field() that usually suffice.
 */
function geo_field($op, &$node, $field, &$node_field, $teaser, $page) {
  dpr("Geo field: $op");
  dpr($field);
  dpr($node_field);
  
  $fn = $field['field_name'];
  // This next line could probably be cached in the CCK field settings....
  $table = db_result(db_query("SELECT table_name FROM {geo_field} WHERE field_name = '%s'", $fn));
  $t = trim(filter_xss($node_field[0]['wkt']));
  
  switch ($op) {
    case 'load':
      $extras = array();
      $res = db_query("SELECT srid($fn) as srid, asText($fn) AS wkt, asText(centroid($fn)) AS centroid, asText(envelope($fn)) as bbox FROM {$table} WHERE vid = %d AND nid = %d", $node->vid, $node->nid);
      while($row = db_fetch_object($res)) {
        $extras[] = (array) $row;
      }
      $o = new StdClass();
      $o->{$fn} = $extras;
      return array($field['field_name'] => $extras);
      break;
    case 'insert':
      // TODO how to write a generic version of this in the API?
      // XXX pg specific
      db_query("INSERT INTO {$table} (vid, nid, $fn) VALUES (%d, %d, GeomFromText('%s', 4326))", $node->vid, $node->nid, $t); // everything is srid 4326 for now
      break;
    case 'update':
      // XXX pg specific
      db_query("UPDATE {$table} SET $fn = GeomFromText('%s', 4326) WHERE vid = %d AND nid = %d", $t, $node->vid, $node->nid);
      break;
    case 'delete':
      db_query("DELETE FROM {$table} WHERE nid = %d, vid = %d", $node->nid, $node->vid); // should we delete by nid only?
      break;
    case 'submit':
      break;
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
  dpr("In my field formatter");
  dpr($item);
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
  dpr("In geo widget $op");
  dpr($items);
  dpr($node);
  switch ($op) {
    case 'prepare form values':
      break;

    case 'form':
      $form = array();
      $fn = $field['field_name'];
      
      $form[$fn] = array('#tree' => TRUE);
      // for now, we just have one item
      $form[$fn][0]['wkt'] = array(
        '#type' => 'textarea',
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

/**
 * Implementation of hook_form_alter()
 * trying to catch changes to fields we control
 */
function geo_form_alter($form_id, &$form_values) {
  // dpr($form_id);
  
  if($form_id = '_content_admin_field_add_new') {
    $form_values['#submit']['geo_creating_field'] = array();
  }
  if($form_id == '_content_admin_field') {
    $form_values['#submit']['geo_admining_field'] = array();
  }
}

function _geo_creating_field($form_id, $form_values) {
  dpr('Creating a new field');
  dpr($form_id);
  dpr($form_values);
  /* pseudo
  - create new table for the field
  */
  
  // XXX this is pgsql only right now....
  
}
 
