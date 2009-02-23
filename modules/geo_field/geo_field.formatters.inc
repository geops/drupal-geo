<?php // $Id$
/**
 * @file
 * Field formatters for geo_field fields.
 */

function theme_geo_formatter_default($element) {
  if (isset($element['#item']['wkt'])) return $element['#item']['wkt'];
}

function theme_geo_formatter_lat($element) {
  if (isset($element['#item']['lat'])) return $element['#item']['lat'];
  // TODO polygons, lines, etc won't have lat/lon but they'll have a centroid.
}

function theme_geo_formatter_lon($element) {
  if (isset($element['#item']['lon'])) return $element['#item']['lon'];
  // TODO polygons, lines, etc won't have lat/lon but they'll have a centroid.
}
