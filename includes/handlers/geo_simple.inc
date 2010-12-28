<?php
// $Id$

/**
 * @file
 * A simple implementation of the Geo API.
 *
 * This file contains a backend-agnostic PHP implementation of OGC's GIS
 * Specification.
 * It includes Basic parsing functions, constructors and other
 * calculations that are easy or necessary to handle in PHP, but a majority
 * of functions will remain unimplemented and left for other backends to handle.
 */

class GeoSimple implements GeoInterface {

  protected $gid;
  protected $name;
  protected $title;
  protected $srid = GEO_UNKNOWN;
  protected $geo_type;

  public function geometryTypes($value = NULL) {
    $types = array(
      GEO_TYPE_POINT => 'point',
      GEO_TYPE_LINESTRING => 'linestring',
      GEO_TYPE_POLYGON => 'polygon',
      GEO_TYPE_COLLECTION => 'collection',
      GEO_TYPE_MULTIPOINT => 'multipoint',
      GEO_TYPE_MULTILINESTRING => 'multilinestring',
      GEO_TYPE_MULTIPOLYGON => 'multipolygon',
    );
    if (!is_null($value)) {
      if (is_numeric($value)) {
        return $types[$value];
      }
      return array_search($value, $types);
    }
    return $types;
  }

  function geoCompatible($g) {
    return (get_class($g) == get_class($this));
  }

  private function error($message) {
    drupal_set_message($message);
    return FALSE;
  }

  private function unimplemented() {
    return $this->error(t('Attempt to call an unimplemented function'));
  }

  public function point($x = NULL, $y = NULL) {
    $g = new GeometryPoint($x, $y);
    $g->setHandler($this);
    return $g;
  }

  public function linestring() {
    $g = New GeometryLinestring();
    $g->setHandler($this);
    $geometries = func_get_args();
    foreach ($geometries as $point) {
      $g->addPoint($point);
    }
    return $g;
  }

  public function polygon() {
    $g = New GeometryPolygon();
    $g->setHandler($this);
    $geometries = func_get_args();
    foreach ($geometries as $linestring) {
      $g->addGeometry($linestring);
    }
    return $g;
  }

  public function multiPoint() {
    $g = New GeometryLinestring();
    $g->setHandler($this);
    $geometries = func_get_args();
    foreach ($geometries as $point) {
      $g->addPoint($point);
    }
    return $g;
  }

  public function multiLinestring() {
    $g = New GeometryMultiLinestring();
    $g->setHandler($this);
    $geometries = func_get_args();
    foreach ($geometries as $linestring) {
      $g->addGeometry($linestring);
    }
    return $g;
  }

  public function multiPolygon() {
    $g = New GeometryMultiPolygon();
    $g->setHandler($this);
    $geometries = func_get_args();
    foreach ($geometries as $polygon) {
      $g->addGeometry($polygon);
    }
    return $g;
  }

  public function geometryCollection() {
    $g = New GeometryCollection();
    $g->setHandler($this);
    $geometries = func_get_args();
    foreach ($geometries as $geom) {
      $g->addGeometry($geom);
    }
    return $g;
  }

  // Implementation-specfic functions.
  public function addPoint($g, $point, $position = NULL) {
    $g->points[] = $point;
  }

  public function geomFromText($wkt, $srid = GEO_SRID_DEFAULT, $type = NULL) {
    preg_match('/^\s*(\D+)\W/Ui', $wkt, $m);
    if (defined('GEO_TYPE_' . ($name = strtoupper($m[1])))) {
      $geo_type = constant('GEO_TYPE_' . $name);
    }

    // Can't determine the geo_type from this WKT string.
    else {
      return $this->error(t('Invalid text'));
    }

    // A specific geometry type is being requested, but it's not in the WKT.
    if (isset($type) && $type != $geo_type) {
      return $this->error(t('Incompatible geometry.'));
    }

    // By default, we're assuming that we'll get a 2D object.
    $dimensions = 2;

    // TODO If there's a Z or an M modifier, we have a 3D or a 4D object.

    // Create an appropriate geometry instance and set its WKT.
    if ($class = GeoSimple::GeometryTypes($geo_type)) {
      $g = $this->$class();
      $g->setWKT($wkt);
      return $g;
    }
    else {
      return $this->error(t('Incompatible geometry'));
    }
  }

