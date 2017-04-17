<?php


/**
* Aggregate information about sorting and/or filtering methods available for entities listed in catalog object pages.
* Defines hook_catalog_sql_info(). @see API.php
*/
function catalog_sql_info($catalog_object, $mode = NULL, $method_field = NULL, $method_property = NULL) {
  $catalog_object->entity_type = isset($catalog_object->entity_type) ? $catalog_object->entity_type : NULL;
  $catalog_object->bundle = isset($catalog_object->bundle) ? $catalog_object->bundle : NULL;

  $sql_info = module_invoke_all('catalog_sql_info', $catalog_object);
  $sql_info += array(
    'orders' => array(),
    'filters' => array(),
  );
  $sql_info['orders'] = array_filter($sql_info['orders']);
  $sql_info['filters'] = array_filter($sql_info['filters']);

  foreach ($sql_info['orders'] as $order_method => $data) {
    $sql_info['orders'][$order_method] += array(
      'description' => isset($data['label']) ? $data['label'] : NULL,
      'label' => NULL,
      'label_asc' => isset($data['label_asc']) ? $data['label_asc'] : NULL,
      'label_desc' => isset($data['label_desc']) ? $data['label_desc'] : NULL,
      'default' => FALSE,
      'by' => NULL,
      'commands' => array(),
      'isnull' => TRUE,
      'weight' => '9999999999999' . $order_method,
    );
  }
  $weight = 0;
  foreach ($sql_info['filters'] as $filter_method => $data) {
    $weight = max($weight, isset($data['weight']) ? $data['weight'] : $weight + .00001);
    $sql_info['filters'][$filter_method] += array(
      'weight' => $weight,
    );
  }

//  m($sql_info);
  drupal_alter('catalog_sql_info', $sql_info, $catalog_object);

  uasort($sql_info['orders'], 'drupal_sort_weight');
  foreach ($sql_info['orders'] as $m => $d) {
    $sql_info['orders'][$m] += array('weight' => NULL,);
    if ($sql_info['orders'][$m]['weight'] === '9999999999999' . $m) {
      $sql_info['orders'][$m]['weight'] = NULL;
    }
  }

  uasort($sql_info['filters'], 'drupal_sort_weight');
//    m($sql_info['filters']);

  if (isset($mode)) {
    $sql_info = isset($sql_info[$mode]) ? $sql_info[$mode] : NULL;
    if (isset($method_field)) {
      $sql_info = isset($sql_info[$method_field]) ? $sql_info[$method_field] : NULL;
      if (isset($method_property)) {
        $sql_info = isset($sql_info[$method_property]) ? $sql_info[$method_property] : NULL;
      }
    }
  }

  return $sql_info ? $sql_info : array();
}

/////// Modules hooks

function business_catalog_sql_info($catalog_object) {
  if ($catalog_object->entity_type === 'user' && $catalog_object->bundle === 'user') {
    return array(
      'orders' => array(
        'is_leader' => array(
          'description' => t('Department head'),
          'label' => t(' Head'),
          // 'by' and 'commands' not defined here. All implemented in business_query_catalog_select_entities_alter()
        ),
      ),

    );
  }
}

/**
* Implements hook_catalog_sql_info(); @see catalog_sql_info()
*/
function features_catalog_sql_info($catalog_object) {
  if ($catalog_object->entity_type == 'node') {
    return array(
      'orders' => array(
        'created' => array(
          'label' => t('Created'),
          'by' => 'base.created DESC',
          'default' => TRUE,
          'isnull' => FALSE,
          'weight' => 1, // after sticky
        ),
        'title' => array(
          'label' => t('Alphabetical'),
          'by' => 'base.title',
          'isnull' => FALSE,
          'weight' => 2,
          'label_asc' => t('A-Z'),
          'label_desc' => t('Z-A'),
        ),
        'changed' => array(
          'description' => t('By modification date'),
          'by' => 'base.changed DESC',
          'isnull' => FALSE,
        ),
        'author' => array(
          'description' => t('Author'),
          'by' => 'u.name',
          'isnull' => FALSE,
          'weight' => 100,
          'commands' => array(
            'innerJoin' => array(
              array(
                'users',
                'u',
                'u.uid = base.uid',
              ),
            ),
          ),
        ),
      ),
    );
  }
}

function user_catalog_sql_info($catalog_object) {
  $result = array();
  if ($catalog_object->entity_type === 'user' && $catalog_object->bundle === 'user') {
    $result = array(
      'orders' => array(
        'created' => array(
          'description' => t('Account creation time'),
          'by' => 'base.created',
          'default' => TRUE,
          'isnull' => FALSE,
          'weight' => 0,
        ),
        'name' => array(
          'label' => t('Username'),
          'by' => 'base.name',
          'weight' => 1,
          'isnull' => FALSE,
        ),
        'mail' => array(
          'label' => t('Email'),
          'by' => 'base.mail',
          'isnull' => FALSE,
        ),
        'access' => array(
          'label' => t('Last access'),
          'by' => 'base.access',
          'isnull' => FALSE,
        ),
        'role' => array(
          'label' => t('Role'),
          'by' => 'role.name',
          'commands' => array(
            'innerJoin' => array(
              array(
                'users_roles',
                'users_roles',
                'users_roles.uid = base.uid',
              ),
              array(
                'role',
                'role',
                'role.rid = users_roles.rid',
              ),
            ),
          ),
        ),
        'user_picture' => array(
          'label' => t('User picture'),
          'isnull' => FALSE,
          'by' => 'IF (base.picture, 0, 1)',
        ),
      ),
      'filters' => array(
        'status' => array(
          'description' => t('Status'),
          'where' => !empty($catalog_object->filters['status']['settings']['status']) ? 'NOT base.status' : 'base.status',
          'default' => TRUE,
          'settings form' => array(
            'status' => array(
              '#type' => 'select',
              '#title' => t('Status'),
              '#title_display' => 'invisible',
              '#options' => array(
                NULL => t('Only non blocked users'),
                'blocked' => t('Only blocked users'),
              ),
              '#default_value' => isset($catalog_object->filters['status']['settings']['status']) ? $catalog_object->filters['status']['settings']['status'] : NULL,
            ),
          ),
        ),
        'exclude_uids' => array(
          'description' => t('Except users'),
          'settings form' => array(
            'uids' => array(
              '#type' => 'textfield',
              '#description' => t('Enter comma-separated user ids list, e.g "1, 2, 3"'),
              '#size' => 10,
              '#default_value' => $uids = isset($catalog_object->filters['exclude_uids']['settings']['uids']) ? $catalog_object->filters['exclude_uids']['settings']['uids'] : 1,
            ),
          ),
        ),
        'user_picture' => array(
          'description' => t('Has picture'),
          'where' => 'base.picture',
        ),
        'online' => array(
          'description' => t('Online users'),
          'isnull' => FALSE,
          'commands' => array(
            'where' => array(
              array(
                'base.access >= :timestamp - :threshold',
                array(':timestamp' => REQUEST_TIME, ':threshold' => isset($catalog_object->filters['access']['settings']['threshold']) ? isset($catalog_object->filters['access']['settings']['threshold']) * 60 : 600),
              ),
            ),
          ),
          'settings form' => array(
            'threshold' => array(
              '#title' => t('Last access timeout'),
              '#description' => t('Interval between current time and last user access time, minutes'),
              '#type' => 'digit',
              '#field_suffix' => t('mins'),
              '#step' => 1,
              '#min' => 5,
              '#max' => 60,
              '#float' => FALSE,
              '#default_value' => isset($catalog_object->filters['access']['settings']['threshold']) ? isset($catalog_object->filters['access']['settings']['threshold']) : 10,
            ),
          ),
        ),
      ),
    );
    if ($uids = preg_split('/([,;]\s*)|(\s+)/', $uids)) {
      $result['filters']['exclude_uids']['commands'] = array(
        'where' => array(
          array(
            'base.uid NOT IN (:uids)',
            array(':uids' => $uids,),
          ),
        ),
      );
    }

    $user_roles = array_map('t', user_roles(TRUE));
    unset($user_roles[DRUPAL_AUTHENTICATED_RID]);
    if ($user_roles) {
      $result['filters']['has_role'] = array(
        'description' => t('Roles'),
        'settings form' => array(
          'roles' => array(
            '#type' => 'checkboxes',
            '#multiple' => TRUE,
            //'#required' => TRUE,
            '#options' => $user_roles,
            '#default_value' =>
              empty($catalog_object->filters['has_role']['enabled']) ? array()
              : (isset($catalog_object->filters['has_role']['settings']['roles']) ? $catalog_object->filters['has_role']['settings']['roles'] : array()),
          ),
        ),
      );
      if (!empty($catalog_object->filters['has_role']['settings']['roles'])) {
        $result['filters']['has_role']['commands'] = array(
          'join' => array(
            array(
              'users_roles',
              'ur',
              'ur.uid = base.uid',
            ),
          ),
          'condition' => array(
            array(
              'ur.rid',
              $catalog_object->filters['has_role']['settings']['roles'],
            ),
          ),
        );
      }
    }

  }

  if ($catalog_object->entity_type === 'node' && $catalog_object->is_block) {
    $result['filters']['current_profile'] = array(
      'description' => t('Only materials belonging to the currently displayed user profile'),
      'settings form' => array(
        array('#markup' => '<div class="messages warning">' . t('Informer will be visible only on users pages like "user/123"') . '</div>'),
      ),
    );
    if (preg_match('/^user\/\d+/', $_GET['q'])) {
      $result['filters']['current_profile']['commands']['where'] = array(
        array(
          'base.uid = :current_profile_uid',
          array(':current_profile_uid' => arg(1)),
        ),
      );
    }
    else {
      $result['filters']['current_profile']['commands']['where'] = array(
        array(
          'base.nid < 0',
        ),
      );
    }
  }
  return $result;
}

