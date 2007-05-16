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
    'point' => array('label' => 'Spatial: Point'),
    'linestring' => array('label' => 'Spatial: Linestring'),
    'polygon' => array('label' => 'Spatial: Polygon'),
    'geometrycollection' => array('label' => 'Spatial: Mixed Collection'),
  );
}

/**
 * Implementation of hook_field_settings()
 */
function geo_field_settings($op, $field) {
  $field_name = $field['field_name'];
  
  switch ($op) {
    case 'form':
      return $form;

    case 'save':
      return NULL; //array('text_processing', 'max_length', 'allowed_values');
    
    case 'database columns':
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
        
        $type = strtoupper($field['type']);
        geo_add_geometry_column('geo_' . $field_name, $field_name, $type); // all cck fields are WGS84 for now
        
        db_query("INSERT INTO {geo_field} VALUES ('%s', '%s')", $field_name, 'geo_' . $field_name);
      }
      
      
      break; //return NULL;
    
    
    case 'tables':
      dpr("In the tables hook");
      $table = array (
        'name' => 'geo_' . $field_name,
        'join' => array(
          'left' => array(
            'table' => 'node',
            'field' => 'vid',
          ),
          'right' => array(
            'field' => 'vid',
          ),
        ),
      );
      
      // this will be useful later
      $view_label = t('Geo: @field', array('@field' => $field['widget']['label'] .' ('. $field['field_name'] .')'));

      /*** Fields ***/
      
      // the first field is the general geometry returning fields.
      $display_options = array(
        'wkt' => t('Geometry'),
        'centroid' => t('Centroid'), 
        'bbox' => t('Bounding Box')
      );
      
      $help = t('The handler drop-down will choose how the data is formatted. The option drop-down allows you to choose how to represent your data: as the complete geometry, as just the center of the geometry, or as just a bounding box for the geometry.');
      
      if ($field['type'] == 'linestring') {
        $display_options['start'] = t('Starting Point');
        $display_options['end'] = t('Ending Point');
        $help .= t(' You may also choose to display the ending point or starting point of this line.');
      }
      
      $table['fields']['geometry'] = array (
        'name' => $view_label,
        'help' => $help,
        'query_handler' => 'geo_views_field_query_handler',
        'handler' => array(
          'geo_views_field_handler_wkt' => t('Well Known Text'),
          'geo_views_field_handler_svg' => t('SVG'),
          'geo_views_field_handler_kml' => t('KML'),
          'geo_views_field_handler_gml' => t('GML')),
        'option' => array(
          '#type' => 'select',
          '#options' => $display_options),
        'content_field' => $field,
      );
      
      /* potential additional fields, depending on type
      all: is valid
      */
      
      // filters are defined here for simplicity, instead of $op == 'filters'
      /* proposed filters
      foreach entered point, linestring, polygon as g
        - isValid(field) -- how would this be implemented in mysql? yuck
        - intersects(g, field)
        - equals(g, field)
        - within(field, g) <-- where g must be polygon
        - distance(g, field) >|<|=.... a value <-- I don't know if this will work with views right now
      just polygons: within(g, field)
                     area(field) cmp integer
                     permiter(field) cmp integer
      just linestrings: length cmp integer
      */
      
      // convenience array holding operators and names
      $relation_operators = array(
        'Equals' => t('Equals'),
        'NOT Equals' => t('Does not equal'),
        'Intersects' => t('Intersects'),
        'NOT Intersects' => t('Does not intersect'),
        'Within' => t('Within'),
        'NOT Within' => t('Not within'),
      );
      $table['filters']['relate'] = array (
        'name' => $view_label . ' -- ' . t('Spatial Relation'),
        'field' => $field['field_name'],
        'help' => t('Filter nodes by a spatial relation to this field. In the "value" box, include the Well-Known-Text representation of the geometry you wish to filter against. For the "Within" operator, your geometry must be a POLYGON.'),
        'operator' => $relation_operators,
        'handler' => 'geo_views_relation_filter_handler',
        'validate' => 'geo_views_relation_filter_validate',
        'content_field' => $field,
      );
      
      /*** Per Type additions ***/
      
      // Fields for scalar values
      // but first we create a template to use
      $scalar_template = array(
        'notafield' => true, // XXX is this necessary? do I want something else here
        'query_handler' => 'geo_views_scalar_field_query_handler', // for modifying the query
        'handler' => array('geo_views_scalar_field_handler' => t('Default')), // for computing and or modifying for display
        'content_field' => $field,
      );
      switch($field['type']) {
        case 'linestring':
          $table['fields']['length'] = $scalar_template;
          $table['fields']['length']['name'] = $view_label . ' -- ' . t('Length');
          $table['fields']['length']['help'] = t('Provides the length of the linestring.');
          $table['fields']['length']['sortable'] = true;
          // TODO provide an option for units (miles, km, m, feet, etc)
          
          // XXX sorts are broken until views adds sorts based on it's table definitions, not just raw SQL
          // sort on the length field
          $table['sorts']['length'] = array(
            'name' => $view_label . ' -- ' . t('Length'),
            'help' => t('Sort on the length of the linestring.'),
          );
          break;
        case 'polygon':
          $table['fields']['area'] = $scalar_template;
          $table['fields']['area']['name'] = $view_label . ' -- ' . t('Area');
          $table['fields']['length']['help'] = t('Provides the area of the polygon.');
          
          $table['fields']['perimeter'] = $scalar_template;
          $table['fields']['perimeter']['name'] = $view_label . ' -- ' . t('Perimeter');
          $table['fields']['perimeter']['help'] = t('Provides the perimeter of the polygon.');

          break;
      }
      
      /* fetch this field
      create a table definition for it, with columns
      wkt, centroid, bbox
      provide filters for distance, within (Polygon/geometrycollection types only)
      return it as node_data_$field_name so that it matches what CCK normally does*/
      // whatever additions / modifications needed on the default definitions
      return array('node_data_' . $field_name => $table);
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
  //dpr("Geo field: $op");
  //dpr($field);
  //dpr($node_field);
  
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
      // XXX delete and test for saftey next two lines
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
 * A views query handler callback to manage getting WKT, etc from the db
 */
function geo_views_field_query_handler($field, &$fieldinfo, &$query) {
  
  switch($field['handler']) {
    case 'geo_views_field_handler_kml':
      $func = 'asKML';
      break; 
    case 'geo_views_field_handler_gml':
      $func = 'asGML';
      break;
    case 'geo_views_field_handler_svg':
      $func = 'asSVG';
      break;
    
    case 'geo_views_field_handler_wkt':
    default:
      $func = 'asText';
      break;
  }
    
  $query->ensure_table($field['tablename']);
  switch($field['options']) {
    case 'wkt':
      $query->add_field($func . '(' . $field['tablename'] . '.' .$fieldinfo['content_field']['field_name'] . ')', NULL, $field['queryname']);
      break;
    case 'centroid':
      $query->add_field($func . '(centroid(' . $field['tablename'] . '.' .$fieldinfo['content_field']['field_name'] . '))', NULL, $field['queryname']);
      break;
    case 'bbox':
      $query->add_field($func . '(envelope(' . $field['tablename'] . '.' .$fieldinfo['content_field']['field_name'] . '))', NULL, $field['queryname']);
      break;
    // start point is only for linestrings, but we already tested that earlier
    case 'start':
      $query->add_field($func . '(StartPoint(' . $field['tablename'] . '.' .$fieldinfo['content_field']['field_name'] . '))', NULL, $field['queryname']);
      break;
    // end point is only for linestrings, but we already tested that earlier
    case 'end':
      $query->add_field($func . '(EndPoint(' . $field['tablename'] . '.' .$fieldinfo['content_field']['field_name'] . '))', NULL, $field['queryname']);
      break;
  }
  // make sure views default query builder does not add anything
  $fieldinfo['notafield'] = true;
  unset($fieldinfo['addlfields']);

}

/**
 * Views filter callback to handler for spatial relation filters (e.g. within, intersect, etc)
*/
function geo_views_relation_filter_handler($op, $filterdata, $filterinfo, &$query) {

  switch($op) {
    case 'handler':
      $func = $filterdata['operator'];
      $field = $filterinfo['table'] . '.' . $filterinfo['field'];
      $clean = db_escape_string($filterdata['value']);
      $query->ensure_table($fieldinfo['table']);
      $query->add_where("GeomFromText('$clean', 4326) && $field AND $func($field, GeomFromText('$clean', 4326))");
      break;
  }
  
}

function geo_views_relation_field_validator($fielddata, $view, $form) {
  // until I have a WKT processor, this is not too useful. :-)
}

/* to implement
  'query_handler' => 'geo_views_scalar_field_query_handler', // for modifying the query
  'handler' => array('geo_views_scalar_field_handler' => t('Default')), // for computing and or modifying for display
*/

/**
 * A views query handler callback to compute length, area, perimeter, etc from db
 */
function geo_views_scalar_field_query_handler($field, &$fieldinfo, &$query) {
  dpr("In the scalar handler");
  dpr($field);
  dpr($fieldinfo);
  
  $query->ensure_table($field['tablename']);
  
  // here we test on the field, as we have unique fields for length, area, perimeter
  switch($field['field']) {
    case 'length':
      $query->add_field('Length(' . $field['tablename'] . '.' .$fieldinfo['content_field']['field_name'] . ')', NULL, $field['queryname']);
      break;
      
    case 'area':
      $query->add_field('Area(' . $field['tablename'] . '.' .$fieldinfo['content_field']['field_name'] . ')', NULL, $field['queryname']);
      break;
      
    case 'perimeter':
      // TODO test this! we're on polygons, for some reason, this doesn't seem right!
      $query->add_field('Length(ConvexHull(' . $field['tablename'] . '.' .$fieldinfo['content_field']['field_name'] . '))', NULL, $field['queryname']);
      
      break;
  }
}
