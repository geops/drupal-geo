<?php // $Id$
/**
 * @file
 * A simple SQL implementation of the Geo API.
 *
 * This file contains a database-agnostic framework for using a SQL backend
 * for spatial data.  It contains basic handlers for accessing and formatting
 * SQL queries, but the actual reference functions are unimplemented.
 *
 * This class should be extended and its subclasses should account for the
 * actual queries.  (See GeoMySQL and GeoPostGIS)
 */

class GeoSQL implements GeoInterface {

  protected $gid;
  protected $title;
  protected $name;
  protected $srid = GEO_UNKNOWN;
  protected $geo_type;
  protected $geo_dimension;

  protected $table_name;
  protected $column_name;
  protected $schema;
  protected $count;

  function __construct($values = NULL) {
    $current = get_object_vars($this);
    if ($values) {
      foreach ((array)$values as $name => $val) {
        if ($name == 'data' && !empty($val) && is_scalar($val)) {
          $this->__construct(unserialize($val));
        }
        elseif (array_key_exists($name, $current) && !is_null($val)) {
          $func = 'set'. str_replace('_', '', $name);
          if (method_exists($this, $func)) {
            $this->$func($val);
          }
          else {
            $this->$name = $val;
          }
          unset($current[$name]);
        }
      }
    }
  }

  function unimplemented() {
    drupal_set_message(t('Attempt to call an unimplemented function'));
    return FALSE;
  }

  /**
   * Take a sql query as returned by a function and expand it.
   */
  function sql($sql, $args = array()) {
    if (is_array($args)) {
      foreach ($args as $i => $g) {
        if (is_object($g)) {

          // If the backend is the same as this one, query the column directly.
          if ($g->geoCompatible($this)) {
            $sql = preg_replace('/%g/', '%s', $sql['sql'], 1);
            $args[$i] = $g->sqlTableAlias();
          }

          // Otherwise, use the Binary representation.
          else {
            $sql = preg_replace('/%g/', '%b', $sql['sql'], 1);
            $args[$i] = $g->asBinary();
          }
        }
      }
    }

    return array('sql' => $sql, 'args' => $args);
  }

  function setGID($value = NULL) {
    $this->gid = (int) $value;
  }

  function gID() {
    return $this->gid;
  }

  function setTitle($value) {
    $this->title = filter_xss($value);
  }

  function title() {
    return $this->title;
  }

  function setSRID($value = NULL) {
    if (is_int($value)) {
      $this->srid = $value;
    }
  }

  function setGeoDimension($value = NULL) {
    if (is_int($value)) {
      $this->geo_dimension = $value;
    }
  }

  function setGeoType($value = NULL) {
    if (is_int($value)) {
      $this->geo_type = $value;
    }
  }

  function geoType() {
    return $this->geo_type;
  }

  public function vars() {
    return get_object_vars($this);
  }

  function setTableName($value) {
    $this->table_name = check_plain($value);
  }

  function tableName() {
    return $this->table_name;
  }

  function setColumnName($value) {
    $this->column_name = check_plain($value);
  }

  function columnName() {
    if (isset($this->table_name)) {
      return check_plain($this->column_name);
    }
  }

  function setSchema($value) {
    if (isset($value) && is_array($value)) {
      $this->schema = $value;
    }
  }

  function schema() {
    if (is_array($this->schema)) {
      return $this->schema;
    }
  }

  function setCount() {
    if (!isset($this->count)) {
      if ($table = $this->tableName()) {
        $this->count = db_result(db_query("SELECT COUNT(1) FROM {%s}", $table));
      }
    }
  }

  function count() {
    $this->setCount();
    return (int) $this->count;
  }

  // Implementation-specfic functions.
  public function addPoint($g, $point, $position = NULL) {
    return $this->unimplemented();
  }

  // Geo type constructors.
  public function point($x, $y) {
    return $this->unimplemented();
  }

  public function linestring() {
    return $this->unimplemented();
  }

  public function polygon() {
    return $this->unimplemented();
  }

  public function multiPoint() {
    return $this->unimplemented();
  }

  public function multiLinestring() {
    return $this->unimplemented();
  }

  public function multiPolygon() {
    return $this->unimplemented();
  }

  public function geometryCollection() {
    return $this->unimplemented();
  }

  public function geomFromText($wkt, $srid = GEO_SRID_DEFAULT, $type = NULL) {
    return $this->unimplemented();
  }

  public function pointFromText($wkt, $srid = GEO_SRID_DEFAULT) {
    return $this->unimplemented();
  }

  public function lineFromText($wkt, $srid = GEO_SRID_DEFAULT) {
    return $this->unimplemented();
  }

  public function polyfromText($wkt, $srid = GEO_SRID_DEFAULT) {
    return $this->unimplemented();
  }

  public function mPointFromText($wkt, $srid = GEO_SRID_DEFAULT) {
    return $this->unimplemented();
  }

  public function mLineFromText($wkt, $srid = GEO_SRID_DEFAULT) {
    return $this->unimplemented();
  }

  public function mPolyFromText($wkt, $srid = GEO_SRID_DEFAULT) {
    return $this->unimplemented();
  }

