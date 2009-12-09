<?php // $Id$

class views_handler_sort_geo extends views_handler_sort {

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
    parent::options_form($form, $form_state);

    // This helper function provides a function chooser and sets up targets.
    geo_views_target_form($form, $form_state, $this, 'sort');

    // Move the sort order input to the bottom.
    $form['order']['#weight'] = 10;
  }

  function query() {
    $this->geo = geo_load(array('gid' => $this->definition['geo_gid']));

    // The GIS function we'll be sorting on.
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
    $field = $this->geo->$function($this->geo, $target);
    $this->query->add_field(NULL, $field, $this->field_alias);

    $this->query->add_orderby(NULL, NULL, $this->options['order'], $this->field_alias);
  }
}