  public function pointFromText($wkt, $srid = GEO_SRID_DEFAULT) {
    return $this->geomFromText($wkt, $srid, GEO_TYPE_POINT);
  }

  public function lineFromText($wkt, $srid = GEO_SRID_DEFAULT) {
    return $this->geomFromText($wkt, $srid, GEO_TYPE_LINESTRING);
  }

  public function polyfromText($wkt, $srid = GEO_SRID_DEFAULT) {
    return $this->geomFromText($wkt, $srid, GEO_TYPE_POLYGON);
  }

  public function mPointFromText($wkt, $srid = GEO_SRID_DEFAULT) {
    return $this->geomFromText($wkt, $srid, GEO_TYPE_MULTIPOINT);
  }

  public function mLineFromText($wkt, $srid = GEO_SRID_DEFAULT) {
    return $this->geomFromText($wkt, $srid, GEO_TYPE_MULTILINESTRING);
  }

  public function mPolyFromText($wkt, $srid = GEO_SRID_DEFAULT) {
    return $this->geomFromText($wkt, $srid, GEO_TYPE_MULTIPOLYGON);
  }

  public function geomCollFromText($wkt, $srid = GEO_SRID_DEFAULT) {
    return $this->geomFromText($wkt, $srid, GEO_TYPE_COLLECTION);
  }

  public function bdPolyFromText($wkt, $srid = GEO_SRID_DEFAULT) {
    return $this->geomFromText($wkt, $srid);
  }

  public function bdMPolyFromText($wkt, $srid = GEO_SRID_DEFAULT) {
    return $this->geomFromText($wkt, $srid);
  }

  public function geomFromWKB($wkb, $srid = GEO_SRID_DEFAULT, $type = NULL) {
    // Fetch the byte order (0 = Big Endian, 1 = Little Endian) and geo type.
    $order = current(unpack('c', substr($wkb, 0, 1)));
    $geo_type = current(unpack(($order ? 'V' : 'N'), substr($wkb, 1, 4)));
    $geo_type = array_search($geo_type, GeoSimple::wkbTypes());

    // A specific geometry type is being requested, but it's not in the WKT.
    if (isset($type) && $type != $geo_type) {
      return $this->error(t('Incompatible geometry.'));
    }

    // By default, we're assuming that we'll get a 2D object.
    $dimensions = 2;

    // TODO If there's a Z or an M modifier, we have a 3D or a 4D object.

    // Create an appropriate geometry instance and set its WKT.
    if ($method = GeoSimple::GeometryTypes($geo_type)) {
      $g = GeoSimple::$method();
      $g->setWKB($wkb);
      return $g;
    }
    else {
      return $this->error(t('Incompatible geometry'));
    }
  }

  public function pointFromWKB($wkb, $srid = GEO_SRID_DEFAULT) {
    return $this->geomFromWKB($wkb, $srid, GEO_TYPE_POINT);
  }

  public function lineFromWKB($wkb, $srid = GEO_SRID_DEFAULT) {
    return $this->geomFromWKB($wkb, $srid, GEO_TYPE_LINESTRING);
  }

  public function polyFromWKB($wkb, $srid = GEO_SRID_DEFAULT) {
    return $this->geomFromWKB($wkb, $srid, GEO_TYPE_POLYGON);
  }

  public function mPointFromWKB($wkb, $srid = GEO_SRID_DEFAULT) {
    return $this->geomFromWKB($wkb, $srid, GEO_TYPE_MULTIPOINT);
  }

