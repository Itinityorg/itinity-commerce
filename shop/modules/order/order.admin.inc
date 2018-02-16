<?php

define('ORDER_STATISTIC_DAY', 60 * 60 * 24);
define('ORDER_STATISTIC_WEEK', ORDER_STATISTIC_DAY * 7);
define('ORDER_STATISTIC_MONTH', ORDER_STATISTIC_DAY * 30);
define('ORDER_STATISTIC_MAX_LIMIT', 60); // Max size of X-axis

/**
 * Orders overview form
 *
 * @see theme_shop_orders_form()
 */
function shop_orders_form($form, &$form_state, $account = NULL) {
	// table header
	$header = array(
		'created' => array('data' => t('Created'), 'field' => 'n.created', 'sort' => 'desc',),
		'order_num' => array('data' => t('#'), 'field' => 'so.order_num',),
		'items_titles' => array('data' => t('Name', array(), array('context' => 'product')), 'field' => 'items_titles',),
		'items_count' => array('data' => t('Positions', array(), array('context' => 'count',)), 'field' => 'items_count',),
		'total_sum' => array('data' => t('Sum'), 'field' => 'total_sum',),
		'delivery_method' => array('data' => t('Delivery'), 'field' => 'so.delivery_method',),
	);
	if (module_invoke('status', 'get_settings', 'node', 'order')) {
		$header['status_field'] = array('data' => t('Implementation status'), 'field' => 'sd.status_id',);
	}
	$header += array(
		'is_paid' => array('data' => t('Payment status'), 'field' => 'is_paid',),
		'uid' => array('data' => t('Customer'), 'field' => module_exists('social_profile') ? 'u.lfm' : 'n.uid',),
		'actions' => array(),
	);

  if ($account) {
    unset($header['uid']);
  }
	$form['#tree'] = TRUE;
	$form['#header'] = $header; // @see theme_shop_orders_form()
	$form['#attached']['js'][] = drupal_get_path('module', 'order') . '/js/order.orders.js';
	$form['#attached']['css'][] = drupal_get_path('module', 'order') . '/css/order.admin.css';
	$form['orders'] = array();

	$values = drupal_get_query_parameters(NULL, array('q', 'submit', 'page',));
  foreach (array_keys($values) as $key) {
    if (!in_array($key, array('created_from', 'created_to', 'order_num', 'items_titles', 'total_sum', 'uid', 'is_paid', 'status_field',), TRUE)) {
      unset($values[$key]);
    }
  }
  if ($account) {
    unset($values['uid']);
  }

	// filters row
	$form['orders']['filters'] = array_combine(array_keys($header), array_pad(array(), count($header), array())); // create array with keys form $header
	$form['orders']['filters']['created'] = array(
		'#type' => 'container',
	//	'#attributes' => array('class' => array('date-element-wrapper'),),
		'created_from' => array(
			'#type' => 'datetime', //'datepicker',
			'#group' => 'from-to',
			'#default_value' => !empty($values['created_from']) ? $values['created_from'] : '',
		),
		'created_to' => array(
			'#type' => 'datetime', //'datepicker',
			'#group' => 'from-to',
			'#default_value' => !empty($values['created_to']) ? $values['created_to'] : '',
		),
	);
	$form['orders']['filters']['order_num'] = array(
		'#type' => 'textfield',
		'#size' => 3,
		'#default_value' => !empty($values['order_num']) ? $values['order_num'] : '',
	);
	$form['orders']['filters']['items_titles'] = array(
		'#type' => 'textfield',
		'#default_value' => !empty($values['items_titles']) ? $values['items_titles'] : '',
	);
	$form['orders']['filters']['total_sum'] = array(
		'#type' => 'digit',
    '#float' => TRUE,
    '#min' => 0,
    '#step' => 0.01,
		'#default_value' => !empty($values['total_sum']) ? $values['total_sum'] : '',
	);

  if (!$account) {
    $form['orders']['filters']['uid'] = array(
      '#type' => 'entity_selector',
      '#selector_entity_type' => 'user',
      '#size' => 10,
      '#default_value' => !empty($values['uid']) ? $values['uid'] : NULL,
    );
  }


	$form['orders']['filters']['is_paid'] = array(
		'#type' => 'select',
		'#options' => array('paid' => t('Paid'), 'not_paid' => t('Not paid'),),
		'#empty_value' => '',
		'#empty_option' => '',
		'#default_value' => !empty($values['is_paid']) ? $values['is_paid'] : array(),
	);
	$form['orders']['filters']['actions'] = array(
		'#type' => 'actions',
		'submit' => array(
			'#type' => 'submit',
			'#value' => t('Filter'),
			'#submit' => array('shop_orders_form_submit'),
			'#attributes' => array('class' => array('js-hide', 'submit',)),
		),
	);
	if (!empty($values)) {
		$form['orders']['filters']['actions']['reset'] = array(
			'#type' => 'submit',
			'#value' => t('Reset'),
			'#submit' => array('shop_orders_form_reset'),
		);
	}
	if (module_invoke('status', 'get_settings', 'node', 'order')) {
		$statuses = status_load_statuses('node', 'order');
		if ($statuses) {
			$form['orders']['filters']['status_field'] = status_get_status_field_form('node', 'order');
			$form['orders']['filters']['status_field'] = $form['orders']['filters']['status_field']['id'];
			$form['orders']['filters']['status_field']['#title'] = '';
			$form['orders']['filters']['status_field']['#default_value'] = !empty($values['status_field']) ? $values['status_field'] : array();
			$form['orders']['filters']['status_field']['#empty_value'] = '';
			$form['orders']['filters']['status_field']['#empty_option'] = '';
		}
	}

	// build orders query
	$query = db_select('node', 'n')->addTag('node_access')->addTag('order_list_admin');
	$query->join('shop_cart_items_ordered', 'scio', 'scio.oid = n.nid');
	$query->join('shop_orders', 'so', 'so.nid = n.nid');
	$query->leftJoin('shop_index', 'si', 'si.entity_type = scio.entity_type AND si.entity_id = scio.entity_id');
	if (module_exists('social_profile')) {
		$query->join('users', 'u', 'u.uid = n.uid');
		$query->fields('u', array('lfm'));
	}
	if (module_invoke('status', 'get_settings', 'node', 'order')) {
		$query->leftJoin('status_data', 'sd', 'sd.entity_type = \'node\' AND sd.entity_id = n.nid');
		$query->fields('sd', array('status_id'));
		$last_status_id = status_get_last_status_id('node', 'order');
    /*
    if (arg(3) === 'closed') { // last status & paid
			$query->condition('sd.status_id', $last_status_id);
      $query->condition('so.is_paid', 0, '>');
		}
		elseif(arg(3) === 'cancelled') { // node status = 0
		  $query->condition('n.status', 0);
    }
    */
    if (arg(3) === 'processed') {
      $query->condition(
        db_or()
          ->condition(db_and()
            ->condition('sd.status_id', $last_status_id)
            ->condition('so.is_paid', 0, '>'))
          ->condition('n.status', 0)
      );
    }
		else {
      $db_or = db_or();
      $db_or->condition('sd.status_id', $last_status_id, '<>')->isNull('sd.status_id')->condition('so.is_paid', 0);
			$query->condition($db_or);
			$query->condition('n.status', 0, '>');
		}
	}
	$query = $query->extend('PagerDefault');
	$query->extend('TableSort')->orderByHeader($header);
	$query->limit(50); // def is 50 !
	$query->fields('n', array('nid', 'uid', 'created', 'title', 'type',));
	$query->fields('scio', array('currency'));
	$query->fields('so', array('order_num', 'payment_method', 'is_paid', 'delivery_method', 'payment_timestamp',));
	$query_expressions = array(
		'items_count' => 'COUNT(*)',
		'total_sum' => 'SUM(scio.qty * (IF(scio.price IS NULL, si.price, scio.price)))',
		'items_titles' => 'GROUP_CONCAT(IF(scio.title IS NULL, si.title, scio.title))',
		'_is_paid' => 'so.is_paid * so.payment_timestamp',
	);
	foreach ($query_expressions as $expression_name => $expression_value) {
		$query->addExpression($expression_value, $expression_name);
	}
	$query->groupBy('nid');
	if ($account) {
		$query->condition(db_or()->condition('n.uid', $account->uid)->condition('so.customer', $account->uid));
	}

	// apply filters
  if (!empty($values['created_from']) || !empty($values['created_to'])) {
    $values += array(
      'created_from' => 0,
      'created_to' => 0x100000000,
    );
    $query->condition('n.created', array($values['created_from'], $values['created_to']), 'BETWEEN');
  }
	if (!empty($values['order_num'])) {
		$query->condition('so.order_num', $values['order_num']);
	}
	if (!empty($values['items_titles'])) {
		$query->having($query_expressions['items_titles'] . ' LIKE :item_title', array(':item_title' => '%' . db_like($values['items_titles']) . '%'));
	}
	if (!empty($values['total_sum'])) {
		$query->having($query_expressions['total_sum'] . '= :total_sum', array(':total_sum' => $values['total_sum']));
	}
	if (!empty($values['status_field'])) {
		$query->condition('sd.status_id', $values['status_field']);
	}
	if (!empty($values['is_paid'])) {
		$is_paid = $values['is_paid'] == 'paid' ? 1 : ($values['is_paid'] == 'not_paid' ? 0 : NULL);
		if ($is_paid !== NULL) {
			$query->condition('so.is_paid', $is_paid);
		}
	}
  if (!empty($values['uid'])) {
    $query->condition('n.uid', $values['uid']);
  }
	// order rows

  $result = $query->execute()->fetchAll();

  // show filters only if number of orders in list > 10
  $form['orders']['filters']['#access'] = !empty($values) || (count($result) > 10);

	foreach ($result as $r) {
		$order_node = node_load($r->nid);
		$form['#order_nodes'][$order_node->nid] = $order_node;

		$row = array_combine(array_keys($header), array_pad(array(), count($header), array()));
		$row['created'] = format_date($order_node->created, 'short');
		$row['order_num'] = l($order_node->order_num, 'node/' . $order_node->nid, array('attributes' => array('class' => array('ajax-popup',))));
		$row['items_titles'] = l(preg_replace('/\.\.\.$/', ' <strong>...</strong>', truncate_utf8(check_plain($r->items_titles), 60, TRUE, TRUE)), 'node/' . $order_node->nid, array('html' => TRUE,));
		$row['items_count'] = $r->items_count;

    $sum = array_sum(shop_build_price('order', $order_node, $order_node->uid));
		$row['total_sum'] = $sum ? theme('price', array('price' => $sum)) : '&nbsp;';
		$row['delivery_method'] = $order_node->delivery_method ? check_plain(shop_delivery_info($order_node, $order_node->delivery_method, 'title')) : '&nbsp;';
    $row['is_paid'] = array();
		if (!$order_node->is_paid) {
      $title = $order_node->payment_method ? money_sources_info($order_node->payment_method, NULL, 'title') : NULL;
      $title = $title ? $title : $order_node->payment_method;
      $row['is_paid'][] = t('Expected') . ($title ? ' <em>(' . $title . ')</em>' : '');
      if ($sum = shop_build_price('order', $order_node)) {
				if($order_node->payment_method) { // don't show payment widget link if payment method is not defined.
//					$source = $order_node->payment_method ? money_sources_info($order_node->payment_method, $order_node->uid) : NULL;
					$source = money_sources_info($order_node->payment_method, $order_node->uid);
					if ($money_payment_widget = money_payment_widget('order', $order_node->nid, array_sum($sum), $source)) {
            if (user_access('administer moneys')) {
              $money_payment_widget['#title'] = t('Make payment');
              $money_payment_widget['#attributes']['target'] = '_blank';
            }
						$row['is_paid'][] = drupal_render($money_payment_widget);
					}
        }
      }
		}
    else {
      // Order is paid
      $row['is_paid'][] = t('Paid');
      foreach (money_transaction_load_multiple(FALSE, array('operation' => 'order', 'operation_key' => $order_node->nid)) as $t) {
        $row['is_paid'][] = '(' .
        theme('entity_link', array(
          'entity_type' => 'money_transaction',
          'entity' => $t,
          'text' => format_date($t->timestamp, 'custom', 'd.m.y'),
          'attributes' => array('class' => array('ajax-popup',)),
          )) . ', ' .
        theme('username', array('account' => user_load($t->uid))) . ')';
      }
      if (count($row['is_paid']) == 1) {
        $row['is_paid'][] = '(' . format_date($order_node->payment_timestamp ? $order_node->payment_timestamp : $order_node->changed, 'custom', 'd.m.y') . ')';
      }
    }
		/*else {
			if (shop_access_order_payment_cancel($order_node)) {
				$row['is_paid'] .= '<br />' .  l(t('Cancel payment'), 'node/' . $order_node->nid . '/payment_cancel', array('query' => drupal_get_destination(), 'attributes' => array('class' => array('form-submit', 'ajax-popup'))));
			}
		}*/
    $row['is_paid'] = implode('<br />', $row['is_paid']);
    if (!$account) {
			$customer = shop_get_customer($order_node);
      $row['uid'] = theme('username', array('account' => $customer));
      if (!$order_node->uid && $order_node->user_ip) {
        // IKW show anonymous tracking id instead of IP
        if (!empty($order_node->tracking_id)) {
          $row['uid'] .= '<br/><small title="' . check_plain('Tracking ID') . '"><em>' . t('Tracking ID') . ': ' . check_plain($order_node->tracking_id) . '</em></small>';
        }
//        $row['uid'] .= '<br /><small title="' . check_plain('IP address') . '"><em>(' . check_plain($order_node->user_ip) . ')</em></small>';
      }
    }
		/*edit order*/
		$row['actions'][] = node_access('update', $order_node) ? l(theme('pictogram', array('icon' => 'pencil')), 'node/' . $order_node->nid . '/edit', array(
			'html' => TRUE,
			'attributes' => array(
				'title' => t('Edit'),
				'class' => array('edit'),
				/* @see addons.css */
			),
			'query' => drupal_get_destination()
		)) : '';

		/*delete order*/
		$row['actions'][] = node_access('delete', $order_node) ? l(theme('pictogram', array('icon' => 'trash')), 'node/' . $order_node->nid . '/delete', array(
			'html' => TRUE,
			'attributes' => array(
				'title' => t('Delete'),
				'class' => array('delete', 'ajax-popup'),
			),
			'query' => drupal_get_destination()
		)) : '';

		$row['actions'] = implode(' ', $row['actions']);

		if (module_invoke('status', 'get_settings', 'node', 'order')) {
			$status_field = status_view_status_field($order_node, array('title_display' => 'invisible', 'status_change_links' => STATUS_CHANGE_LINKS_NEXT,));
			$row['status_field'] = render($status_field['status_field']);
		}

		// wrap cells in #markup
		foreach ($row as $id => $cell) {
			$row[$id] = array('#markup' => $cell);
		}

		$form['orders'][$order_node->nid] = $row;
	}

	$form['pager']['#markup'] = theme('pager');
	$form['actions'] = array(
		'#type' => 'actions',
		'add_order' => array(
			'#type' => 'link',
			'#href' => 'node/add/order',
			'#options' => array('query' => array(drupal_get_destination())),
			'#title' => t('Add order'),
			'#access' => _order_node_add_access(), // node_access('create', 'order'),
			'#attributes' => array('class' => array('form-submit')),
		),
	);

	return $form;
}

