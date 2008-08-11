<?php // $Id$

/**
 * Return the geo backend database type.
 *
 * @return string
 *  The name of the backend: postgis, cck, mysql_spatial
 */
function geo_backend_type() {
  static $backend;

  if (!isset($backend)) {
    switch ($GLOBALS['db_type']) {
      case 'pgsql':
        // use the postgis backend if installed
        if (db_result(db_query("SELECT 1 FROM pg_catalog.pg_proc 
          WHERE proname = 'postgis_version'"))) {
          $backend = 'postgis';
        }
        break;

      case 'mysql':
      case 'mysqli':
        // Capabilities check.
        // If it can convert between native and wkt, assume it's working.
        if (db_result(@db_query("SELECT AsText(GeomFromText('POINT(1 1)'))")) == 'POINT(1 1)') {
          $backend = 'mysql_spatial';
        }
    }
  }

  return $backend;
}

function geo_wkt_from_point($lat, $lon) {
  return 'POINT('. $lon .' '. $lat .')';
}


function geo_field_select($field) {
  return " SRID($field) as srid, 
    asText($field) AS wkt,
    asText(centroid($field)) AS centroid,
    asText(envelope($field)) as bbox,
    Dimension($field) as dimension,
    GeometryType($field) AS type ";
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
function geo_geo_from_wkt($wkt, $srid = GEO_DEFAULT_SRID) {
  return " GeomFromText('". $wkt ."') ";
}

function geo_wkt_validate($wkt) {
}
