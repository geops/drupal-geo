<?php //$Id$

class views_handler_field_geo_field extends content_handler_field_multiple {
  function query() {
    parent::query();

    if (!$table = $this->table_alias) {
      $table = $this->ensure_my_table();
    }

    // geo_field_select will return the SRID, Envelope, Centroid, and WKB data.
    $this->geo_field = $table .'.'. $this->content_field['field_name'] .'_geo';
    $this->query->add_field(NULL, geo('field_select', $this->geo_field), NULL, array('aggregate' => TRUE));
  }

  function pre_render(&$values) { 
    foreach ($values as $i => $row) {
      // Re-assemble the components of the selected item.
      $item = array();
      $base = substr($this->field_alias, 0, -4);
      foreach (array('wkb', 'bbox', 'srid') as $key) {
        $column = $base .'_'. $key;
        $item[$key] = db_decode_blob($row->$column);
      }
      $values[$i]->{$this->field_alias} = $item;
    }
  }

  function render(&$values) {
    $values->build_mode = 'views';
    
    // Get the formatter's theme name.  This will be used by geo_set_value().
    $name = $this->options['format'];
    $formatter = _content_get_formatter($name, $this->content_field['type']);
    $theme = $formatter['module'] .'_formatter_'. $name;

    // Convert the resulting output to a format recognized by the theme function
    $item = geo_set_value($values->{$this->field_alias}, $theme);

    // Do what the field module was going to do.
    $item['#delta'] = !empty($values->{$this->table_alias . '_delta'}) ? $values->{$this->table_alias . '_delta'} : 0;
    return $this->render_link(content_format($this->content_field, $item, $this->options['format'], $values), $values);
  }
}
