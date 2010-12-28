<?php
// $Id$

/**
 * This handler exists for one reason only: to provide a filter group for geo
 * data, which can be 'OR'ed.
 */

class views_handler_argument_string_geo extends views_handler_argument_string {

  function option_definition() {
    $options = parent::option_definition();
    $options['group'] = array('default' => 'geo');
    return $options;
  }
}