  public function mLineFromWKB($wkb, $srid = GEO_SRID_DEFAULT) {
    return $this->geomFromWKB($wkb, $srid, GEO_TYPE_MULTILINESTRING);
  }

  public function mPolyFromWKB($wkb, $srid = GEO_SRID_DEFAULT) {
    return $this->geomFromWKB($wkb, $srid, GEO_TYPE_MULTIPOLYGON);
  }

  public function geomCollFromWKB($wkb, $srid = GEO_SRID_DEFAULT) {
    return $this->geomFromWKB($wkb, $srid, GEO_TYPE_COLLECTION);
  }

  public function bdPolyFromWKB($wkb, $srid = GEO_SRID_DEFAULT) {
    return $this->geomFromWKB($wkb, $srid);
  }

  public function bdMPolyFromWKB($wkb, $srid = GEO_SRID_DEFAULT) {
    return $this->geomFromWKB($wkb, $srid);
  }

  public function asText($g) {
    return GeoSimple::parseWKB($g, 'wkt');
  }

  public function asBinary($g) {
    if (isset($g->wkb)) {
      return $g->wkb;
    }

    // Make sure the geometry is fully-formed.
    GeoSimple::parseWKT($g);

    // Some common header values for WKB.
    $wkb_types = GeoSimple::wkbTypes();
    $g->wkb = pack('cL', GeoSimple::byteOrder(), $wkb_types[$g->geoType()]);

    // Recursively call the binaryData function to build out the rest.
    $g->wkb .= GeoSimple::binaryData($g);

    return $g->wkb;
  }

  private function binaryData($g) {
    switch ($g->geometryType()) {

      case 'point':
        return pack('dd', $g->X(), $g->Y());

      case 'linestring':
      case 'multipoint':
        $wkb = pack('L', $g->numPoints());
        foreach ($g->points as $point) {
          $wkb .= GeoSimple::binaryData($point);
        }
        return $wkb;

      case 'polygon':
      case 'multilinestring':
        $wkb = pack('L', count($g->rings));
        foreach ($g->rings as $line) {
          $wkb .= GeoSimple::binaryData($line);
        }
        return $wkb;

      case 'multipolygon':
      case 'collection':
        $wkb = pack('L', count($g->geometries));
        foreach ($g->geometries as $g) {
          $wkb .= GeoSimple::binaryData($g);
        }
        return $wkb;
    }
  }

  public function dimension($g) {
    return $this->unimplemented();
  }

  public function geometryType($g = NULL) {
    if (is_null($g)) {
      $g = $this;
    }
    return GeoSimple::geometryTypes($g->geoType());
  }

  public function SRID($g = NULL) {
    if (isset($g->srid)) {
      return $g->srid;
    }
    return GEO_UNKNOWN;
  }

  public function isEmpty($g) {
    switch ($this->geometryType($g)) {

      case 'point':
        return (int) ($g->x() == NULL || $g->y() == NULL);

      case 'curve':
      case 'linestring':
        return (int) (count($g->points) == 0);

      case 'surface':
      case 'polygon':
        return (int) (count($g->rings) == 0); // TODO

      case 'collection':
      case 'multipoint':
      case 'multicurve':
      case 'multilinestring':
      case 'multisurface':
      case 'multipolygon':
        return (int) (count($g->geometries) == 0); // TODO

      default:
        return GEO_UNKNOWN;
    }
  }

  public function isSimple($g) {
    return $this->unimplemented();
  }

  public function boundary($g) {
    return $this->unimplemented();
  }

