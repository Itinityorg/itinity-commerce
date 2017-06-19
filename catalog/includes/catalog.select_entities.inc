<?php


/**
* Helper function for prevent duplicated commands in $query object. Use it instead $query->funcName() if you not sure that desired command with same parameters is not was added earlier
* @param $query - db_select() object
* @param $commands - associative array of commands.
    Key - callback name (e.g. 'addJoin'),
    values - array of arrays (e.g. if we want to add two inner joins, values must be contains two arrays: array(array('shop_index', 'si'), array('shop_cart', 'sc'))
* Usage:
    Standard pattern:
    $query->condition('base.nid', 100, '>');
    Own pattern:
    _catalog_sql_query_add_commands($query, array('condition', array(array('base.nid', 100, '>'))));

*/
function _catalog_sql_query_add_commands($query, $commands, $comment = NULL) {
  foreach ($commands as $command => $calls) {
    foreach ($calls as $params) {
      $c = $command;
      if ($command === 'join' || $command === 'innerJoin') {
        $c = 'addJoin';
        array_unshift($params, 'INNER');
      }
      elseif ($command === 'leftJoin') {
        $c = 'addJoin';
        array_unshift($params, 'LEFT OUTER');
      }
      elseif ($command === 'rightJoin') {
        $c = 'addJoin';
        array_unshift($params, 'RIGHT OUTER');
      }
      $hash = md5($c . serialize($params));
      if (empty($query->_catalog_added_commands) || !in_array($hash, $query->_catalog_added_commands, TRUE)) {
        $query->_catalog_added_commands[] = $hash;
        if ($c === 'extend') {
          $query = call_user_func_array(array($query, $c), $params);
        }
        else {
          call_user_func_array(array($query, $c), $params);
        }

      }
    }
  }
}

