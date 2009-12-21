<?php // $Id$

class views_handler_filter_geo extends views_handler_filter_numeric {

  var $no_single = TRUE;

  function init(&$view, $options) {
    parent::init($view, $options);
    $this->geo = geo_load(array('gid' => $this->definition['geo_gid']));
  }

  function option_definition() {
    $options = parent::option_definition();
    $options['operator'] = array('default' => '<=');
    $options['function'] = array('default' => 'distance');
    $options['input'] = array('default' => 'geo_latlon');

    $options['value'] = array(
      'contains' => array(
        'min' => array('default' => ''),
        'max' => array('default' => ''),
        'value' => array('default' => ''),
        'units' => array('default' => 'mi'),
        'target' => array('default' => array()),
      ),
    );
    $options['target'] = array('default' => array());
    return $options;
  }

  function value_form(&$form, &$form_state) {
    $exposed = !empty($form_state['exposed']);

    // Use the default value_form from the numeric handler.
    parent::value_form($form, $form_state);

    // Add "units"
    $form['value']['units'] = array(
      '#type' => 'select',
      '#title' => t('Units'),
      '#options' => geo_units(),
      '#default_value' => $this->value['units'],
    );
  }

  function options_form(&$form, &$form_state) {
    $group = 'filter';

    $functions = $this->geo->functionNames($group);
    $form['function'] = array(
      '#type' => 'radios',
      '#title' => t($group .' function to use'),
      '#default_value' => $this->options['function'],
      '#options' => $functions,
    );

    // Ensure that these inputs get appropriate weights.

    $form['op_val_start'] = array();
    $form['operator'] = array();
    $form['value'] = array();
    $form['op_val_end'] = array();
    $form['target_prefix'] = array();
    $form['input'] = array();
    $form['target'] = array();
    $form['target_suffix'] = array();

    parent::options_form($form, $form_state);

    // Special-case distance filters, which have additional inputs.
    $dst_functions = $num_functions = $target_functions = array();
    foreach ($this->geo->functionNames('filter') as $name => $info) {
      $info = $this->geo->functionInfo($name);
      if (substr($name, 0, 8) == 'distance') {
        $dst_functions[] = $name;
      }
      if ($info['returns'] == 'float') {
        $num_functions[] = $name;
      }
      if ($info['requires target']) {
        $target_functions[] = $name;
      }
    }

    // Hide the value form if
    $form['op_val_start']['#type'] = 'hidden';
    $form['op_val_start']['#prefix'] = '<div>';
    $form['op_val_start']['#process'] = array('views_process_dependency');
    $form['op_val_start']['#dependency'] = array('radio:options[function]' => $num_functions );
    $form['op_val_end']['#suffix'] = '</div><br clear="all" />';

    // The value form contained units.  Hide these if N/A.
    $form['value']['units']['#process'] = array('views_process_dependency');
    $form['value']['units']['#dependency'] = array('radio:options[function]' => $dst_functions );

    $form['target_prefix'] = array(
      '#type' => 'hidden',
      '#id' => 'geo-target-options',
      '#prefix' => '<div><fieldset id="geo-target-options"><legend>' . t('Provide a target for this function') . '</legend>',
      '#process' => array('views_process_dependency'),
      '#dependency' => array('radio:options[function]' => $target_functions),
    );

    // If applicable, add the target value.
    $form['input'] = array(
      '#type' => 'select',
      '#title' => 'Widget',
      '#default_value' => $this->options['input'],
      '#process' => array('views_process_dependency'),
      '#dependency' => array('radio:options[function]' => $target_functions),
    );

    $form['target'] = array('#tree' => TRUE);

    foreach (geo_inputs() as $name => $info) {
      $element = $info['element'];

      $form['input']['#options'][$name] = $info['label'];

      $element = array_merge($element, array(
        '#title' => $this->options['label'],
        '#default_value' => $this->options['target'][$name],
        '#gis_type' => $this->geo->geometryType(),
        '#gis_output' => 'wkt', // The format expected by this handler.
      ));

      $element['#process'][] = 'views_process_dependency';
      $element['#dependency'] = array('edit-options-input' => array($name));

      $form['target'][$name] = $element;
    }
    $form['target_suffix'] = array(
      '#type' => 'markup',
      '#value' => '</fieldset></div>',
    );
  }

  function options_submit(&$form, &$form_state) {
    parent::options_submit($form, $form_state);

    // Set the value
    $input = $form_state['values']['options']['input'];
    $value = $form_state['values']['options']['target'][$input];
    $form_state['values']['options']['value']['target'] = $value;
  }

