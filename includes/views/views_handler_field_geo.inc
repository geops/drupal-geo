<?php //$Id$

class views_handler_field_geo extends views_handler_field {

  function options_definition() {
    $options = parent::option_definition();
    $options['function'] = array('default' => 'AsText');
    $options['target'] = array(
      'input' => array('default' => 'geo_latlon'),
      'value' => array('default' => array()),
    );
    return $options;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    $geo = geo_load(array('gid' => $this->definition['geo_gid']));

    // Select which function to use for display.
    $options = $geo->functionNames('display');

    // TODO add functions that are formatters for geo fields (e.g maps!)

    $form['function'] = array(
      '#type' => 'radios',
      '#title' => t('Display format'),
      '#default_value' => $this->options['function'],
      '#options' => $options,
    );

    // TODO some of these might have a target (e.g. distance) add target stuff here.
  }

  function query() {
    $this->geo = geo_load(array('gid' => $this->definition['geo_gid']));

    $table = $this->ensure_my_table();
    $this->geo->setTableName($table);
    $this->geo->setColumnName($this->field);

    // For now, we fetch the result as text data.
    $function = $this->options['function'];
    if (!$function) return;

    $this->field_alias .= '_'. strtolower($function);

    // Add the field, using the Geo API method for getting it.
    $field = $this->geo->$function($this->geo);
    $this->query->add_field(NULL, $field, $this->field_alias);
  }
}
