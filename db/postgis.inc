<?php // $Id$

/**
 * Postgresql + PostGIS backend for the geo field module
 *
 * @author Mark Fredrickson
 * @version $Id$
 * @package geo
 */

/*** API Functions ***/

/**
 * Adds a geometry column to the specified table.
 * 
 * @param $table
 *   The name of the table to which to add the column. Do not include braces ({}).
 * @param $field_name
 *   The name of the field to use.
 * @param $type
 *   The OpenGIS type of the column.
 *   Valid types are: POINT, LINESTRING, POLYGON, MULTIPOINT, MULTILINESTRING, MULTIPOLYGON, GEOMETRYCOLLECTION
 * @param $srid
 *   The Spatial Reference ID of this column's projection. The most commonly used SRID is GEO_SRID_DEFAULT, which corresponds to unprojected lat-long in WGS84.
 * @return boolean
 * Success or failure
 */
function geo_db_add_field(&$ret, $table, $field, $spec) {
  // NOTE for now, all geometries are 2d. deal with it
  // TODO is there a way to get the current schema? until i figure this out, everything is public
  $res = db_query("SELECT AddGeometryColumn('public', '{". $table ."}', '%s', %d, '%s', 2);", $column, $srid, $type);
  // TODO perhaps send the message returned from the db to watchdog?
  if (db_num_rows($res)) {
    // if successful, add a spatial index on the field
    db_query("CREATE INDEX {". $table ."}_". $column ."_idx ON {". $table ."} USING GIST ($column GIST_GEOMETRY_OPS)");
    return true;
  }
}

/**
 * Remove a geometry column from the specified table.
 *
 * @param $table
 *   The name of the table from which to drop the column. Do not include braces ({}).
 * @param $field_name
 *   The name of the field to drop.
 *
 * @return boolean
 *  Success or failure
 *
 */
function geo_db_drop_field($table, $column) {
  $res = db_query("SELECT DropGeometryColumn('public', '{%s}', '%s'", $table, $column);
  // NOTE do I need to drop my spatial index too? should probably check pg docs
  return db_num_rows($res);
}

function geo_db_field_select($field) {
  return " srid($field) as srid, asText($field) AS wkt, asText(centroid($field)) AS centroid, asText(envelope($field)) as bbox ";
}

/**
 * Insert a geometry object into the database on the specified row
 *
 * @param $table
 *  The name of the table that currently holds the geometry information.
 * @param $field_name
 *  The name of the field in which to insert the data.
 * @return void
 */
function geo_db_field_from_wkt($wkt, $srid = GEO_DEFAULT_SRID) {
  return " GeomFromText('". $wkt ,"') ";
}

function geo_db_field_validate($wkt) {
}

/*
 * Views Functions
 * 
 * These functions modify the queries sent to the database to take advantage
 * of db level transformations to output the geometries as various formats,
 * compute centroids, start and end points, area, length, etc.
 *
 * For other database backends that don't provide these functions, they
 * can be emulated at the views formatting phase using a different set
 * of handlers.
 */

/**
 * A views query handler callback to manage getting WKT, etc from the db
 * The function signature is determined by views.module
 *
 * This query handler modifies a views query to retrieve data as WKT, KML, etc.
 */
