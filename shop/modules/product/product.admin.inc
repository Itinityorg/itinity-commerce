<?php

/**
 * Callback for /node/%node/product_requests
 */
function product_node_requests_form($form, &$form_state, $node) {
	return product_requests_form($form, $form_state, 'node', $node->nid);
}

/**
 * Callback for /admin/shop/requests
 */
function product_requests_form($form, &$form_state, $entity_type = NULL, $entity_id = NULL) {
	$query = db_select('shop_product_requests', 'spr')->fields('spr');
	$query->join('shop_index', 'si', 'si.entity_type = spr.entity_type AND si.entity_id = spr.entity_id');
	$query->fields('si', array('title', 'qty', 'entity_uri',))->addExpression('si.qty', 'in_stock_qty');
	$query->addExpression('spr.qty', 'needed_qty');
	$query = $query->extend('PagerDefault')->extend('TableSort');
	$filtered_by_entity = FALSE;
	if ($entity_type && $entity_id && ($entity = entity_load($entity_type, array($entity_id)))) {
		$filtered_by_entity = TRUE;
		$query->condition('si.entity_type', $entity_type);
		$query->condition('si.entity_id', $entity_id);
		drupal_set_title(t('Requests for %title', array('%title' => entity_label($entity_type, reset($entity)))), PASS_THROUGH);
	}
	$header = array(
		'_title' => array(
			'data' => t('Name', array(), array('context' => 'product')),
			'field' => 'si.title',
		),
		'_in_stock_qty' => array(
			'data' => t('In stock'),
			'field' => 'in_stock_qty',
		),
		'_needed_qty' => array(
			'data' => t('Needed'),
			'field' => 'in_stock_qty',
		),
		'_user_name' => array(
			'data' => t('Username'),
			'field' => 'spr.user_name',
		),
		'_user_phone' => array(
			'data' => t('Phone'),
			'field' => 'spr.user_phone',
		),
		'_user_email' => array(
			'data' => t('E-mail'),
			'field' => 'spr.user_email',
		),
		'_created' => array(
			'data' => t('Created'),
			'field' => 'spr.created',
			'sort' => 'desc',
		),
		'_processed' => array(
			'data' => t('Processed'),
			'field' => 'spr.processed',
		),
	);
	if ($filtered_by_entity) {
		unset($header['_title']);
	}
	$query->limit(10)->orderByHeader($header);
	$rows = array(
	
	);
	foreach ($query->execute() as $r) {		
		$account = $r->user_email ? user_load_by_mail($r->user_email) : NULL;
		$rows[$r->id] = array(
			'_title' => array(
				'data' => l($r->title, $r->entity_uri),
			),
			'_in_stock_qty' => array(
				'data' => !isset($r->in_stock_qty) ? t('In stock') : (!($r->in_stock_qty * 1) ? t('Not in stock') : $r->in_stock_qty),
			),
			'_needed_qty' => array(
				'data' => $r->needed_qty,
			),
			'_user_name' => array(
				'data' => $account ? l($r->user_name ? $r->user_name : format_username($account), 'user/' . $account->uid) : check_plain($r->user_name),
			),
			'_user_phone' => array(
				'data' => $r->user_phone ? check_plain($r->user_phone) : '-',
			),
			'_user_email' => array(
				'data' => $r->user_email ? l($r->user_email, 'mailto:' . $r->user_email) : '-',
			),
			'_created' => array(
				'data' => format_date($r->created, 'short'),
			),
			'_processed' => array(
				'data' => $r->processed ? format_date($r->processed, 'short') : t('no'),
			),
		);
		if ($filtered_by_entity) {
			unset($rows[$r->id]['_title']);
		}
		foreach ($rows[$r->id] as $key => $item) {
			if ($r->processed || !($r->in_stock_qty * 1)) {
				$rows[$r->id][$key]['style'] = 'opacity:0.8;';
			}
			if ((!$r->processed && $r->in_stock_qty * 1) && in_array($key, array('_in_stock_qty', '_processed'), TRUE)) {
				$rows[$r->id][$key]['style'][] = 'background-color:#FF9';
			}
		}
	}
	if ($rows) {
		$form['actions'] = array(
			'#type' => 'actions',
			'delete' => array(
				'#type' => 'submit',
				'#value' => t('Delete'),
			),
			'processed' => array(
				'#type' => 'submit',
				'#value' => t('Mark as processed'),
			),
		);
	}
	$form['requests'] = array(
		$rows ? '#type' : '#theme' => $rows ? 'tableselect' : 'table',
		'#header' => $header,
		$rows ? '#options' : '#rows' => $rows ? $rows : array(array(array('data' => t('No requests found'), 'colspan' => count($header)))),
	);
	$form['pager']['#theme'] = 'pager';

	return $form;
}