/**
* Select entities and returns array of entity id's
* Use $predefined_entity_ids when you want to determine that catalog_object may contains or not may contains desired entities in output
*/
function _catalog_select_entities($catalog_object, $rss_mode = FALSE, $return_all = FALSE, $predefined_entity_ids = NULL) {
  //$time_start = microtime(1);
  //if (CATALOG_DEVMODE_ON) {
    //drupal_set_message('Catalog switched in development mode!', 'warning', FALSE);
 // }
  // Get requred SQL information for entity type
  $entity_info = entity_get_info($catalog_object->entity_type);
  $entity_id_key = $entity_info['entity keys']['id'];

  // Build initial sql query

  $query = db_select($entity_info['base table'], 'base');
  $query->addMetaData('base_table', $entity_info['base table']);
  // Add bundle condition (like n.type = 'news') for base table
  if ($catalog_object->bundle && !empty($entity_info['entity keys']['bundle'])) { // entity such as user not contains bundle key
    $query->condition('base.' . $entity_info['entity keys']['bundle'], $catalog_object->bundle);
  }
  $query->fields('base', array($entity_id_key));
  _catalog_sql_query_add_commands($query, array('where' => array(array('base.' . $entity_id_key . ' > 0',)))); // do not select anonymous users and prevent further selects zero ids form other tables
  if ($predefined_entity_ids) {
    $query->condition('base.' . $entity_id_key, $predefined_entity_ids);
  }


  // Collect taxonomy terms
  $main_term = !empty($catalog_object->current_params['term']->tid) ? $catalog_object->current_params['term'] : taxonomy_term_load($catalog_object->tid);
  $tids = array();

  if ($main_term) {
    $tids[$main_term->tid] = $main_term->tid;
    foreach (taxonomy_get_tree($main_term->vid, $main_term->tid, $catalog_object->depth > 0 ? $catalog_object->depth - 1 : NULL) as $t) {
      $tids[$t->tid] = $t->tid;
    }
  }
  elseif (!empty($catalog_object->vid)) {
    foreach (taxonomy_get_tree($catalog_object->vid, 0, $catalog_object->depth > 0 ? $catalog_object->depth - 1 : NULL) as $t) {
      $tids[$t->tid] = $t->tid;
    }   
  }

  // Filter entities by terms ids
  if ($tids) {
    _catalog_sql_query_add_commands($query, array('join' => array(array('taxonomy_entity_index', 'ti', 'ti.entity_id = base.' . $entity_id_key . ' AND ti.entity_type = :entity_type', array(':entity_type' => $catalog_object->entity_type)))));
    _catalog_sql_query_add_commands($query, array('condition' => array(array('ti.tid', $tids))));
  }
  // Get orphaned entities (not linked to vocabulary)
  elseif (!empty($catalog_object->vid) && empty($catalog_object->current_params['term']->tid)) {
    _catalog_sql_query_add_commands($query, array('leftJoin' => array(array('taxonomy_entity_index', 'ti', 'ti.entity_id = base.' . $entity_id_key . ' AND ti.entity_type = :entity_type', array(':entity_type' => $catalog_object->entity_type)))));
    _catalog_sql_query_add_commands($query, array('isNull' => array(array('ti.tid'))));
  }
  // Process orders and filters settings
  // Note: at this time orders and filters may be changed at runtime (e.g. by catalog_object_apply_query_url() callback) and inconsistent with real settings in database
  // TODO: use ->current_params storage?
  $catalog_sql_info = catalog_sql_info($catalog_object);
  if (!empty($catalog_object->preselected_entities)) {
    _catalog_sql_query_add_commands($query, array(
      'where' => array(
        array(
        'base.nid IN (:ids)', array(':ids' => $catalog_object->preselected_entities,),
        ),
    )));
  }
  else {
    foreach ($catalog_object->filters as $filter_method => $settings) {
      if (!empty($settings['enabled'])) {
        if (!empty($catalog_sql_info['filters'][$filter_method]['commands'])) {
          _catalog_sql_query_add_commands($query, $catalog_sql_info['filters'][$filter_method]['commands']);
        }
        if (!empty($catalog_sql_info['filters'][$filter_method]['where'])) {
          _catalog_sql_query_add_commands($query, array('where' => array(array($catalog_sql_info['filters'][$filter_method]['where']))));
        }
      }
    }
  }

  $enable_pager = !$rss_mode;
  // Apply order when possible

  foreach ($catalog_object->orders as $order_method => $data) {
    if (!empty($data['enabled'])) {
      // Some orders methods may be incompatible with pager, or pagination is useless (e.g. randomized selection)
      // Also, we do not show pager in block (informer) mode
      if (!empty($catalog_sql_info['orders'][$order_method]['suppress pager'])) {
        $enable_pager = FALSE;
      }


      // Insert additional query commands
      if (!empty($catalog_sql_info['orders'][$order_method]['commands'])) {
        _catalog_sql_query_add_commands($query, $catalog_sql_info['orders'][$order_method]['commands']);
      }
      if (!empty($catalog_sql_info['orders'][$order_method]['by'])) {
        $order_by = $catalog_sql_info['orders'][$order_method]['by'];

        $desc = !empty($catalog_object->orders[$order_method]['descending']);
        // Detect order method with forced DESC and reverse descending option.
        if (preg_match('/\sDESC$/i', $order_by)) {
          $desc = !$desc;
        }

        // Remove DESC|ASC from end
        $order_by = preg_replace('/\s+(DESC|ASC)$/i', '', $order_by);

        // Place NULL or '' fields at the end of result (like yandex market)
        _catalog_sql_query_add_commands($query, array(
          'orderBy' => array(
            array(
              !empty($catalog_sql_info['orders'][$order_method]['isnull']) ? "IF ($order_by IS NULL OR $order_by = '', 1, 0), $order_by" : $order_by,
              $desc ? 'DESC' : 'ASC',
            ),
          ),
        ));
      }
    }
  }

  $query->addTag('catalog_select_entities');
    $query->addTag('catalog_object:' . $catalog_object->cid);
  //$query->{'#catalog_object'} = $catalog_object; //<<  TODO: Remove it, use metadata
  $query->addMetaData('catalog_object', $catalog_object);
  $query->addMetaData('op', 'view'); // needed for node access
  /// Access checks
  global $user;

  if ($catalog_object->entity_type == 'node') {
    $query->addTag('node_access');
    // GRABBED AND MODIFIED FROM NODE MODULE. See recent nodes block code, node_get_recent() function.
    if (!user_access('bypass node access')) {
      if (user_access('view own unpublished content')) {
        _catalog_sql_query_add_commands($query, array('where' => array(array('base.status = :status OR base.uid = :uid', array(':status' => NODE_PUBLISHED, ':uid' => $user->uid)))));
      }
      else {
        _catalog_sql_query_add_commands($query, array('condition' => array(array('base.status', NODE_PUBLISHED,))));
      }
    }
  }
  if ($catalog_object->entity_type == 'user') {
    if (!user_access('administer users') && !user_access('administer permissions')) {
      // Display blocked accounts only for owners or admins
      _catalog_sql_query_add_commands($query, array('where' => array(array('base.status > 0 OR base.uid = :uid', array(':uid' => $user->uid)))));
    }
    else {
      _catalog_sql_query_add_commands($query, array('where' => array(array('base.status > 0'))));
    }
  }
  $query->distinct();

  $limit = 0;
  if (!$return_all) {
    
    ///$limit = 50;
    
    // Init defaults
    $mode = !empty($catalog_object->materials_settings['mode']) ? $catalog_object->materials_settings['mode'] : 'list';
    $mode_settings = isset($catalog_object->materials_settings['modes'][$mode]['settings']) ? $catalog_object->materials_settings['modes'][$mode]['settings'] : array();
    
    $mode_settings += array(
      'cols' => 1,
      'rows' => $mode === 'table' ? 50 : 10,
    );
    $limit = $mode_settings['cols'] * $mode_settings['rows'];
    /////if (isset($mode_settings['cols']) && isset($mode_settings['rows'])) {
    /////  $limit = $mode_settings['cols'] * $mode_settings['rows'];
    /////}
    /////elseif (isset($mode_settings['rows'])) {
    /////  $limit = $mode_settings['rows'];
    /////}

    // phone / tablet / computer - if phone: informer - <= 4 items , catalog - <=10
    if (module_invoke('utils', 'client_device_type') === 'phone') {
      $limit = min($catalog_object->is_block ? 4 : 10, $limit);
    }
  }
  /*
  switch ($catalog_object->materials_settings['mode']) {
    case 'list':
      $limit = $catalog_object->materials_settings['table_settings']['rows'];
      break;
    case 'table':
      $limit = $catalog_object->materials_settings['list_settings']['cols'] * $catalog_object->materials_settings['list_settings']['rows'];
      break;
  }
  */
  $enable_pager = $enable_pager && $limit && !$return_all && (!$predefined_entity_ids || count($predefined_entity_ids) > $limit);
  $catalog_object->_enable_pager = $enable_pager; // Internal temporary property. Needs for rendering pager elements in catalog_object_build_content() function.
  if ($enable_pager) {
    $pager_element = PagerDefault::$maxElement;
    $catalog_object->_pager_element = $pager_element; // Internal temporary property. 
    $query = $query->extend('PagerDefault')->limit($limit)->element($pager_element);
    //$query->element(_catalog_pager_element($catalog_object->cid));
  }
  elseif (!$return_all) {
    $query->range(0, $limit);
  }
  //q($query);
  $result = $query->execute()->fetchCol();

  if ($enable_pager && !$result) {
      unset($catalog_object->_pager_element);
      PagerDefault::$maxElement--;
      global $pager_page_array, $pager_total, $pager_total_items, $pager_limits;
      unset($pager_total_items[$pager_element]);
      unset($pager_total[$pager_element]);
      unset($pager_page_array[$pager_element]);
      unset($pager_limits[$pager_element]);
  }

  return $result;
}


