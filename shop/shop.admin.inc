<?php

/**
 * Cart analytics form
 */
function shop_carts_list_form($form, &$form_state) {
	module_load_include('inc', 'shop', 'shop.cart.class');

	$header = array(
		'created' => array('data' => t('Created'), 'field' => 'sc.created', 'sort' => 'desc',),
		'changed' => array('data' => t('Changed'), 'field' => 'sc.changed', 'sort' => 'desc',),
		'cart_num' => array('data' => t('# cart'), 'field' => 'sc.cart_id',),
		'cart_status' => array('data' => t('Status'),),
		'order_num' => array('data' => t('# order'),),
		'user' => array('data' => t('User'),),
		'cart_content' => array('data' => t('Cart content'),),
		'cart_sum' => array('data' => t('Sum'),),
		'notes' => array('data' => t('Notes'),)
	);
	$query = db_select('shop_cart', 'sc');
	$query->fields('sc', array('cart_id', 'uid',));
	$query = $query->extend('PagerDefault');
	$query->extend('TableSort')->orderByHeader($header);
	$query->limit(50);
	$rows = array();
	$row_template = array_fill_keys(array_keys($header), array('data' => '&nbsp;',));
	foreach ($query->execute() as $result) {
		$shop_cart = shop_cart($result->uid, NULL, $result->cart_id);
		$row = $row_template;
		$row['created']['data'] = format_date($shop_cart->created, 'short');
		$row['changed']['data'] = format_date($shop_cart->changed, 'short');
		$row['cart_num']['data'] = $shop_cart->cart_id;
		$row['user']['data'] = theme('username', array('account' => user_load($shop_cart->uid)));
		if (!empty($shop_cart->items)) {
			$item_links = array();
			foreach ($shop_cart->items as $cart_item) {
				$item_links[] = array(
					'title' => $cart_item->title . '</a> - <small>' . theme('price', array(
						'price' => $cart_item->price,
						'currency' => $cart_item->currency,
						'qty' => $cart_item->qty * 1,
						)) . '</small>',
					'html' => TRUE,
					'attributes' => array(
						'class' => array('popup'),
					),
					'href' => $cart_item->entity_uri,
				);
			}
			$row['cart_content']['data'] = array('#theme' => 'links', '#links' => $item_links,);
		}
		if ($shop_cart->total_sum) {
			$row['cart_sum']['data'] = theme('price', array('price' => $shop_cart->total_sum,));
		}
		$rows[] = $row;
	}

	$form['carts'] = array(
		'#theme' => 'table',
		'#header' => $header,
		'#rows' => $rows,
		'#tree' => TRUE,
	);
	$form['pager']['#markup'] = theme('pager');

	return $form;
}

