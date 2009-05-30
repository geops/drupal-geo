<?php // $Id$

class views_handler_filter_geo extends views_handler_filter {

  var $no_single = TRUE;

  function option_definition() {
    $options = parent::option_definition();
    $options['expose']['contains']['geo_widget'] = array('default' => '');
    return $options;
  }

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

  function expose_form_left(&$form, &$form_state) {
    // Get initial form from the views_handler_filter class.
    parent::expose_form_left($form, $form_state);

    // Provide a selection of input handlers.
    $input_types = array();
    foreach (_content_widget_types() as $name => $info) {
      if (in_array('geo', $info['field types'])) $input_types[$name] = $info;
    }
    // TODO this is proof of concept and not actually working.
    $form['expose']['geo_widget'] = array(
      '#type' => 'select',
      '#title' => 'Widget',
      '#options' => $input_types;
      '#default_value' => $this->options['expose']['geo_widget'],
    );
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

    if ($geo_table = $this->definition['geo_table']) {
      $geo_table = $this->query->ensure_table($geo_table, $this->relationship);
      $field = $geo_table . '.geo';
    }

    foreach (geo('query_function', $func, $field, $value, $srid) as $sql) {
      $this->query->add_where(0, $sql);
    }
  }
}
