<?php

function product_list_form($form, &$form_state, $node_type = NULL) {
	$destination = drupal_get_destination();
	$module_path = drupal_get_path('module', 'product');
	$form['#attached']['js'][] = $module_path . '/product.list.js';

	$entity_type = 'node';
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

	// Gather type information.
	$instances = field_info_instances($entity_type, $bundle);
	$extra_fields = field_info_extra_fields($entity_type, $bundle, 'display');
	$field_list = $extra_fields + $instances;

	// build available column list
	// field order: created - changed - article - images - title - qty - price - other_stuff - actions
	$columns = array(
		'created' => '',
		'changed' => '',
		'product_article' => '',
		'_images' => array(),
		'title' => '',
		'qty' => '',
		'price' => '',
		'_other' => array(),
	);
	foreach ($field_list as $name => $data) {
		// exclude form collisions with buy buttons
		if ($name == 'buy_button' || $name == 'buy_button2') {
			continue;
		}

		// add image fileds
		if (!empty($data['field_name'])) {
			$field = field_info_field($data['field_name']);
			if ($field['module'] == 'image') {
				$columns['_images'][$field['field_name']] = $data['label'];
			}
		}

		// insert column labels
		if (isset($columns[$name])) {
			$columns[$name] = $data['label'];
		}
		else {
			$columns['_other'][$name] = $data['label'];
		}
	}
	$defaul_columns = $columns;
	unset($defaul_columns['_images'], $defaul_columns['_other']);
	$tmp = array();
	array_walk_recursive($columns, function ($value, $key) use (&$tmp) { $tmp[$key] = $value; });
	$columns = $tmp;
	$enabled_columns = !empty($_SESSION['product_list_form_columns']) ? $_SESSION['product_list_form_columns'] : $defaul_columns;
	foreach ($enabled_columns as $name) {
		if (!empty($columns[$name])) {
			$enabled_columns[$name] = $columns[$name];
		}
	}

  if (!shop_get_info('balances', 'settings', 'track')) {
    unset($enabled_columns['qty'], $columns['qty']);
  }  
	// columns select
	$form['columns_container'] = array(
		'#type' => 'fieldset',
		'#title' => t('Columns'),
		'#collapsible' => TRUE,
		'#collapsed' => TRUE,
		'columns' => array(
			'#type' => 'select',
			'#multiple' => TRUE,
			'#options' => $columns,
			'#default_value' => array_keys($enabled_columns),
		),
		'submit' => array(
			'#type' => 'submit',
			'#value' => t('Save'),
			'#submit' => array('product_list_form_columns_submit'),
		),
	);
	if ($entity_type == 'node') {
		$form['columns_container']['#description'] = t('You can !edit some field viewing settings for this list',
			array('!edit' => l(t('edit'), "admin/structure/types/manage/$bundle/display/teaser"),)
		);
	}


	// table header
	$header = array();
	foreach ($enabled_columns as $column_name => $column_title) {
		$header[$column_name] = array('data' => $column_title);
		switch ($column_name) {
			case 'changed':
				$header[$column_name]['field'] = 'n.changed';
				$header[$column_name]['sort'] = 'desc';
				break;
			case 'created':
				$header[$column_name]['field'] = 'n.created';
				break;
			case 'title':
				$header[$column_name]['field'] = 'n.title';
				break;
			case 'qty':
				$header[$column_name]['field'] = 'si.qty';
				break;
			case 'price':
				$header[$column_name]['field'] = 'si.price';
				break;
		}
	}
	$header['actions'] = t('Actions');
	$form['#header'] = $header;

	// build query
	$query = db_select($entity_type, 'n')->extend('PagerDefault')->extend('TableSort');
	$query->fields('n', array('nid'));
	$query->condition('n.type', $bundle);
	$query->innerJoin('shop_index', 'si', 'si.entity_id = n.nid');
	$query->condition('si.entity_type', $entity_type);
	// apply filters to query
	$filters = drupal_get_query_parameters(NULL, array('q', 'submit'));
	// article field filter
//	if (!empty($filters['article'])) {
//		$article_query = new EntityFieldQuery();
//		$article_query->entityCondition('entity_type', 'node');
//		$article_query->entityCondition('bundle', $bundle);
//		$article_query->fieldCondition('product_article', 'value', $filters['article'], 'CONTAINS');
//		$article_filtered_nodes = $article_query->execute();
//		$article_filtered_nids = 0;
//		if (!empty($article_filtered_nodes['node'])) {
//			$article_filtered_nids = array_keys($article_filtered_nodes['node']);
//		}
//		$query->condition('n.nid', $article_filtered_nids);
//	}
	if (!empty($filters['title'])) {
		$query->condition('n.title', '%' . db_like($filters['title']) . '%', 'LIKE');
	}
	if (!empty($filters['price'])) {
		$query->condition('si.price', $filters['price']);
	}
	if (!empty($filters['qty']) && shop_get_info('balances', 'settings', 'track')) {
		$query->condition('si.qty', $filters['qty']);
	}
	// apply taxonomy filters, process taxonomy order
//	$instance_labels = array();
//	foreach ($taxonomy_instances as $instance) {
//		$instance_labels[] = $instance['label'];
//	}
//	if (array_intersect_key($taxonomy_instances, $filters) || (!empty($_GET['order']) && in_array($_GET['order'], $instance_labels))) {
//		$query->leftJoin('taxonomy_entity_index', 'tei', 'tei.entity_id = si.entity_id AND tei.entity_type = \'node\'');
//		$query->addExpression('IF (tei.tid IS NULL, 0, tei.tid)', 'tei_tid');
//		foreach ($taxonomy_instances as $instance_name => $instance) {
//			if (!empty($filters[$instance_name])) {
//				$query->condition('tei.tid', $filters[$instance_name]);
//				$query->isNotNull('tei.tid');
//			}
//		}
//	}
	$query->limit($limit)->orderByHeader($header);
	$nids = $query->execute()->fetchCol();
	$nodes = $nids ? node_load_multiple($nids) : array();
	$nodes_build = $nodes ? node_view_multiple($nodes, NULL) : array();
	$nodes_build = !empty($nodes_build['nodes']) ? $nodes_build['nodes'] : array();

	// build table rows
	$row_sample = array_combine(array_keys($header), array_pad(array(), count($header), array()));
	$form['entities'] = array('#tree' => TRUE);
	// filter row
	$form['entities']['filter'] = $row_sample;
	foreach ($row_sample as $field_name => $field) {
		switch ($field_name) {
			case 'title':
				$form['entities']['filter']['title'] = array(
					'#type' => 'textfield',
					'#default_value' => !empty($filters['title']) ? $filters['title'] : '',
				);
				break;
			case 'price':
				$form['entities']['filter']['price'] = array(
					'#type' => 'digit',
					'#step' => 100,
					'#size' => 9,
					'#default_value' => !empty($filters['price']) ? $filters['price'] : '',
				);
				break;
			case 'qty':
				$form['entities']['filter']['qty'] = array(
					'#type' => 'digit',
					'#step' => 1,
					'#size' => 9,
					'#default_value' => !empty($filters['qty']) ? $filters['qty'] : '',
				);
				break;
			case 'actions':
				$form['entities']['filter']['actions'] = array(
					'#type' => 'actions',
					'submit' => array(
						'#type' => 'submit',
						'#value' => t('Filter'),
						'#submit' => array('product_list_filter_submit'),
						'#attributes' => array('class' => array('js-hide', 'submit',)),
					),
					'reset' => array(
						'#type' => 'submit',
						'#value' => t('Reset'),
						'#submit' => array('product_list_filter_reset'),
					),
				);
				if (empty($filters)) {
					$form['entities']['filter']['actions']['reset']['#attributes'] = array('class' => array('js-hide'));
				}
				break;
		}
	}

	// content rows
	foreach ($nodes as $nid => $node) {
		$form['entities'][$nid] = $row_sample;
		foreach ($row_sample as $field_name => $field) {
			$cell = & $form['entities'][$nid][$field_name];
			if (!empty($nodes_build[$nid][$field_name])) {
				$cell = $nodes_build[$nid][$field_name];
			}

			switch ($field_name){
				// IKW: title suffix for imported products
				case 'title':
					$title_suffix = '';
					if (!empty($node->c1_id)) {
						$title_suffix = '1c id: ' . check_plain($node->c1_id);
					}
					if (!empty($node->doc_import_id)) {
						$title_suffix = 'csv id: ' . check_plain($node->doc_import_id);
					}
					if ($title_suffix) {
						$cell['#suffix'] = "<small class=\"description\">$title_suffix<small>";
					}
					break;
				// remove qty field title
				case 'qty':
					$cell['#title'] = '';
					break;
				case 'actions':
					if (node_access('update', $node)) {
						$cell['edit'] = array(
							'#type' => 'link',
							'#title' => theme('pictogram', array('icon' => 'pencil')),
							'#href' => 'node/' . $node->nid . '/edit',
							'#options' => array('query' => $destination, 'html' => TRUE),
							'#attributes' => array(
								'title' => t('Edit'),
								'class' => 'edit',
							),
						);
					}
					if (node_access('delete', $node)) {
						$cell['delete'] = array(
							'#type' => 'link',
							'#title' => theme('pictogram', array('icon' => 'trash')),
							'#href' => 'node/' . $node->nid . '/delete',
							'#options' => array('query' => $destination, 'html' => TRUE),
							'#attributes' => array(
								'title' => t('Delete'),
								'class' => 'delete ajax-popup',
							),
						);
					}
					break;
			}
		}
	}

	$form['pager'] = array('#markup' => theme('pager'));

	return $form;
}

