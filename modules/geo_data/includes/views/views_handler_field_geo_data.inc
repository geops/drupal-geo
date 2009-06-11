<?php //$Id$

class views_handler_field_geo_data extends content_handler_field_multiple {
  function query() {
    $table = $this->ensure_my_table();

    if ($geo_table = $this->definition['geo_table']) {
      $geo_table = $this->query->ensure_table($geo_table);
    }

    // geo_field_select will return the SRID, Envelope, Centroid, and WKB data.
    $alias = $this->ensure_my_table() .'_'. $this->field;
    $func = geo('field_select', $this->real_field, NULL, $target);
    $this->query->add_field('', $func, $alias, array('aggregate' => TRUE));

    foreach ($this->content_field['geo_data_value'] as $field) {
      $this->query->add_field($geo_table, $field);
    }
    $key = $this->query->add_field($geo_table, $this->content_field['geo_data_key']);
  }

  function render($values) {
    $field = $this->content_field;
    $options = $this->options;
    $db_info = content_database_info($field);
    $geo_table = $this->definition['geo_table'];

    // $values will be used as a fake $node object;
    // we provide a build_mode for rendering.
    // TODO: we can stick any value in there -
    // what would make most sense ?  row_style ?
    $values->build_mode = 'views';

    $item = array();

    $item['value'] = $values->{$geo_table.'_'. $field['geo_data_key']};

    $item['wkb'] = $values->{$geo_table.'_wkb'};
    $item['bbox'] = $values->{$geo_table.'_bbox'};

    foreach ($this->content_field['geo_data_value'] as $column) {
      $value = $values->{$geo_table.'_'. $column};
      $item['geo_data_'. $column] = $value;
      if (!isset($item['label']) && in_array($column, $field['geo_data_value'])) {
        $item['label'] = $value;
      }
    }
    $item['#delta'] = !empty($values->{$this->table_alias . '_delta'}) ? $values->{$this->table_alias . '_delta'} : 0;
    return $this->render_link(content_format($field, $item, $options['format'], $values), $values);
  }
}