function theme_shop_orders_form($variables) {
	$form = $variables['form'];
	$header = $form['#header'];
	$order_nodes = & $form['#order_nodes'];

	$output = '';
	if (!empty($form['orders'])) {
		$rows = array();
		foreach (element_children($form['orders']) as $id) {
			foreach (element_children($form['orders'][$id]) as $key) {
				$rows[$id]['data'][$key] = array('data' => render($form['orders'][$id][$key]),);
				if ($key == 'status_field') {
					$is_last_status = !empty($order_nodes[$id]->status_field['is_last_status']);
					$rows[$id]['data'][$key]['style'] = $is_last_status ? 'background-color:#B9D6AF;' : '';
					$rows[$id]['data'][$key]['title'] = $is_last_status && isset($order_nodes[$id]->status_field['name']) ? $order_nodes[$id]->status_field['name'] : NULL;
				}
				if ($key == 'is_paid') {
					$rows[$id]['data'][$key]['style'] = !empty($order_nodes[$id]->is_paid) ? 'background-color:#B9D6AF;' : '';
				}
			}
			if ($id == 'filters') {
				$rows[$id]['class'][] = 'filter-row';
			}
			elseif (!empty($order_nodes[$id])) {
				if (!$order_nodes[$id]->status) {
					$rows[$id]['class'][] = 'unpublished';
				}
			}
		}
    if (count($rows) <= 1) {
      $rows['empty'] = array(array(
        'colspan' => count($header),
        'data' => t('Orders not found'),
      ));
    }
		$output .= theme('table', array(
			'header' => $header,
			'rows' => $rows,
			'attributes' => array('id' => array('shop-orders-form'), 'width' => '100%'),
		));
	}

	$output .= drupal_render_children($form);

	return $output;
}