function product_requests_form_validate($form, &$form_state) {
	if (!array_filter($form_state['values']['requests'])) {
		form_error($form['requests'], t('No items selected.'));
	}
}


function product_requests_form_submit($form, &$form_state) {
	if ($form_state['values']['op'] === $form['actions']['delete']['#value']) {
		$deleted = 0;
		foreach ($form_state['values']['requests'] as $id => $checked) {
			if ($checked) {
				$deleted ++;
				product_request_delete($id);
			}
		}
		if ($deleted) {
			drupal_set_message(format_plural($deleted, '1 record has been deleted', '@count records has been deleted'));
		}
	}
	if ($form_state['values']['op'] === $form['actions']['processed']['#value']) {
		$updated = 0;
		foreach ($form_state['values']['requests'] as $id => $checked) {
			if ($checked) {
				if (($request = product_request_load($id)) && !$request->processed) {
					$request->processed = REQUEST_TIME;
					product_request_save($request);
					$updated ++;
				}
			}
		}
		if ($updated) {
			drupal_set_message(format_plural($updated, '1 record has been updated', '@count records has been updated'));
		}
	}
}

/**
 * Callback for /admin/shop/requests/most_required
 */
function product_most_required_form($form, &$form_state) {
	$query = db_select('shop_index', 'si')->fields('si');
	$query->join('shop_product_requests', 'spr', 'spr.entity_type = si.entity_type AND spr.entity_id = si.entity_id AND NOT spr.processed');
	$query->addExpression('-COUNT(*)', 'requests_total');
	$query->addExpression('SUM(spr.qty)', 'required_qty');
	$query->groupBy('CONCAT(si.entity_type, \':\', si.entity_id)');
	$query = $query->extend('PagerDefault')->extend('TableSort');
	$header = array(
		'_title' => array(
			'data' => t('Name', array(), array('context' => 'product')),
			'field' => 'si.title',
		),
		'_requests_total' => array(
			'data' => t('Unprocessed requests'),
			'field' => 'requests_total',
			'sort' => 'asc',
		),
		'_qty' => array(
			'data' => t('In stock'),
			'field' => 'si.qty',
		),
		'_required_qty' => array(
			'data' => t('Needed'),
			'field' => 'requests_total',
		),
	);
	$query->limit(10)->orderByHeader($header);
	$rows = array();
	foreach ($query->execute() as $r) {
		$rows[] = array(
			'data' => array(
				'_title' => array(
					'data' => l($r->title, $r->entity_uri),
				),
				'_requests_total' => array(
					'data' => l(-$r->requests_total, 'admin/shop/requests/' . $r->entity_type . '/' . $r->entity_id),// . ' ' . l(t('overview'), 'admin/shop/requests/' . $r->entity_type . '/' . $r->entity_id),
				),
				'_qty' => array(
					'data' => !isset($r->qty) ? t('In stock') : (!($r->qty * 1) ? t('Not in stock') : $r->qty),
				),
				'_required_qty' => array(
					'data' => $r->required_qty,
				),
			),
		);
	}
	$form['list'] = array(
		'#theme' => 'table',
		'#rows' => $rows,
		'#header' => $header,
	);
	return $form;
}

/**
 * Callback for /admin/shop/products/sales
 */
