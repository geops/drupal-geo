<?php // $Id$

class views_handler_sort_geo extends views_handler_sort {

  function init(&$view, $options) {
    parent::init($view, $options);
    $this->geo = geo_load(array('gid' => $this->definition['geo_gid']));
  }

  function option_definition() {
    $options = parent::option_definition();
    $options['function'] = array('default' => 'distance');
    $options['target'] = array(
      'input' => array('default' => 'geo_latlon'),
      'value' => array('default' => array()),
    );
    return $options;
  }

  function options_form(&$form, &$form_state) {
    // Call my parent's options_form method.
    parent::options_form($form, $form_state);

    // Use the geo_views helper function to display the target settings.
    geo_views_target_form($form, $form_state, $this, 'sort');
  }

  function query() {
    $this->geo = geo_load(array('gid' => $this->definition['geo_gid']));

    // The GIS function we'll be sorting on.
    $function = $this->options['function'];

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
    $this->field_alias = $this->field .'_'. $function;

    // Ensure that the query acts on the table and column aliases defined here.
    $this->geo->setTableName($table);
    $this->geo->setColumnName($this->field);

    // Add the field, using the Geo API method for getting it.
    $field = $this->geo->$function($this->geo, $target);
    $this->query->add_field(NULL, $field, $this->field_alias);

    $this->query->add_orderby(NULL, NULL, $this->options['order'], $this->field_alias);
  }
}
