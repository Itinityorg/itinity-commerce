<?php

// Deprecated code placed here



/**
 * Entities selection for in-page/block displaying.
 * Returns ids list.
 */
function DEPRECATED_catalog_select_entities($catalog_object, $rss_mode = FALSE) {
//	timer_start('_catalog_select_entities');

	/* order by table header*/
	if (!$catalog_object->is_block && !$rss_mode && isset($_GET['order'])) {
		//spike for order by entity title
		$entities = field_info_extra_fields($catalog_object->entity_type, $catalog_object->bundle, 'display');
		//use default catalog query
		if ($entities['title']['label'] == $_GET['order']) {
			$direction = isset($_GET['sort']) ? $_GET['sort'] : 'desc';
			if ($direction == 'desc') {
				$catalog_object->order_flag = 'node_title_desc';
			}
			else {
				$catalog_object->order_flag = 'node_title';
			}
		} else {
			//get field names by label
			$instances = field_info_instances($catalog_object->entity_type, $catalog_object->bundle);
			foreach ($instances as $field_name => $instance) {
				if ($instance['label'] == $_GET['order']) {
					$sort_field = $field_name;
					break;
				}
			}
		}
	}


	// Supress pager displaying where order is random
	$pager = !$rss_mode && $catalog_object->order_flag !== 'random';

	global $user;
	$result = array();
	if (!($cache_id = _catalog_cache_id($catalog_object))) { // For development you may disable cached selected entities, enable CATALOG_DEVMODE_ON constant
		if (user_is_sysadmin()) {
			drupal_set_message('Catalog switched in development mode!', 'warning', FALSE);
		}
	} else {
		// Get cached entities if possible, for best performance
		if ($data = cache_get($cache_id, 'cache_catalog_entities')) {
			$result = $data->data;
			if ($catalog_object->order_flag === 'random') {
				// more shuffle for shuffled entities -)
				shuffle($result['entities']);
			}
			if ($pager && isset($result['pager_page_array']) && isset($result['pager_total'])) {
				// We cache pagers variables also.
				global $pager_page_array, $pager_total;
				$pager_page_array[_catalog_pager_element($catalog_object->cid)] = $result['pager_page_array'];
				$pager_total[_catalog_pager_element($catalog_object->cid)] = $result['pager_total'];
			}

			return $result['entities'];
		}
	}

	if (!($entity_info = entity_get_info($catalog_object->entity_type))) {
		return array();
	}
	$base_table = $entity_info['base table'];
	$base_table_key = $entity_info['entity keys']['id'];
	$base_table_alias = substr($catalog_object->entity_type, 0, 1); /* u. or n.*/

	// Collate order metods into array. E.g. 1st is n.sticky DESC for nodes by drupal defaults for node listings (see taxonomy_select_nodes())
	/*$order = $catalog_object->entity_type == 'node' ? ($catalog_object->order_flag === 'random' ? array() : array('n.sticky' => 'DESC',)) : array();*/
	$order = $catalog_object->entity_type == 'node' ? ( /*$catalog_object->order_flag === 'random' || */$catalog_object->order_flag == '' ? array('n.sticky' => 'DESC',) : array() ) : array();//weight is only for default

	///strpos($order_flag, 'node_changed') === 0 || strpos($order_flag, 'node_created') === 0, material_actuality

	// Invoke global hook for retrieve other order methods provided by modules with self specified entities. See also hook_catalog_collate_sql().
	if ($orders = catalog_collate_sql($catalog_object->entity_type, $catalog_object->bundle, $catalog_object->order_flag, 'order', 'params', $catalog_object)) {
		$order += $orders;
		$material_actuality =
			strpos($catalog_object->order_flag, 'node_changed') === 0 || strpos($catalog_object->order_flag, 'node_created') === 0
				? $catalog_object->material_actuality
				: NULL;
	}
	else {
		$order[$base_table_alias . '.created'] = 'DESC';
		$material_actuality = $catalog_object->material_actuality;
	}

	$query = db_select($base_table, $base_table_alias);
	if ($catalog_object->order_flag === 'random') {
		$query->orderRandom();
	}
	if ($material_actuality) {
		$query->condition($base_table_alias . '.' . (strpos($catalog_object->order_flag, 'node_changed') === 0 ? 'changed': 'created'), REQUEST_TIME - (3600 * 24 * $material_actuality), '>=');
	}
	if ($catalog_object->entity_type == 'node') {
        $query->condition('n.type', $catalog_object->bundle);
        $query->addTag('node_access');

        //some crunches for poll module
        if ($catalog_object->topicality && $catalog_object->bundle == 'poll') {
					$query->addJoin('left', 'poll', 'pl', 'n.nid = pl.nid');
					$query->condition('pl.active', 1, '=');
				}
        //

		// GRABBED AND MODIFIED FROM NODE MODULE. See recent nodes block code, node_get_recent() function.
		if (!user_access('bypass node access')) {
			if (user_access('view own unpublished content')) {
				$query->condition(db_or()
						->condition('n.status', NODE_PUBLISHED)
						->condition('n.uid', $user->uid)
				);
			}
			else {
				$query->condition('n.status', NODE_PUBLISHED);
			}
		}
	}

	if ($catalog_object->entity_type == 'user') {
		if (!user_access('administer users') && !user_access('administer permissions')) {
			$query->condition(db_or()
					->condition('u.status', NODE_PUBLISHED)
					->condition('u.uid', $user->uid)
			);
		}
		else {
			$query->condition('u.status', NODE_PUBLISHED);
		}
	}

	// Collect taxonomy terms
	$tids = !empty($catalog_object->current_params['term']->tid) ? array($catalog_object->current_params['term']->tid => $catalog_object->current_params['term']->tid) : array();
	if ($catalog_object->tid) {
		$tids[$catalog_object->tid] = $catalog_object->tid;
	}
	if (!empty($catalog_object->vid) && !$tids) {
		foreach (taxonomy_get_tree($catalog_object->vid, $catalog_object->tid ? $catalog_object->tid : 0, $catalog_object->depth > 0 ? $catalog_object->depth - 1 : NULL) as $t) {
			$tids[$t->tid] = $t->tid;
		}
	}

	if ($tids || !empty($catalog_object->vid)) {
		foreach (taxonomy_term_load_multiple($tids) as $term) {
			$tids[$term->tid] = $term->tid;
			foreach (taxonomy_get_tree($term->vid, $term->tid, $catalog_object->depth > 0 ? $catalog_object->depth - 1 : NULL) as $t) {
				$tids[$t->tid] = $t->tid;
			}
		}
	}

	// Exclude current on-page displayed entity where catalog object is block and block_exclude_current option is ON
	if ($catalog_object->is_block && $catalog_object->block_exclude_current && ($obj = menu_get_object())) {
		list($id, $vid, $bundle) = entity_extract_ids($catalog_object->entity_type, $obj);
		if ($bundle == $catalog_object->bundle && $id) {
			$query->condition($base_table_alias . '.' . $base_table_key, $obj->{$base_table_key}, ' != ');
		}
	}

	$join_condition = 'ti.entity_id = ' . $base_table_alias . '.' . $base_table_key . ' AND ti.entity_type = :entity_type';
	$join_params = array(':entity_type' => $catalog_object->entity_type);
	// If catalog object is block, add interactive with current term, displayed on page, if possible
	if ($catalog_object->is_block && $catalog_object->interact && $catalog_object->vid && arg(0) === 'catalog' && count(arg()) > 2) {
		$args = arg();
		$last_tid = array_pop($args);
		$tids = array($last_tid => $last_tid);
		$subterm_tree = taxonomy_get_tree($catalog_object->vid, $last_tid);
		if ($subterm_tree) {
			foreach ($subterm_tree as $subterm_term) {
				$tids[$subterm_term->tid] = $subterm_term->tid;
			}
		}
	}

	if ($tids) {
		$query->join('taxonomy_entity_index', 'ti', $join_condition, $join_params);
		$query->condition('ti.tid', $tids);
	}
	elseif (!empty($catalog_object->vid) && empty($catalog_object->current_params['term']->tid)) {
		// Show unattached entities in root of catalog
		// SLOW QUERY ON ALOE39.RU (~15000 nodes)
		// LEFT JOIN NEEDS TO OPTIMIZATION
		// OLD:
		/*$query->leftJoin('taxonomy_entity_index', 'ti', 'ti.entity_id = ' . $base_table_alias . '.' . $base_table_alias . 'id AND ti.entity_type = :entity_type', array(':entity_type' => $catalog_object->entity_type));
		$query->condition('ti.tid', NULL);*/

		$query->leftJoin('taxonomy_entity_index', 'ti', $join_condition, $join_params);
		$query->isNull('ti.tid');
	}

	// Collect personal joins for specific entity type/bundle filtration
	foreach (array_filter(array_keys(array_flip(array($catalog_object->order_flag,)))) as $flag) {
		foreach (catalog_collate_sql($catalog_object->entity_type, $catalog_object->bundle, $flag, 'join', 'params', $catalog_object) as $key => $joins) {
			call_user_func_array(array($query, 'addJoin'), $joins);
		}
	}
	// Collect personal wheres for specific entity type/bundle filtration
	foreach (catalog_collate_sql($catalog_object->entity_type, $catalog_object->bundle, NULL, 'filter', 'params', $catalog_object) as $key => $conditions) {
		foreach ($conditions as $condition) {
			call_user_func_array(array($query, 'where'), $condition);
		}
	}

	// More tags for using in other modules (see hook_query_(TAG_)alter())
	$query->addTag('catalog_select_entities');
  $query->addTag('catalog_object:' . $catalog_object->cid);
	$query->{'#catalog_object'} = $catalog_object;

	// Limit for selected entities. If table_view is enabled, we ignore cols count, else limit = cols*rows
	$limit = ($catalog_object->table_view ? 1 : $catalog_object->cols) * $catalog_object->rows;

	if ($pager) {
		$count_query = clone $query;
		$count_query->addExpression('COUNT(DISTINCT ' . $base_table_alias . '.' . $base_table_key . ')');
		$query = $query->extend('PagerDefault')->element(_catalog_pager_element($catalog_object->cid));
		$query = $query->limit($limit);
		$query->setCountQuery($count_query);
	} else {
		$query->range(0, $limit);
	}

	$query->addField($base_table_alias, $base_table_key);

	$query->distinct(TRUE);
	if ($catalog_object->order_flag !== 'random') {
		foreach ($order as $field => $direction) {
			$query->orderBy($field, $direction);
		}
	}
	//select entities and order by field
	//I see no other ways to extend db_select fith EntityFieldQuery
	if (isset($sort_field)) {
		$direction = isset($_GET['sort']) ? $_GET['sort'] : NULL;
		//get order conditions for adding field condition primary
		$order =& $query->getOrderBy();
		//get field column name for sorting
		$field = field_info_field($sort_field);
		$storage = $field['storage']['details']['sql'];
		foreach (reset($storage) as $table => $keys) {
			$query->rightJoin($table, 'sort_field', $base_table_alias . '.' . $base_table_key . ' = sort_field.entity_id');
			$query->condition('sort_field.entity_type', $catalog_object->entity_type);
			$query->condition('sort_field.bundle', $catalog_object->bundle);
			if (isset($keys['value'])) {
				$order = array(
					$keys['value']	=> $direction,
				) + $order;
			}
			//if field doesn't have 'value' column use first column in storage
			else {
				$order = array(
					reset($keys)	=> $direction,
				) + $order;
			}
		}
	}
	$result['entities'] = $query->execute()->fetchCol();
	if ($pager) {
		global $pager_page_array, $pager_total;
		$result['pager_page_array'] = $pager_page_array[_catalog_pager_element($catalog_object->cid)];
		$result['pager_total'] = $pager_total[_catalog_pager_element($catalog_object->cid)];
	}
	// Cache result
	if ($cache_id) {
		cache_set($cache_id, $result, 'cache_catalog_entities', $material_actuality
					? REQUEST_TIME + (3600 * 24 * $material_actuality) : CACHE_PERMANENT);
	}
	// More shuffle for random selected entities
	if ($catalog_object->order_flag === 'random') {
		shuffle($result['entities']);
	}
//	var_dump(timer_read('_catalog_select_entities'));
	return $result['entities'];
}