function product_trade_form($form, &$form_state) {
	$values = drupal_get_query_parameters(NULL, array('q', 'submit', 'page',));
	$form['#tree'] = TRUE;
	$form['settings'] = array(
		'#type' => 'container',
		'#attributes' => array('class' => array(/*'date-element-wrapper', */'container-inline',),),
		'form' => array(
			'#type' => 'datetime', //'datepicker',
			'#group' => 'from-to',
			'#default_value' => !empty($values['form']) ? $values['form'] : '',
		),
		'to' => array(
			'#type' => 'datetime', //'datepicker',
			'#group' => 'from-to',
			'#default_value' => !empty($values['to']) ? $values['to'] : '',
		),
	);
	$form['settings']['actions'] = array(
		'#type' => 'actions',
		'submit' => array(
			'#type' => 'submit',
			'#value' => t('Save'),
			'#attributes' => array('class' => array('submit',)),
		),
	);
	if (!empty($values)) {
		$form['settings']['actions']['reset'] = array(
			'#type' => 'submit',
			'#value' => t('Reset'),
			'#submit' => array('product_trade_form_reset_submit'),
		);
	}


	$header = array(
		'title' => array('data' => t('Title'), 'field' => 'title',),
		'deals' => array('data' => t('Deals'), 'field' => 'deals'), // number of orders
		'saled_qty' => array('data' => t('Sales volume'), 'field' => 'saled_qty'),
		'proceeds' => array('data' => t('Proceeds'), 'field' => 'proceeds', 'sort' => 'desc',),
	);
	$rows = array();
	$row_sample = array_combine(array_keys($header), array_pad(array(), count($header), array()));

	$q = db_select('shop_cart_items_ordered', 'scio');
	$q->leftJoin('shop_index', 'si', 'si.entity_type = scio.entity_type AND si.entity_id = scio.entity_id');
	$q->leftJoin('shop_orders', 'so', 'scio.oid = so.nid');
	$q->fields('scio', array('entity_id', 'entity_type', 'currency', 'frozen',));
	$q->fields('si', array('currency',));
	$q->condition('so.is_paid', 1);
	if (!empty($values['form']) && !empty($values['to'])) {
		$q->condition('scio.changed', array($values['form'], $values['to']), 'BETWEEN');
	}
	$q->addExpression('(IF(scio.title IS NULL, si.title, scio.title))', 'title');
	$q->addExpression('COUNT(scio.oid)', 'deals');
	$q->addExpression('SUM(scio.qty)', 'saled_qty');
	$q->addExpression('SUM(scio.qty * (IF(scio.price IS NULL, si.price, scio.price)))', 'proceeds');
	$q->groupBy('scio.entity_type');
	$q->groupBy('scio.entity_id');
	$q = $q->extend('PagerDefault')->extend('TableSort');
	$q->limit(50)->orderByHeader($header);
	$results = $q->execute();
	foreach ($results as $result) {
		$title = !empty($result->title) ? $result->title : '';
		if (empty($result->frozen)) {
			$entity = entity_load($result->entity_type, array($result->entity_id));
			if (!empty($entity) && $title) {
				$entity = current($entity);
				$uri = entity_uri($result->entity_type, $entity);
				$title = l($title, $uri['path']);
			}
		}

		$currency = shop_get_info('currency', 'default');
		if (!empty($result->currency)) {
			$currency = $result->currency;
		}
		elseif (!empty($result->si_currency)) {
			$currency = $result->si_currency;
		}

		$row = $row_sample;
		$row['title'] = $title;
		$row['deals'] = $result->deals;
		$row['saled_qty'] = $result->saled_qty;
		$row['proceeds'] = '<div class="product-price field-name-product-price-base field-name-product-price-currency">' .
			theme('price', array('price' => $result->proceeds, 'currency' => $currency)) .
			'</div>';
		$rows[] = $row;
	}

	$form['list'] = array(
		'#theme' => 'table',
		'#rows' => $rows,
		'#header' => $header,
	);

	$form['pager'] = array('#markup' => theme('pager'));

	return $form;
}

function product_trade_form_submit($form, &$form_state) {
	$query = array_filter($form_state['values']['settings']);
	unset($query['actions']);
	$form_state['redirect'] = array($_GET['q'], array('query' => $query,));
}

function product_trade_form_reset_submit($form, &$form_state) {
	$form_state['redirect'] = $_GET['q'];
}

/**
 * Callback for /admin/shop/products/sales_by_taxonomy
 */
