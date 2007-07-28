<?php
// $Id$

/**
 * Views related functionality for the geo module
 *
 * @author Mark Fredrickson
 * @version $Id$
 * @package geo
 **/

/*
 * Views hooks
 */
 
/**
 * Implemenation of hook_views_tables().
 */
function geo_views_tables() {
  /* pseudo
  1. load all the data from geo_link left joined to the geo_field
  2. foreach, create the table
  */
  
  $res = db_query("SELECT * FROM {geo_link} l LEFT JOIN {geo_field} f ON l.gfid = f.gfid");
  while ($row = db_fetch_array($res)) {
    $tables['geo_link_'. $row['table_name'] .'_'. $row['field_name']] =
      geo_views_create_geometry_table($row['table_name'], 
                                      $row['field_name'],
                                      t('Geo: @field_name'), // XXX add this to geo_field table
                                      $row['left_table'],
                                      $row['left_field'],
                                      $row['right_field']);
  }
  return $tables;
}

/*
 * Functions for creating views tables for geometry objects
 */
 
/**
 * Creates the definition of a table with a geometry field in it.
 *
 * @param $table_name
 *  The name of the database table where the field is located
 * @param $field_name
 *  The name of the geometry field we are working on.
 * @param $label_prefix
 *  A string of text to prepend to the label, e.g. "Point: My Point Field (field_foo)"
 * @param $left_table
 *  The views table where we should attach this definition. e.g. 'node'
 * @param $left_field
 *  The field in the left table to use in the join. e.g. 'nid'
 * @param $right_field
 *  The column in your table that will be used in the join e.g. 'nid'
 * @param $extra_data
 *  An array of additional key value pairs to to add to each field, filter, and sort to be used in later handlers
 * @param $join_extras
 *  An array of additional join parameters. See the views developer documentation for more information on the form of this array.
 * @return array
 *  The views table array. Caller should wrap it in a $tables array, and give it an alias as its key.
 *  Eg. return array('my_table_alias' => geo_views_create_geometry_table(...));
 * @author Mark Fredrickson
 */
function geo_views_create_geometry_table($table_name, $field_name, $label_prefix, $left_table, $left_field, $right_field, $extra_data = array(), $join_extras = NULL) {
  // first, fetch the geometry table information to find out about this field
  $known_fields = geo_fields();
  foreach ($known_fields as $module_fields) {
    foreach ($module_fields as $field) {
      if ($field['field_name'] == $field_name && $field['table_name'] = $table_name) {
        $this_field = $field;
      }
    }
  }
  
  if (!isset($this_field)) {
    // couldn't find the field in the info table, so we have to bail
    return;
  }
  
  $table = views_new_table($table_name, 'geo', $left_table, $left_field, $right_field, $join_extras);

  /*** Fields & Filters ***/
  
  // the first field is the general geometry returning fields. Set up an array to iterate through
  $geometry_fields = array(
    'wkt' => t('Geometry'),
    'centroid' => t('Centroid'), 
    'bbox' => t('Bounding Box'),
  );
  
  if ($field['type'] == 'linestring') {
    $geometry_fields['start'] = t('Starting Point');
    $geometry_fields['end'] = t('Ending Point');
  }
  
  $field_help = t('The handler drop-down will choose how the data is formatted.');
  $field_addl_keys = array(
    'query_handler' => 'geo_views_field_query_handler',
    'handler' => geo_views_spatial_field_handlers(),
  );
  $field_addl_keys = array_merge($field_addl_keys, $extra_data);
  
  // set up some the corresponding filter data
  $filter_help = t('Filter nodes by a spatial relation to this field. In the "value" box, include the Well-Known-Text representation of the geometry you wish to filter against. For the "Within" operator, your geometry must be a POLYGON.');
  $filter_addl_keys = array(
    'operator' => geo_views_spatial_relation_operators(),
    'handler' => 'geo_views_relation_filter_handler',
    'validate' => 'geo_views_relation_filter_validate',
  );
  $filter_addl_keys = array_merge($filter_addl_keys, $extra_data);
  
  foreach ($geometry_fields as $key => $label) {
    // add the field to the table
    views_table_add_field($table, $key, "$label_prefix: $label", $field_help, $field_addl_keys);
    
    // add the spatial relation filter for this field
    $filter_addl_keys['field'] = $key;
    views_table_add_filter($table, $key, "$label_prefix: $label", $filter_help, $filter_addl_keys);
  }
  
  /* potential additional fields, depending on type
  all: is valid
  */
  
  // filters are defined here for simplicity, instead of $op == 'filters'
  /* proposed filters
  foreach entered point, linestring, polygon as g
    - distance(g, field) >|<|=.... a value <-- I don't know if this will work with views right now
  just polygons: within(g, field)
                 area(field) cmp integer
                 permiter(field) cmp integer
  just linestrings: length cmp integer
  */
  
  /*** Per Type additions ***/
  
  // Fields for scalar values
  // but first we create a template to use
  $scalar_template = array(
    'notafield' => true, // XXX is this necessary? do I want something else here
    'query_handler' => 'geo_views_scalar_field_query_handler', // for modifying the query
    'handler' => array('geo_views_scalar_field_handler' => t('Default')), // for computing and or modifying for display
    'content_field' => $field,
  );
  switch ($field['type']) {
    case 'linestring':
      $table['fields']['length'] = $scalar_template;
      $table['fields']['length']['name'] = $view_label .' -- '. t('Length');
      $table['fields']['length']['help'] = t('Provides the length of the linestring.');
      $table['fields']['length']['sortable'] = true;
      // TODO provide an option for units (miles, km, m, feet, etc)
      
      // XXX sorts are broken until views adds sorts based on it's table definitions, not just raw SQL
      // sort on the length field
      $table['sorts']['length'] = array(
        'name' => $view_label .' -- '. t('Length'),
        'help' => t('Sort on the length of the linestring.'),
      );
      break;
    case 'polygon':
      $table['fields']['area'] = $scalar_template;
      $table['fields']['area']['name'] = $view_label .' -- '. t('Area');
      $table['fields']['length']['help'] = t('Provides the area of the polygon.');
      
      $table['fields']['perimeter'] = $scalar_template;
      $table['fields']['perimeter']['name'] = $view_label .' -- '. t('Perimeter');
      $table['fields']['perimeter']['help'] = t('Provides the perimeter of the polygon.');

      break;
  }
  
  return $table;
}

/*
 * Views operators, handlers, and other useful functions
 */
 
/**
 * Returns an array of operators for spatial relations.
 *
 * @return array
 * @author Mark Fredrickson
 */
function geo_views_spatial_relation_operators() {
  return array(
    'Equals' => t('Equals'),
    'NOT Equals' => t('Does not equal'),
    'Intersects' => t('Intersects'),
    'NOT Intersects' => t('Does not intersect'),
    'Within' => t('Within'),
    'NOT Within' => t('Not within'),
  );
}

/**
 * Returns an array of the standard spatial handlers for geometry fields
 *
 * @return array
 * @author Mark Fredrickson
 */
function geo_views_spatial_field_handlers() {
  // perhaps define a hook for additiona spatial field handlers, eg. a wkt -> gmap handler
  return array(
    'geo_views_field_handler_wkt' => t('Well Known Text'),
    'geo_views_field_handler_svg' => t('SVG'),
    'geo_views_field_handler_kml' => t('KML'),
    'geo_views_field_handler_gml' => t('GML'),
  );  
}