/**
* Implements hook_catalog_collate_sql()
*/
function catalog_catalog_collate_sql($entity_type, $bundle) {
	if ($entity_type == 'node') {
		return array(
		'node_created_desc' => array(
			'order' => array(
				'title' => t('Created date'),
				'params' => array(
					'n.created' => 'DESC',
					),
				),
			),
		'node_created' => array(
			'order' => array(
				'title' => t('Created date, oldest first'),
				'params' => array(
					'n.created' => 'ASC',
					),
				),
			),
		'node_changed_desc' => array(
			'order' => array(
				'title' => t('Changed date'),
				'params' => array(
					'n.changed' => 'DESC',
					),
				),
			),
		'node_changed' => array(
			'order' => array(
				'title' => t('Changed date, oldest first'),
				'params' => array(
					'n.changed' => 'ASC',
					),
				),
			),
		'node_title' => array(
			'order' => array(
				'title' => t('Alphabetical'),
				'params' => array(
					'n.title' => 'ASC',
					),
				),
			),
		'node_title_desc' => array(
			'order' => array(
				'title' => t('Alphabetical') . ', ' . t('reverse order'),
				'params' => array(
					'n.title' => 'DESC',
					),
				),
			),
/*		'comment_last' => array(
			'order' => array(
				'title' => t('Last comment'),
				'params' => array(
					'ncs.last_comment_timestamp' => 'DESC',
					),
				),
			),*/
		'random' => array(
			'order' => array(
				'title' => t('Random (can slow down the server)'),
				'params' => array(
					'RAND()' => 'ASC',
					),
				),
			),

		);
	}
	if ($entity_type == 'user') {
		return array(
		'user_name' => array(
			'order' => array(
				'title' => t('Name'),
				'params' => array(
					'u.name' => 'ASC',
					),
				),
			),
		'user_mail' => array(
			'order' => array(
				'title' => t('Email'),
				'params' => array(
					'u.mail' => 'ASC',
					),
				),
			),
		'user_created' => array(
			'order' => array(
				'title' => t('Account creation date'),
				'params' => array(
					'u.created' => 'ASC',
					),
				),
			),
		'user_access' => array(
			'order' => array(
				'title' => t('Account last access date/time'),
				'params' => array(
					'u.access' => 'ASC',
					),
				),
			),
		);
	}
}