function social_profile_catalog_sql_info($catalog_object) {
  if ($catalog_object->entity_type === 'user' && $catalog_object->bundle === 'user') {
    return array(
      'orders' => array(
        'phone' => array(
          'label' => t('Phone'),
          'by' => 'base.phone',
        ),
      ),
    );
  }
}

function social_profile_catalog_sql_info_alter(&$sql_info, $catalog_object) {
  if ($catalog_object->entity_type === 'user' && $catalog_object->bundle === 'user') {
    foreach ($sql_info['orders'] as $order_method => $options) {
      if (isset($sql_info['orders'][$order_method]['by']) && $sql_info['orders'][$order_method]['by'] == 'base.name') {
        $sql_info['orders'][$order_method]['by'] = 'base.lfm';
        unset($sql_info['orders'][$order_method]['isnull']);
      }
    }
  }
  foreach ($sql_info['orders'] as $order_method => $options) {
    if (isset($sql_info['orders'][$order_method]['by']) && preg_match('/u\.name(\s+(A|DE)SC)?\s*$/', $sql_info['orders'][$order_method]['by'])) {
      $sql_info['orders'][$order_method]['by'] = preg_replace('/u\.name(\s+(A|DE)SC)?\s*$/', 'u.lfm$1', $sql_info['orders'][$order_method]['by']);
    }
  }
}


function product_catalog_sql_info($catalog_object) {
  if (shop_get_info('entity_types', $catalog_object->entity_type, $catalog_object->bundle)) {
    $entity_info = entity_get_info($catalog_object->entity_type);
    return array(
      'orders' => array(
        'price' => array(
          'label' => t('Price'),
          'by' => 'si.price',
          'commands' => array(
            'innerJoin' => array(
              array(
                'shop_index',
                'si',
                'si.entity_type = :entity_type AND si.entity_id = base.' . $entity_info['entity keys']['id'],
                array(':entity_type' => $catalog_object->entity_type,),
              ),
            ),
          ),
        ),
        'qty' => array(
          'label' => t('Qty'),
          'by' => 'si.qty DESC',
          'commands' => array(
            'innerJoin' => array(
              array(
                'shop_index',
                'si',
                'si.entity_type = :entity_type AND si.entity_id = base.' . $entity_info['entity keys']['id'],
                array(':entity_type' => $catalog_object->entity_type,),
              ),
            ),
          ),
        ),
      ),
      'filters' => array(
        'in_stock' => !shop_get_info('balances', 'settings', 'track') ? array() : array( // filter method internal key.
          'description' => t('In stock'),
          'where' => 'si.qty > 0 OR si.qty IS NULL',
          'exposed' => TRUE,
          'commands' => array(
            'innerJoin' => array(
              array(
                'shop_index',
                'si',
                'si.entity_type = :entity_type AND si.entity_id = base.' . $entity_info['entity keys']['id'],
                array(':entity_type' => $catalog_object->entity_type,),
              ),
            ),
          ),
        ),
      ),
    );
  }
}