  public function envelope($g) {
    // TODO this was copped from geo.inc and may not be fully functional.
    $x = $g->x;
    $y = $g->y;

    $offset_x = $limit / (GEO_DEGREE_M * cos($y));
    $offset_y = $limit / GEO_DEGREE_M;
    $offset_x = $limit / GEO_DEGREE_M; // TODO Miles
    $offset_y = $limit / round(GEO_DEGREE_M * cos($x / 92215));

    // Create a linestring that represents a closed box around the envelope.
    $line = $this->linestring();
    $line->addPoint($x - $offset_x, $y + $offset_y);
    $line->addPoint($x + $offset_x, $y + $offset_y);
    $line->addPoint($x + $offset_x, $y - $offset_y);
    $line->addPoint($x - $offset_x, $y - $offset_y);
    $line->addPoint($x - $offset_x, $y + $offset_y);

    // Return a polygon constructed from the envelope linestring.
    return $this->polygon($line);
  }

  public function x($point) {
    GeoSimple::parseGeometry($point);
    return (float) $point->x;
  }

  public function y($point) {
    GeoSimple::parseGeometry($point);
    return (float) $point->y;
  }

  public function startPoint($curve) {
    GeoSimple::parseGeometry($curve);
    return $curve->points[0];
  }

  public function endPoint($curve) {
    GeoSimple::parseGeometry($curve);
    return end($curve->points);
  }

  public function isClosed($curve) {
    return ($this->startPoint($curve) == $this->endPoint($curve));
  }

  public function isRing($curve) {
    return ($this->isClosed($curve) && $this->isSimple($curve));
  }

  public function length($curve) {
    GeoSimple::parseGeometry($curve);
    return $this->unimplemented();
  }

  public function numPoints($linestring) {
    GeoSimple::parseGeometry($linestring);
    if (isset($linestring->points)) {
      return count($linestring->points);
    }
    return 0;
  }

  public function pointN($linestring, $n) {
    GeoSimple::parseGeometry($linestring);
    if (isset($linestring->points)) {
      return $linestring->points[$n - 1];
    }
  }

  public function centroid($surface) {
    return $this->unimplemented();
  }

  public function pointOnSurface($surface) {
    return $this->unimplemented();
  }

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

  public function numGeometries($collection) {
    GeoSimple::parseGeometry($collection);
    return count($collection->geometries);
  }