function catalog_collate_sql($entity_type = NULL, $bundle = NULL, $id = NULL, $op = NULL, $field = NULL, $catalog_object = NULL) {
	$result = module_invoke_all('catalog_collate_sql', $entity_type, $bundle, $catalog_object);

	if (isset($field)) {
		foreach ($result as $idx => $data) {
			foreach ($data as $idx2 => $data2) {
				$result[$idx][$idx2] = isset($data2[$field]) ? $data2[$field] : NULL;
			}
			$result[$idx] = array_filter($result[$idx]);
		}
	}
	if (isset($op)) {
		foreach ($result as $idx => $data) {
			$result[$idx] = isset($data[$op]) ? $data[$op] : NULL;
		}
		$result = array_filter($result);
	}
	if (isset($id)) {
		$result = !empty($result[$id]) ? array_filter($result[$id]) : NULL;
	}
	return $result ? $result : array();
}




function business_catalog_collate_sql($entity_type, $bundle) {
	if ($entity_type == 'user') {
		return array(
			'user_is_leader' => array(
				'order' => array(
					'title' => t(' Head'),
					'params' => array(
						'bcw.leader' => 'DESC',
						),
				),
			),
		);
	}
}


/**
 * Implements hook_catalog_collate_sql()
 *
 * Add custom sorting option for catalogs with task bundles
 */
