<?php // $Id$

/**
 * Implementation of hook_views_data()
 */ 
function geo_data_views_data() {
  $data = array();
  $data['geo_data']['table']['group']  = t('Geospatial Data');
  return $data;
}