  public function geometryN($collection, $n) {
    GeoSimple::parseGeometry($collection);
    if (isset($collection->geometries)) {
      return $collection->geometries[$n - 1];
    }
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

  // Not part of the spec, but too handy to pass up.
  function simplify($g, $tolerance = 10) { }

  // Implementation-specfic functions: useful primarily to this application.
  function setGID($value = NULL) {
    $this->gid = (int) $value;
  }

  public function gid() {
    return NULL;
  }

  function setName($value) {
    $this->name = preg_replace('/[^a-z0-9_]/', '', strtolower($value));
  }

  function name() {
    return $this->name;
  }

  function setTitle($value) {
    $this->title = filter_xss($value);
  }

  public function title() {
    return NULL;
  }

  function setSRID($value = NULL) {
    if (is_int($value)) {
      $this->srid = $value;
    }
    else {
      $this->srid = GEO_SRID_DEFAULT;
    }
  }

  function setGeoType($value = NULL) {
    if (is_int($value)) {
      $this->geo_type = $value;
    }
    else {
      $this->geo_type = GEO_UNKNOWN;
    }
  }

  function geoType() {
    return $this->geo_type;
  }

  public function vars() {
    return get_object_vars($this);
  }

  /*
   * A helper function that parses WKT or WKB into valid members of a geometry
   * instance.  For performance reasons, a geometry can be instantiated but not
   * parsed or expanded into its component parts.  This function is invoked
   * by accessor functions to ensure that the requested information is there
   * when you need it.
   */
  public function parseGeometry($g) {
    // Already done.
    if ($g->parsed) {
      return $g;
    }

    if (!isset($g->wkb)) {
      if (isset($g->wkt)) {
        // Convert the well-known text to an object.
        GeoSimple::parseWKT($g);
      }

      // Convert the object to WKB.
      $g->asBinary();
    }
    if (isset($g->wkb)) {
      return GeoSimple::parseWKB($g);
    }
    return GeoSimple::error(t('Unable to parse geometry'));
  }

  private function wkbTypes() {
    return array(
      GEO_TYPE_POINT => 1,
      GEO_TYPE_LINESTRING => 2,
      GEO_TYPE_POLYGON => 3,
      GEO_TYPE_MULTIPOINT => 4,
      GEO_TYPE_MULTILINESTRING => 5,
      GEO_TYPE_MULTIPOLYGON => 6,
      GEO_TYPE_COLLECTION => 7,
    );
  }

  public function byteOrder() {
    return current(unpack('c', pack('S', 1)));
  }

  public function parseWKT($g, $wkt = NULL) {
    if (!isset($wkt)) {
      $wkt = $g->wkt;
    }

    // Already done.
    if ($g->parsed) {
      return;
    }

    // The WKT for a point will be short and sweet.  Parse it and quit.
    if ($g->geometryType() == 'point') {
      GeoSimple::parseWKTPointData($wkt, $g);
      $g->parsed = TRUE;
      return $g;
    }

    else {
      // Put our WKT data into memory so we can cruise around with fread.
      $fp = fopen("php://memory", 'r+');
      $length = strlen($wkt);
      fwrite($fp, $wkt);

      // Seek to the initial '('.
      fseek($fp, strlen($g->geometryType()) + 1);

      $text = '';
      while (!$done && (ftell($fp) < $length)) {
        $c = fread($fp, 1);
        $text .= $c;

        if ($c == ',' || $c == ')' || $c == '(') {
          switch ($g->geometryType()) {

            // A simple collection of points, e.g. LINESTRING(1 2, 3 4).
            case 'linestring':
            case 'multipoint':
              $g->addPoint(GeoSimple::parseWKTPointData($text));
              $text = '';

              if ($c == ')') {
                $done = TRUE;
              }
              break;

              // A nested collection of points, e.g. POLYGON((1 2, 3 4)).
            case 'polygon':
            case 'multilinestring':
              if (strlen($text) > 1 && isset($line)) {
                $line->addPoint(GeoSimple::parseWKTPointData($text));
                $text = '';
              }
              if ($c == ')' || $c == '(') {
                if ($c == ')' && isset($line) && count($line->points)) {
                  $g->addGeometry($line);
                }
                $line = GeoSimple::linestring();
              }
              if ($text == ')') {
                $done = TRUE;
              }

              break;

              // A nested collection of complex, e.g. MULTIPOLYGON(((1 2, 3 4))).
            case 'multipolygon': // TODO
              break;

              // A nested collection of WKT strings, e.g.
              // GEOMETRYCOLLECTION(POINT(4 6),LINESTRING(4 6,7 10))
            case 'collection': // TODO
              break;
          }
        }
      }
      fclose($fp);

      $g->parsed = TRUE;
      return $g;
    }
  }

  private function parseWKTPointData($text, $point = NULL) {
    if (preg_match_all('/[0-9.]+/', $text, $matches)) {
      list($x, $y, $z) = $matches[0];
      if ($point) {
        $point->setX($x);
        $point->setY($y);
        $point->setZ($z);
      }
      else {
        $point = GeoSimple::point($x, $y, $z);
      }
      $point->parsed = TRUE;
      return $point;
    }
  }

  /**
   * Use our WKB parser to return WKT, Arrays, etc.
   * See geo.wkb.inc for more details.
   * TODO it's likely that geo.wkb.inc will be deprecated and its functionality
   * moved here.
   */
  public function parseWKB($g, $format = NULL) {
    $format = strtolower($format);
    if (isset($g->$format)) {
      return $g->$format;
    }
    $result = geo_wkb_get_data($g->asBinary(), $format);
    return $g->$format = $result['value'];
  }
}