function task_catalog_collate_sql($entity_type, $bundle, $catalog_object) {
	$result = array();
	if ($entity_type == 'node' && task_get_settings($bundle)) {
		$result['task_IKW_sorting']['order'] = array(
			'title'  => t('completed, priority, deadline'),
			'params' => array(
				'ta.completed' => 'ASC',
				'ta.priority'  => 'DESC',
				'ta.deadline'  => 'DESC',
			),
		);

		$result['task_IKW_sorting']['join']['params'] = array(array('INNER', 'task', 'ta', 'ta.nid = n.nid'),);
	}

	return $result;
}



/**
 * Implements hook_catalog_collate_sql()
 */
function node_event_catalog_collate_sql($entity_type, $bundle, $catalog_object) {
//  return;
	$result = array();
	if ($entity_type == 'node' && $bundle == 'event') {
		$time = REQUEST_TIME;

		$result['event_dates']['order'] = array(
			'title'  => t('Date of the event'),
			'params' => array(
				'IF(ed.event_date_date1 IS NULL, NULL, IF(ed.event_date_date2 > ' . $time . ', 0, 1))'                    => 'ASC',
				'IF(ed.event_date_date1 IS NULL, NULL, IF(ed.event_date_date2 > ' . $time . ', ed.event_date_date1, 1))'  => 'ASC',
				'IF(ed.event_date_date1 IS NULL, NULL, IF(ed.event_date_date2 <= ' . $time . ', ed.event_date_date2, 1))' => 'DESC',
			),
		);

		$result['event_dates']['join']['params'] = array(
			array('LEFT', 'field_data_event_date', 'ed', 'ed.entity_id = n.nid',),
		);

		if (!empty($catalog_object) && $catalog_object->topicality && $catalog_object->order_flag == 'event_dates') {
			$result['event_dates']['filter']['params'][] = array('ed.event_date_date2 > :time', array(':time' => $time));
		}
	}

	return $result;
}


