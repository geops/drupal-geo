<?php // $Id$

class views_handler_relationship_geo extends views_handler_relationship {
  function query() {
    // Hard-code our own join handler.
    //drupal_set_message('<pre>'.print_r($this, 1));
    return parent::query();
  }
}

?>