function shop_orders_form_submit($form, &$form_state) {
	if (!empty($form_state['values']['orders']['filters'])) {
		$filters = $form_state['values']['orders']['filters'];
		if (!empty($filters['created'])) {
			$filters += $filters['created'];
			unset($filters['created']);
		}
		unset($filters['actions']);
		$filters = array_filter($filters);
		$form_state['redirect'] = array($_GET['q'], array('query' => $filters,));
	}
}

function shop_orders_form_reset($form, &$form_state) {
	$form_state['redirect'] = $_GET['q'];
}

/**
 * @todo delete
 */
function orders_by_status_page() {
	$build = array();

	$statuses = status_load_statuses('node', 'order');
	if ($statuses) {
		$last_status_id = end($statuses)->id;
		$count_by_status = array_combine(array_keys($statuses), array_pad(array(), count($statuses), 0));

		$q = db_select('status_data', 'sd');
		$q->fields('sd', array('status_id'));
		$q->leftJoin('node', 'n', 'sd.entity_id = n.nid');
		$q->leftJoin('shop_orders', 'so', 'sd.entity_id = so.nid');
    $q->addTag('order_list_admin');
		$q->condition('sd.entity_type', 'node');
		$q->condition('n.type', 'order');
		//IKW: "Кроме снятых с публикации и уже закрытых ( есть исп и оплата)"
		$q->condition('n.status', 1, '>=');
		$q->where('NOT(sd.status_id = :last_status_id AND so.is_paid = 1)', array(':last_status_id' => $last_status_id));
		$q->addExpression('COUNT(*)', 'cnt');
		$q->groupBy('sd.status_id');
		$results = $q->execute();
		foreach ($results as $result) {
			$count_by_status[$result->status_id] = $result->cnt;
		}

		$graph_options = array(
			'title' => array('text' => '',),
			'chart' => array('type' => 'column',),
			'credits' => array('enabled' => FALSE,),
			'xAxis' => array('categories' => array(),),
			'yAxis' => array('title' => '',),
			'legend' => array('enabled' => FALSE,),
			'plotOptions' => array('series' => array('dataLabels' => array('enabled' => TRUE, 'format' => '<b>{point.y}</b>'))),
			'series' => array(),
		);
		$graph_data = array('name' => t('Count'), 'data' => array(),);
		foreach ($count_by_status as $status_id => $count) {
			$graph_options['xAxis']['categories'][] = $statuses[$status_id]->name;
			$graph_data['data'][] = (int) $count;
		}
		$graph_options['series'][] = $graph_data;

		$build[] = array(
			'#theme' => 'graph',
			'#options' => $graph_options,
		);
	}
	else {
		drupal_set_message(t('Order have no statuses'), 'warning');
	}

	return $build;
}

