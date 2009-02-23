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
  db_query("CREATE SPATIAL INDEX {$table}_${field}_idx ON  {". $table ."} ($field)");

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

function geo_mysql_spatial_query_function($func, $field, $value, $srid) {
  return array("$func(GeomFromText('$value', $srid), $field)");
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
function geo_mysql_spatial_tables($exclude = NULL) {
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
function geo_mysql_spatial_table_desc($table) {
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
function geo_mysql_spatial_version() {
  // No clue what to do here...
//  return db_result(db_query("SELECT postgis_version()"));
}