  public function geomCollFromText($wkt, $srid = GEO_SRID_DEFAULT) {
    return $this->unimplemented();
  }

  public function bdPolyFromText($wkt, $srid = GEO_SRID_DEFAULT) {
    return $this->unimplemented();
  }

  public function bdMPolyFromText($wkt, $srid = GEO_SRID_DEFAULT) {
    return $this->unimplemented();
  }

  public function geomFromWKB($wkb, $srid = GEO_SRID_DEFAULT, $type = NULL) {
    return $this->unimplemented();
  }

  public function pointFromWKB($wkb, $srid = GEO_SRID_DEFAULT) {
    return $this->unimplemented();
  }

  public function lineFromWKB($wkb, $srid = GEO_SRID_DEFAULT) {
    return $this->unimplemented();
  }

  public function polyFromWKB($wkb, $srid = GEO_SRID_DEFAULT) {
    return $this->unimplemented();
  }

  public function mPointFromWKB($wkb, $srid = GEO_SRID_DEFAULT) {
    return $this->unimplemented();
  }

  public function mLineFromWKB($wkb, $srid = GEO_SRID_DEFAULT) {
    return $this->unimplemented();
  }

  public function mPolyFromWKB($wkb, $srid = GEO_SRID_DEFAULT) {
    return $this->unimplemented();
  }

  public function geomCollFromWKB($wkb, $srid = GEO_SRID_DEFAULT) {
    return $this->unimplemented();
  }

  public function bdPolyFromWKB($wkb, $srid = GEO_SRID_DEFAULT) {
    return $this->unimplemented();
  }

  public function bdMPolyFromWKB($wkb, $srid = GEO_SRID_DEFAULT) {
    return $this->unimplemented();
  }

  public function asText($g) {
    return $this->unimplemented();
  }

  public function asBinary($g) {
    return $this->unimplemented();
  }

  public function dimension($g = NULL) {
    if (!$g) return $this->geo_dimension;

    return $this->unimplemented();
  }

  public function geometryType($g = NULL) {
    // Use the GeoSimple function, which maintains the constant -> name mappings.
    if (!$g) return GeoSimple::geometryType($this);

    return $this->unimplemented();
  }

  public function SRID($g = NULL) {
    if (!$g) return $this->srid;

    return $this->unimplemented();
  }

  public function isEmpty($g) {
    return $this->unimplemented();
  }

  public function isSimple($g) {
    return $this->unimplemented();
  }

  public function boundary($g) {
    return $this->unimplemented();
  }

  public function envelope($g) {
    return $this->unimplemented();
  }

  public function x($point) {
    return $this->unimplemented();
  }

  public function y($point) {
    return $this->unimplemented();
  }

  public function startPoint($curve) {
    return $this->unimplemented();
  }

  public function endPoint($curve) {
    return $this->unimplemented();
  }

  // also: multicurve.
  public function isClosed($curve) {
    return $this->unimplemented();
  }

  public function isRing($curve) {
    return $this->unimplemented();
  }

  // also: multicurve.
  public function length($curve) {
    return $this->unimplemented();
  }

  public function numPoints($linestring) {
    return $this->unimplemented();
  }

  public function pointN($linestring, $n) {
    return $this->unimplemented();
  }

  // also: multisurface.
  public function centroid($surface) {
    return $this->unimplemented();
  }

  // also: multisurface.
  public function pointOnSurface($surface) {
    return $this->unimplemented();
  }

  // also: multisurface.
  public function area($surface) {
    return $this->unimplemented();
  }

  public function exteriorRing($polygon) {
    return $this->unimplemented();
  }

  public function numInteriorRing($polygon) {
    return $this->unimplemented();
  }

  public function interiorRingN($polygon, $n) {
    return $this->unimplemented();
  }

  public function numGeometries($geomcollection) {
    return $this->unimplemented();
  }

  public function geometryN($geomCollection, $n) {
    return $this->unimplemented();
  }

  public function equals($g1, $g2) {
    return $this->unimplemented();
  }

  public function disjoint($g1, $g2) {
    return $this->unimplemented();
  }

  public function touches($g1, $g2) {
    return $this->unimplemented();
  }

  public function within($g1, $g2) {
    return $this->unimplemented();
  }

  public function overlaps($g1, $g2) {
    return $this->unimplemented();
  }

  public function crosses($g1, $g2) {
    return $this->unimplemented();
  }

  public function intersects($g1, $g2) {
    return $this->unimplemented();
  }

  public function contains($g1, $g2) {
    return $this->unimplemented();
  }

  public function relate($g1, $g2, $pattern = NULL) {
    return $this->unimplemented();
  }

  public function distance($g1, $g2) {
    return $this->unimplemented();
  }

  public function intersection($g1, $g2) {
    return $this->unimplemented();
  }

  public function difference($g1, $g2) {
    return $this->unimplemented();
  }

  public function union($g1, $g2) {
    return $this->unimplemented();
  }

  public function symDifference($g1, $g2) {
    return $this->unimplemented();
  }

  public function buffer($g1, $distance) {
    return $this->unimplemented();
  }

  public function convexHull($g1) {
    return $this->unimplemented();
  }
}
