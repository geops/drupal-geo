<?php // $Id$

class views_handler_filter_geo extends views_handler_filter_numeric {

  var $no_single = TRUE;

  function option_definition() {
    $options = parent::option_definition();
    $options['function'] = array('default' => 'distance');
    $options['target'] = array(
      'input' => array('default' => 'geo_latlon'),
      'value' => array('default' => array()),
    );
    $options['units'] = array('default' => 'mi');
    return $options;
  }

  function options_form(&$form, &$form_state) {
   parent::options_form($form, $form_state);

    // This helper function provides a function chooser and sets up targets.
    geo_views_target_form($form, $form_state, $this, 'filter');


//    $form['function']['#weight'] = 0;
//    $form['distance']['#weight'] = 2;

    // Special-case distance filters, which have additional inputs.
    $dst_functions = $num_functions = array();
    foreach ($this->geo->functionNames('filter') as $name => $info) {
      $info = $this->geo->functionInfo($name);
      if (substr($name, 0, 8) == 'distance') {
        $dst_functions[] = $name;
      }
      if ($info['returns'] == 'float') {
        $num_functions[] = $name;
      }
    }

    // Set weights on the elements returned by the numeric handler.
    $form['op_val_start']['#weight'] = 1;
    $form['operator']['#weight'] = 3;
    $form['value']['#weight'] = 4;
    $form['op_val_end']['#weight'] = 5;

    // Hide the numeric inputs from anything that doesn't return numeric values.
    $form['op_val_start']['#type'] = 'hidden';
    $form['op_val_start']['#id'] = 'value-options';
    $form['op_val_start']['#prefix'] = '<div>';
    $form['op_val_start']['#process'] = array('views_process_dependency');
    $form['op_val_start']['#dependency'] = array('radio:options[function]' => $num_functions);

    $form['op_val_end']['#suffix'] = '</div><br clear="all" />';

    // Units.
    $form['units'] = array(
      '#type' => 'select',
      '#title' => t('Units'),
      '#options' => geo_units(),
      '#default_value' => $this->options['units'],
      '#process' => array('views_process_dependency'),
      '#dependency' => array('radio:options[function]' => $num_functions ),
      '#weight' => 6,
    );

   // drupal_set_message('<pre>'.print_r($form, 1));

/*
    $form['distance']['distance'] = array(
      '#type' => 'textfield',
      '#title' => t('Distance'),
      '#default_value' => $this->options['distance']['distance'],
      '#size' => 5,
      '#process' => array('views_process_dependency'),
      '#dependency' => array('edit-options-function' => $dst_functions),
      '#prefix' => '<div class="container-inline">',
    );
    $form['distance']['units'] = array(
      '#type' => 'select',
      '#title' => t('Units'),
      '#options' => geo_units(),
      '#default_value' => $this->options['distance']['units'],
      '#process' => array('views_process_dependency'),
      '#dependency' => array('edit-options-function' => $dst_functions ),
      '#suffix' => '</div>',
    );
*/
  }

  function exposed_form(&$form, &$form_state) {
    parent::exposed_form($form, $form_state);

    $key = $this->options['expose']['identifier'];

    // Get the selected input element (e.g. latlon, geocoded textbox, etc.)
    $input = geo_input_element_info($this->options['target']['input']);

    if(substr($this->options['function'], 0, 8) == 'distance') {
      // TODO add distancy stuff here.
      $form[$key] = $input;
    }
    else {
      $form[$key] = $input;
    }
  }

  function query() {
    $this->geo = geo_load(array('gid' => $this->definition['geo_gid']));

    // The GIS function we'll be filtering on.
    $function = $this->options['function'];

    // Figure out whether or not we need a target value to compare against.
    $target = NULL;

    $info = $this->geo->functionInfo($function);
    if ($info['requires target']) {
      $input = $this->options['target']['input'];
      $value = $this->options['target']['value'][$input];
      $target = geo_value($value, 'wkt', 'array'); // TODO assumption of array
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
      if ($units = $this->options['units']) {
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