/**
* Implements custom field type entity_reference
*/
function _field_entity_reference_catalog_sql_info($catalog_object) {
  $result = array();
  $entity_info = entity_get_info($catalog_object->entity_type);
  // Map of names of columns contains data for ordering.
  $known_columns = array( // module => table column name
    'entity_reference' => 'id',
  );
  $field_types = field_info_field_types();
  foreach (field_info_instances($catalog_object->entity_type, $catalog_object->bundle) as $field_name => $instance) {
    if (!empty($instance['settings']['node_type'])) {
      $field_info_field = field_info_field($field_name);
      if (!empty($field_info_field['storage']) && $field_info_field['storage']['type'] === 'field_sql_storage' && !empty($field_info_field['columns'])) {
        $table = 'field_data_' . $field_name;
        if (!empty($known_columns[$field_info_field['module']])) {
          $column_name = $known_columns[$field_info_field['module']];
          //m($instance);
          //  m($instance['settings']['node_type']);  // m($instance['bundle']);
          $it = !empty($catalog_object->filters[$field_name]['settings']['it']) && is_numeric($catalog_object->filters[$field_name]['settings']['it'])
            ? $catalog_object->filters[$field_name]['settings']['it'] : 0;

          $result['filters'][$field_name] = array(
            'description' => t('%name', array(
              '%name' => $instance['label'],
              '!descr' => '<em><small>(' . check_plain(drupal_strtolower(t($field_types[$field_info_field['type']]['label']))) . ')</small></em>',
            )),
            'settings form' => array(
              'it' => array(
                '#type' => 'entity_selector',
                '#selector_entity_type' => $instance['entity_type'],
                '#selector_bundles' => array($instance['settings']['node_type']),
                /*'#selector_roles' =>  ($instance['entity_type']==='node')
                  ? db_query('SELECT DISTINCT(n.nid) FROM {node} n WHERE n.type = :nt', array(':nt' => $instance['settings']['node_type']))->fetchCol()
                  : db_query('SELECT DISTINCT(uid) FROM {user}')->fetchCol(), */
                '#default_value' => $it,
              ),
            ),
            'commands' => array(
              'innerJoin' => array(
                array(
                  $table,
                  $table,
                  $table . '.entity_type = :entity_type AND ' . $table . '.entity_id = base.' . $entity_info['entity keys']['id'],
                  array(':entity_type' => $instance['entity_type'],),
                ),
              ),
              'where' => array(
                //array($table . '.' . $field_name . '_fid',),
                array(
                  $table . '.' . $field_name . '_id = :it',
                  array(':it' => $it),
                ),
              ),
            ),
          );


        }
      }
    }


  }
  /* TODO! */
  // (...................)
  return $result;
}
/**
* Implements hook_catalog_sql_info() defined in catalog.module. See catalog_sql_info()
*/
function field_catalog_sql_info($catalog_object) {

  $result = _field_entity_reference_catalog_sql_info($catalog_object);
  //  m($result);
  $entity_info = entity_get_info($catalog_object->entity_type);

  // Map of names of columns contains data for ordering.
  $known_columns = array( // module => table column name
    'taxonomy' => 'tid',
    'image' => 'fid',
    'file' => 'fid',
    'text' => 'value',
    'list' => 'value',
    'number' => 'value',
  );
  $field_types = field_info_field_types();
  foreach (field_info_instances($catalog_object->entity_type, $catalog_object->bundle) as $field_name => $instance) {
    $field_info_field  = field_info_field($field_name);
        //m($field_info_field['type']);

    if ($field_info_field['type'] === 'taxonomy_term_reference') {
      $table_alias = 'ti2' . $field_name;
      $result['orders'][$field_name] = array(
        'description' => t('By field %name !descr', array('%name' => $instance['label'], '!descr' => '<em><small>(' . check_plain(drupal_strtolower(t($field_types[$field_info_field['type']]['label']))) . ')</small></em>')),
        'by' => $table_alias . '.sort_breadcrumb',
        'commands' => array(
          'leftJoin' => array(
            array(
              'taxonomy_entity_index',
              $table_alias,
              $table_alias . '.entity_id = base.' . $entity_info['entity keys']['id'] . ' AND ' . $table_alias . '.entity_type = :entity_type AND ' . $table_alias . '.field_name = :field_name',
              array(
                ':entity_type' => $catalog_object->entity_type,
                ':field_name' => $field_name,
              ),
            ),
          ),
        ),
      );
    }
    else {
      if (!empty($field_info_field['storage']) && $field_info_field['storage']['type'] === 'field_sql_storage' && !empty($field_info_field['columns'])) {
        $table = 'field_data_' . $field_name;
        if (!empty($known_columns[$field_info_field['module']])) {
          $column_name = $known_columns[$field_info_field['module']];
        }
        else { // If we don't known about default column used for data, we use first column in table as default
          $column_name = key($field_info_field['columns']);
        }
        if (isset($field_info_field['columns'][$column_name]['size']) && $field_info_field['columns'][$column_name]['size'] === 'big') {
          // Skip fulltext columns
          continue;
        }
        // $field_info_field['module'] entity_reference
        //m($instance['label']); m($field_info_field['module']);
        $result['orders'][$field_name] = array(
          'description' => t('By field %name !descr', array('%name' => $instance['label'], '!descr' => '<em><small>(' . check_plain(drupal_strtolower(t($field_types[$field_info_field['type']]['label']))) . ')</small></em>')),
          'by' => $table . '.' . $field_name . '_' . $column_name,
          'commands' => array(
            'leftJoin' => array(
              array(
                $table,
                $table,
                $table . '.entity_type = :entity_type AND ' . $table . '.entity_id = base.' . $entity_info['entity keys']['id'],
                array(':entity_type' => $catalog_object->entity_type,),
              ),
            ),
          ),
        );
      }
    }


  }

  // Filters:
  foreach (field_info_instances($catalog_object->entity_type, $catalog_object->bundle) as $field_name => $instance) {
    $field_info_field  = field_info_field($field_name);
    $table = 'field_data_' . $field_name;
    if (in_array($field_info_field['type'], array('number_decimal', 'number_integer'), TRUE)) {
      $min = isset($catalog_object->filters[$field_name]['settings']['min']) ? $catalog_object->filters[$field_name]['settings']['min'] : NULL;
      $max = isset($catalog_object->filters[$field_name]['settings']['max']) ? $catalog_object->filters[$field_name]['settings']['max'] : NULL;
      $result['filters'][$field_name] = array(
        'description' => t('Materials with value of %name field in between', array('%name' => $instance['label'])),
        'settings form' => array(
          '#prefix' => '<div class="container-inline">',
          '#suffix' => '</div>',
          'min' => array(
            '#title' => t('Min'),
            '#title_display' => 'invisible',
            '#type' => 'digit',
            '#size' => $field_info_field['type'] === 'number_decimal' ? 4 : 5,
            '#step' => $field_info_field['type'] === 'number_decimal' ? .5 : 1,
            '#float' => $field_info_field['type'] === 'number_decimal',
            '#default_value' => min($max, $min),
          ),
          'max' => array(
            '#title' => t('Max'),
            '#title_display' => 'invisible',
            '#prefix' => t('and'),
            '#type' => 'digit',
            '#size' => $field_info_field['type'] === 'number_decimal' ? 4 : 5,
            '#step' => $field_info_field['type'] === 'number_decimal' ? .5 : 1,
            '#float' => $field_info_field['type'] === 'number_decimal',
            '#default_value' => max($max, $min),
          ),
        ),
        'commands' => array(
          'innerJoin' => array(
            array(
                $table,
                $table,
                $table . '.entity_type = :entity_type AND ' . $table . '.entity_id = base.' . $entity_info['entity keys']['id'],
                array(':entity_type' => $catalog_object->entity_type,),
            ),
          ),
          'where' => array(
            array(
              $table . '.' . $field_name . '_value BETWEEN :value1 AND :value2',
              array(
                ':value1' => min($max, $min),
                ':value2' => max($max, $min),
              ),
            ),
          ),
        ),
      );
    }
    if ($field_info_field['type'] == 'list_boolean') {
      $result['filters'][$field_name] = array(
        'description' => t('Materials with checked %name option', array('%name' => $instance['label'])),
        'commands' => array(
          'innerJoin' => array(
            array(
                $table,
                $table,
                $table . '.entity_type = :entity_type AND ' . $table . '.entity_id = base.' . $entity_info['entity keys']['id'],
                array(':entity_type' => $catalog_object->entity_type,),
            ),
          ),
          'where' => array(
            array(
              $table . '.' . $field_name . '_value',
            ),
          ),
        ),
      );
    }
    if ($field_info_field['type'] === 'image') {
      $result['filters'][$field_name] = array(
        'description' => t('Only materials that have picture in field %name', array('%name' => $instance['label'])),
        'commands' => array(
          'innerJoin' => array(
            array(
                $table,
                $table,
                $table . '.entity_type = :entity_type AND ' . $table . '.entity_id = base.' . $entity_info['entity keys']['id'],
                array(':entity_type' => $catalog_object->entity_type,),
            ),
          ),
          'where' => array(
            array(
              $table . '.' . $field_name . '_fid',
            ),
          ),
        ),
      );
    }
    if ($field_info_field['type'] === 'taxonomy_term_reference') {
      $form_value = $catalog_object->filters[$field_name]['settings'] ?? NULL;
      $result['filters'][$field_name] = array(
        'description' => t('Field %name', array('%name' => $instance['label'])),
        'settings form' => array(
          '#type' => 'textfield',
          '#autocomplete_path' => 'taxonomy/autocomplete/' . $field_name,
          '#default_value' => $form_value,
        ),
      );

      if ($form_value) {
        $result['filters'][$field_name]['commands'] = array(
          'leftJoin' => array(
            array(
              'taxonomy_index',
              'taxonomy_index',
              'taxonomy_index.nid = base.nid',
            ),
          ),
          'innerJoin' => array(
            array(
              'taxonomy_term_data',
              'taxonomy_term_data',
              'taxonomy_term_data.tid = taxonomy_index.tid AND taxonomy_term_data.name LIKE :name',
              array(':name' => '%' . db_like($form_value) . '%'),
            ),
          ),
        );
      }
    }
  }

  return $result;
}