/**
 * @todo delete
 */
function orders_graph_form($form, &$form_state) {
	$values = drupal_get_query_parameters(NULL, array('q', 'submit', 'page',));

	module_load_include('inc', 'shop', 'shop.admin');
	$form += _shop_date_filter_for_graph_subform();

	$graph_options = array(
		'title' => array('text' => '',),
		'chart' => array('zoomType' => 'x'),
		'series' => array(),
		'credits' => array('enabled' => FALSE),
		'xAxis' => array(
			'type' => 'datetime',
			'title' => array('text' => 'X'),
		),
		'yAxis' => array(
			'title' => array('text' => 'Y'),
			'allowDecimals' => FALSE,
		),
		'legend' => array(
			'layout' => 'vertical',
			'align' => 'right',
			'verticalAlign' => 'middle',
			'borderWidth' => 0,
		),
	);

	$graph_data = array();
	// calculate orders count and group by grouping setting
	$q = db_select('node', 'n');
  $q->addTag('order_list_admin');
	$q->innerJoin('shop_orders', 'so', 'n.nid = so.nid');
	if (module_exists('status')) {
		$last_status_id = status_get_last_status_id('node', 'order');
		if ($last_status_id) {
			$q->leftJoin('status_data','sd','n.nid = sd.entity_id AND sd.entity_type = \'node\'');
			$q->fields('sd', array('status_id', 'last_status_timestamp',));
		}
	}
	$q->fields('n', array('created', 'status', 'changed',));
	$q->fields('so', array('is_paid', 'payment_timestamp',));
	$q->condition('n.type', 'order');
	if (!empty($values['form']) && !empty($values['to'])) {
		$q->condition('n.created', array($values['form'], $values['to']), 'BETWEEN');
	}
	$q->orderBy('n.created');
	$data = $q->execute();
	$results = array();
	foreach ($data as $_data) {
		$results[] = $_data;
	}
	unset($data);
	if (!empty($results)) {
		$graph_data['created'] = array('name' => t('Created'), 'data' => array());
		$graph_data['canceled'] = array('name' => t('Canceled'), 'data' => array());
		$graph_data['closed'] = array('name' => t('Closed'), 'data' => array());
		$graph_data['inwork'] = array('name' => t('In work'), 'data' => array());

		// build date interval
		$min_created = PHP_INT_MAX;
		foreach ($results as $result) {
			$min_created = min($min_created, $result->created);
		}
		$begin_ts = !empty($values['form']) ? +$values['form'] : $min_created;
		$end_ts = !empty($values['to']) ? '@' . +$values['to'] : 'now';
		$items_by_interval = array('created' => array(), 'canceled' => array(), 'closed' => array(), 'inwork' => array(),);
		$begin = new DateTime('@' . $begin_ts);
		$begin->setTime(0, 0, 0);
		$end = new DateTime($end_ts);
		$interval = new DateInterval(!empty($values['grouping']) ? $values['grouping'] : 'P1M');
		$daterange = new DatePeriod($begin, $interval, $end);
		foreach ($daterange as $idx => $date) {
			$next_date = clone $date;
			$next_date->add($interval);
			$date_ts = $date->getTimestamp();
			$next_date_ts = $next_date->getTimestamp();
			$items_by_interval['created'][$date_ts] = 0;
			$items_by_interval['canceled'][$date_ts] = 0;
			$items_by_interval['closed'][$date_ts] = 0;
			$items_by_interval['inwork'][$date_ts] = 0;
			foreach ($results as $result) {
				// count created orders
				if ($result->created >= $date_ts && $result->created <= $next_date_ts) {
					$items_by_interval['created'][$date_ts]++;
				}
				// count canceled orders (status == 0)
				if (!$result->status && $result->changed >= $date_ts && $result->changed <= $next_date_ts) {
					$items_by_interval['canceled'][$date_ts]--; // IKW: for inverted graph
				}
				// count closed orders (is_paid == 1 and last_status(if installed))
				if ($result->is_paid) {
					if (module_exists('status')) {
						if ($result->last_status_timestamp >= $date_ts && $result->last_status_timestamp <= $next_date_ts) {
							$items_by_interval['closed'][$date_ts]--;
						}
					}
					else {
						if ($result->payment_timestamp >= $date_ts && $result->payment_timestamp <= $next_date_ts) {
							$items_by_interval['closed'][$date_ts]--;
						}
					}
				}
				// count inwork orders
				// @todo:
			}
		}

		if ($items_by_interval) {
			foreach ($items_by_interval as $type => $interval_values) {
				foreach ($interval_values as $timestamp => $items_count) {
					$graph_data[$type]['data'][] = array($timestamp * 1000, $items_count); // *1000 needed for JS timestamps
				}
			}
		}
	}
	$graph_options['series'][] = $graph_data['created'];
	$graph_options['series'][] = $graph_data['canceled'];
	$graph_options['series'][] = $graph_data['closed'];
//	$graph_options['series'][] = $graph_data['inwork']; // @todo:

	$form['graph_container'] = array(
		'#theme' => 'graph',
		'#options' => $graph_options,
		'#weight' => 100,
	);

	return $form;
}

