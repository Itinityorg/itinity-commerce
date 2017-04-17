<?php // $Id: catalog.object.controller.inc, v 1.0 2011/04/29 10:02:15 ITKLD Exp $



class CatalogObjectController extends DrupalDefaultEntityController {
  protected function buildQuery($ids, $conditions = array(), $revision_id = FALSE) {
    //custom code here
    //.......
    //standard call:
    $query = parent::buildQuery($ids, $conditions, $revision_id);
    return $query;
  }
  protected function attachLoad(&$catalog_objects, $revision_id = FALSE) {
    //custom code here
    /*if (module_exists('translation') && drupal_multilingual()) {
      foreach(db_query('SELECT * FROM {catalog_objects_translations} WHERE cid IN (:cids)', array(':cids' => array_keys($catalog_objects))) as $r) {
        $catalog_objects[$r->cid]->translations[$r->langcode] = unserialize($r->data);
      }
    }*/
    foreach ($catalog_objects as $cid => $catalog_object) {
     // Process serialized columns
      foreach (array('orders', 'filters', 'block_settings') as $key) {
        if (empty($catalog_objects[$cid]->$key)) {
          $catalog_objects[$cid]->$key = array();
        }
        else {
          $catalog_objects[$cid]->$key = unserialize($catalog_objects[$cid]->$key);
        }
      }
    }
    //standard call:
    parent::attachLoad($catalog_objects, $revision_id);
  }
}
