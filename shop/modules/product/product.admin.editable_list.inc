<?php

/**
 * Callback for /admin/shop/products/editable_list
 * Quick editable table for products.
 *
 * @see theme_product_editable_list_form()
 */
function product_editable_list_form($form, &$form_state, $node_type = NULL) {
	$destination = drupal_get_destination();
	$module_path = drupal_get_path('module', 'product');
	$form['#attached']['js'][] = $module_path . '/product.list.js';
	$form['#attached']['css'][] = $module_path . '/product.editable_list.css';
	$form['#control_changes'] = TRUE; // @see addons_form_alter();
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
	$limit = 50; // IKW
	$limit = 10;
	$node_type = node_type_get_type($bundle);

	$taxonomy_instances = array();
	foreach (field_info_instances('node', $bundle) as $instance_name => $instance) {
		$field = field_info_field($instance['field_name']);
		if ($field && $field['type'] == 'taxonomy_term_reference') {
			$taxonomy_instances[$instance_name] = $instance;
			$taxonomy_instances[$instance_name]['field'] = $field;
		}
	}
	$form['#taxonomy_instances'] = $taxonomy_instances;

	// table header
	$header = array(
		array('data' => t('Changed'), 'field' => 'n.changed', 'sort' => 'desc'),
		array('data' => t('Article number'), 'class' => array('article')),
		array(
			'data' => (!empty($node_type->title_label) ? $node_type->title_label : t('Title')),
			'field' => 'n.title',
			'class' => array('title'),
		),
		array('data' => t('Price'), 'field' => 'si.price', 'class' => array('price'),),
		array('data' => t('Quantity'), 'field' => 'si.qty', 'class' => array('qty'),),
	);
	foreach ($taxonomy_instances as $instance) {
		$header[] = array('data' => $instance['label'], 'field' => 'tei_tid');
	}
	$header[] = array('data' => t('Operations'));
	if (module_exists('node_rank')) {
		$header[] = array('data' => '%', 'field' => 'n.rank');
	}
	$form['#header'] = $header; // for theming function @see theme_product_editable_list_form()

	// query for products
	$query = db_select('node', 'n')->extend('PagerDefault')->extend('TableSort');
	$query->fields('n', array('nid'));
	if (module_exists('node_rank')) {
		$query->fields('n', array('rank'));
	}
	$query->condition('n.type', $bundle);
	$query->innerJoin('shop_index', 'si', 'si.entity_id = n.nid');
	$query->condition('si.entity_type', 'node');
	// apply filters to query
	$filters = drupal_get_query_parameters(NULL, array('q', 'submit'));
	// article field filter
	if (!empty($filters['article'])) {
		$article_query = new EntityFieldQuery();
		$article_query->entityCondition('entity_type', 'node');
		$article_query->entityCondition('bundle', $bundle);
		$article_query->fieldCondition('product_article', 'value', $filters['article'], 'CONTAINS');
		$article_filtered_nodes = $article_query->execute();
		$article_filtered_nids = 0;
		if (!empty($article_filtered_nodes['node'])) {
			$article_filtered_nids = array_keys($article_filtered_nodes['node']);
		}
		$query->condition('n.nid', $article_filtered_nids);
	}
	if (!empty($filters['title'])) {
		$query->condition('n.title', '%' . db_like($filters['title']) . '%', 'LIKE');
	}
	if (!empty($filters['price'])) {
		$query->condition('si.price', $filters['price']);
	}
	if (!empty($filters['currency'])) {
		$query->condition('si.currency', $filters['currency']);
	}
	if (!empty($filters['qty'])) {
		$query->condition('si.qty', $filters['qty']);
	}
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
				$query->condition('tei.tid', $filters[$instance_name]);
				$query->isNotNull('tei.tid');
			}
		}
	}
	$query->limit($limit)->orderByHeader($header);
	$nids = $query->execute()->fetchCol();
	$nodes = node_load_multiple($nids);

	module_load_include('inc', 'node', 'node.pages');
	$form['nodes'] = array('#tree' => TRUE);

	// filter row
	$form['nodes']['filter'] = array(array());
	$form['nodes']['filter']['article'] = array(
		'#type' => 'textfield',
		'#default_value' => !empty($filters['article']) ? $filters['article'] : '',
		'#size' => 8,
	);
	$form['nodes']['filter']['title'] = array(
		'#type' => 'textfield',
		'#default_value' => !empty($filters['title']) ? $filters['title'] : '',
	);
	$form['nodes']['filter']['price'] = array(
		'#type' => 'digit',
		'#step' => 100,
		'#size' => 9,
		'#default_value' => !empty($filters['price']) ? $filters['price'] : '',
	);
	$form['nodes']['filter']['qty'] = array(
		'#type' => 'digit',
		'#step' => 1,
		'#size' => 9,
		'#default_value' => !empty($filters['qty']) ? $filters['qty'] : '',
	);
	// taxonomy filter columns
	foreach ($taxonomy_instances as $instance_name => $instance) {
		$form['nodes']['filter'][$instance_name] = array(
			'#type' => 'select',
			'#options' => taxonomy_allowed_values($instance['field']),
			'#empty_option' => '',
			'#default_value' => !empty($filters[$instance_name]) ? $filters[$instance_name] : '',
		);
	}
	$form['nodes']['filter']['actions'] = array(
		'#type' => 'actions',
		'submit' => array(
			'#type' => 'submit',
			'#value' => t('Filter'),
			'#submit' => array('product_editable_list_filter_submit'),
			'#attributes' => array('class' => array('js-hide', 'submit',)),
		),
		'reset' => array(
			'#type' => 'submit',
			'#value' => t('Reset'),
			'#submit' => array('product_editable_list_filter_reset'),
		),
	);
	if (module_exists('node_rank')) {
		$form['nodes']['filter']['rank'] = array('#type' => 'markup',);
	}
	if (empty($filters)) {
		$form['nodes']['filter']['actions']['reset']['#attributes'] = array('class' => array('js-hide'));
	}

	// product rows
	if ($nodes) {
		$form['#original_nodes'] = $nodes; // for validate function @see product_editable_list_form_validate()
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
			$form['nodes'][$node->nid] = array();
			$node_form = & $form['nodes'][$node->nid];

			// changed
			$node_form['changed'] = array('#markup' => format_date($node->changed, 'short'));

			// artice
			$node_form['product_article'] = array();
			$article = field_get_items('node', $node, 'product_article');
			if ($article) {
				$article = current($article);
				$node_form['product_article'] = array('#markup' => $article['safe_value']);
			}

			// title
			$node_form['title'] = array();
			if (!empty($t_node_form['title'])) {
				$node_form['title'] = $t_node_form['title'];
				_product_editable_list_form_field_helper($node_form['title']);
				$node_form['title']['#size'] = 30;
			}

			// price
			$node_form['price'] = array('price' => array(), 'currency' => array());
			if (!empty($t_node_form['price']['price'])) {
				$t_node_form['price']['price']['#title_display'] = 'invisible';
				$t_node_form['price']['price']['#default_value'] = $node->price;
				$node_form['price']['price'] = $t_node_form['price']['price'];
			}

			// currency
			if (!empty($t_node_form['price']['currency'])) {
				$t_node_form['price']['currency']['#title_display'] = 'invisible';
				$t_node_form['price']['currency']['#default_value'] = $node->currency;
				$node_form['price']['currency'] = $t_node_form['price']['currency'];
			}
			// qty
			$node_form['qty'] = array('#type' => 'item', '#inline' => TRUE,);
			$node_form['qty']['qty']['#markup'] = isset($node->qty) ? $node->qty : t('Unlimited');
			// qty_type
			if (!empty($t_node_form['qty']['qty_type'])) {
				$node_form['qty']['qty_type'] = $t_node_form['qty']['qty_type'];
				$node_form['qty']['qty_type']['#title'] = '';
			}

			// taxonomy field instances
			foreach ($taxonomy_instances as $instance_name => $instance) {
				$node_form[$instance_name] = array();
				if (!empty($t_node_form[$instance_name])) {
					$field_lang = $t_node_form[$instance_name]['#language'];
					$node_form[$instance_name] = $t_node_form[$instance_name];
					_product_editable_list_form_field_helper($node_form[$instance_name][$field_lang]);
				}
			}

			// Build a list of all the accessible operations for the current node.
      if (module_exists('shop_variants')) {
        $tree = shop_variants_node_load_tree($node);
				$node_form['operations']['variants'] = array(
					'#type' => 'link',
					'#title' => t('Variants') . '<sup>' . (count($tree) - 1) . '</sup>',
          '#options' => array(
            'html' => TRUE,
            'query' => drupal_get_destination(),
          ),
					'#href' => 'node/' . $node->nid . '/variants',
					'#suffix' => '<br />',
				);
      }
			$node_form['operations']['view'] = array(
				'#type' => 'link',
				'#title' => theme('pictogram', array('icon' => 'eye')),
				'#href' => 'node/' . $node->nid,
				'#options' => array('html' => TRUE),
				'#attributes' => array(
					'title' => t('View'),
					'class' => array('view'),
				),
			);

			if (node_access('update', $node)) {
				$node_form['operations']['edit'] = array(
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
				$node_form['operations']['delete'] = array(
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

			if (module_exists('node_rank')) {
				$node_form['filled'] = array(
					'data' => array(array('#markup' => isset($node->rank) ? round($node->rank) . '%' : '')),
				);
			}
		}
	}

	$form['pager'] = array('#markup' => theme('pager'));

	$form['actions'] = array(
		'#type' => 'actions',
		'submit' => array('#type' => 'submit', '#value' => t('Save'),),
	);

	return $form;
}

/**
 * Theming function
 *
 * @see product_editable_list_form()
 */
function theme_product_editable_list_form($variables) {
	$form = $variables['form'];

	$output = '';
  if (!empty($form['switch_links'])) {
    $output .= render($form['switch_links']);
  }
	if (!empty($form['nodes'])) {
		$rows = array();
		foreach (element_children($form['nodes']) as $nid) {
			foreach (element_children($form['nodes'][$nid]) as $key) {
				$rows[$nid]['data'][$key] = array('data' => render($form['nodes'][$nid][$key]),);
			}

			if ($nid == 'filter') {
				$rows[$nid]['class'] = array('filter-row');
			}
		}

		$output .= theme('table', array(
			'empty' => t('Content not found.'),
			'header' => $form['#header'],
			'rows' => $rows,
			'attributes' => array('id' => array('product-editable-list'), 'width' => '100%'),
		));
	}

	$output .= drupal_render_children($form);

	return $output;
}

function product_editable_list_filter_submit($form, &$form_state) {
	if (!empty($form_state['values']['nodes']['filter'])) {
		$filters = array_filter($form_state['values']['nodes']['filter']);
		unset($filters['actions']);
		$form_state['redirect'] = array($_GET['q'], array('query' => $filters,));
	}
}

function product_editable_list_filter_reset($form, &$form_state) {
	$form_state['redirect'] = $_GET['q'];
}

function product_editable_list_form_validate($form, &$form_state) {
	//find nodes that got changed
	//validate changes
	//if all is ok, add $submitted_node to $form_state['#changed_nodes'] then call node_save() in submit handler
	$form_state['#changed_nodes'] = array();
	$values = $form_state['values'];
	if (!empty($values['op']) && !empty($values['submit']) && $values['submit'] == $values['op']) {
		if (!empty($form['#original_nodes']) && !empty($values['nodes'])) {
			unset($values['nodes']['filter']); //remove filter form element values

			foreach ($values['nodes'] as $nid => $submitted_values) {
				$original_node = $form['#original_nodes'][$nid];
				$submitted_node = clone $original_node;
				$submitted_node->title = $submitted_values['title'];
				$submitted_node->price = $submitted_values['price']['price'];
				$submitted_node->currency = $submitted_values['price']['currency'];
				$submitted_node->qty_type = $submitted_values['qty']['qty_type'];

				$changed = FALSE;
				//check title for changing
				if (_product_editable_list_field_value_is_changed($submitted_node->title, $original_node->title)) {
					$changed = TRUE;
				}
				//check price for changing
				if (_product_editable_list_field_value_is_changed($submitted_node->price, $original_node->price)) {
					$changed = TRUE;
				}
				//check qty_type for changing
				if (_product_editable_list_field_value_is_changed($submitted_node->qty_type, $original_node->qty_type)) {
					$changed = TRUE;
				}
				//check taxonomy fields for changing
				if (!empty($form['#taxonomy_instances'])) {
					foreach ($form['#taxonomy_instances'] as $instance_name => $instance) {
						$submitted_node->{$instance_name} = !empty($submitted_values[$instance_name]) ? $submitted_values[$instance_name] : array();

						$original_taxonomy_value = field_get_items('node', $original_node, $instance_name);
						$submitted_taxonomy_value = field_get_items('node', $submitted_node, $instance_name);
						if (!empty($submitted_taxonomy_value) && empty($submitted_taxonomy_value[0]['tid'])) {
							$submitted_taxonomy_value = FALSE;
						}
						if ($original_taxonomy_value != $submitted_taxonomy_value) {
							$changed = TRUE;
						}
					}
				}

				if ($changed) {
					$form_state['#changed_nodes'][$nid] = $submitted_node;
				}
			}
		}
	}
}

function product_editable_list_form_submit($form, &$form_state) {
	$values = $form_state['values'];
	if (!empty($values['op']) && !empty($values['submit']) && $values['submit'] == $values['op']) {
		if (!empty($form_state['#changed_nodes']) && !empty($values['nodes'])) {

			foreach ($form_state['#changed_nodes'] as $changed_node) {

				if (!empty($form['#taxonomy_instances'])) {
					foreach ($form['#taxonomy_instances'] as $instance => $label) {
						$language = field_language('node', $changed_node, $instance);
						if ($changed_node->{$instance}[$language][0]['tid'] == '') {
							$changed_node->{$instance}[$language] = array();
						}
					}
				}

				node_save($changed_node);
				$node_type = node_type_get_type($changed_node);
				drupal_set_message(t('@type %title has been updated.', array(
					'@type' => $node_type->name,
					'%title' => $changed_node->title
				)));
			}

		}
	}
}

function _product_editable_list_form_field_helper(&$field) {
	$field['#title_display'] = 'invisible';
	$field['#required'] = FALSE;
	$field['#defaults_loaded'] = FALSE;
	$field['#description'] = '';
}

function _product_editable_list_field_value_is_changed($v1, $v2) {
	$result = FALSE;

	if (!empty($v1) && !empty($v2)) {
		if ($v1 != $v2) {
			$result = TRUE;
		}
	}
	elseif (empty($v1) && !empty($v2)) {
		$result = TRUE;
	}
	elseif (!empty($v1) && empty($v2)) {
		$result = TRUE;
	}

	return $result;
}