function product_trade_taxonomy_form($form, &$form_state) {
	// taxonomy field of shop entities
	$entity_info = entity_get_info();

	foreach (field_info_fields() as $field) {
		if ($field['module'] === 'taxonomy') {
			$vocabulary_name = $field['settings']['allowed_values'][0]['vocabulary'];
			$vocabulary = taxonomy_vocabulary_machine_name_load($vocabulary_name);

			foreach ($field['bundles'] as $entity_type => $bundles) {
				foreach ($bundles as $bundle) {
					if (shop_get_info('entity_types', $entity_type, $bundle)) {
						$q = db_select('taxonomy_entity_index', 'tei');
						$q->fields('tei', array('tid'));
						$q->condition('tei.entity_type', $entity_type);
						$q->condition('tei.bundle', $bundle);
						$q->condition('tei.field_name', $field['field_name']);
						$q->leftJoin('shop_cart_items_ordered', 'scio', 'tei.entity_type = scio.entity_type AND tei.entity_id = scio.entity_id');
						$q->leftJoin('shop_index', 'si', 'si.entity_type = scio.entity_type AND si.entity_id = scio.entity_id');
						$q->leftJoin('shop_orders', 'so', 'scio.oid = so.nid');
						$q->condition('so.is_paid', 1);
						$q->addExpression('SUM(scio.qty * (IF(scio.price IS NULL, si.price, scio.price)))', 'proceeds');
						$q->groupBy('tei.tid');
						$q->orderBy('proceeds', 'DESC');

						$results = array();
						$total_sum_by_field = 0;
						$percent_by_field = 0;
						foreach ($q->execute() as $result) {
							$results[] = $result;
							$total_sum_by_field += $result->proceeds;
						}

						$graph_options = array(
							'title' => array('text' => $vocabulary->name . ' ' . $entity_info[$entity_type]['label'] . ' ' . $entity_info[$entity_type]['bundles'][$bundle]['label'],),
							'credits' => array('enabled' => FALSE),
							'tooltip' => array('pointFormat' => '{series.name}: <b>{point.percentage:.1f}%</b>',),
							'plotOptions' => array(
								'pie' => array(
									'allowPointSelect' => TRUE,
									'cursor' => 'pointer',
									'dataLabels' => array(
										'enabled' => TRUE,
										'color' => '#000000',
										'connectorColor' => '#000000',
										'format' => '<b>{point.name}</b>: {point.percentage:.1f} %'
									)
								)
							),
							'series' => array(),
						);
						$graph_data = array(
							'type' => 'pie',
							'name' => t('Sell share'),
							'data' => array(),
						);
						foreach ($results as $result) {
							$percent = round(($result->proceeds * 100) / $total_sum_by_field, 2);
							$percent_by_field += $percent;
							if ($percent_by_field > 95) {
								$graph_data['data'][] = array('name' => t('Other'), 'y' => 100 - $percent_by_field,);
								break;
							}
							else {
								$term = taxonomy_term_load($result->tid);
								$graph_data['data'][] = array('name' => check_plain($term->name), 'y' => $percent,);
							}
						}
						$graph_options['series'][] = $graph_data;

						$form[] = array(
							'#theme' => 'graph',
							'#options' => $graph_options,
						);
					}
				}
			}
		}

	}

	return $form;
}

/**
 * Callback for /admin/shop/products/manage_price
 */
