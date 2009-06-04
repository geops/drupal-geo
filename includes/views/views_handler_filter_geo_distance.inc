<?php //$Id$

class views_handler_filter_geo_distance extends views_handler_filter {

  function option_definition() {
    $options = parent::option_definition();
    $options['expose']['contains']['gis_input'] = array('default' => '');
    return $options;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['geo_distance'] = array(
      '#type' => 'textfield',
      '#title' => t('Distance'),
      '#default_value' => $this->options['geo_distance'],
    );

    $form['geo_units'] = array(
      '#type' => 'select',
      '#title' => t('Units'),
      '#options' => geo_units(),
      '#default_value' => $this->options['geo_units'],
    );

    $form['value'] = array( '#tree' => TRUE );
    $form['value']['lat'] = array(
      '#type' => 'textfield',
      '#title' => t('Latitude'),
      '#default_value' => $this->options['geo_target']['lat'],
    );
    $form['value']['lon'] = array(
      '#type' => 'textfield',
      '#title' => t('Longitude'),
      '#default_value' => $this->options['geo_target']['lon'],
    );
  }

  function expose_form_left(&$form, &$form_state) {
    // Get initial form from the views_handler_filter class.
    parent::expose_form_left($form, $form_state);
    $form['expose']['gis_input'] = geo('gis_input_selector', $this->options['exp
ose']['gis_input']);
  }

  function exposed_form(&$form, &$form_state) {
    $info = module_invoke_all('gis_input_info');
    $info = $info[$this->options['expose']['gis_input']];
    $key = $this->options['expose']['identifier'];
    $form[$key] = array_merge($info['element'], array(
      '#title' => $info['title'],
      '#default_value' => $this->value,
      '#gis_type' => 'point', // TODO derived.
    ));
  }

  function query() {
    //$target = $this->options['geo_target'];
    $target = current($this->value);

		// If there's not already a field with this calculation, add one.
    if (!isset($this->view->field[$this->field])) {
      $alias = $this->ensure_my_table() .'_'. $this->field;
      $func = geo('query_distance', $this->real_field, NULL, $target);
      $this->query->add_field('', $func, $alias, array('aggregate' => TRUE));
    }

    // Set the distance limit to meters.
    $limit = $this->options['geo_distance'];
    $limit = (int) geo_unit_convert($limit, $this->options['geo_units'], 'm');

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
