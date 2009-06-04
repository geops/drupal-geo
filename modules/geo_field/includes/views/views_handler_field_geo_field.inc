<?php //$Id$

class views_handler_field_geo_field extends content_handler_field_multiple {
  function query() {
    parent::query();

    if (!$table = $this->table_alias) {
      $table = $this->ensure_my_table();
    }

    // geo_field_select will return the SRID, Envelope, Centroid, and WKB data.
    $geo_field = $table .'.'. $this->content_field['field_name'] .'_geo';
    $this->query->add_field(NULL, geo('field_select', $geo_field), NULL, array('aggregate' => TRUE));
  }

  function not_render($values) {
    $field = $this->content_field;
    $options = $this->options;
    $db_info = content_database_info($field);

    // $values will be used as a fake $node object;
    // we provide a build_mode for rendering.
    // TODO: we can stick any value in there -
    // what would make most sense ?  row_style ?
    $values->build_mode = 'views';

    $item = array();

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