function shop_carts_graph_form($form, &$form_state) {
	$values = drupal_get_query_parameters(NULL, array('q', 'submit', 'page',));

	$form += _shop_date_filter_for_graph_subform();

	$graph_options = array(
		'title' => array('text' => '',),
		'chart' => array('zoomType' => 'x'),
		'series' => array(),
		'credits' => array('enabled' => FALSE),
		'xAxis' => array(
			'type' => 'datetime',
			'title' => array('text' => t('Time')),
		),
		'yAxis' => array(
			'title' => array('text' => t('Number of new carts')),
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
  /*
	$q = db_select('node', 'n');
	$q->fields('n', array('created'));
	$q->condition('n.type', 'order');
	if (!empty($values['form']) && !empty($values['to'])) {
		$q->condition('n.created', array($values['form'], $values['to']), 'BETWEEN');
	}
	$data = $q->execute()->fetchCol();
  */

  $q = db_select('shop_cart', 'sc');
  $q->fields('sc', array('created'));
  if (!empty($values['from']) && !empty($values['to'])) {
    $q->condition('sc.created', array($values['from'], $values['to']), 'BETWEEN');
  }
  $data = $q->execute()->fetchCol();

	if (!empty($data)) {
		$graph_data['orders'] = array('name' => t('Carts'), 'data' => array());

    // build date interval
    $begin_ts = !empty($values['from']) ? $values['from'] : min($data);
		$end_ts = !empty($values['to']) ? $values['to'] : REQUEST_TIME;

    $items_by_interval = array();

    $begin = new DateTime();
    $begin->setTimestamp($begin_ts);

    $end = new DateTime();
    $end->setTimestamp($end_ts);

		$interval = new DateInterval(!empty($values['grouping']) ? $values['grouping'] : 'P1M');
		$daterange = new DatePeriod($begin, $interval, $end);
		foreach ($daterange as $date) {
			$next_date = clone $date;
			$next_date->add($interval);
			$date_ts = $date->getTimestamp();
			$next_date_ts = $next_date->getTimestamp();
			$items_by_interval[$date_ts] = 0;
			foreach ($data as $timestamp) {
				if ($timestamp >= $date_ts && $timestamp <= $next_date_ts) {
					$items_by_interval[$date_ts]++;
				}
			}
		}
		if ($items_by_interval) {
			foreach ($items_by_interval as $timestamp => $items_count) {
				$graph_data['orders']['data'][] = array($timestamp * 1000, $items_count); // *1000 needed for JS timestamps
			}
		}
	}
	$graph_options['series'][] = $graph_data['orders'];

	$form['graph_container'] = array(
		'#theme' => 'graph',
		'#options' => $graph_options,
		'#weight' => 100,
	);

	return $form;
}

function shop_payments_form($form, &$form_state){
//	$def_currency = variable_get('site_default_currency', 'RUB');
  $def_currency = shop_get_info('currency', 'default');

  /*
	$header = array(
		'date' => array('data' => t('Date'), 'field' => 'scio.changed', 'sort' => 'desc',),
		'total' => array('data' => t('Sum'), 'field' => 'total',),
		'order_num' => array('data' => t('Order number'), 'field' => 'so.order_num',),
		'payment_method' => array('data' => t('Payment method'), 'field' => 'payment_method',),
	);
*/

  $header = array(
    'date' => array('data' => t('Date')),
    'total' => array('data' => t('Sum')),
    'order_num' => array('data' => t('Order number')),
    'payment_method' => array('data' => t('Payment method')),
  );

	$rows = array();

  $q = db_select('shop_orders', 'so');
  $q->fields('so', array('nid'));
  $q->leftJoin('node', 'n', 'n.nid = so.nid');
  $q->condition('so.is_paid', 1);
  $q->orderBy('n.changed', 'DESC');
  $q = $q->extend('PagerDefault')->limit(50);
  $nids = $q->execute()->fetchCol();

  $row_template = array_fill_keys(array_keys($header), array());

  foreach(node_load_multiple($nids) as $nid => $node) {
    $customer = shop_get_customer($node);
    $price = array_sum(shop_build_price('order', $node, $customer));

    $row = $row_template;
    $row['date'] = format_date($node->changed, 'short');
    $row['total'] = theme('price', array('price' => $price, 'currency' => $def_currency));
    $row['order_num'] = l($node->order_num, "node/{$nid}", array('attributes' => array('class' => array('popup',))));
    $row['payment_method'] = money_sources_info($node->payment_method, NULL, 'title');
    $rows[] = $row;
  }

  /*
	$q = db_select('shop_cart_items_ordered', 'scio');
	$q->leftJoin('shop_index', 'si', 'si.entity_type = scio.entity_type AND si.entity_id = scio.entity_id');
	$q->leftJoin('shop_orders', 'so', 'scio.oid = so.nid');
	$q->condition('so.is_paid', 1);
	$q->fields('so', array('payment_method', 'order_num', 'nid',));
	$q->fields('scio', array('changed', 'currency',));
	$q->fields('si', array('currency',));
	$q->addExpression('SUM(scio.qty * (IF(scio.price IS NULL, si.price, scio.price)))', 'total');
//	$q->orderBy('scio.changed', 'DESC');
	$q->groupBy('scio.oid');
	$q = $q->extend('PagerDefault');
	$q->extend('TableSort')->orderByHeader($header);
	$q->limit(50);

	$row_template = array_fill_keys(array_keys($header), array());
	foreach ($q->execute() as $result) {
		$currency = !empty($result->currency) ? $result->currency : (!empty($result->si_currency) ? $result->si_currency : $def_currency);
		$row = $row_template;
		$row['date'] = format_date($result->changed, 'short');
		$row['total'] = theme('price', array('price' => $result->total, 'currency' => $currency));
		$row['order_num'] = l($result->order_num, 'node/' . $result->nid, array('attributes' => array('class' => array('popup',))));
    $money_source_info = money_sources_info($result->payment_method);
		$row['payment_method'] = !empty($money_source_info['title']) ? $money_source_info['title'] : $result->payment_method;
		$rows[] = $row;
	}
	*/

	$form['table'] = array(
		'#theme' => 'table',
		'#rows' => $rows,
		'#header' => $header,
	);
	$form['pager']['#markup'] = theme('pager');

	return $form;
}

function shop_payments_graph_form($form, &$form_state){
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
	$graph_data = array('type'=> 'area', 'name' => t(shop_get_info('currency', 'default')), 'data' => array());

  /*
	$q = db_select('shop_cart_items_ordered', 'scio');
	$q->leftJoin('shop_index', 'si', 'si.entity_type = scio.entity_type AND si.entity_id = scio.entity_id');
	$q->leftJoin('shop_orders', 'so', 'scio.oid = so.nid');
	$q->fields('scio', array('changed'));
	$q->condition('so.is_paid', 1);
	if (!empty($values['form']) && !empty($values['to'])) {
		$q->condition('scio.changed', array($values['form'], $values['to']), 'BETWEEN');
	}
	$q->addExpression('SUM(scio.qty * (IF(scio.price IS NULL, si.price, scio.price)))', 'total');
	$q->groupBy('scio.oid');
	$results = $q->execute();

  $data = array();
	foreach ($results as $result) {
		$data[] = $result;
	}
	unset($results);
  */

  $q = db_select('shop_orders', 'so');
  $q->fields('so', array('nid'));
  $q->leftJoin('node', 'n', 'n.nid = so.nid');
  $q->condition('so.is_paid', 1);
  $q->orderBy('n.changed', 'DESC');
  if (!empty($values['from']) && !empty($values['to'])) {
    $q->condition('n.changed', array($values['from'], $values['to']), 'BETWEEN');
  }
  $nids = $q->execute()->fetchCol();

  foreach (node_load_multiple($nids) as $nid => $node) {
    $customer = shop_get_customer($node);

    $data[] = (object) array(
      'changed' => $node->changed,
      'total' => array_sum(shop_build_price('order', $node, $customer)),
    );
  }


	if (!empty($data)) {
		// build date interval
		$min_changed = PHP_INT_MAX;
		foreach ($data as $_values) {
			$min_changed = min($_values->changed, $min_changed);
		}
		$begin_ts = $min_changed;
		$end_ts = 'now';
		$begin = new DateTime('@' . $begin_ts);
		$begin->setTime(0, 0, 0);
		$end = new DateTime($end_ts);
		$interval = new DateInterval(!empty($values['grouping']) ? $values['grouping'] : 'P1M');
		$daterange = new DatePeriod($begin, $interval, $end);
		$total_by_intervals = array();
		foreach ($daterange as $idx => $date) {
			$next_date = clone $date;
			$next_date->add($interval);
			$date_ts = $date->getTimestamp();
			$next_date_ts = $next_date->getTimestamp();
			$total_by_intervals[$date_ts] = 0;
			foreach ($data as $values) {
				if ($values->changed >= $date_ts && $values->changed <= $next_date_ts) {
					$total_by_intervals[$date_ts] += $values->total;
				}
			}
		}
		if ($total_by_intervals) {
			foreach ($total_by_intervals as $timestamp => $total) {
				$graph_data['data'][] = array($timestamp * 1000, $total); // *1000 needed for JS timestamps
			}
		}
	}
	$graph_options['series'][] = $graph_data;
	$form['graph'] = array(
		'#theme' => 'graph',
		'#options' => $graph_options,
	);

	return $form;
}

function _shop_date_filter_for_graph_subform() {
	$values = drupal_get_query_parameters(NULL, array('q', 'submit', 'page',));

	$subform['#tree'] = TRUE;
	$subform['filters'] = array(
		'#type' => 'container',
		'#attributes' => array('class' => array('container-inline',),),
		'from' => array(
			'#type' => 'datetime',
			'#group' => 'from-to',
			'#default_value' => !empty($values['from']) ? $values['from'] : '',
		),
		'to' => array(
			'#type' => 'datetime',
			'#group' => 'from-to',
			'#default_value' => !empty($values['to']) ? $values['to'] : '',
		),
		'grouping' => array(
			'#type' => 'select',
			'#title' => t('Group by'),
			'#options' => array(
				'P1D' => t('Day'),
				'P1W' => t('Week'),
				'P1M' => t('Month'),
			),
			'#default_value' => !empty($values['grouping']) ? $values['grouping'] : 'P1M',
		),
	);
	$subform['filters']['actions'] = array(
		'#type' => 'actions',
		'submit' => array(
			'#type' => 'submit',
			'#value' => t('Save'),
			'#attributes' => array('class' => array('submit',)),
			'#submit' => array('_shop_date_filter_for_graph_subform_submit'),
		),
	);
	if (!empty($values)) {
		$subform['filters']['actions']['reset'] = array(
			'#type' => 'submit',
			'#value' => t('Reset'),
			'#submit' => array('_shop_date_filter_for_graph_subform_reset'),
		);
	}
	return $subform;
}

function _shop_date_filter_for_graph_subform_submit($form, &$form_state) {
	$query = array_filter($form_state['values']['filters']);

  if(!empty($query['to'])) {
    $query['to'] += 60 * 60 * 24 - 1; // 23:59:59
  }

	unset($query['actions']);
	$form_state['redirect'] = array($_GET['q'], array('query' => $query,));
}

function _shop_date_filter_for_graph_subform_reset($form, &$form_state) {
	$form_state['redirect'] = $_GET['q'];
}
