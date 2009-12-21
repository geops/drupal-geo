<?php // $Id$

class views_handler_relationship_geo extends views_handler_relationship {
  function query() {
    // Hard-code our own join handler.
    return parent::query();
  }
}

?>