function geo_views_field_query_handler($field, &$fieldinfo, &$query) {
  
  switch ($field['handler']) {
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
  switch ($field['options']) {
    case 'wkt':
      $query->add_field($func .'('. $field['tablename'] .'.'. $fieldinfo['content_field']['field_name'] .')', NULL, $field['queryname']);
      break;
    case 'centroid':
      $query->add_field($func .'(Centroid('. $field['tablename'] .'.'. $fieldinfo['content_field']['field_name'] .'))', NULL, $field['queryname']);
      break;
    case 'bbox':
      $query->add_field($func .'(Envelope('. $field['tablename'] .'.'. $fieldinfo['content_field']['field_name'] .'))', NULL, $field['queryname']);
      break;
    // start point is only for linestrings, but we already tested that earlier
    case 'start':
      $query->add_field($func .'(StartPoint('. $field['tablename'] .'.'. $fieldinfo['content_field']['field_name'] .'))', NULL, $field['queryname']);
      break;
    // end point is only for linestrings, but we already tested that earlier
    case 'end':
      $query->add_field($func .'(EndPoint('. $field['tablename'] .'.'. $fieldinfo['content_field']['field_name'] .'))', NULL, $field['queryname']);
      break;
  }
  // make sure views default query builder does not add anything
  $fieldinfo['notafield'] = true;
  unset($fieldinfo['addlfields']);
}

/**
 * Views filter callback to handler for spatial relation filters (e.g. within, intersect, etc)
 * The function signature is determined by views.module
 *
 * This filter is used to for the spatial relation filtering. It adds a bounding box clause to
 * the query to speed up indexed queries, and refines by the supplied function.
 */
function geo_views_relation_filter_handler($op, $filterdata, $filterinfo, &$query) {
  switch ($op) {
    case 'handler':
      $func = $filterdata['operator'];
      $field = $filterinfo['table'] .'.'. $filterinfo['field'];
      $clean = db_escape_string($filterdata['value']);
      $query->ensure_table($fieldinfo['table']);
      
      // these relations should have overlapping boundingboxes
      if (in_array(array('Equals', 'Within', 'Intersects'), $func)) {
        $use_index = "GeomFromText('$clean', GEO_SRID_DEFAULT) && $field AND ";
      }
      else { // if the boundingboxes don't touch, we know we are good OR use the function
        $use_index = "NOT(GeomFromText('$clean', GEO_SRID_DEFAULT) && $field) OR ";
      }
      
      $query->add_where($use_index ."$func($field, GeomFromText('$clean', GEO_SRID_DEFAULT))");
      break;
  }
}

/**
 * A views query handler callback to compute length, area, perimeter, etc from db
 */
function geo_views_scalar_field_query_handler($field, &$fieldinfo, &$query) {
  //dpr("In the scalar handler");
  //dpr($field);
  //dpr($fieldinfo);
  
  $query->ensure_table($field['tablename']);
  
  // here we test on the field, as we have unique fields for length, area, perimeter
  switch ($field['field']) {
    case 'length':
      $query->add_field('Length('. $field['tablename'] .'.'. $fieldinfo['content_field']['field_name'] .')', NULL, $field['queryname']);
      break;
      
    case 'area':
      $query->add_field('Area('. $field['tablename'] .'.'. $fieldinfo['content_field']['field_name'] .')', NULL, $field['queryname']);
      break;
      
    case 'perimeter':
      // TODO test this! we're on polygons, for some reason, this doesn't seem right!
      $query->add_field('Length(ConvexHull('. $field['tablename'] .'.'. $fieldinfo['content_field']['field_name'] .'))', NULL, $field['queryname']);
      
      break;
  }
}

/*** Helper functions ***/

/**
 * Return a list of fields, keyed by table, of geo tables.
 * 
 * @param $table
 *  an optional string of the table name to look in
 *
 * @return array
 *  an array of fields, keyed by table
 */
function _geo_enabled_tables() {
  static $tables;

  if (!is_array($tables)) {
    $tables = array();

    // Query for all available geometry columns.
    $res = db_query("SELECT c.relname AS table, a.attname AS field
      FROM pg_catalog.pg_attribute a
      LEFT JOIN pg_catalog.pg_class c ON a.attrelid = c.oid
      WHERE pg_catalog.format_type(a.atttypid, a.atttypmod) = 'geometry'
      AND  c.relname != 'geometry_dump'
      ORDER BY c.relname, a.attnum");

    while ($row = db_fetch_object($res)) {
      if (!isset($tables[$row->table])) {
        $tables[$row->table] = array();
      }
      $tables[$row->table][] = $row->field;
    }
  }
  return $tables;
}

/**
 * Parse out the table descriptions from the geometry information stored in the table.
 */
function _geo_table_desc($table) {
  $res = db_query("SELECT  a.attname AS name, 
    col_description(c.oid, a.attnum) AS description,
    format_type(a.atttypid, a.atttypmod) AS type,
    a.attnotnull AS not_null,
    d.adsrc AS default   
    FROM pg_class c
    LEFT JOIN pg_attribute a ON (a.attrelid = c.oid)    LEFT OUTER JOIN pg_attrdef d ON (d.adrelid = c.oid AND d.adnum = a.attnum)
    WHERE c.relname = '%s'
    AND a.attnum > 0
    AND NOT a.attisdropped
    AND a.attrelid = c.oid
    ORDER BY a.attnum", $table);

  $columns = array();
  $geo = array();
  while ($row = db_fetch_array($res)) {
    if ($row['type'] == 'geometry') {
      $geo[] = $row['name'];
    }

    $columns[$row['name']] = $row;
    $columns[$row['name']]['not_null'] = (bool) str_replace('f', '', $row['not_null']);
  }

  $res = db_query("SELECT c.relname AS table, 
      pg_catalog.pg_get_constraintdef(r.oid, true) AS constraint
    FROM pg_catalog.pg_constraint r
    LEFT JOIN pg_catalog.pg_class c ON r.conrelid = c.oid
    WHERE c.relname = '%s' AND r.contype = 'c'", $table);

  /*
   Find constraints that are similar to the ones here, and set attributes

   CHECK (geometrytype(the_geom) = 'MULTIPOLYGON'::text OR the_geom IS NULL)
   CHECK (ndims(the_geom) = 2)
   CHECK (srid(the_geom) = 26915)
  */
  while ($row = db_fetch_object($res)) {
    $table = $row->table;
    foreach ($geo as $f) {
      if (strpos($row->constraint, "ndims($f)")) {
        $columns[$f]['dimensions'] = preg_replace('/\D/', '', $row->constraint);
      }
      elseif (strpos($row->constraint, "srid($f)")) {
        $columns[$f]['srid'] = preg_replace('/\D/', '', $row->constraint);
      }
      elseif (strpos($row->constraint, "geometrytype($f)")) {
        $columns[$f]['geometry_type'] = preg_replace('/.*\'(\w+)\'.*/', '$1', $row->constraint);
      }
    }
  }
  return $columns;
}

// provide a quick way to get the SRID of a column and use it in the storage
function _geo_fromtext($string, $table, $field) {
  $srid = db_result(db_query("SELECT srid FROM {gis_sources} 
    WHERE table = '%s' AND field = '%s'"));

  return "GeomFromText('$string', $srid)";
}

// Point to latlon
function _geo_latlonfrompoint($wkt, $srid = GEO_SRID_DEFAULT) {
  // Yep. Not efficient.
  return array(
    'lat' => db_result(db_query("SELECT X(GeomFromText('%s', %d))", $wkt, $srid)),
    'lon' => db_result(db_query("SELECT Y(GeomFromText('%s', %d))", $wkt, $srid)),
  );
}

// returns the PostGIS version information
function _geo_version() {
  return db_result(db_query("SELECT postgis_version()"));
}