function orders_graph_form_submit($form, &$form_state) {
	$query = array_filter($form_state['values']['settings']);
	unset($query['actions']);
	$form_state['redirect'] = array($_GET['q'], array('query' => $query,));
}

function orders_graph_form_reset_submit($form, &$form_state) {
	$form_state['redirect'] = $_GET['q'];
}


/**
 * New statistics
 * @see https://itinity.ru/node/2056
 */
function orders_statistics_page($mode = 'graph') {
  $build = array();

  $filters = order_statistics_get_filters();

  $filter_form = drupal_get_form('order_statistics_filter_form', $filters);
  $build['filter'] = $filter_form;

  $data = order_statistics_data($filters);

  switch ($mode) {
    case 'graph':
      $build['content'] = order_statistics_graph($data);
      break;
      // @todo
//    case 'table':
//      $build['content'] = array();
//      break;
//    default:
//      $build['content'] = array();
//      break;
  }

  return $build;
}


/**
 * Filters form
 * 'scale' filter depends on 'period' filter
 *
 * 'Daily' scale is available for period 1 - 60 days
 * 'Weekly' scale is available for period 7 - 420 days (or 1 - 60 weeks)
 * 'Monthly' scale is available for period 30 - 1800 days (or 1 - 60 month)
 */

function order_statistics_filter_form($form, &$form_state, $filters = array()) {
  $form['#attached']['js'][] = drupal_get_path('module', 'order') . '/js/order.statistics.js';

  $days = array(0 => 'Sunday', 1 => 'Monday', 2 => 'Tuesday', 3 => 'Wednesday', 4 => 'Thursday', 5 => 'Friday', 6 => 'Saturday');
  $first_day = variable_get('date_first_day', 1);
  $last_day = ($first_day + 6) % 7;

  $week_period_start = date('Y-m-d', strtotime("This week {$days[$first_day]}", REQUEST_TIME));
  $week_period_end = date('Y-m-d', strtotime("This week {$days[$last_day]}", REQUEST_TIME));

  $month_period_start = date('Y-m-d', strtotime("First day of this month", REQUEST_TIME));
  $month_period_end = date('Y-m-d', strtotime("Last day of this month", REQUEST_TIME));

  $filters = order_statistics_get_filters();
  $period = strtotime($filters['to']) - strtotime($filters['from']);

  $form['period'] = array(
    '#type' => 'container',
    '#inline' => TRUE,
    'title' => array(
      '#markup' => t('Period') . ': ',
      '#attributes' => array(
        'class' => array('bold')
      ),
    ),
    'week' => array(
      '#type' => 'link',
      '#title' => t('Current week'),
      '#href' => 'admin/shop/orders/statistics',
      '#options' => array(
        'query' => array(
          'from' => $week_period_start,
          'to' => $week_period_end,
          'scale' => 'daily',
        ),
      ),
    ),
    'month' => array(
      '#prefix' => ' / ',
      '#type' => 'link',
      '#title' => t('Current month'),
      '#href' => 'admin/shop/orders/statistics',
      '#options' => array(
        'query' => array(
          'from' => $month_period_start,
          'to' => $month_period_end,
          'scale' => ($filters['scale'] != 'monthly') ? $filters['scale'] : 'weekly',
        ),
      ),
    ),
    'strict_link' => array(
      '#prefix' => ' / ',
      '#type' => 'link',
      '#href' => 'admin/shop/orders/statistics',
      '#title' => t('Strict'),
      '#id' => 'orders-statistics-strict',
    ),
    'strict' => array(
      '#type' => 'container',
      '#inline' => TRUE,
      '#attributes' => array(
        'style' => 'display: none',
      ),
      'from' => array(
        '#type' => 'datetime',
        '#group' => 'from-to',
        '#default_value' => strtotime($filters['from']),
      ),
      'to' => array(
        '#prefix' => ' - ',
        '#type' => 'datetime',
        '#group' => 'from-to',
        '#default_value' => strtotime($filters['to']),
      ),
      'submit' => array(
        '#type' => 'submit',
        '#value' => t('Submit'),
      ),
    ),
  );

  $form['scale'] = array(
    '#type' => 'container',
    '#inline' => TRUE,
    'title' => array(
      '#markup' => t('Scale', array(), array('context' => 'orders-statistics')) . ': ',
    ),
    'daily' => array(
      '#type' => 'link',
      '#title' => t('Per day'),
      '#href' => 'admin/shop/orders/statistics',
      '#options' => array(
        'query' => array('scale' => 'daily') + $filters,
      ),
      '#access' => _order_statistics_filter_form_scale_access($filters['from'], $filters['to'], 'daily'),
    ),
    'sep1' => array(
      '#markup' => ' / ',
      '#access' => _order_statistics_filter_form_scale_access($filters['from'], $filters['to'], 'daily') && _order_statistics_filter_form_scale_access($filters['from'], $filters['to'], 'weekly'),
    ),
    'weekly' => array(
      '#type' => 'link',
      '#title' => t('Per week'),
      '#href' => 'admin/shop/orders/statistics',
      '#options' => array(
        'query' => array('scale' => 'weekly') + $filters,
      ),
      '#access' => _order_statistics_filter_form_scale_access($filters['from'], $filters['to'], 'weekly'),
    ),
    'sep2' => array(
      '#markup' => ' / ',
      '#access' => _order_statistics_filter_form_scale_access($filters['from'], $filters['to'], 'weekly') && _order_statistics_filter_form_scale_access($filters['from'], $filters['to'], 'monthly'),
    ),
    'monthly' => array(
      '#type' => 'link',
      '#title' => t('Per month'),
      '#href' => 'admin/shop/orders/statistics',
      '#options' => array(
        'query' => array('scale' => 'monthly') + $filters,
      ),
      '#access' => _order_statistics_filter_form_scale_access($filters['from'], $filters['to'], 'monthly'),
    ),
  );

  if ($filters['from'] == $week_period_start && $filters['to'] == $week_period_end) {
    $form['period']['week']['#attributes']['class'][] = 'set-active';
  }
  elseif ($filters['from'] == $month_period_start && $filters['to'] == $month_period_end) {
    $form['period']['month']['#attributes']['class'][] = 'set-active';
  }
  else {
    unset($form['period']['strict']['#attributes']['style']);
    $form['period']['strict_link']['#attributes']['class'][] = 'set-active';
    $form['period']['strict']['from']['#default_value'] = $filters['from'];
    $form['period']['strict']['to']['#default_value'] = $filters['to'];
  }

  $form['scale'][$filters['scale']]['#attributes']['class'][] = 'set-active';

  return $form;
}



