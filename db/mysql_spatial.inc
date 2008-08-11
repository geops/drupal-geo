<?php // $Id$

/**
 * @file
 * MySQL Spatial Extensions backend for the geo field module
 *
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
function geo_mysql_spatial_add_field(&$ret, $table, $field, $spec) {
  // NOTE for now, all geometries are 2d. deal with it
  $ret[] = db_query("ALTER TABLE {%s} ADD %s %s NOT NULL", $table, $field, $spec['type']);
  db_query("CREATE SPATIAL INDEX {". $table ."}_${field}_idx ON {$table} ($field)");

  // @@@ Error handling
  return true;
}

/**
 * Remove a geometry column from the specified table.
 *
 * @param $table
 *   The name of the table from which to drop the column.
 * @param $field_name
 *   The name of the field to drop.
 *
 * @return boolean
 *  Success or failure
 */
function geo_mysql_spatial_drop_field($table, $column) {
  db_query("ALTER TABLE {%s} DROP COLUMN %s", $table, $column);
  return true;
}

function geo_field_validate($wkt, $srid) {
/*
     $delta = 0; // @@@ Multivalue?
      if (!empty($node_field[0]['wkt'])) {
        $res = db_result(@db_query("SELECT GeometryType(GeomFromText('%s', $srid))", $node_field[0]['wkt']));
        if (empty($res)) {
          form_set_error($field['field_name'] .']['. $delta .'][wkt', t('Unable to parse WKT.'));
        }
        else {
          if ($geo_field['geometry_type'] != $res) {
            form_set_error($field['field_name'] .']['. $delta .'][wkt', t('Wrong geometry type. Got %result, was expecting %type.', array('%result' => $res, '%type' => $geo_field['geometry_type'])));
          }
        }
      }

*/
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
function geo_db_tables($exclude = NULL) {
  static $tables;

  if (!is_array($tables)) {
    $tables = array();

    // Query for all available geometry columns.
    // @@@ Mysql5 + only.
    $res = db_query("SELECT TABLE_NAME AS 'table', COLUMN_NAME AS 'field'
      FROM information_schema.columns
      WHERE TABLE_SCHEMA = database()
      AND DATA_TYPE IN ('GEOMETRY','POINT','LINESTRING','POLYGON','MULTIPOINT',
      'MULTILINESTRING','MULTIPOLYGON','GEOMETRYCOLLECTION')
      ORDER BY TABLE_NAME, COLUMN_NAME");

    while ($row = db_fetch_object($res)) {
      // Ignore exclusions.
      if ($exclude && preg_match($exclude, $row->table)) continue;

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
function geo_db_table_desc($table) {
  $res = db_query("SELECT COLUMN_NAME AS 'name',
    COLUMN_COMMENT AS 'description',
    DATA_TYPE AS 'type',
    IS_NULLABLE AS 'null_ok',
    COLUMN_DEFAULT AS 'default'
    FROM information_schema.columns
    WHERE TABLE_SCHEMA = database()
    AND TABLE_NAME = '%s' ORDER BY ORDINAL_POSITION", $table);

  $columns = array();
  $geo = array();
  while ($row = db_fetch_array($res)) {
    if ($row['type'] == 'geometry') {
      $geo[] = $row['name'];
    }

    $columns[$row['name']] = $row;
    $columns[$row['name']]['not_null'] = $row['null_ok']=='YES' ? FALSE : TRUE;
    unset($columns[$row['name']]['null_ok']);
  }

  // @@@ I have no clue what this section is for. ~Bdragon

/*  $res = db_query("SELECT c.relname AS table, 
      pg_catalog.pg_get_constraintdef(r.oid, true) AS constraint
    FROM pg_catalog.pg_constraint r
    LEFT JOIN pg_catalog.pg_class c ON r.conrelid = c.oid
    WHERE c.relname = '%s' AND r.contype = 'c'", $table);
*/

  /*
   Find constraints that are similar to the ones here, and set attributes

   CHECK (geometrytype(the_geom) = 'MULTIPOLYGON'::text OR the_geom IS NULL)
   CHECK (ndims(the_geom) = 2)
   CHECK (srid(the_geom) = 26915)
  */
  //while ($row = db_fetch_object($res)) {
  //  $table = $row->table;
  //  foreach($geo as $f) {
  //    if (strpos($row->constraint, "ndims($f)" )) {
  //      $columns[$f]['dimensions'] = preg_replace('/\D/','', $row->constraint);
  //    }
  //    elseif (strpos($row->constraint, "srid($f)" )) {
  //      $columns[$f]['srid'] = preg_replace('/\D/','', $row->constraint);
  //    }
  //    elseif (strpos($row->constraint, "geometrytype($f)" )) {
  //      $columns[$f]['geometry_type'] = preg_replace('/.*\'(\w+)\'.*/','$1', $row->constraint);
  //    }
  //  }
  //}

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
  // No clue what to do here...
//  return db_result(db_query("SELECT postgis_version()"));
}