function product_manage_price_form($form, &$form_state, $node_type = NULL) {
	$destination = drupal_get_destination();
	$bundle = $node_type && shop_get_info('entity_types', 'node', $node_type) ? $node_type : 'product';
  $switch_links = array(array('title' => t('Content type') . ':'));
  foreach (array_filter(shop_get_info('entity_types', 'node')) as $t) {
    if ($n = node_type_get_name($t)) {
      $switch_links[] = array(
        'title' => $n,
        'href' => 'admin/shop/products/' . $t,
      );
    }
  }
  if (count($switch_links) > 1) {
    $form['switch_links'] = array(
      '#theme' => 'links',
      '#links' => $switch_links,
      '#attributes' => array(
        'class' => array('inline',),
      ),
    );
  }
	$limit = 50;
	$node_type = node_type_get_type($bundle);

	$taxonomy_instances = array();
	foreach (field_info_instances('node', $bundle) as $instance_name => $instance) {
		$field = field_info_field($instance['field_name']);
		if ($field && $field['type'] == 'taxonomy_term_reference') {
			$taxonomy_instances[$instance_name] = $instance;
			$taxonomy_instances[$instance_name]['field'] = $field;
		}
	}
	$form['#tree'] = TRUE;
  

	// table header
	$header = array(
		'changed'    => array('data' => t('Changed'), 'field' => 'n.changed', 'sort' => 'desc'),
		'article'    => array('data' => t('Article number'), 'class' => array('article')),
		'title'      => array(
			'data'  => (!empty($node_type->title_label) ? $node_type->title_label : t('Title')),
			'field' => 'n.title',
			'class' => array('title'),
		),
		'price'      => array('data' => t('Price'), 'field' => 'si.price', 'class' => array('price'),),
		'qty'        => array('data' => t('Quantity'), 'field' => 'si.qty', 'class' => array('qty'),),
		'operations' => array('data' => t('Operations')),
	);
	$form['#header'] = $header;

	// query
	$query = db_select('node', 'n')->extend('PagerDefault')->extend('TableSort');
	$query->fields('n', array('nid'));
	$query->condition('n.type', $bundle);
	$query->innerJoin('shop_index', 'si', 'si.entity_id = n.nid');
	$query->condition('si.entity_type', 'node');
	// apply filters to query
	$filters = drupal_get_query_parameters(NULL, array('q', 'submit'));
	// apply taxonomy filters, process taxonomy order
	$instance_labels = array();
	foreach ($taxonomy_instances as $instance) {
		$instance_labels[] = $instance['label'];
	}
	if (array_intersect_key($taxonomy_instances, $filters) || (!empty($_GET['order']) && in_array($_GET['order'], $instance_labels))) {
		$query->leftJoin('taxonomy_entity_index', 'tei', 'tei.entity_id = si.entity_id AND tei.entity_type = \'node\'');
		$query->addExpression('IF (tei.tid IS NULL, 0, tei.tid)', 'tei_tid');
		foreach ($taxonomy_instances as $instance_name => $instance) {
			if (!empty($filters[$instance_name])) {
				$term = taxonomy_term_load($filters[$instance_name]);
				$tree = taxonomy_get_tree($term->vid, $term->tid);
				$tids = [];
				$tids[] = $term->tid;
				foreach ($tree as $_term) {
					$tids[] = $_term->tid;
				}
				$query->condition('tei.tid', $tids);
				$query->isNotNull('tei.tid');
			}
		}
	}
	$query->limit($limit)->orderByHeader($header);
	$nids = $query->execute()->fetchCol();
	$nodes = node_load_multiple($nids);
	$form['#nodes'] = $nodes;

	// filter form
	$form['filter'] = array(
		'#type'       => 'fieldset',
		'#attributes' => array('class' => array('container-inline',)),
	);
	foreach ($taxonomy_instances as $instance_name => $instance) {
		$form['filter'][$instance_name] = array(
			'#title'         => $instance['label'],
			'#type'          => 'select',
			'#options'       => taxonomy_allowed_values($instance['field']),
			'#empty_option'  => '',
			'#default_value' => !empty($filters[$instance_name]) ? $filters[$instance_name] : '',
		);
	}
	$form['filter']['actions'] = array(
		'#type'  => 'actions',
    'reset' => array(
      '#type' => 'link',
      '#title' => theme('pictogram', array('icon' => 'cancel')),
      '#href' => $_GET['q'],
      '#options' => array(
        'html' => TRUE,
      ),
      '#attributes' => array(
        'title' => t('Reset'),
        'class' => array('form-submit',),
      ),
      '#access' => !empty($filters),
    ),
		'submit' => array(
			'#type'       => 'submit',
			'#value'      => t('Filter'),
			'#submit'     => array('product_manage_price_filter_submit'),
			'#attributes' => array('class' => array('submit',)),
		),
	);

	// build table rows
	$form['nodes'] = array();
	module_load_include('inc', 'node', 'node.pages');
  foreach ($nodes as $node) {
    // obtaining node form
    $node->revision = FALSE;
    if (empty($node->menu)) {
      $node->menu = NULL;
    }
    $t_form_state = array('node' => $node);
    $t_node_form = node_form(array(), $t_form_state, $node);
    $t_form_id = 'node_form';
    drupal_alter(array('form', 'form_' . $t_form_id), $t_node_form, $t_form_state, $t_form_id);

    $row = array_combine(array_keys($header), array_pad(array(), count($header), array()));
    

    // changed
    $row['changed']['#markup'] = format_date($node->changed, 'short');

    // artice
    $row['article'] = array();
    $article = field_get_items('node', $node, 'product_article');
    if ($article) {
      $article = current($article);
      $row['article']['#markup'] = $article['safe_value'];
    }

    // title
    $row['title'] = array(
      '#theme' => 'entity_link',
      '#entity_type' => 'node',
      '#entity' => $node,
      '#attributes' => array(
        'class' => array(
          'popup',
        ),
        'target' => '_blank',
      ),
    );
    // price
    $row['price'] = array('price' => array(), 'currency' => array());
    if (!empty($t_node_form['price']['price'])) {
      $t_node_form['price']['price']['#title_display'] = 'invisible';
      $t_node_form['price']['price']['#default_value'] = $node->price;
      $row['price']['price'] = $t_node_form['price']['price'];
    }

		// currency
		if (!empty($t_node_form['price']['currency'])) {
			$t_node_form['price']['currency']['#title_display'] = 'invisible';
			$t_node_form['price']['currency']['#default_value'] = $node->currency;
			$row['price']['currency'] = $t_node_form['price']['currency'];
		}

    // qty
    $row['qty'] = array();
    if (!empty($node->qty) && !empty($node->qty_type)) {
      $row['qty']['#markup'] = $node->qty . ' ' . $node->qty_type;
    }

    // Build a list of all the accessible operations for the current node.
    $row['operations'] = array();
    if (node_access('update', $node)) {
      $row['operations']['edit'] = array(
        '#type' => 'link',
        '#title' => theme('pictogram', array('icon' => 'pencil')),
        '#href' => 'node/' . $node->nid . '/edit',
        '#options' => array('query' => $destination, 'html' => TRUE),
        '#attributes' => array(
          'title' => t('Edit'),
          'class' => array('edit'),
        ),
      );
    }
    if (node_access('delete', $node)) {
      $row['operations']['delete'] = array(
        '#type' => 'link',
        '#title' => theme('pictogram', array('icon' => 'trash')),
        '#href' => 'node/' . $node->nid . '/delete',
        '#options' => array('query' => $destination, 'html' => TRUE),
        '#attributes' => array(
          'title' => t('Delete'),
          'class' => array('delete'),
        ),
      );
    }
    $form['nodes'][$node->nid] = $row;
  }

	$form['pager']['#theme'] = 'pager';

	$form['actions'] = array(
		'#type'  => 'actions',
		'#tree'  => FALSE,
		'submit' => array(
      '#type' => 'submit',
      '#value' => t('Save'),
      '#submit' => array('_product_manage_price_form_submit'),
     ),
	);

	return $form;
}