function _order_statistics_filter_form_scale_access($from, $to, $scale) {
  $scales = array();

  $from = is_numeric($from) ? $from : strtotime($from);
  $to = is_numeric($to) ? $to : strtotime($to);
  $period = $to - $from;

  if ((ORDER_STATISTIC_DAY <= $period) && ($period <= ORDER_STATISTIC_DAY * ORDER_STATISTIC_MAX_LIMIT)) {
    $scales[] = 'daily';
  }
  if ((ORDER_STATISTIC_WEEK <= $period) && ($period <= ORDER_STATISTIC_WEEK * ORDER_STATISTIC_MAX_LIMIT)) {
    $scales[] = 'weekly';
  }
  if ((ORDER_STATISTIC_MONTH <= $period) && ($period <= ORDER_STATISTIC_MONTH * ORDER_STATISTIC_MAX_LIMIT)) {
    $scales[] = 'monthly';
  }

  return in_array($scale, $scales);
}


function order_statistics_filter_form_validate($form, &$form_state) {
  $filters = order_statistics_get_filters();
  $values = &$form_state['values'];

  if (_order_statistics_filter_form_scale_access($values['from'], $values['to'], $filters['scale'])) {
    $values['scale'] = $filters['scale'];
  }
  else {
    foreach (array('monthly', 'weekly', 'daily') as $scale) {
      if (_order_statistics_filter_form_scale_access($values['from'], $values['to'], $scale)) {
        $values['scale'] = $scale;
        break;
      }
    }
  }

  if (empty($values['scale'])) {
    form_set_error('strict', t('Period is too large!'));
  }
}


