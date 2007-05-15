<?php

/**
 * Postgresql + PostGIS backend for the geo field module
 *
 * @author Mark Fredrickson
 * @version $Id$
 * @package geo
 **/
 
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
 *   The Spatial Reference ID of this column's projection. The most commonly used SRID is 4326, which corresponds to unprojected lat-long in WGS84.
 * @return boolean
 * Success or failure
 **/
function geo_add_geometry_column($table, $field_name, $type = 'POINT', $srid = '4326') {
  // NOTE for now, all geometries are 2d. deal with it
  // TODO is there a way to get the current schema? until i figure this out, everything is public
  $res = db_query("SELECT AddGeometryColumn('public', '{$table}', '%s', %d, '%s', 2);", $field_name, $srid, $type);
  // TODO perhaps send the message returned from the db to watchdog?
  return db_num_rows($res);
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
 **/
function geo_drop_geometry_column($table, $field_name) {
  $res = db_query("SELECT DropGeometryColumn('public', '{$table}', '%s'", $field_name);
  return db_num_rows($res);
}

/**
 * Move a geometry field from one table to another. Implicitly calls add_geometry_column() on the new table, and drop geometry columnh on the old table.
 *
 * @param $field_name
 *  The name of the field to move.
 * @param $old_table
 *  The name of the table that currently holds the geometry information.
 * @param $new_table
 *  The name of the new table that should contain the information.
 * @return boolean
 *  Success or failure
 **/
function geo_move_geometry_column($field_name, $old_table, $new_table) {
  // TODO do I need a $key to know how to link the two tables? probably.
}

/**
 * Insert a geometry object into the database on the specified row
 *
 * @param $table
 *  The name of the table that currently holds the geometry information.
 * @param $field_name
 *  The name of the field in which to insert the data.
 * @return void
 **/
function geo_update_geometry_data($table, $field_name, $where, $geometry) {
  db_query("UPDATE $table SET $field_name = GeomFromText('%s') WHERE $where", $geometry['wkt']);
  return true; // TODO better return value someday
}