function status_catalog_sql_info($catalog_object) {
  if ($catalog_object->entity_type == 'node' && status_get_settings($catalog_object->entity_type, $catalog_object->bundle)) {
    return array(
      'orders' => array(
        'status_field' => array(
          'description' => t('Status'),
          'by' => 'status.weight',
          'isnull' => FALSE,
          'commands' => array(
            'join' => array(
              array(
                'status_data',
                'sd',
                'sd.entity_id = base.nid AND sd.entity_type = \'node\'',
              ),
              array(
                'status',
                'status',
                'status.id = sd.status_id',
              ),
            ),
          ),
        ),
      ),
    );
  }
}


function node_catalog_sql_info($catalog_object) {
  if ($catalog_object->entity_type === 'node') {
    $text = isset($catalog_object->filters['node_title']['settings']['it']) ? $catalog_object->filters['node_title']['settings']['it'] : NULL;
    $result['filters'] = array(
      'node_title' => array(
        'description' => t('Title'),
        'settings form' => array(
          'it' => array(
            '#inline' => TRUE,
            '#type' => 'textfield',
            '#default_value' => $text,
            '#description' => t('Title contains this text'),
          ),
        ),
        'commands' => array(
          'where' => array(
            array(
              'base.title LIKE :like',
              array(':like' => '%' . db_like($text) . '%',),
            ),
          ),
        ),
      ),
    );
    if (module_exists('search')) {
      $search_info = search_get_info(FALSE);
      if (!empty($search_info['node'])) {
        $search_extend_settings = variable_get('search_settings_entity_node', array());
        if (!module_exists('search_extend') || !isset($search_extend_settings[$catalog_object->bundle]['enabled']) || $search_extend_settings[$catalog_object->bundle]['enabled']) {
          $result['filters']['fulltext'] = array(
            'description' => t('Text contains'),
            'label' => t('Text contains'),
            'settings form' => array(
              'text' => array(
                '#title' => t('Text contains'),
                '#type' => 'textfield',
                '#default_value' => isset($catalog_object->filters['fulltext']['settings']['text']) ? $catalog_object->filters['fulltext']['settings']['text'] : NULL,
              ),
            ),
          );
          if (isset($catalog_object->filters['fulltext']['settings']['text'])) {
            if (drupal_strlen($text = search_simplify($catalog_object->filters['fulltext']['settings']['text']))) {
              $search_commands = array($text => array($text, 'node'));
              foreach (preg_split('/\s+/u', $text) as $t) {
                if (drupal_strlen($t = trim($t))) {
                  $search_commands[$t] = array($t, 'node');
                }
              }
              if ($search_commands) {
                $result['filters']['fulltext']['commands'] = array(
                  'innerJoin' => array(
                    array(
                      'search_index',
                      'i',
                      'i.sid = base.nid'
                    ),
                  ),
                  'extend' => array(
                    array(
                      'SearchQuery'
                    ),
                  ),
                  'searchExpression' => array_values($search_commands),
                  'setOption' => array(
                    array('type', 'base.type'),
                    array('language', 'base.language'),
                  ),
                  'executeFirstPass' => array(
                    array(),
                  ),
                );
              }
            }
          }
        }
      }
    }
    return $result;
  }
}


function node_rank_catalog_sql_info($catalog_object) {
  if ($catalog_object->entity_type === 'node' && shop_get_info('entity_types', $catalog_object->entity_type, $catalog_object->bundle)) {
    return array(
      'orders' => array(
        'node_rank' => array(
          'description' => t('By fill factor'),
          'label' => t('Fill factor'),
          'by' => 'base.rank DESC',
        ),
      ),
    );
  }
}


function catalog_catalog_sql_info($catalog_object) {
  $entity_info = entity_get_info($catalog_object->entity_type);
  
  $result = array();
  $result['orders'] = array(
    'random' => array(
      'description' => t('Random') . '<br /><small class="marker">'  . t('can slow down the server') . '</small>',
      'label' => t('Random'),
      'by' => 'random_field', // @see orderRandom()
      'isnull' => FALSE,
      'suppress pager' => TRUE,
      'commands' => array(
        'orderRandom' => array(
          array(),
        ),
      ),
    ),
  );
  if ($catalog_object->entity_type == 'node') {
    $result['orders']['sticky'] = array(
      'description' => t('By weight'),
      //'label' => t('Weight'),
      'by' => 'base.sticky DESC' . (empty($catalog_object->orders['created']['enabled']) ? ', base.nid DESC' : ''), // Bug fix: Add order by 'nid' to avoid random sorting (some servers returns random sorted nodes where "sticky" = 0)
      'default' => TRUE, // n.sticky DESC for nodes is drupal defaults for node listings (see taxonomy_select_nodes() default $order param)
      'isnull' => FALSE,
      'weight' => 0,
    );
    $result['orders']['status'] = array(
      'description' => t('By published'),
      'by' => 'base.status ASC',
      'isnull' => FALSE,
      'weight' => 999,
    );
    $threshold = isset($catalog_object->filters['material_actuality']['settings']['days']) ? $catalog_object->filters['material_actuality']['settings']['days'] : 7;
    $result['filters']['material_actuality'] = array(
      'description' => t('Display materials are not older than'),
      'settings form' => array(
        'days' => array(
          '#inline' => TRUE,
          '#type' => 'digit',
          '#title' => t('Actuality threshold'),
          '#title_display' => 'invisible',
          '#min' => 1,
          '#max' => 365,
          '#step' => 1,
          '#default_value' => $threshold,
          '#size' => 2,
          '#field_suffix' => t('days'),
        ),
      ),
      'commands' => array(
        'where' => array(
          array(
            'base.created >= :time - (3600 * 24 * :threshold)',
            array(':time' => REQUEST_TIME, ':threshold' => $threshold,),
          ),
        ),
      ),
    );
  }
  if ($catalog_object->is_block) {
    $result['filters']['exclude_current'] = array(
      'description' => t('Exclude object already displayed in main content'),
      'default' => TRUE,
    );
    if (($current_entity = catalog_menu_get_entity()) && !empty($current_entity->catalog_info_entity_type) && $current_entity->catalog_info_entity_type === $catalog_object->entity_type) {
      list($id, $vid, $bundle) = entity_extract_ids($current_entity->catalog_info_entity_type, $current_entity);

      if ($catalog_object->bundle === $bundle) {
        $result['filters']['exclude_current']['commands'] = array(
          'condition' => array(
            array(
              'base.' . $entity_info['entity keys']['id'],
              $id,
              '<>',
            ),
          ),
        );
      }
    }
  }
  
  if (drupal_multilingual()) {
    if (!empty($entity_info['entity keys']['language'])) {
      $opts = array('' => t('Current active'));
      foreach (language_list() as $lang => $l) {
        if ($l->enabled) {
          $opts[$lang] = t('Only !name lang', array('!name' => drupal_strtolower(t($l->name))));
        }
      }
      $def_langcode = isset($catalog_object->filters['language']['settings']['langcode']) ? $catalog_object->filters['language']['settings']['langcode'] : NULL;
      if ($def_langcode) {
        $opts += array($def_langcode => $def_langcode);
      }
      $result['filters']['language'] = array(
        'description' => t('Language'),
        'settings form' => array(
          '#inline' => TRUE,
          'langcode' => array(
            '#type' => 'select',
            '#options' => $opts,    
            '#inline' => TRUE,  
            '#default_value' => $def_langcode,
          ),
        ),
        'commands' => array(
          'condition' => array(
            array(
              'base.' . $entity_info['entity keys']['language'],
              array(LANGUAGE_NONE, $def_langcode ? $def_langcode : $GLOBALS['language']->language),
            ),
          ),
        ),
      );
    }
  }


  // ********************   created fields
  $form = array();
  if (!empty($catalog_object->filters['created']['settings']['curr_month'])) {
    $created[1] =  strtotime(date("Y-m-01"));
    $created[2] =  strtotime(date("Y-m-".date("t")));
  } else {
    foreach (array(1, 2) as $d_numb) {
      $created[$d_numb] = isset($catalog_object->filters['created']['settings']['it'.$d_numb]) ? $catalog_object->filters['created']['settings']['it'.$d_numb] : '';
    }
    if (!empty($catalog_object->filters['created']['settings']['one_day'])) $created[2] = $created[1];
  }
  $form['it1'] = array(
    '#type' => 'datetime',
    '#time' => FALSE,
    '#title' => t('from'),
    '#inline' => TRUE,
    '#default_value' => $created[1] ? $created[1] : NULL,
    '#states' => array(
      'visible' => array(
        'input[name="filters[created][settings][curr_month]"]' => array('checked' => FALSE,),
      ),
    ),
  );
  $form['it2'] = array(
    '#type' => 'datetime',
    '#time' => FALSE,
    '#title' => t('to'),
    '#inline' => TRUE,
    '#default_value' => $created[2] ? $created[2] : NULL,
    '#states' =>  array(
      'visible' => array(
        'input[name="filters[created][settings][curr_month]"]' => array('checked' => FALSE,),
        'input[name="filters[created][settings][one_day]"]' => array('checked' => FALSE,),
      ),
    ),
  );
  $form['one_day'] = array (
    '#type' => 'checkbox',
    '#title' => t('One day'),
    '#default_value' => !empty($catalog_object->filters['created']['settings']['one_day']),
    '#states' =>  array(
      'visible' => array(
        'input[name="filters[created][settings][curr_month]"]' => array('checked' => FALSE,),
      ),
    ),
   );
  $form['curr_month'] = array (
    '#type' => 'checkbox',
    '#title' => t('Current month'),
    '#default_value' => !empty($catalog_object->filters['created']['settings']['curr_month']),
   );

  $result['filters']['created'] = array(
    '#field_prefix' => '<div class="container-inline">',
    '#field_suffix' => '</div>',
    'description' => t('Created'),
     'settings form' => $form,
    'commands' => array(
          'where' => array(
             array('base.created > :it1', array(':it1' => $created[1])),
             array('base.created < :it2', array(':it2' => $created[2] + 24*60*60)),
            ),
    ),
  );


  return $result;
}