/**
* Implements hook_catalog_collate_sql()
*/
function social_profile_catalog_collate_sql($entity_type, $bundle) {
	if ($entity_type == 'user') {
		return array(
			'user_lfm' => array(
				'order' => array(
					'title' => t('L.F.M.'),
					'params' => array(
						'u.lfm' => 'ASC',
						),
				),
			),
			'user_lfm_desc' => array(
				'order' => array(
					'title' => t('L.F.M.') . ', ' . t('reverse order'),
					'params' => array(
						'u.lfm' => 'DESC',
						),
				),
			),
			'user_phone' => array(
				'order' => array(
					'title' => t('Phone'),
					'params' => array(
						'u.phone' => 'ASC',
						),
				),
			),
			'user_phone_desc' => array(
				'order' => array(
					'title' => t('Phone') . ', ' . t('reverse order'),
					'params' => array(
						'u.phone' => 'DESC',
						),
				),
			),
		);
	}
}


/**
 * Implements hook_query_TAG_alter()
 * -catalog_select_entities
 */
function DEPRECATED_new_price_query_catalog_select_entities_alter(QueryAlterableInterface $query) {
	// Get for full catalog object placed as #property in $query.

	$catalog_object = $query->getMetaData('catalog_object');
	
	if (!empty($catalog_object->old_price_filter)) {
		$entity_info = entity_get_info($catalog_object->entity_type);
		$entity_id_field = $entity_info['entity keys']['id'];
		$table_alias = substr($catalog_object->entity_type, 0, 1); //not 100%

		$query->rightJoin('shop_index', 'si', 'si.entity_id = ' . $table_alias . '.' . $entity_id_field);
		$query->condition('si.old_price', 0, '<>');
	}
}



/**
 * Implements hook_query_TAG_alter()
 * - catalog_select_entities
 */
function DEPRECATED_shop_variants_query_catalog_select_entities_alter(QueryAlterableInterface $query) {
	if ($is_pager_query = (get_class($query) !== 'PagerDefault')) { // Perverted logic for determine pager query.
		$catalog_object = $query->{'#catalog_object'};
	}
	else {
		$catalog_object = $query->getCountQuery()->{'#catalog_object'};
	}

	if ($catalog_object->entity_type != 'node' || $catalog_object->bundle != 'product' || $query->hasTag('do_not_alter')) {
		return;
	}

	//leave only 1 node, that have variants
	if (empty($catalog_object->show_variants)) {
		$query->condition(db_or()->isNull('n.variant')->where('n.variant = n.nid'));
	}
}




function _catalog_entity_view($object, $entity_type, $id) {
	field_attach_prepare_view($entity_type, array($id => $object), 'full');

	entity_prepare_view($entity_type, array($id => $object));

	$build = array(
		'#view_mode' => $entity_type != 'taxonomy_term' ? NULL : 'full', //?????!
		//'#language' => $GLOBALS['language_content']->language,
	);
	if ($entity_type === 'taxonomy_term') {
		$build['#term'] = $object;
	}
	$build += field_attach_view($entity_type, $object, 'full', $GLOBALS['language_content']->language);
	if ($entity_type != 'node_type' && ($data = $entity_type == 'taxonomy_term'	? check_markup($object->description, isset($object->format)	? $object->format : NULL, '', TRUE) : filter_xss_admin($object->description))) {
		$build['description'] = array(
			'#type' => 'item',
			'#title' => t('Description'),
			'#markup' => $data,
			//'#weight' => 0,
			'#prefix' => '<div class="' . str_replace('_', '-', $entity_type) . '-description">',
			'#suffix' => '</div>',
		);
	}
	drupal_alter(array($entity_type . '_view', 'entity_view'), $build, $entity_type);
	return $build;
}



