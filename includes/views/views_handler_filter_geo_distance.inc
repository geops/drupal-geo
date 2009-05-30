<?php //$Id$

class views_handler_filter_geo_distance extends views_handler_filter {
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
  }

  function query() {
    $target = $this->options['geo_target'];

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