function poll_catalog_sql_info($catalog_object) {
  if ($catalog_object->entity_type === 'node' && $catalog_object->bundle === 'poll') {
    $result = array();
    $result['filters']['actual'] = array(
      'description' => t('Only active polls'),
      'commands' => array(
        'join' => array(
          array(
            'poll',
            'poll',
            'poll.nid = base.nid AND poll.active'
          ),
        ),
      ),
    );
    $result['orders']['votes'] = array(
      'description' => t('Votes count'),
      'label' => t('Votes count'),
      'by' => 'COUNT(poll_vote.nid) DESC',
      'commands' => array(
        'join' => array(
          array(
            'poll_vote',
            'poll_vote',
            'poll_vote.nid = base.nid'
          ),
        ),
        'groupBy' => array(
          array(
            'poll_vote.nid',
          ),
        ),
      ),
    );
    return $result;
  }
}


function new_price_catalog_sql_info($catalog_object) {
  if (shop_get_info('entity_types', $catalog_object->entity_type, $catalog_object->bundle)) {
    $entity_info = entity_get_info($catalog_object->entity_type);

    $join_params = array(
      'shop_index',
      'si',
      'si.entity_type = :entity_type AND si.entity_id = base.' . $entity_info['entity keys']['id'],
      array(':entity_type' => $catalog_object->entity_type,),
    );

    return array(
      'filters' => array(
        'old_price' => array(
          'description' => t('Show only objects that are on sale'),
          'where' => 'si.new_price > 0 AND si.new_price < si.price',
          'commands' => array(
            'join' => array($join_params),
          ),
        ),
      ),
      'orders' => array(
        'old_price' => array(
          'label' => t('New price'),
          'by' => 'np',
          'isnull' => TRUE,
          'commands' => array(
            'join' => array($join_params),
            'addExpression' => array(array('IF (si.new_price > 0 AND si.new_price < si.price, si.new_price, NULL)', 'np'),),
          ),
        ),
      ),
    );
  }
}


function shop_variants_catalog_sql_info($catalog_object) {
  if ($catalog_object->entity_type == 'node' && shop_get_info('entity_types', 'node', $catalog_object->bundle)) {//$catalog_object->bundle == 'product') {


    // IKW: http://itinity.ru/node/1008
    if (!module_exists('node_specifications') || !array_intersect_key(_node_specifications_get_filter_query_symbols(), drupal_get_query_parameters())) {
      $entity_info = entity_get_info($catalog_object->entity_type);
      $result = array(
        'filters' => array(
          'hide_variants' => array(
            'description' => t('Hide variants of product'),
            'default' => TRUE,
            'settings form' => array(
              'level' => array(
                '#inline' => TRUE,
                '#title' => '',
                '#type' => 'select',
                '#options' => array(
                  // IKW: Опции: Показать только главный вариант - Показать только первый уровень иерархии - Показать первый и второй уровни иерархии
                  NULL => t('Display only main variant'),
                  1 => t('Display only 1st level of hierarchy'), // IKW - пишем одно, имеем в виду другое - в реале должны показаны ТОЛЬКО корневые варианты с depth = 0
                  2 => t('Display 1st and 2nd levels of hierarchy'),// IKW - пишем одно, имеем в виду другое - в реале должны показаны ТОЛЬКО корневые варианты с depth = 0 плюс варианты с уровнем 1, но не более
                ),
                '#default_value' => isset($catalog_object->filters['hide_variants']['settings']['level']) ? $catalog_object->filters['hide_variants']['settings']['level'] : NULL,
              ),
            ),
            'commands' => array(
              'leftJoin' => array(
                array(
                  'shop_variants',
                  'sv',
                  'sv.nid = base.nid',
                ),
              ),
            ),
          ),
        ),
      );
      if (empty($catalog_object->filters['hide_variants']['settings']['level'])) {
        // Отобразить только главные варианты (т.е. те, которые не являются дочерними вообще к чему-либо)
        $result['filters']['hide_variants']['commands']['where'][] = array('sv.nid IS NULL');
      }
      else {
        $level = $catalog_object->filters['hide_variants']['settings']['level'];
        if ($level == 1) {// IKW - пишем одно, имеем в виду другое - в реале должны показаны ТОЛЬКО корневые варианты с depth = 0
          $result['filters']['hide_variants']['commands']['where'][] = array('sv.nid IS NULL OR depth = 0');
        }
        else {// IKW - пишем одно, имеем в виду другое - в реале должны показаны ТОЛЬКО корневые варианты с depth = 0 плюс варианты с уровнем 1, но не более
          $result['filters']['hide_variants']['commands']['where'][] = array('sv.nid IS NULL OR depth = 0 OR depth = 1');
        }
      }
      return $result;
    }
  }
}

function shop_variants_catalog_sql_info_alter(&$sql_info, $catalog_object) {
  if (shop_get_info('entity_types', $catalog_object->entity_type, $catalog_object->bundle)) {
    if (!empty($sql_info['filters']['in_stock'])) {
      $sql_info['filters']['in_stock']['where'] .= ' OR si.variant = si.entity_id';
    }
  }
}