function theme_product_list_form($variables) {
	$form = $variables['form'];
	$output = '';
  if (!empty($form['switch_links'])) {
    $output .= render($form['switch_links']);
  }
	$output .= render($form['columns_container']);
	if (!empty($form['entities'])) {
		$rows = array();
		foreach (element_children($form['entities']) as $nid) {
			foreach (element_children($form['entities'][$nid]) as $key) {
				$rows[$nid]['data'][$key] = array('data' => render($form['entities'][$nid][$key]),);
			}

			if ($nid == 'filter') {
				$rows[$nid]['class'] = array('filter-row');
			}
		}

		$output .= theme('table', array(
			'empty' => t('Content not found.'),
			'header' => $form['#header'],
			'rows' => $rows,
			'attributes' => array('id' => array('product-list'), 'width' => '100%'),
		));
	}

	$output .= drupal_render_children($form);

	return $output;
}

function product_list_form_columns_submit($form, &$form_state) {
	if (!empty($form_state['values']['columns'])) {
		$_SESSION['product_list_form_columns'] = $form_state['values']['columns'];
	}
}

function product_list_filter_submit($form, &$form_state) {
	if (!empty($form_state['values']['entities']['filter'])) {
		$filters = array_filter($form_state['values']['entities']['filter']);
		unset($filters['actions']);
		$form_state['redirect'] = array($_GET['q'], array('query' => $filters,));
	}
}

function product_list_filter_reset($form, &$form_state) {
	$form_state['redirect'] = $_GET['q'];
}