  function expose_form_right(&$form, &$form_state) {
    parent::expose_form_right($form, $form_state);

    $form['expose']['show_value'] = array(
      '#type' => 'select',
      '#title' => t('Show value as'),
      '#options' => array(
        'none' => t('Do not allow user to change the value'),
        'select' => t('A list of predefined values'),
        'textfield' => t('A freeform text field'),
      ),
      '#description' => t('Expose the length value for distance and length queries.'),
      '#default_value' => $this->options['expose']['show_value'],
    );
    $form['expose']['value_options'] = array(
      '#type' => 'textfield',
      '#title' => t('Value options'),
      '#description' => t('The list of values a user can select from, separated by commas.'),
      '#process' => array('views_process_dependency'),
      '#dependency' => array('edit-options-expose-show-value' => array('select')),
      '#default_value' => $this->options['expose']['value_options'],
    );

    $form['expose']['show_units'] = array(
      '#type' => 'checkbox',
      '#title' => t('Show units'),
      '#description' => t('Allow user to select units of measurement for distance and length queries.'),
      '#default_value' => $this->options['expose']['show_units'],
    );
  }

  function exposed_form(&$form, &$form_state) {
    $key = $this->options['expose']['identifier'];

    // We need to pretend the user already input the defaults.
    if (!isset($form_state['input'][$key])) {
      $form_state['input'][$key] = $this->value;
    }

    $form[$key] = array( '#tree' => TRUE );

    if ($this->options['expose']['show_value'] == 'none') {
      $form[$key]['value'] = array(
        '#type' => 'value',
        '#value' => $this->value['value'],
      );
    }
    else {
      // Use the default value_form from the numeric handler.
      parent::value_form($form[$key], $form_state);

      if ($this->options['expose']['show_value'] == 'select') {
        $options = array();
        foreach (explode(',', $this->options['expose']['value_options']) as $op) {
          $op = trim($op);
          $options[$op] = $op;
        }

        foreach (element_children($form[$key]) as $item) {
          $current = $form_state['input'][$key][$item];
          if (!isset($options[$current])) {
            $form_state['input'][$key][$item] = NULL;
          }
          $form[$key][$item] = array(
            '#type' => 'select',
            '#multiple' => FALSE,
            '#options' => $options,
            '#default_value' => isset($options[$current]) ? $current : NULL,
          );
          // TODO default value.
        }
      }
    }

    // Set units.
    if ($this->options['expose']['show_units']) {
      $form[$key]['units'] = array(
        '#type' => 'select',
        '#options' => geo_units(),
        '#default_value' => $this->value['units'],
      );
    }
    else {
      $form[$key]['units'] = array(
        '#type' => 'value',
        '#value' => $this->value['units'],
      );
    }

    // Set input.
    $element = geo_input_element_info($this->options['input']);
    $element['#default_value'] = $this->value['target'];

    $form[$key]['target'] = $element;

    $form[$key]['#attributes'] = array(
    );
  }

  function query() {

    // The GIS function we'll be filtering on.
    $function = $this->options['function'];

    // Figure out whether or not we need a target value to compare against.
    $target = NULL;

    $info = $this->geo->functionInfo($function);

    if ($info['requires target']) {
      $input = geo_input_info($this->options['input']);
      $value = $this->value['target'];

      // TODO this is not how we should be handling this.
      if ($func = $input['gis callback']) {
        $target = $func($input, $value, 'wkt:'. $this->geo->geometryType());
      }
      else {
        $target = geo_value($value, 'wkt', 'array'); // TODO assumption of array
      }

      // Prevent an erroneous query if there's no target to
      if (!$target) return;
    }

    $table = $this->ensure_my_table();
    $this->field_alias = $this->field .'_'. $function;

    // Ensure that the query acts on the table and column aliases defined here.
    $this->geo->setTableName($table);
    $this->geo->setColumnName($this->field);

    // Add the field, using the Geo API method for getting it.
    $query = $this->geo->$function($this->geo, $target);

    if ($info['returns'] == 'boolean') {
      $this->query->add_where(0, $query);
    }

    // Adopt the behavior of the views_filter_numeric parent class.
    if ($info['returns'] == 'float') {
   		// TODO Add a bounding box filter to leverage the geo index for performance

      // Convert the limiting values to meters.
      if ($units = $this->value['units']) {
        foreach (array('value', 'min', 'max') as $key) {
          $this->value[$key] = (int) geo_unit_convert($this->value[$key], $units, 'm');
        }
      }

      // Call the limiting function, just like the numeric handler would have.
      $info = $this->operators();
      if (!empty($info[$this->operator]['method'])) {
        $this->{$info[$this->operator]['method']}($query);
      }
    }
  }
}
