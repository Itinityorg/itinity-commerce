<?php

function shop_admin_panel_page() {
	$build = array();
	$build['#theme'] = 'shop_admin_panel_page'; // @see theme_shop_admin_panel_page()

	// get today/yseterday timstamps
	$date = new DateTime();
	$today_ts = $date->setTime(0, 0, 0)->getTimestamp();
	$yesterday_ts = $date->modify('-1 day')->getTimestamp();
	unset($date);

	// sales funnel
	// @todo:
//	$build['sales_funnel'] = array(
//		'#type' => 'fieldset',
//		'#title' => t('Sales funnel'),
//	);

	// purchases dynamics
  if (module_exists('order')) {
    $build['purchases_dynamics'] = array(
      '#type' => 'fieldset',
      '#title' => l(t('Purchases dynamics'), 'admin/shop/orders'),
    );
  }
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
	$q->fields('n', array('created'));
	$q->condition('n.type', 'order');
	$data = $q->execute()->fetchCol();
	if (!empty($data)) {
		$graph_data['orders'] = array('type'=> 'area', 'name' => t('Orders'), 'data' => array());
		// build date interval
		$begin_ts = min($data);
		$end_ts = 'now';
		$items_by_interval = array();
		$begin = new DateTime('@' . $begin_ts);
		$begin->setTime(0, 0, 0);
		$end = new DateTime($end_ts);
		$interval_string = 'P1W';
		$interval = new DateInterval($interval_string);
		$daterange = new DatePeriod($begin, $interval, $end);
		foreach ($daterange as $idx => $date) {
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
			foreach ($items_by_interval as $timestamp => $total) {
				$graph_data['orders']['data'][] = array($timestamp * 1000, $total); // *1000 needed for JS timestamps
			}
		}
	}
	$graph_options['series'][] = isset($graph_data['orders']) ? $graph_data['orders'] : array();
	$build['purchases_dynamics']['graph'] = array(
		'#theme' => 'graph',
		'#options' => $graph_options,
	);

	// revenue dynamics
	$build['revenue_dynamics'] = array(
		'#type' => 'fieldset',
		'#title' => l(t('Revenue dynamics'), 'admin/shop/products/sales'),
	);
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
  if (module_exists('order')) {
    $graph_data = array('type'=> 'area', 'name' => t(shop_get_info('currency', 'default')), 'data' => array());
    $q = db_select('shop_cart_items_ordered', 'scio');
    $q->leftJoin('shop_index', 'si', 'si.entity_type = scio.entity_type AND si.entity_id = scio.entity_id');
    $q->innerJoin('shop_orders', 'so', 'scio.oid = so.nid');
    $q->fields('scio', array('changed'));
    $q->condition('so.is_paid', 1);
    $q->addExpression('SUM(scio.qty * (IF(scio.price IS NULL, si.price, scio.price)))', 'total');
    $q->groupBy('scio.oid');
    $results = $q->execute();
    $data = array();
    foreach ($results as $result) {
      $data[] = $result;
    }
    unset($results);
    if (!empty($data)) {
      // build date interval
      $min_changed = PHP_INT_MAX;
      foreach ($data as $values) {
        $min_changed = min($values->changed, $min_changed);
      }
      $begin_ts = $min_changed;
      $end_ts = 'now';
      $begin = new DateTime('@' . $begin_ts);
      $begin->setTime(0, 0, 0);
      $end = new DateTime($end_ts);
      $interval_string = 'P1W';
      $interval = new DateInterval($interval_string);
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
    $build['revenue_dynamics']['graph'] = array(
      '#theme' => 'graph',
      '#options' => $graph_options,
    ); 
  
    if (module_exists('status')) {
      $statuses = status_load_statuses('node', 'order');
      // orders progress
      $build['orders_progress'] = array(
        '#type' => 'fieldset',
        '#title' => l(t('Orders progress'), 'admin/shop/orders/by_status'),
      );
      if (empty($statuses)) {
        $build['orders_progress']['notice']['#markup'] = t('Order have no statuses');
      }
      else {
        $last_status_id = end($statuses)->id;
        $count_by_status = array_combine(array_keys($statuses), array_pad(array(), count($statuses), 0));
        $q = db_select('status_data', 'sd');
        $q->fields('sd', array('status_id'));
        $q->leftJoin('node', 'n', 'sd.entity_id = n.nid');
        $q->leftJoin('shop_orders', 'so', 'sd.entity_id = so.nid');
        $q->condition('sd.entity_type', 'node');
        $q->condition('n.type', 'order');
        //IKW: "Кроме снятых с публикации и уже закрытых (есть исп и оплата)"
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
        $build['orders_progress']['graph'] = array(
          '#theme' => 'graph',
          '#options' => $graph_options,
        );
      }

      // orders journal
      $build['orders_journal'] = array(
        '#type' => 'fieldset',
        '#title' => l(t('Orders journal'), 'admin/shop/orders'),
      );
      // select orders where status was changed from yseterday
      $q = db_select('shop_orders','so');
      $q->leftJoin('status_data', 'sd', 'sd.entity_type = \'node\' AND sd.entity_id = so.nid');
      $q->fields('so', array('nid', 'order_num',));
      $q->fields('sd', array('status_id', 'last_status_timestamp',));
      $q->condition('sd.last_status_timestamp', $yesterday_ts, '>');
      $q->orderBy('sd.last_status_timestamp', 'DESC');
      $results = $q->execute()->fetchAllAssoc('nid');
      if (empty($results)) {
        $build['orders_journal'][] = array('#markup' => t('No orders'),);
      }
      else {
        // split orders into today and yesterday sections
        $build['orders_journal']['today'] = array(
          '#type' => 'item',
          '#access' => FALSE,
        );
        $build['orders_journal']['yesterday'] = array(
          '#type' => 'item',
          '#title' => t('Yesterday'),
          '#access' => FALSE,
        );
        foreach ($results as $result) {
          if (empty($result->last_status_timestamp) || empty($result->status_id)) {
            continue;
          }
          $markup = '';
          $markup .= format_date($result->last_status_timestamp, 'custom', 'H:s') . ' ';
          $markup .= t('Order') . ' ' . l($result->order_num, 'node/' . $result->nid);
          $markup .= ' ' . $statuses[$result->status_id]->name;
          if ($result->last_status_timestamp < $today_ts) {
            $build['orders_journal']['yesterday']['#access'] = TRUE;
            $build['orders_journal']['yesterday'][$result->nid] = array('#type' => 'item', '#markup' => $markup,);
          }
          else {
            $build['orders_journal']['today']['#access'] = TRUE;
            $build['orders_journal']['today'][$result->nid] = array('#type' => 'item', '#markup' => $markup,);
          }
        }
      }
    }   
  }

  
  

	// product journal
	$build['product_journal'] = array(
		'#type' => 'fieldset',
		'#title' => l(t('Product journal'), 'admin/shop/products'),
	);
	// product count
	$product_count = db_query('SELECT COUNT(*) FROM {shop_index}')->fetchField();
	$build['product_journal']['total']['#markup'] = t('Products') . ': ' . $product_count;
	// product updates(c1, csv)
	// @todo:
	// split products into today and yesterday sections
	$build['product_journal']['today'] = array(
		'#type' => 'item',
		'#access' => FALSE,
	);
	$build['product_journal']['yesterday'] = array(
		'#type' => 'item',
		'#title' => t('Yesterday'),
		'#access' => FALSE,
	);
	$q = db_select('shop_index', 'si');
	$q->innerJoin('node', 'n', 'n.nid = si.entity_id');
	$q->fields('si', array('price', 'currency', 'entity_uri', 'title',));
	$q->fields('n', array('nid', 'created', 'changed',));
	$q->condition('si.entity_type', 'node');
	$q->condition('n.changed', $yesterday_ts, '>');
	$q->orderBy('n.changed', 'DESC');
	$results = $q->execute();
	if (empty($results)) {
		$build['product_journal'][] = array('#markup' => t('No products'),);
	}
	else {
		foreach ($results as $result) {
			$markup = '';
			$markup .= format_date($result->changed, 'custom', 'H:s');
			$markup .= ' ' . '<small>' . t('updated') . '</small>';  // @todo: ask IK when product is new or updated
			$markup .= ' ' . l($result->title, $result->entity_uri);
			$markup .= ' ' . theme('price', array('price' => $result->price, 'currency' => $result->currency));
			if ($result->changed < $today_ts) {
				$build['product_journal']['yesterday']['#access'] = TRUE;
				$build['product_journal']['yesterday'][$result->nid] = array('#type' => 'item', '#markup' => $markup,);
			}
			else {
				$build['product_journal']['today']['#access'] = TRUE;
				$build['product_journal']['today'][$result->nid] = array('#type' => 'item', '#markup' => $markup,);
			}
		}
	}

	return $build;
}