function entity_counters_catalog_sql_info($catalog_object) {
  if ($catalog_object->entity_type == 'node') {
    $result = array(
      'orders' => array(
        'views_count' => array(
          'label' => t('Amount of views'),
          'by' => 'IF (history.uid IS NULL OR history.uid = \'\', 1, 0), views_count DESC',
          'isnull' => FALSE,
          'commands' => array(
            'leftJoin' => array(
              array(
                'history',
                'history',
                'history.nid = base.nid',
              ),
            ),
            'addExpression' => array(
              array(
                'COUNT(history.uid)',
                'views_count',
              ),
            ),
            'groupBy' => array(
              array(
                'history.nid',
              ),
            ),
          ),
        ),
      ),
    );
    if (module_exists('comment') && variable_get('comment_' . $catalog_object->bundle) != COMMENT_NODE_CLOSED && variable_get('comment_maintain_node_statistics', TRUE) /* <<-- @see comment_node_insert()*/) {
      $result['orders'] += array(
        'comment_count' => array(
          'label' => t('Amount of comments'),
          'isnull' => FALSE,
          'by' => 'node_comment_statistics.comment_count DESC',
          'commands' => array(
            'Join' => array(
              array(
                'node_comment_statistics',
                'node_comment_statistics',
                'node_comment_statistics.nid = base.nid',
              ),
            ),
          ),
        ),
        'last_comment' => array(
          'label' => t('Last comment'),
          'isnull' => FALSE,
          'by' => 'IF (node_comment_statistics.comment_count, 0, 1), node_comment_statistics.last_comment_timestamp DESC',
          'commands' => array(
            'Join' => array(
              array(
                'node_comment_statistics',
                'node_comment_statistics',
                'node_comment_statistics.nid = base.nid',
              ),
            ),
          ),
        ),
      );
    }
    return $result;
  }
}


function field_date_catalog_sql_info($catalog_object) {
  $result = array();
  $filters = array(
    'field_date_upcoming' => t('Upcoming'),
    'field_date_actual' => t('Current'),
    'field_date_completed' => t('Completed', array(), array('context' => 'field_date')),
  );

  foreach (field_info_instances($catalog_object->entity_type, $catalog_object->bundle) as $instance) {
    if (empty($instance['field_name'])) {
      continue;
    }

    $field = field_info_field($instance['field_name']);
    if ($field['type'] != 'field_date' || empty($field['storage']['details']['sql']['FIELD_LOAD_CURRENT'])) {
      continue;
    }

    $table_name = key($field['storage']['details']['sql']['FIELD_LOAD_CURRENT']);
    if (db_table_exists($table_name)) {
      $column1 = $field['storage']['details']['sql']['FIELD_LOAD_CURRENT'][$table_name]['date1'];
      if (!empty($field['settings']['second_date'])) {
        $column2 = $field['storage']['details']['sql']['FIELD_LOAD_CURRENT'][$table_name]['date2'];
      }

      $join_params = array(
        $table_name,
        $table_name,
        "$table_name.entity_type = :entity_type AND $table_name.entity_id = base.nid",
        array(':entity_type' => $catalog_object->entity_type,),
      );

      //in case when entity has > 2 field_date instances
      $filter_name = 'field_date_actuality_' . $instance['field_name'];

      //actuality filters
      if (!empty($column2)) {
        $result['filters'][$filter_name] = array(
          'description' => t('Actuality') . ' (' . $instance['field_name'] . ')',
          'commands' => array('join' => array($join_params),),
          'settings form' => array(
            'actuality_settings' => array(
              '#type' => 'checkboxes',
              '#options' => $filters,
              '#default_value' => isset($catalog_object->filters[$filter_name]['settings']['actuality_settings'])
                  ? $catalog_object->filters[$filter_name]['settings']['actuality_settings']
                  : array(),
            ),
          ),
        );

        if (!empty($catalog_object->filters[$filter_name]['settings']['actuality_settings'])) {
          $filter_wheres = array(
            'field_date_upcoming' => ":time < $table_name.$column1",
            'field_date_actual' => "$table_name.$column1 < :time AND :time < $table_name.$column2",
            'field_date_completed' => "$table_name.$column2 < :time"
          );

          $wheres = array();
          foreach (array_filter($catalog_object->filters[$filter_name]['settings']['actuality_settings']) as $filter) {
            $wheres[] = $filter_wheres[$filter];
          }
          if ($wheres) {
            $wheres = implode(' OR ', $wheres);
            $result['filters'][$filter_name]['commands']['where'] = array(array($wheres, array(':time' => REQUEST_TIME)));
          }
        }
      }


      //actuality order
      $order_name = 'field_date_actual_order_' . $instance['field_name'];

      $result['orders'][$order_name] = array(
        'description' => t('Sort by date field (@field)', array('@field' => $instance['field_name'])),
        'isnull' => TRUE,
        'commands' => array('leftJoin' => array($join_params),),
        'by' => "$table_name.$column1", //simple order by timestamp if only one field provided in settings
      );
      if (!empty($column2)) {
        $result['orders'][$order_name]['commands']['addExpression'] = array(
          array(':time', 'cur_time', array(':time' => REQUEST_TIME),),
          array("IF ($table_name.$column1 IS NULL, 0, $table_name.$column1)", 'd1',),
          array("IF ($table_name.$column2 IS NULL, 0, $table_name.$column2)", 'd2',),
        );
        $result['orders'][$order_name]['isnull'] = FALSE;
        $result['orders'][$order_name]['by'] = 'ABS(d1 - cur_time)';
      }

      //remove unnecessary join
      if (!empty($catalog_object->filters[$filter_name]['enabled'])) {
        unset($result['orders']['field_date_actual_order']['commands']['leftJoin']);
      }
    }
  }

  return $result;
}



function project_catalog_sql_info($catalog_object) {

  if ($catalog_object->entity_type == 'node' && $catalog_object->bundle == 'project') {

    $result['filters'] = array(
      'root' => array(
        'description' => t('Only root projects'),
        'where' => 'NOT ta.parent',
        'commands' => array(
          'join' => array(
            array(
              'task',
              'ta',
              'ta.nid = base.nid',
            ),
          ),
        ),
        'default' => TRUE,
      ),    
    );
    
    $options = array();
    foreach (status_load_statuses($catalog_object->entity_type, $catalog_object->bundle) as $status) {
      $options[$status->id] = $status->name;
    }
    if ($options) {
      if ($allowed_status_ids = isset($catalog_object->filters['status_id']['settings']['it']) ? array_values(array_filter($catalog_object->filters['status_id']['settings']['it'])) : array()) {
        $allowed_status_ids = array_intersect($allowed_status_ids, array_keys($options));
      }
      $result['filters']['status_field'] = array(
        'description' => t('Project readiness'),
        'settings form' => array(
          'it' => array(
            '#type' => 'checkboxes',
            '#multiple' => TRUE,
            '#options' => $options,
            '#default_value' => empty($catalog_object->filters['status_field']['enabled']) ? array() : $allowed_status_ids,
          ),
        ),   
      );
      if ($allowed_status_ids) {
        $result['filters']['status_field']['commands'] = array(
          'join' => array(
            array(
              'status_data', 
              'sd', 
              'sd.entity_id = base.nid AND sd.entity_type = \'node\'',
            ),
          ),
          'condition' => array(
            array(
              'sd.status_id',
              $allowed_status_ids,
            ),
          ),        
        );
      }
    }
    return $result;
  }
}