/**
 * Theming function
 *
 * @see product_manage_price_form()
 */
function theme_product_manage_price_form($variables) {
	$form = $variables['form'];

	$output = '';
  if (!empty($form['switch_links'])) {
    $output .= render($form['switch_links']);
  }
	$output .= drupal_render($form['filter']);

	if (!empty($form['nodes'])) {
		$rows = array();
		foreach (element_children($form['nodes']) as $nid) {
			foreach (element_children($form['nodes'][$nid]) as $key) {
				$rows[$nid]['data'][$key] = array('data' => render($form['nodes'][$nid][$key]),);
			}
		}

		$output .= theme('table', array(
			'empty' => t('Content not found.'),
			'header' => $form['#header'],
			'rows' => $rows,
			'attributes' => array('id' => array('product-manage-price'), 'width' => '100%'),
		));
	}

	$output .= drupal_render_children($form);

	return $output;
}

function product_manage_price_filter_submit($form, &$form_state) {
	if (!empty($form_state['values']['filter'])) {
		$filters = array_filter($form_state['values']['filter']);
		unset($filters['actions']);
		$form_state['redirect'] = array($_GET['q'], array('query' => $filters,));
	}
}

function _product_manage_price_form_submit($form, &$form_state) {
  $saved = 0;
  foreach ($form_state['values']['nodes'] as $nid => $data) {
    $data['price']['price'] *= 1;
    if ($form['#nodes'][$nid]->price * 1 != $data['price']['price']) {
      $form['#nodes'][$nid]->price = $data['price']['price'];
      $form['#nodes'][$nid]->currency = $data['price']['currency'];
      node_save($form['#nodes'][$nid]);
      $saved++;
    }elseif($form['#nodes'][$nid]->currency != $data['price']['currency']){
			$form['#nodes'][$nid]->currency = $data['price']['currency'];
			node_save($form['#nodes'][$nid]);
			$saved++;
		}
  }
  if ($saved) {
    drupal_set_message(format_plural($saved, 'Updated 1 good.', 'Updated @count goods.'));
  }
}










