<?php //$Id$

class views_handler_field_geo_field extends content_handler_field {

  function query() {
    $this->geo = geo_load(array('name' => $this->content_field['field_name']));

    // Set up a table name and field name that are unique to this query.
    $table = $this->ensure_my_table();
    $this->field_alias = $table .'_'. $this->field;

    // Ensure that the query acts on the table and column aliases defined here.
    $this->geo->setTableName($table);
    $this->geo->setColumnName($this->field);

    // For now, we fetch the result as binary data.
    $func = 'AsBinary';

    // Add the field, using the Geo API method for getting it.
    $field = $this->geo->$func($this->geo);
    $this->query->add_field(NULL, $field, $this->field_alias, array('aggregate' => TRUE));

    $this->add_additional_fields();
  }

  function pre_render(&$values) {

    $name = $this->options['format'];
    $formatter = _content_get_formatter($name, $this->content_field['type']);
    $theme = $formatter['module'] .'_formatter_'. $name;

    foreach ($values as $i => $row) {
      // For some reason, this gets called repeatedly.  Don't double-process.
      if (is_array($row->{$this->field_alias})) return;

      $field = array(
        'srid' => $this->geo->srid(),
        'wkb' => db_decode_blob($row->{$this->field_alias}),
      );

      // Convert the output to a format recognized by the theme function
      if ($value = geo_set_value($field, $theme));
      $values[$i]->{$this->field_alias} = $value['geo'];
    }
  }
}
