<?php //$Id$

class views_handler_field_geo extends views_handler_field {
  function options_form(&$form, &$form_state) {
    // TODO Implement settings for AsKML, AsSVG, AsText, etc.
    // By the way, those would only apply to postgis for now.
    $form['geo_format'] = array(
      '#type' => 'select',
      '#options' => geo('query_functions', ($this->definition['geo_type'])),
      '#default_value' => $this->options['geo_format'],
    );
  }

  function query() {
    $format = $this->options['geo_format'];

    // Add the geo formula as a field.
    $func = 'AsText('. geo('query_'. $format, $this->real_field). ')';
    $this->table_alias .= '_'. $format;
    $this->query->add_field(NULL, $func, $this->table_alias);
  }
}
