<?php //$Id$

class views_handler_field_geo extends views_handler_field {

  function init(&$view, $options) {
    parent::init($view, $options);
    $this->geo = geo_load(array('gid' => $this->definition['geo_gid']));
  }

  function options_definition() {
    $options = parent::option_definition();
    $options['function'] = array('default' => 'AsText');
    $options['target'] = array(
      'value' => array('default' => array()),
    );
    $options['input'] = array('default' => '');
    return $options;
  }

  function options_form(&$form, &$form_state) {
    // Call my parent's options_form method.
    parent::options_form($form, $form_state);

    // Use the geo_views helper function to display the target settings.
    geo_views_target_form($form, $form_state, $this, 'display');
  }

  function query() {

    // The GIS Function we'll be using to query the database.
    $function = $this->options['function'];
    if (!$function) return;

    // Figure out whether or not we need a target value to compare against.
    $target = NULL;

    $info = $this->geo->functionInfo($function);
    if ($info['requires target']) {
      $input = geo_input_info($this->options['input']);
      $value = $this->options['target'][$this->options['input']];

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
    $this->geo->setTableName($table);
    $this->geo->setColumnName($this->field);

 //   $this->field_alias .= '_'. strtolower($function);

    // Add the field, using the Geo API method for getting it.
    $field = $this->geo->$function($this->geo, $target);
    $alias = $this->real_field .'_'. strtolower($function);
    $this->field_alias = $this->query->add_field(NULL, $field, $alias);
//    $this->query->add_field(NULL, $field, $this->field_alias);
  }
}
