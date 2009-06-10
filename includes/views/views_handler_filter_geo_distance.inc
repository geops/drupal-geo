<?php //$Id$

class views_handler_filter_geo_distance extends views_handler_filter {

  function option_definition() {
    $options = parent::option_definition();
    $options['gis_input'] = array('default' => 'geo_wkt');
    $options['gis_input_value'] = array('default' => '');
    return $options;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['gis_input'] = geo('gis_input_selector', $this->options['gis_input']);
  }

  function value_form(&$form, &$form_state, $key = 'value') {
    $form[$key] = array('#tree' => TRUE);
    $form[$key]['geo_distance'] = array(
      '#type' => 'textfield',
      '#title' => t('Distance'),
      '#default_value' => $this->value['geo_distance'],
      '#size' => 5,
    );

    $form[$key]['geo_units'] = array(
      '#type' => 'select',
      '#title' => t('Units'),
      '#options' => geo_units(),
      '#default_value' => $this->value['geo_units'],
    );

    $info = module_invoke_all('gis_input_info');
    $info = $info[$this->options['gis_input']];
    $form[$key]['target'] = array_merge($info['element'], array(
      '#title' => $this->options['label'],
      '#default_value' => $this->value['gis_input_value'],
      '#gis_type' => 'point', // TODO derived.
      '#weight' => 1,
    ));

    // Create an additional field to store the input as it was submitted.
    $form[$key]['gis_input_value'] = $form['value']['target'];
    $form[$key]['gis_input_value']['#access'] = FALSE;
  }

  function value_validate($form, &$form_state, $key = 'value') {
    parent::value_validate($form, $form_state);
    form_set_value($form[$key]['gis_input_value'], $form[$key]['target']['#gis_input_value'], $form_state);
  }

  function exposed_form(&$form, &$form_state) {
    $key = $this->options['expose']['identifier'];
    $this->value_form($form, $form_state, $key);
  }

  function exposed_validate($form, &$form_state) {
    $key = $this->options['expose']['identifier'];
    $this->value_validate($form, $form_state, $key);
  }

  function accept_exposed_input(&$input) {
    $key = $this->options['expose']['identifier'];
    $value = $input[$key];

    // Ensure that the entered value is stored in the user's session.
    $input[$key]['target'] = $input[$key]['gis_input_value'];

    if ($value['geo_distance'] && $value['geo_units'] && $value['target']) {
      $this->value = $value;
      return TRUE;
    }
    return FALSE;
  }

  function query() {
    $value  = $this->value;
    $limit  = $value['geo_distance'];
    $units  = $value['geo_units'];
    $target = $value['target'];

		// If there's not already a field with this calculation, add one.
    if (!isset($this->view->field[$this->field])) {
      $alias = $this->ensure_my_table() .'_'. $this->field;
      $func = geo('query_distance', $this->real_field, NULL, $target);
      $this->query->add_field('', $func, $alias, array('aggregate' => TRUE));
    }

    // Set the distance limit to meters.
    $limit = (int) geo_unit_convert($limit, $units, 'm');

    // Add a bounding box filter to leverage the geo index for performance.
    $bbox = geo('make_bbox', $target, $limit);
    $func = geo('query_within', $this->real_field, GEO_SRID_DEFAULT, $bbox);
    $this->query->add_where('', $func);

    // Add a distance filter to limit on a circular radius.
    if (!$field = $this->field_alias) {
      $table  = $this->ensure_my_table();
      $field = $table .'_' . $this->field;
    }
    $this->query->add_having(0, "$field < %d", $limit);
  }
}
