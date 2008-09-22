<?php // $Id$

class views_handler_filter_geo extends views_handler_filter {

  var $no_single = TRUE;

  function construct() {
    parent::construct();
    // If this was a CCK field, set the content_field definition.
      if (function_exists('content_fields')) {
        $this->content_field = content_fields($this->definition['content_field_name']);
        $this->additional_fields = $this->definition['additional fields'];
      }
  }

  function operator_options() {
    return geo('query_operators', $this->definition['geo_type']);
  }

  function exposed_form(&$form, &$form_state) {
    $key = $this->options['expose']['identifier'];
    $form[$key] = array('#type' => 'textfield', '#default_value' => $this->value,);
  }

  function query() {
    $table = $this->ensure_my_table();
    $func = $this->operator;
    $srid = GEO_SRID_DEFAULT;

    $value = db_escape_string($this->value);

    if (true) { // TODO decide if it's incoming wkt or a geo field.
      $value = "GeomFromText('$value', $srid)";
    }
    if ($geo_table = $this->definition['geo_table']) {
      $geo_table = $this->query->ensure_table($geo_table, $this->relationship);
      $field = $geo_table . '.geo';
    }
      
    // These relations should have overlapping bounding boxes.
    if (in_array($func, array('Equals', 'Within', 'Intersects'))) {
      //$index = "$value && $field AND "; // TODO NOT WORKING?
    }
    else { // Bounding boxes don't touch, we know can use OR use the function
      //$index = "NOT($value && $field) OR ";
    }
      
    $this->query->add_where(0, $index ."$func($value, $field)");
  }
}
