<?php
/**
* CRUD operations
*/


function catalog_object_save($catalog_object) {
  
  // fill non existing keys
  $catalog_object = (array)$catalog_object;
  
  $catalog_object += array(
    'depth' => NULL,
    'machine_name' => '',
    'is_block' => 0,
    'block_exclude_current' => NULL,
    'cols' => 1,
    'rows' => 10,
  );
  $catalog_object = (object)$catalog_object;
  $catalog_object->machine_name = drupal_strtolower($catalog_object->machine_name);


  //for taxonomy_get_tree 0-or-NULL-sensitive function
  if ($catalog_object->depth === '') {
    $catalog_object->depth = NULL;
  }

  $transaction = db_transaction();
  try {

    if (!($is_new = empty($catalog_object->cid))) {
      if (!isset($catalog_object->original)) {
        $catalog_object->original = entity_load_unchanged('catalog_object', $catalog_object->cid);
      }
      if (!$catalog_object->is_block) {
        field_attach_presave('catalog_object', $catalog_object);
        module_invoke_all('entity_presave', $catalog_object, 'catalog_object');
      }
    }
    
    if ($result = drupal_write_record('catalog_objects', $catalog_object, $is_new ? array() : 'cid')) {

      entity_info_cache_clear();
      /*db_query('DELETE FROM {catalog_objects_translations} WHERE cid = :cid', array(':cid' => $catalog_object->cid));
      if (!empty($catalog_object->translations)) {

        foreach ($catalog_object->translations as $lang => $data) {
          db_query('INSERT INTO {catalog_objects_translations} SET cid = :cid, langcode = :lang, data = :data', array(
          ':cid' => $catalog_object->cid,
          ':lang' => $lang,
          ':data' => serialize($data),
          ));
        }
      }*/

      db_ignore_slave();
      if (!$is_new) {
        if (!$catalog_object->is_block) {
          field_attach_update('catalog_object', $catalog_object);
          module_invoke_all('entity_update', $catalog_object, 'catalog_object');
        }
      } else {
        if (!$catalog_object->is_block) {
          field_attach_create_bundle('catalog_object', $catalog_object->cid);
        }
      }
      $previous_catalog_object = isset($catalog_object->original) ? $catalog_object->original : NULL;
      unset($catalog_object->original);
      if (!$catalog_object->is_block) {
        entity_get_controller('catalog_object')->resetCache(array($catalog_object->cid));
      }

      // Menu operations:::
      if (!$catalog_object->is_block) {
      
        if (isset($catalog_object->menu)) {
          $link = &$catalog_object->menu;
          if (empty($link['enabled'])) {
            catalog_object_menu_tree_delete($catalog_object->cid);
          }
          elseif ($link['link_title'] = trim($link['link_title'])) {
            $link['link_path'] = 'catalog/' . $catalog_object->cid;
            if ($link['description'] = trim($link['description'])) {
              $link['options']['attributes']['title'] = $link['description'];
            }
            else {
              unset($link['options']['attributes']['title']);
            }
            $link['module'] = 'catalog';
            if (!menu_link_save($link)) {
              drupal_set_message(t('There was an error saving the menu link.'), 'error');
            }
            else {
              variable_set('menu_parent_catalog_objects', $catalog_object->menu['menu_name']);
            }
            _catalog_set_menu_rebuild_task($catalog_object->cid);
          }
        }
        ///Field manage tabs items needs rebuild. Kostyl'.:
        if (!$previous_catalog_object) {
          drupal_flush_all_caches();
        }
        elseif (!$catalog_object->vid && !$catalog_object->tid) {
          // Remove unused view modes (level_0..9) (@see catalog_entity_info())
          $bundle_settings = field_bundle_settings('catalog_object', $catalog_object->cid);
          foreach ($bundle_settings['view_modes'] as $idx => $data) {
            if (preg_match('/^level_\d+$/', $idx)) {
              $bundle_settings['view_modes'][$idx]['custom_settings'] = FALSE;
            }
          }
          field_bundle_settings('catalog_object', $catalog_object->cid, $bundle_settings);
        }
        field_info_cache_clear();
            // ^^  Needed for rebuild extra fields cache - 'terms' and 'object' extra fields is dynamically removed in catalog_field_extra_fields() where catalog not use taxonomy.
      }

      
    }
  }
  catch (Exception $e) {
    $transaction->rollback();
    watchdog_exception('catalog_object', $e);
    throw $e;
  }

  _catalog_object_rebuild_aliases($catalog_object);
  _catalog_cache_clear();
  
  return $result ? $catalog_object : FALSE;
}


function catalog_object_load_multiple($cids = array(), $conditions = array(), $reset = FALSE) {
  return entity_load('catalog_object', $cids, $conditions, $reset);
}

function catalog_object_load($cid) {
  $catalog_objects = catalog_object_load_multiple(array($cid), array());
  return $catalog_objects ? reset($catalog_objects) : FALSE;
}


function catalog_object_delete_multiple($cids) {
  $deleted = 0;
  if ($cids) {
    $section_has_deleted = FALSE;
    foreach (catalog_object_load_multiple($cids) as $cid => $catalog_object) {
      if (!$catalog_object->is_block) {
        module_invoke_all('entity_delete', $catalog_object, 'catalog_object');
        field_attach_delete('catalog_object', $catalog_object);
      }
      else {
        // See also block_custom_block_delete_submit()
        $deleted += db_delete('block')->condition('module', 'catalog')->condition('delta', $cid)->execute();
        db_delete('block_role')->condition('module', 'catalog')->condition('delta', $cid)->execute();
      }
      if (db_delete('catalog_objects')->condition('cid', $cid)->execute() ? ($catalog_object->is_block ? 0 : 1) : 0) {
        $deleted ++;
        $section_has_deleted = TRUE;
        field_attach_delete_bundle('catalog_objects', $cid);
      }
      catalog_object_delete_exposed_presets($catalog_object);

      db_delete('block_node_type')->condition('module', 'catalog')->condition('delta', $cid)->execute();

      if (!$catalog_object->is_block) {
        catalog_object_menu_tree_delete($cid);
        if (variable_get('site_frontpage', 'node') === 'catalog/' . $cid) {
          variable_set('site_frontpage', 'node');
        }
        foreach (db_query('SELECT pid FROM {url_alias} WHERE source LIKE :like OR source = :src',
          array(':like' => 'catalog/' . $cid . '/%', ':src' => 'catalog/' . $cid))->fetchCol() as $pid) {
          path_delete($pid);
        }
      }
    }
    if ($deleted) {
      if ($section_has_deleted) {
        drupal_flush_all_caches();
        variable_set('menu_rebuild_needed', TRUE);
      }
      else {
        cache_clear_all();
      }
      _catalog_cache_clear();
    }
  }
  return $deleted;
}

function catalog_object_delete($cid) {
  return catalog_object_delete_multiple(array($cid));
}