function order_statistics_filter_form_submit($form, &$form_state) {
  $form_state['redirect'] = array(
    'admin/shop/orders/statistics',
    array(
      'query' => array(
        'from' => date('Y-m-d', $form_state['values']['from']),
        'to' => date('Y-m-d', $form_state['values']['to']),
        'scale' => $form_state['values']['scale'],
      )
    )
  );
}


function order_statistics_get_filters() {
  $filters = drupal_get_query_parameters();

  $filters += array(
    'from' => date('Y-m-d', strtotime("First day of this month", REQUEST_TIME)),  // month period start
    'to' => date('Y-m-d', strtotime("Last day of this month", REQUEST_TIME)), // month period end
    'scale' => 'daily',
  );

  return $filters;
}


function order_statistics_data($filters) {
  $from_timestamp = strtotime($filters['from']);
  $to_timestamp = strtotime($filters['to']) + 60 * 60 * 24 - 1;

  $query = db_select('node', 'n');
  $query->addTag('order_list_admin');
  $query->innerJoin('shop_orders', 'so', 'n.nid = so.nid');

  if (module_exists('status')) {
    $first_status_id = status_get_first_status_id('node', 'order');
    $last_status_id = status_get_last_status_id('node', 'order');
    if ($first_status_id && $last_status_id) {
      $query->leftJoin('status_data', 'sd', 'n.nid = sd.entity_id AND sd.entity_type = \'node\'');
      $query->fields('sd', array('status_id'));
    }
  }

  $query->fields('n', array('created', 'status',));
  $query->fields('so', array('is_paid',));
  $query->condition('n.type', 'order');
  $query->condition('n.created', array($from_timestamp, $to_timestamp), 'BETWEEN');
  $query->orderBy('n.created');

  $periods = array();

  if ($filters['scale'] == 'daily') {
    $date_format = 'd.m.Y';
    $interval = new DateInterval('P1D');
  }
  elseif ($filters['scale'] == 'weekly') {
    $date_format = 'W.Y';
    $interval = new DateInterval('P7D');
  }
  else { // Monthly
    $date_format = 'F Y';
    $interval = new DateInterval('P1M');
  }

  $iterator = new DateTime($filters['from']);

  while ($iterator->getTimestamp() <= strtotime($filters['to'])) {
    $periods[] = $iterator->format($date_format);
    $iterator->add($interval);
  }

  foreach (status_load_statuses('node', 'order') as $status_id => $status) {
    $results[$status_id] = array(
      'name' => $status->name,
      'data' => array_fill_keys($periods, 0),
      'color' => $status->color,
    );
  }

  foreach ($query->execute() as $row) {
    if (!empty($row->status_id)) {
      $period = date($date_format, $row->created);
      $results[$row->status_id]['data'][$period]++;
    }
  }

  return $results;
}


