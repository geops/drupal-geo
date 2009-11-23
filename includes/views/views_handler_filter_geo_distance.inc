<?php //$Id$

class views_handler_filter_geo_distance extends views_handler_filter {

  function option_definition() {
    $options = parent::option_definition();
    $options['gis_input'] = array('default' => 'geo_latlon');
    $options['gis_input_value'] = array('default' => array());
    return $options;
  }

  function value_form(&$form, &$form_state) {
    $exposed = $form_state['exposed'];
    $key = $exposed ? $this->options['expose']['identifier'] : 'value';

    // TODO handle this based on exposdieness - or delete it outright?
    if ($exposed && empty($form_state['input'])) {
      $form_state['input'][$key] = $this->value;
    }

    if (!$exposed) {
      $form['gis_input'] = geo('gis_input_selector', $this->options['gis_input']);
    }

    $form['value'] = array('#tree' => TRUE);

    $form['value']['geo_distance'] = array(
      '#type' => 'textfield',
      '#title' => t('Distance'),
      '#default_value' => $this->value['geo_distance'],
      '#size' => 5,
      '#access' => !$exposed || $this->options['show_geo_distance'],
      '#process' => array('views_process_dependency'),
      '#dependency' => array('edit-options-value-gis-input'),
    );

    $form['value']['geo_units'] = array(
      '#type' => 'select',
      '#title' => t('Units'),
      '#options' => geo_units(),
      '#default_value' => $this->value['geo_units'],
      '#access' => !$exposed || $this->options['show_geo_units'],
      '#process' => array('views_process_dependency'),
      '#dependency' => array('edit-options-value-gis-input'),
    );
  
    // The "target" value gets converted to the value that is actually used.
    // The gis_input_value field stores the input as it was entered.
    $form['value']['target'] = array('#tree' => TRUE);

    foreach (module_invoke_all('gis_input_info') as $name => $info) {
      // The exposed form only shows one input.
      if ($exposed && ($name != $this->options['gis_input'])) continue;

      // Ignore nonexistent element types.
      if (!$element = _element_info($info['element']['#type'])) continue;
      //unset($element['#parents'], $element['#tree']);
      unset($element['#parents']);

      //$value = is_array($this->value['gis_input_value']) ? $this->value['gis_input_value'][$name] : NULL;
      $value = is_array($this->value['target']) ? $this->value['target'][$name] : NULL;

      $element = array_merge($element, $info['element']);
      $element = array_merge($element, array(
        '#title' => $this->options['label'],
        '#default_value' => $value,
        '#gis_type' => 'point', // TODO derived.
        '#gis_input' => $info['gis input'], // The format expected by the element.
        '#gis_output' => 'array', // The format expected by this handler.
     //   '#access' => (!$exposed || ($name == $this->options['gis_input'])),
      ));

      if (!$exposed) {
        $element['#process'][] = 'views_process_dependency';
        $element['#dependency'] = array('edit-options-gis-input' => array($name));
      }

      // Add this element to the form.
      $element['#tree'] = TRUE;
      $form['value']['target'][$name] = $element;
    }
  }

  function accept_exposed_input(&$input) {
    if (empty($this->options['exposed'])) {
      return TRUE;
    }

    $key = $this->options['expose']['identifier'];
    $value = $input[$key];

    // Ensure that the entered value is stored in the user's session.
    //$input[$key]['target'] = $input[$key]['gis_input_value'];

    if (!$this->_value_is_empty($value['target'][$this->options['gis_input']])) {
      $this->value = $value;
      return TRUE;
    }
    elseif (!$this->_value_is_empty($this->value['target'][$this->options['gis_input']])) {
      return TRUE;
    }

    // TODO really?
    return FALSE;
  }

  function admin_summary() {
    $out = '';
    if ($this->value) {
      $out = (int) $this->value['geo_distance'];
      $out .= ' '. geo_units($this->value['geo_units']);
      $out .= ' '. t('from');
      $out .= ' '. $this->value['target'][$this->options['gis_input']];
    }
    return $out;
  }

  function _value_is_empty($value) {
    // TODO this is not very flexible...
    if (is_scalar($value)) return empty($value);
    if (is_array($value)) {
      $val = end($value);
      return empty($val);
    }
  }

  function query() {
    $value  = $this->value;

    $limit  = $value['geo_distance'];
    $units  = $value['geo_units'];
    if ($value['target']) {
      $target = $value['target'][$this->options['gis_input']];
    }
    else return;

    foreach (module_invoke_all('gis_input_info') as $name => $info) {
      if ($name == $this->options['gis_input']) {
        // TODO the output format (array) should not be hard-coded.
        $target = geo_value_convert($target, $info['gis input'], 'array');
        break;
      }
    }

    // Set the distance limit to meters.
    $limit = (int) geo_unit_convert($limit, $units, 'm');

    $srid = GEO_SRID_DEFAULT;
		// If there's not already a field with this calculation, add one.
    if (!isset($this->view->field[$this->field])) {
      $alias = $this->ensure_my_table() .'_'. $this->field;
      $dst = $func = geo('query_distance', $this->real_field, $srid, $target);
      //$this->query->add_field('', $func, $alias, array('aggregate' => TRUE));
      $this->query->add_where('', "$func < $limit");
    }

    // Add a bounding box filter to leverage the geo index for performance.
    $bbox = geo('make_bbox', $target, $limit);
    $func = geo('query_within', $this->real_field, $srid, $bbox);
    $this->query->add_where('', $func);

    // Add a distance filter to limit on a circular radius.
    if (!$field = $this->field_alias) {
      $table  = $this->ensure_my_table();
      $field = $table .'_' . $this->field;
    }
    //$this->query->add_having(0, "$field < %d", $limit);
    $this->query->add_having(0, "$dst < %d", $limit);
  }
}
