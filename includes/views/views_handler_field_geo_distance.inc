<?php //$Id$

class views_handler_field_geo_distance extends views_handler_field_numeric {

  function option_definition() {
    $options = parent::option_definition();
    $options['geo_target_type'] = array('default' => 'filter');
    $options['geo_units'] = array('default' => 'mi');
    return $options;
  }

  function options_form(&$form, &$form_state) {
    $target_types = array(
      'filter' => t('Use from filter'),
      'value' => t('Enter value'),
    );

    $form['geo_target_type'] = array(
      '#type' => 'select',
      '#title' => t('Target'),
      '#options' => $target_types,
      '#default_value' => $this->options['geo_target_type'],
    );

    $form['geo_target'] = array( '#tree' => TRUE );
    $form['geo_target']['lat'] = array(
      '#type' => 'textfield',
      '#title' => t('Latitude'),
      '#default_value' => $this->options['geo_target']['lat'],
    );
    $form['geo_target']['lon'] = array(
      '#type' => 'textfield',
      '#title' => t('Longitude'),
      '#default_value' => $this->options['geo_target']['lon'],
    );

    $form['geo_units'] = array(
      '#type' => 'select',
      '#title' => t('Units'),
      '#options' => geo_units(),
      '#default_value' => $this->options['geo_units'],
      '#weight' => 1,
    );
    parent::options_form($form, $form_state);
  }

  function query() {
    $this->field_alias = $alias = $this->ensure_my_table() .'_'. $this->field;

    switch ($this->options['geo_target_type']) {
      case 'filter':
        // Use the point defined in the corresponding filter definition.
        if (isset($this->view->filter[$this->field])) {
          $target = $this->view->filter[$this->field]->value['target'];
          if (!isset($target['type'])) {
            $target = current($target);
          }
        }
        break;

      case 'value':
        // Use the user-entered target.
        $target = $this->options['geo_target'];
        break;
    }

    // No target to measure distance from. Add a zero value so that the column
    // will exist for sorting and other purposes.
    if (!$target) {
      $this->query->add_field('', 0, $alias);
      $this->no_column = TRUE;
      return;
    }

    // Add the spatial distance function as a field.
    $func = geo('query_distance', $this->real_field, NULL, $target);
    $this->query->add_field('', $func, $alias, array('aggregate' => TRUE));
  }

  function render($values) {
    if (isset($this->no_column)) {
      unset($values->{$this->field_alias});
    }
    else {
      $value = $values->{$this->field_alias};

      // The equation was done in meters, so convert to the appropriate units.
      $value = geo_unit_convert($value, 'm', $this->options['geo_units']);
      $values->{$this->field_alias} = $value;
    }

    // Let the numeric handler do the rest.
    return parent::render($values);
  }
}