function order_statistics_graph($data) {
//  drupal_add_js('http://code.highcharts.com/modules/exporting.js', array('type' => 'external', 'scope' => 'footer', 'weight' => 998));
//  drupal_add_js('https://rawgithub.com/highslide-software/rounded-corners/master/rounded-corners.js', array('type' => 'external', 'scope' => 'footer', 'weight' => 999));

  $filters = order_statistics_get_filters();

  $build = array();

  $series = array();
  $x_axis = array();

  foreach ($data as $status_id => $series_item) {
    if (empty($x_axis)) {
      $x_axis = array_keys($series_item['data']);
    }

    $series[] = array(
      'name' => $series_item['name'],
      'data' => array_values($series_item['data']),
      'color' => $series_item['color'],
      'borderRadiusTopLeft' => 3,
      'borderRadiusTopRight' => 3,
    );
  }

  $graph_options = array(
    'chart' => array(
      'type' => 'column'
    ),
    'title' => array(
      'text' => t('Orders statistics')
    ),
    'xAxis' => array(
      'categories' => $x_axis,
      'labels' => array(
        'rotation' => -45,
      ),
    ),
    'yAxis' => array(
      'min' => 0,
      'title' => array(
        'text' => t('Number of orders'),
      ),
//      'stackLabels' => array(
//        'enabled' => TRUE,
//      ),
    ),
    'tooltip' => array(
      'headerFormat' => '<b>{point.x}</b><br/>',
      'pointFormat' => '{series.name}: {point.y}<br/>' . t('Total') . ': {point.stackTotal}'
    ),
    'plotOptions' => array(
      'column' => array(
        'stacking' => 'normal',
        'pointPadding' => 0,
        'groupPadding' => 0.1,
        'dataLabels' => array(
//          'enabled' => TRUE,
//          'color' => "(Highcharts.theme && Highcharts.theme.dataLabelsColor) || 'white'",
        ),
      ),
    ),
    'series' => $series,
  );

  $build['graph-wrapper'] = array(
    '#type' => 'container',
    'graph' => array(
      '#theme' => 'graph',
      '#options' => $graph_options,
      '#modules' => array('exporting.js'),
      '#plugins' => array('rounded-corners.js'),
    ),
  );

  return $build;
}