<?php // $Id$

class GeoSQLMySQL extends GeoSQL {

  public function point($x, $y) {
    return $this->sql("Point(%f, %f)", array($x, $y));
  }

  private function mysqlExpandedInput($geometries, $function) {
    $args = $sql = array();
    foreach ($geometries as $g) {
      $sql[] = '%g';
      $args[] = $g;
    }

    return $this->sql($function ."(", join(', ', $sql) .")", $args);
  }

  public function setSRID($value = NULL) {
    if ($value && is_numeric($value)) {
      $this->srid = $value;
    }
    // Attempt to retreive the the SRID from one of the data values.
    // note that this only works when the column has data.
    elseif (($table = $this->tableName()) && ($column = $this->columnName())) {
      if ($srid = db_result(db_query("SELECT SRID(%s)
          FROM {". $table ."} LIMIT 1", $column))) {

        $this->srid = $srid;
      }
    }
  }

  public function setGeoType($value = NULL) {
    if (!is_null($value) && is_numeric($value)) {
      $this->geo_type = $value;
    }
    // Attempt to retreive the the geo_type from one of the data values.
    // note that this only works when the column has data.
    elseif (($table = $this->tableName()) && ($column = $this->columnName())) {
      $type = db_result(db_query("SELECT GeometryType(%s)
        FROM {". $table ."} LIMIT 1", $column));

      // MySQL returns a named type (POINT, POLYGON, etc.)  Convert it.
      if (defined('GEO_TYPE_'. $type)) {
        $this->geo_type = constant('GEO_TYPE_'. $type);
      }
    }
  }

  protected function addIndex() {
    if (!$this->indexed) return;

    $name = $this->tableName() .'__'. $this->columnName(). '_idx';
    if (strlen($name) > 6) {
      db_query("CREATE SPATIAL INDEX $name ON  {". $this->tableName() ."} (%s)", $this->columnName());
    }
  }

  protected function dropIndex() {
    if ($this->indexed) return;

    $name = $this->tableName() .'__'. $this->columnName(). '_idx';
    if (strlen($name) > 6) {
      db_query("DROP INDEX $name ON  {". $this->tableName() ."}");
    }
  }

  public function linestring() {
    $args = func_get_args();
    return $this->mysqlExpandedInput($args, 'Linestring');
  }

  public function polygon() {
    $args = func_get_args();
    return $this->mysqlExpandedInput($args, 'Polygon');
  }

  public function multiPoint() {
    $args = func_get_args();
    return $this->mysqlExpandedInput($args, 'MultiPoint');
  }

  public function multiLinestring() {
    $args = func_get_args();
    return $this->mysqlExpandedInput($args, 'MultiLinestring');
  }

  public function multiPolygon() {
    $args = func_get_args();
    return $this->mysqlExpandedInput($args, 'MultiPolygon');
  }

  public function geometryCollection() {
    return $this->unimplemented();
  }

  public function geomFromText($wkt, $srid = GEO_SRID_DEFAULT, $type = NULL) {
    return $this->sql("GeomFromText('%s', %d)", array($wkt, $srid));
  }

  public function pointFromText($wkt, $srid = GEO_SRID_DEFAULT) {
    return $this->sql("PointFromText('%s', %d)", array($wkt, $srid));
  }

  public function lineFromText($wkt, $srid = GEO_SRID_DEFAULT) {
    return $this->sql("LineFromText('%s', %d)", array($wkt, $srid));
  }

  public function polyFromText($wkt, $srid = GEO_SRID_DEFAULT) {
    return $this->sql("PolygonFromText('%s', %d)", array($wkt, $srid));
  }

  public function mPointFromText($wkt, $srid = GEO_SRID_DEFAULT) {
    return $this->sql("MPointFromText('%s', %d)", array($wkt, $srid));
  }

  public function mLineFromText($wkt, $srid = GEO_SRID_DEFAULT) {
    return $this->sql("MLineFromText('%s', %d)", array($wkt, $srid));
  }

  public function mPolyFromText($wkt, $srid = GEO_SRID_DEFAULT) {
    return $this->sql("MPolyFromText('%s', %d)", array($wkt, $srid));
  }

  public function geomCollFromText($wkt, $srid = GEO_SRID_DEFAULT) {
    return $this->sql("GeomCollFromText('%s', %d)", array($wkt, $srid));
  }

  public function geomFromWKB($wkb, $srid = GEO_SRID_DEFAULT, $type = NULL) {
    return $this->sql("GeomFromWKB(%b, %d)", array($wkb, $srid));
  }

  public function pointFromWKB($wkb, $srid = GEO_SRID_DEFAULT) {
    return $this->sql("PointFromWKB(%b, %d)", array($wkb, $srid));
  }

  public function lineFromWKB($wkb, $srid = GEO_SRID_DEFAULT) {
    return $this->sql("LineFromWKB(%b, %d)", array($wkb, $srid));
  }

  public function polyFromWKB($wkb, $srid = GEO_SRID_DEFAULT) {
    return $this->sql("LineFromWKB(%b, %d)", array($wkb, $srid));
  }

  public function mPointFromWKB($wkb, $srid = GEO_SRID_DEFAULT) {
    return $this->sql("MPointFromWKB(%b, %d)", array($wkb, $srid));
  }

  public function mLineFromWKB($wkb, $srid = GEO_SRID_DEFAULT) {
    return $this->sql("MPointFromWKB(%b, %d)", array($wkb, $srid));
  }

  public function mPolyFromWKB($wkb, $srid = GEO_SRID_DEFAULT) {
    return $this->sql("MPolyFromWKB(%b, %d)", array($wkb, $srid));
  }

  public function geomCollFromWKB($wkb, $srid = GEO_SRID_DEFAULT) {
    return $this->sql("GeomCollFromWKB(%b, %d)", array($wkb, $srid));
  }

  public function asText($g) {
    return $this->sql("AsText(%g)", array($g));
  }

  public function asBinary($g) {
    return $this->sql("AsBinary(%g)", array($g));
  }

  public function dimension($g) {
    if ($g) return $this->sql("Dimension(%g)", array($g));
    return $this->geo_dimension;
  }

  public function geometryType($g = NULL) {
    if ($g) return $this->sql("GeometryType(%g)", array($g));

    // If the geometry is NULL, derive the geometrytype for this instance.
    $this->setGeoType();
    return GeoSimple::geometryType($this);
  }

  public function SRID($g = NULL) {
    if ($g) return $this->sql("SRID(%g)", array($g));

    // If the geometry is NULL, derive the SRID for this instance.
    $this->setSRID();
    return $this->srid;
  }

  public function envelope($g) {
    return $this->sql("Envelope(%g)", array($g));
  }

  public function x($g) {
    return $this->sql("X(%g)", array($g));
  }

  public function y($g) {
    return $this->sql("Y(%g)", array($g));
  }

  public function startPoint($g) {
    return $this->sql("StartPoint(%g)", array($g));
  }

  public function endPoint($g) {
    return $this->sql("EndPoint(%g)", array($g));
  }

  public function isClosed($g) {
    return $this->sql("IsClosed(%g)", array($g));
  }

  public function length($g) {
    return $this->sql("GLength(%g)", array($g));
  }

  public function numPoints($g) {
    return $this->sql("NumPoints(%g)", array($g));
  }

  public function pointN($g, $n) {
    return $this->sql("PointN(%g, %d)", array($g, $n));
  }

  public function centroid($g) {
    return $this->sql("Centroid(%g)", array($g));
  }

  public function area($g) {
    return $this->sql("Area(%g)", array($g));
  }

  public function exteriorRing($g) {
    return $this->sql("ExteriorRing(%g)", array($g));
  }

  public function numInteriorRing($g) {
    return $this->sql("NumInteriorRings(%g)", array($g));
  }

  public function interiorRingN($g, $n) {
    return $this->sql("InteriorRingN(%g)", array($g, $n));
  }

  public function numGeometries($g) {
    return $this->sql("NumGeometries(%g)", array($g));
  }

  public function geometryN($g, $n) {
    return $this->sql("GeometryN(%g)", array($g, $n));
  }

  public function equals($g1, $g2) {
    return $this->sql("MBREqual(%g, %g)", array($g1, $g2));
  }

  public function disjoint($g1, $g2) {
    return $this->sql("MBRDisjoint(%g, %g)", array($g1, $g2));
  }

  public function touches($g1, $g2) {
    return $this->sql("MBRTouches(%g, %g)", array($g1, $g2));
  }

  public function within($g1, $g2) {
    return $this->sql("MBRWithin(%g, %g)", array($g1, $g2));
  }

  public function overlaps($g1, $g2) {
    return $this->sql("MBROverlaps(%g, %g)", array($g1, $g2));
  }

  public function crosses($g1, $g2) {
    return $this->sql("Crosses(%g, %g)", array($g1, $g2));
  }

  public function intersects($g1, $g2) {
    return $this->sql("MBRIntersects(%g, %g)", array($g1, $g2));
  }

  public function contains($g1, $g2) {
    return $this->sql("MBRContains(%g, %g)", array($g1, $g2));
  }
}