function task_catalog_sql_info($catalog_object) {

  if ($catalog_object->entity_type !== 'node' || !task_get_settings($catalog_object->bundle)) {
    return;
  }
  $result = array();
  global $user;
  $result['orders'] = array(
    'completed' => array(
      'description' => t('Completed'),
      'by' => 'ta.completed',
      'isnull' => FALSE,
      'commands' => array(
        'innerJoin' => array(
          array(
            'task',
            'ta',
            'ta.nid = base.nid',
          ),
        ),
      ),
    ),
    'priority' => array(
      'description' => t('Priority'),
      'by' => 'ta.priority',
      'commands' => array(
        'innerJoin' => array(
          array(
            'task',
            'ta',
            'ta.nid = base.nid',
          ),
        ),
      ),
    ),
    'start' => array(
      'description' => t('Start working'),
      'by' => 'ta.start',
      'commands' => array(
        'join' => array(
          array(
            'task',
            'ta',
            'ta.nid = base.nid',
          ),
        ),
      ),
    ),
    'deadline' => array(
      'description' => t('Deadline'),
      'by' => 'ta.deadline',
      'commands' => array(
        'join' => array(
          array(
            'task',
            'ta',
            'ta.nid = base.nid',
          ),
        ),
      ),
    ),
    'responsible' => array(
      'description' => t('Responsible'),
      'label' => t('Responsible'),
      'by' => 'u.name',
      'commands' => array(
        'join' => array(
          array(
            'task',
            'ta',
            'ta.nid = base.nid',
          ),
          array(
            'users',
            'u',
            'u.uid = ta.responsible',
          ),
        ),
      ),
    ),
    'internal_number' => array(
      'description' => t('Internal number'),
      'by' => 'ta.internal_number DESC',
      'isnull' => FALSE,
      'commands' => array(
        'join' => array(
          array(
            'task',
            'ta',
            'ta.nid = base.nid',
          ),
        ),
      ),
    ),
    'man_hours' => array(
      'description' => t('Man hour'),
      'by' => 'ta.man_hours DESC',
      'isnull' => FALSE,
      'commands' => array(
        'join' => array(
          array(
            'task',
            'ta',
            'ta.nid = base.nid',
          ),
        ),
      ),
    ),
  );
  if (!module_exists('man_hours')) {
    unset($result['orders']['man_hours']);
  }
  $result['filters'] = array(
    'current_responsible' => array(
      'description' => t('Only those tasks for which the user is responsible'),
      'commands' => array(
        'join' => array(
          array(
            'task',
            'ta',
            'ta.nid = base.nid',
          ),
        ),
        'where' => array(
          array(
            'ta.responsible = :uid',
            array(':uid' => $GLOBALS['user']->uid),
          ),
        ),
      ),
    ),
  );


  //title field
  $title = isset($catalog_object->filters['title']['settings']['it']) ? $catalog_object->filters['title']['settings']['it'] : NULL;
  $result['filters']['title'] = array(
    'description' => t('Title of task'),
    'settings form' => array(
        'it' => array(
        '#type'          => 'textfield',
        '#default_value' => $title,
        '#description'   => t('Task title contains this text'),
      ),
    ),
    'commands' => array(
          'where' => array(
        //$query->condition('base.title', '%' . db_like($value) . '%', 'LIKE');
            array('base.title LIKE :title',  array(':title' => '%' . db_like($title) . '%'), ),
           ),
    ),
  );
  //status field
  $options = array();
  foreach (status_load_statuses($catalog_object->entity_type, $catalog_object->bundle) as $status) {
    $options[$status->id] = $status->name;
  }
  
  if ($options) {
    if ($allowed_status_ids = isset($catalog_object->filters['status_id']['settings']['it']) ? array_values(array_filter($catalog_object->filters['status_id']['settings']['it'])) : array()) {
      $allowed_status_ids = array_intersect($allowed_status_ids, array_keys($options));
    }
    $result['filters']['status_id'] = array(
      'description' => t('Status'),
      'settings form' => array(
        'it' => array(
          '#type' => 'checkboxes',
          '#multiple' => TRUE,
          '#options' => $options,
          '#default_value' => empty($catalog_object->filters['status_id']['enabled']) ? array() : $allowed_status_ids,),
      ),
    );
    if ($allowed_status_ids) {
      $result['filters']['status_id']['commands'] = array(
        'join' => array(
          array(
            'status_data',
            'sd',
            'sd.entity_id = base.nid AND sd.entity_type = \'node\'',
          ),
        ),
        'condition' => array(
          array(
            'sd.status_id',  
            $allowed_status_ids,
          ),
        ),      
      );
    }
  }
  //priority field
  _task_load_include('priority'/*, $form_state*/);
    $priority = isset($catalog_object->filters['priority']['settings']['it']) ? $catalog_object->filters['priority']['settings']['it'] : array();
    $result['filters']['priority'] = array(
      'description' => t('Priority'),
       'settings form' => array(
         'it' => array(
           '#type' => 'checkboxes',
          '#multiple' => TRUE,
          '#options' => task_get_priority_field_values($catalog_object->bundle),
          '#default_value' =>
             empty($catalog_object->filters['priority']['enabled']) ? array() : $priority,
        ),
      ),
      'commands' => array(
        'condition' => array( array(  'ta.priority',  $priority,  ),  ),
      ),
    );
    // ***************  START fields ******************************
    $form = array();
    if (!empty($catalog_object->filters['start']['settings']['curr_month'])) {
      $start[1] = strtotime('midnight first day of this month'); // analog strtotime('0:0'));
      $start[2] = strtotime('last day of this month');
    } else {
      foreach (array(1, 2) as $d_numb) {
        $start[$d_numb] = isset($catalog_object->filters['start']['settings']['it'.$d_numb]) ? $catalog_object->filters['start']['settings']['it'.$d_numb] : '';
      }
      if (!empty($catalog_object->filters['start']['settings']['curr_day'])) {
        $start[1] = $start[2] = strtotime('0:0'); // analog  strtotime('midnight this day')
      }
    }

    $form['it1'] = array(
      '#type' => 'datetime',
      '#group' => 'task_start_fields',
      '#time' => FALSE,
      '#title' => t('from'),
      '#inline' => TRUE,
      '#default_value' => $start[1] ? $start[1] : NULL,
      '#states' => array(
        'visible' => array(
          'input[name="filters[start][settings][curr_month]"]' => array('checked' => FALSE,),
          'input[name="filters[start][settings][curr_day]"]' => array('checked' => FALSE,),
        ),
      ),
    );
    $form['it2'] = array(
      '#type' => 'datetime',
      '#group' => 'task_start_fields',
      '#time' => FALSE,
      '#title' => t('to'),
      '#inline' => TRUE,
      '#default_value' => $start[2] ? $start[2] : NULL,
      '#states' =>  array(
        'visible' => array(
          'input[name="filters[start][settings][curr_month]"]' => array('checked' => FALSE,),
          'input[name="filters[start][settings][curr_day]"]' => array('checked' => FALSE,),
        ),
      ),
    );
    $form['curr_day'] = array (
      '#type' => 'checkbox',
      '#title' => t('Current day'),
      '#default_value' => !empty($catalog_object->filters['start']['settings']['curr_day']),
      '#states' =>  array(
        'visible' => array(
          'input[name="filters[start][settings][curr_month]"]' => array('checked' => FALSE,),
        ),
      ),
     );
    $form['curr_month'] = array (
      '#type' => 'checkbox',
      '#title' => t('Current month'),
      '#default_value' => !empty($catalog_object->filters['start']['settings']['curr_month']),
     );

    $result['filters']['start'] = array(
      '#type'         => 'item',
      '#field_prefix' => '<div class="container-inline">',
      '#field_suffix' => '</div>',
      'description' => t('Start working'),
       'settings form' => $form,
      'commands' => array(
            'where' => array(
               array(
                 'ta.start > :it1',  array(':it1' => $start[1]),
               ),
               array(
                 'ta.start < :it2',  array(':it2' => $start[2] + 24*60*60),
               ),
              ),
      ),
    );

    // ***************  deadline fields ******************************
    _task_load_include('deadline', $form_state);
    $form = array();
    if (!empty($catalog_object->filters['deadline']['settings']['curr_month'])) {
      $deadline[1] = strtotime('midnight first day of this month'); // analog strtotime('0:0'));
      $deadline[2] = strtotime('last day of this month');
    }
    else {
      foreach (array(1, 2) as $d_numb) {
        $deadline[$d_numb] = isset($catalog_object->filters['deadline']['settings']['it'.$d_numb]) ? $catalog_object->filters['deadline']['settings']['it'.$d_numb] : '';
      }
      if (!empty($catalog_object->filters['deadline']['settings']['curr_day'])) {
        $deadline[1] = $deadline[2] = strtotime('0:0'); // analog  strtotime('midnight this day')
      }
    }

    $form['it1'] = task_get_deadline_field_form($deadline[1]);
    $form['it1']['#time'] = FALSE;
    $form['it1']['#title'] = t('from');
    $form['it1']['#states'] = array(
      'visible' => array(
        'input[name="filters[deadline][settings][curr_month]"]' => array('checked' => FALSE,),
        'input[name="filters[deadline][settings][curr_day]"]' => array('checked' => FALSE,),
      ),
    );

    $form['it2'] = task_get_deadline_field_form($deadline[2]);
    $form['it2']['#time'] = FALSE;
    $form['it2']['#title'] = t('to');
    $form['it2']['#states'] = array(
      'visible' => array(
        'input[name="filters[deadline][settings][curr_month]"]' => array('checked' => FALSE,),
        'input[name="filters[deadline][settings][curr_day]"]' => array('checked' => FALSE,),
      ),
    );

    $form['curr_day'] = array (
      '#type' => 'checkbox',
      '#title' => t('One day'),
      '#default_value' => !empty($catalog_object->filters['deadline']['settings']['curr_day']),
      '#states' =>  array(
        'visible' => array(
          'input[name="filters[deadline][settings][curr_month]"]' => array('checked' => FALSE,),
        ),
      ),
     );
    $form['curr_month'] = array (
      '#type' => 'checkbox',
      '#title' => t('Current month'),
      '#default_value' => !empty($catalog_object->filters['deadline']['settings']['curr_month']),
     );

    $result['filters']['deadline'] = array(
      '#type'         => 'item',
      '#field_prefix' => '<div class="container-inline">',
      '#field_suffix' => '</div>',
      'description' => t('Deadline'),
       'settings form' => $form,
      'commands' => array(
            'where' => array(
               array(
                 'ta.deadline > :it1',  array(':it1' => $deadline[1]),
               ),
               array(
                 'ta.deadline < :it2',  array(':it2' => $deadline[2] + 24*60*60),
               ),
              ),
      ),
    );
    //author field
    _task_load_include('responsible', $form_state);
    if (!empty($catalog_object->filters['author']['settings']['current'])) {
        $author = $user->uid;
      } else {
      $author = !empty($catalog_object->filters['author']['settings']['it']) && is_numeric($catalog_object->filters['author']['settings']['it'])
        ? $catalog_object->filters['author']['settings']['it'] : 0;
      }
    $form = array(
      '#inline' => TRUE,
      'it' => array(
        '#type' => 'entity_selector',
        '#selector_entity_type' => 'user',
        '#selector_roles' => db_query('SELECT DISTINCT(ur.rid) FROM {users_roles} ur INNER JOIN {node} n ON n.uid = ur.uid WHERE n.type = :nt', array(':nt' => $catalog_object->bundle))->fetchCol(),
        '#default_value' => $author,
        '#states' => array(
          'visible' => array(
            'input[name="filters[author][settings][current]"]' => array('checked' => FALSE,),
          ),
        ),
      ),
      'current' => ($user->uid) ? array (
        //'#field_prefix' => '&nbsp;',
        '#type' => 'checkbox',
        '#title' => catalog_access('update', $catalog_object) ? t('Current user') : t('Set me'),
        '#default_value' => !empty($catalog_object->filters['author']['settings']['current']),
       ) : array(),
    );

    $result['filters']['author'] = array(
      '#inline' => TRUE,
      'description' => t('Author'),
       'settings form' =>  $form,
      'commands' => array(
            'where' => array(
               array('base.uid = :uid', array(':uid' => $author), ),
              ),
      ),
    );

    //responsible field
    if (!empty($catalog_object->filters['responsible']['settings']['current'])) {
        $responsible = $user->uid;
      } else {
      $responsible = !empty($catalog_object->filters['responsible']['settings']['it']) && is_numeric($catalog_object->filters['responsible']['settings']['it'])
        ? $catalog_object->filters['responsible']['settings']['it'] : 0;
      }
    $form = array(
      '#inline' => TRUE,
      'it' => array(
        '#type' => 'entity_selector',
        '#selector_entity_type' => 'user',
        '#selector_roles' => db_query('SELECT DISTINCT(ur.rid) FROM {users_roles} ur INNER JOIN {task} t ON t.responsible = ur.uid INNER JOIN {node} n ON n.nid = t.nid WHERE n.type = :nt', array(':nt' => $catalog_object->bundle))->fetchCol(),
        '#default_value' => $responsible,
        '#states' => array(
          'visible' => array(
            'input[name="filters[responsible][settings][current]"]' => array('checked' => FALSE,),
          ),
        ),
      ),
      'current' => ($user->uid) ? array (
        '#type' => 'checkbox',
        '#title' => catalog_access('update', $catalog_object) ? t('Current user') : t('Set me'),
        '#default_value' => !empty($catalog_object->filters['responsible']['settings']['current']),
       ) : array(),
    );

    $result['filters']['responsible'] = array(
      'description' => t('Responsible'),
       'settings form' =>  $form,
      'commands' => array(
            'where' => array(
               array('ta.responsible = :it',  array(':it' => $responsible), ),
              ),
      ),
    );

    //internal number
    $internal_number = !empty($catalog_object->filters['internal_number']['settings']['it']) ? $catalog_object->filters['internal_number']['settings']['it'] : '';
    $result['filters']['internal_number'] = array(
      'description' => t('Internal number'),
       'settings form' => array(
         'it' => array(
          '#type'             => 'textfield',
          '#default_value'    => $internal_number,
          '#size'             => 5,
          '#element_validate' => array('element_validate_number'),
         ),
       ),
      'commands' => array(
            'where' => array(
               array('ta.internal_number = :it',  array(':it' => $internal_number), ),
              ),
      ),
    );
   return $result;
}


function shop_groups_catalog_sql_info($catalog_object) {
  if ($catalog_object->entity_type === 'node' && shop_get_info('entity_types', 'node', $catalog_object->bundle)) {
    return array(
      'filters' => array(
        'only_groups' => array(
          'description' => t('Only groups of products'),
          'commands' => array(
            'join' => array(
              array(
                'shop_groups_index',
                'sgi',
                'sgi.g_id = base.nid AND sgi.g_entity_type = \'node\'',
              ),
            ),
          ),
        ),
        'hide_grouped' => array(
          'description' => t('Hide products in groups'),
          'where' => 'base.nid NOT IN (SELECT sgi.item_id FROM {shop_groups_index} sgi WHERE sgi.item_entity_type = \'node\')',
        ),
      ),
    );
  }
}



function entity_reference_catalog_sql_info_alter(&$sql_info, $catalog_object) {
  $entity_info = entity_get_info($catalog_object->entity_type);
  foreach (entity_reference_field_instances($catalog_object->entity_type, $catalog_object->bundle) as $field_name => $inst) {
    if (!empty($sql_info['orders'][$field_name])) {
      $sql_info['orders'][$field_name]['by'] = $field_name . '_node.' . $entity_info['entity keys']['label'];
      $sql_info['orders'][$field_name]['default'] = TRUE;
      $sql_info['orders'][$field_name]['commands']['leftJoin'][] = array(
        $entity_info['base table'],
        $field_name . '_' . $entity_info['base table'],
        $field_name . '_' . $entity_info['base table'] . '.' . $entity_info['entity keys']['id'] . ' = field_data_' . $field_name . '.' . $field_name . '_id',
      );
    }
  }
}










