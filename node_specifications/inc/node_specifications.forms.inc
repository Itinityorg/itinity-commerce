<?php

function node_specifications_form($form, &$form_state, $specification = NULL) {

	$path = drupal_get_path('module', 'node_specifications');
	// Ensure this include file is loaded when the form is rebuilt from the cache.
	$form_state['build_info']['files']['form'] = $path . 'inc/node_specifications.forms.inc';

	if (!$specification) {
		$specification = (object) array(
			'nsid' => NULL,
			'type' => NULL,
			'name' => NULL,
			'description' => NULL,
      'as_extrafields' => NULL,
      'go_deep' => TRUE,
		);
	}
	drupal_set_title(!empty($specification->name) ? t('Edit specification set !name', array('!name' => $specification->name)) : t('Create new specification set'));

	$form['#tree'] = TRUE;
	$form['#specification'] = $specification;
	$form['#attached']['js'][] = $path . '/js/node_specifications_edit_form.js';
	$form['#attached']['css'][] = $path . '/css/node_specifications.css';
	$form['#attributes']['class'][] = 'node-specifications-edit-form';

	$form['specification']['name'] = array(
		'#type' => 'textfield',
		'#title' => t('Name'),
		'#default_value' => $specification->name,
		'#required' => TRUE,
		'#size' => 32,
	);
	$form['specification']['type'] = array(
		'#type' => 'machine_name',
		'#title' => t('Machine name'),
		'#default_value' => $specification->type,
		'#maxlength' => 32,
		'#required' => TRUE,
		'#machine_name' => array(
			'exists' => 'node_specifications_machine_name_load',
			'source' => array('specification', 'name'),
		),
	);
	$form['specification']['description'] = array(
		'#type' => 'textarea',
		'#title' => t('Description'),
		'#description' => t('Describe this set'),
		'#default_value' => $specification->description,
		'#rows' => 3,
	);
  $form['specification']['as_extrafields'] = array(
    '#type' => 'checkbox',
    '#title' => t('Fields to extrafields'),
    '#description' => t('Provide specification fields as separete extrafields'),
    '#default_value' => $specification->as_extrafields,
  );

	$form['filters'] = array(
		'#type' => 'fieldset',
		'#title' => t('Apply for'),
		/////////'#tree' => TRUE,
	);

// rsi ---- >
// m($specification);
  $node_type = '';
  if (isset($form_state['values']['filters']['node_type'])) { // <- from ajax
    $node_type = $form_state['values']['filters']['node_type'];
  }
  elseif (!empty($specification->node_type)) {
    $node_type = $specification->node_type;
  }
  elseif (isset($form_state['input']['filters']['node_type'])) {
    $node_type = $form_state['input']['filters']['node_type'];
  }

	$form['filters']['node_type'] = array(
		'#type' => 'select',
		'#options' => array(NULL => '- ' . t('Select') . ' -') + node_type_get_names(),
        '#required' => TRUE,
		'#title' => t('Node types'),
		'#default_value' => $node_type,
		'#ajax' => array(
			'callback' => 'node_specifications_load_vocabularies_ajax_callback',
			'wrapper' => 'node-specifications-vocabulary',
		),
	);

	$vocabularies = array();

	// получим доступные машинные имена словарей для  нода
	if ($node_type) {
  	$instances = field_info_instances('node', $node_type);
  	foreach (array_keys($instances) as $field_name) {
  		$field = field_info_field($field_name);
  		if ($field['type'] == 'taxonomy_term_reference' && isset($field['settings']['allowed_values'][0]['vocabulary'])) {
  			$vocabularies[] = $field['settings']['allowed_values'][0]['vocabulary'];
  		}
  	}
	}
  
	$form['filters']['vocabulary'] = array(
		'#prefix' => '<div id="node-specifications-vocabulary">',
		'#suffix' => '</div>'
	);

  
  
  
  if (isset($form_state['values']['filters']['vocabulary']['vid'])) { // <- from ajax
    $default_vid = $form_state['values']['filters']['vocabulary']['vid'];
  }
  elseif (isset($specification->vid))  {
    $default_vid = $specification->vid;
  }
  elseif (isset($form_state['input']['filters']['vocabulary']['vid'])) {
    $default_vid = $form_state['input']['filters']['vocabulary']['vid'];
  } else {
    $default_vid = NULL;
  }
 //else {
 //  if (!empty($specification->vid)) {
 //    $default_vid = $specification->vid;
 //  }
 //}
	if ($vocabularies) {
    $form['filters']['vocabulary'] += array(
  			'#type' => 'item'
    );
	  $term_lists = array();
    
    foreach (taxonomy_vocabulary_load_multiple(NULL, array('machine_name' => $vocabularies)) as $vocabulary) {
      foreach (taxonomy_get_tree($vocabulary->vid) as $term) {
        $term_lists[$vocabulary->vid][$term->tid] = str_repeat('-', $term->depth) . ($term->depth ? ' ' : '') . $term->name;
      }
      // + вычислим схождение массивов терминов чтоб узнать подключеный словарь (исключая ситуацию ajax vid=0)
      if (!empty($term_lists[$vocabulary->vid]) && !$default_vid && !empty($specification->tids) && empty($form_state['values']['filters']['vocabulary']['vid'])) {
        $default_vid = count(array_intersect_key($term_lists[$vocabulary->vid], $specification->tids)) ? $vocabulary->vid : 0; 
      }
      $vocabularies_names[$vocabulary->vid] = $vocabulary->name;      
    }
  // rsi
  	$form['filters']['vocabulary']['vid'] = array(
  		'#type' => 'select',
  		'#options' => array(0 => '- ' . t('Not selected') . ' -') + $vocabularies_names,
  		'#title' => t('Vocabularies'),
  		'#default_value'	=> $default_vid,
  		'#ajax' => array(
  			'callback' => 'node_specifications_load_vocabulary_terms_ajax_callback',
  			'wrapper' => 'node-specifications-vocabulary-terms',
  		),
  	);
   	$form['filters']['vocabulary']['terms'] = array(
   		'#prefix' => '<div id="node-specifications-vocabulary-terms">',
   		'#suffix' => '</div>',
   	);
    if ($default_vid) {
   	  $form['filters']['vocabulary']['terms'] += array(
   	    '#type' => 'item',
  			'#title' => '', //$vocabulary->name,
      );
  		$form['filters']['vocabulary']['terms']['go_deep'] = array(
  			'#type' => 'checkbox',
  			'#title' => t('Apply on selected term sublevels'),
  			'#default_value' => !empty($specification->go_deep),
  		);
  		$form['filters']['vocabulary']['terms']['tids'] = array(
  			'#type' => 'select',
  			'#size' => 10,
  			'#multiple' => TRUE,
  			'#title' => t('Terms'),
        '#title_display' => 'invisible',
  			'#options' => $term_lists[$default_vid],
  			'#default_value' =>  isset($specification->tids) ? $specification->tids : $form_state['input']['filters']['vocabulary']['terms']['tids'] ?? array(),
  		);
    }
	}

	$settings = array();
	if (!empty($specification->type) && !empty($specification->fields)) {
		foreach ($specification->fields as $field) {
			if (in_array($field['field_type'], array('list_text', 'color_select', 'image')) && isset($field['vars'])) {
				$variants = array();
				foreach ($field['vars'] as $var) {
					$variants[] = array(
						'var_id' => $var['var_id'],
						'value' => isset($var['var_value']) ? $var['var_value'] : NULL,
						'data' => isset($var['var_data']) ? $var['var_data'] : NULL,
					);
				}
				$field[$field['field_type']] = $variants;
			}
			$settings[] = $field;
		}
	}
  //m($settings);

	$form['fields'] = array(
		'#title' => t('Fields'),
		'#type' => 'multifield_wrapper',
		'#default_value' => $settings,
		'#add_button' => t('Add specification'),
		'#elements_to_cells' => FALSE,
	);
	$form['fields']['delta'] = array('#type' => 'hidden',);
	$form['fields']['weight'] = array('#type' => 'hidden',);

	$form['fields']['c1'] = array('#markup' => '<div class="container-inline">');
	$form['fields']['title'] = array(
		'#type' => 'textfield',
		'#size' => 40,
		'#title' => t('Name'),
		'#required' => TRUE,
	);
  
   // @TODO: 'enabled' is obsolete column in node_specifications_fields or not ? Where it used? @see also node_specifications_node_view()
	$form['fields']['enabled'] = array(array('#type' => 'hidden', '#default_value' => TRUE));
  
	$form['fields']['important'] = array(
		'#type' => 'checkbox',
		'#title' => t('Required', array(), array('context' => 'specification')),
	);
//	$form['fields']['in_title'] = array(
//		'#type' => 'checkbox',
//		'#title' => t('Add to title'),
//	);
	$form['fields']['c1_end'] = array('#markup' => '</div>');

	$form['fields']['c2'] = array('#markup' => '<div class="container-inline">');
	$form['fields']['field_type'] = array(
		'#type' => 'select',
		'#title' => t('Type'),
		'#options' => _node_specifications_get_fieldtype_names(),
		'#attributes' => array('class' => array('node-spec-field-type-selector'),),
		'#prefix' => '<div class="field-type">',
		'#suffix' => '</div>',
	);

	$units = module_exists('shop_units') ? shop_units_options(TRUE) : array();

	$form['fields']['unit_id'] = array(
		'#type' => 'select',
		'#title' => t('Units'),
		'#options' => $units,
		'#access' => !empty($units),
		'#empty_value' => '',
		'#prefix' => '<div class="js-hide unit-id-select">',
		'#suffix' => '</div>',
	);
	$form['fields']['multiple'] = array(
		'#type' => 'checkbox',
		'#title' => t('Multiple choise', array(), array('context' => 'specification')),
		'#prefix' => '<div class="js-hide multiple-checkbox">',
		'#suffix' => '</div>',
	);
	$form['fields']['c2_end'] = array('#markup' => '</div>');

	// list_text field
	$form['fields']['list_text'] = array(
		'#type' => 'multifield_wrapper',
		'#attributes' => array('class' => array('js-hide', 'list_text', 'sub-fields'),),
		'#add_button' => '+ ' . t('Value'),
		'#max' => 500,
	);
	$form['fields']['list_text']['value'] = array(
		'#type' => 'textfield',
		'#size' => 50,
		'#title' => t('Variants'),
	);
	$form['fields']['list_text']['var_id'] = array('#type' => 'hidden',);

	// color_select field
	$form['fields']['color_select'] = array(
		'#type' => 'multifield_wrapper',
		'#attributes' => array('class' => array('js-hide', 'color_select', 'sub-fields'),),
		'#add_button' => '+ ' . t('Color'),
		'#max' => 500,
	);
	$form['fields']['color_select']['value'] = array(
		'#type' => 'colorpicker',
		'#description' => '',
		'#is_background' => TRUE,
		'#use_hash' => FALSE,
		'#title' => t('Color'),
	);
	$form['fields']['color_select']['data'] = array(
		'#type' => 'textfield',
		'#title' => t('Color name'),
	);
	$form['fields']['color_select']['var_id'] = array('#type' => 'hidden',);

	// image field
	$form['fields']['image'] = array(
		'#type' => 'multifield_wrapper',
		'#attributes' => array('class' => array('js-hide', 'image', 'sub-fields'),),
		'#add_button' => '+ ' . t('Image'),
		'#max' => 500,
	);
	$form['fields']['image']['value'] = array(
		'#title' => t('Image'),
		'#type' => 'managed_file',
		'#progress_indicator' => 'bar',
		'#progress_message' => t('Uploading...'),
		'#upload_location' => 'public://node_specifications_images/',
		'#upload_validators' => array(
			'file_validate_extensions' => array('gif png jpg jpeg'),
		),
	);
	$form['fields']['image']['data'] = array(
		'#type' => 'textfield',
		'#title' => t('Image dsescription'),
	);
	$form['fields']['image']['var_id'] = array('#type' => 'hidden',);

	$form['fields']['hint'] = array('#type' => 'hidden',);

	$form['actions'] = array(
		'#type' => 'container',
		'#weight' => 40,
		'#attributes' => array('class' => array('form-actions')),
	);
	$form['actions']['submit'] = array(
		'#type' => 'submit',
		'#value' => t('Save set'),
		'#submit' => array('node_specifications_form_submit'),
	);

	if (!empty($form['#specification']->nsid)) {
		$form['actions']['delete'] = array(
			'#type' => 'submit',
			'#value' => t('Delete set'),
			'#submit' => array('node_specifications_form_delete_submit'),
			'#weight' => 45,
		);
	}
	$form['#validate'][] = 'node_specifications_form_validate';

	return $form;
}

function node_specifications_load_vocabularies_ajax_callback($form, &$form_state) {
	return $form['filters']['vocabulary'];
}
function node_specifications_load_vocabulary_terms_ajax_callback($form, &$form_state) { //rsi
	return $form['filters']['vocabulary']['terms'];
}


function node_specifications_form_validate($form, &$form_state) {
	$specification = $form['#specification'];

	if (empty($specification->type)) {
		if (preg_match('/^[0-9]/', $form_state['values']['specification']['type'])) {
			form_set_error('specification][type', t('The machine name must start from letter.'));
		}
	}
}

function node_specifications_form_submit($form, &$form_state) {
	$specification = $form['#specification'];
  $as_extrafields_orig = $specification->as_extrafields;
	//base properties
	foreach ($form_state['values']['specification'] as $key => $value) {
		$specification->$key = $value;
	}

  
	unset($specification->tids, $specification->vid, $specification->fields);

//	m($form_state['values']['filters']['vocabulary']);
	if (!empty($form_state['values']['filters']['node_type'])) {
		$specification->node_type = $form_state['values']['filters']['node_type'];
	}
  else {
    $specification->node_type = NULL;
  }
  
  if (!empty($form_state['values']['filters']['vocabulary']['vid'])) {
    $specification->vid = $form_state['values']['filters']['vocabulary']['vid'];
    if (!empty($form_state['values']['filters']['vocabulary']['terms']['tids'])) {
      if ($terms = taxonomy_term_load_multiple(FALSE, array('vid' => $specification->vid, 'tid' => $form_state['values']['filters']['vocabulary']['terms']['tids']))) {
        $specification->tids = array_keys($terms);
      }
    }
  }
  
  $specification->go_deep = !empty($form_state['values']['filters']['vocabulary']['terms']['go_deep']);

	//fields
	foreach ($form_state['values']['fields'] as $field) {
		$field_type = $field['field_type'];
		if (in_array($field_type, array('list_text', 'color_select', 'image'), FALSE) && !empty($field[$field_type])) {
			$vars = array();
			foreach ($field[$field_type] as $variant) {
				$vars[] = array(
					'var_id' => isset($variant['var_id']) ? $variant['var_id'] : NULL,
					'var_value' => isset($variant['value']) ? $variant['value'] : NULL,
					'var_data' => isset($variant['data']) ? $variant['data'] : NULL,
				);
			}

			if ($vars) {
				$field['vars'] = $vars;
			}
			unset($field[$field_type]);
		}
		$specification->fields[] = $field;
	}
  
  
	if (node_specifications_save($specification)) {
    if ($as_extrafields_orig != $specification->as_extrafields) {
      field_cache_clear();
    }
		drupal_set_message(t('Specification set %name saved.', array('%name' => $specification->name)));
	}
	$form_state['redirect'] = 'admin/structure/node_specifications/' . $specification->nsid;
}

function node_specifications_form_delete_submit($form, &$form_state) {
	$form_state['redirect'] = 'admin/structure/node_specifications/' . $form['#specification']->nsid . '/delete';
}

function node_specifications_delete_form($form, &$form_state, $specification) {
	$form_state['specification'] = $specification;

	// Ensure this include file is loaded when the form is rebuilt from the cache.
	$form_state['build_info']['files']['form'] = drupal_get_path('module', 'node_specifications') . 'inc/node_specifications.forms.inc';

	$form['#submit'][] = 'node_specifications_delete_form_submit';

	$form = confirm_form(
		$form,
		t('Are you sure you want to delete the %name specification set?', array('%name' => $specification->name)),
		'admin/structure/node_specifications',
		('<p>' . t('This action cannot be undone.') . '</p>'),
		t('Delete'),
		t('Cancel'),
		'confirm'
	);

	return $form;
}

function node_specifications_delete_form_submit($form, &$form_state) {
	$specification = $form_state['specification'];
	node_specifications_delete($specification);
	drupal_set_message(t('The specification set %name has been deleted.', array('%name' => $specification->name)));
	$form_state['redirect'] = 'admin/structure/node_specifications';
}

/**
 * @see node_specifications_fields_overview_submit()
 * @see theme_node_specifications_fields_overview()
 */
function node_specifications_fields_overview($form, &$form_state, $specification) {

	if (!$specification) {
		drupal_not_found();
    drupal_exit();
	}
  else {
    drupal_set_title(t('Specification set') . ': ' . $specification->name);

  //  m($specification);
    $form['#tree'] = TRUE;
    $form['#specification'] = $specification;
    $form['#header'] = array(
      '_name' => t('Name', array(), array('context' => 'product')),
      '_in_title' => t('Use in title'),
      '_weight' => t('Weight'),
    );
    $fieldtype_names = _node_specifications_get_fieldtype_names();

    $summary = array();
    if ($specification->node_type) {
      if ($node_type = node_type_load($specification->node_type)) {
        $summary[] = t('Type of material') . ': ' . check_plain($node_type->name);
      }
    }

    if ($specification->vid && ($vocab = taxonomy_vocabulary_load($specification->vid))) {
      
      $summary[] = t('Vocabulary') . ': ' . check_plain($vocab->name);
      $vocabs_markup = array();
      if (!empty($specification->tids)) {
        if ($terms = taxonomy_term_load_multiple($specification->tids, array('vid' => $specification->vid))) {
          $_markup_terms = array();
          foreach ($terms as $term) {
            $_markup_terms[] = $term->name;
          }
          $summary[] =  t('Category') . ': ' . check_plain(implode(', ', $_markup_terms));
        }
      }
      if ($specification->go_deep) {
        $summary[] = t('Include subterms');
      }
    }
    if ($summary) {
      $form['specification_info'] = array(
        '#type' => 'item',
        '#title' => t('Uses') . ' :',
        '#markup' => implode('<br />', $summary),
      );
    }

    if (!empty($specification->fields)) {
      $form['specification_fields'] = array();
      foreach ($specification->fields as $field) {
        $form['specification_fields'][$field['delta']] = array(
          'name' => array(
            '#type' => 'item',
            '#markup' => check_plain($field['title'] . ' (' . $fieldtype_names[$field['field_type']] . ')') . ($field['important'] ? ' ' . theme('form_required_marker') : ''),
          ),
          //'important' => array(
          //  '#type' => 'checkbox',
          //  '#default_value' => $field['important'],
          //),
          'in_title' => array(
            '#type' => 'checkbox',
            '#default_value' => $field['in_title'],
          ),
          'weight' => array(
            '#type' => 'weight',
            '#delta' => 10,
            '#title_display' => 'invisible',
            '#title' => t('Weight'),
            '#default_value' => $field['weight'],
          ),
        );
      }
    }
    $form['specification_fields']['#suffix'] = '<small><em>' . theme('form_required_marker') . ' - ' . drupal_strtolower(t('Required fields')) . '</em></small>';
    $form['actions'] = array(
      '#type' => 'actions',
      '#tree' => FALSE,
      'submit' => array(
        '#type' => 'submit', 
        '#value' => t('Save'),
      ),
    );

    return $form;    
  }

}

function node_specifications_fields_overview_submit($form, &$form_state) {
	if (!empty($form['#specification'])) {
		if ($specification = node_specifications_load($form['#specification']->nsid)) {
      $form_fields = $form_state['values']['specification_fields'];
      foreach ($specification->fields as $idx => $field) {
        if (!empty($form_fields[$field['delta']])) {
          foreach ($form_fields[$field['delta']] as $field_name => $field_value) {
            $specification->fields[$idx][$field_name] = $field_value;
          }
        }
      }
      node_specifications_save($specification);
      drupal_set_message(t('The changes have been saved.'));      
    }
	}
}

// List of all node specifications //Ruban
// see else theme_node_specifications_overview_form - theme automatic run for this form
function node_specifications_overview_form($form, &$form_state) {

	drupal_set_title(t('Node specifications'));
//	$header = array(t('Name'), t('Description'), t('Operations'),);
	$admin_path = 'admin/structure/node_specifications/';

//	$form['#header'] = array(t('Name', array(), array('context' => 'product')), t('Required to fill'), t('Use in title'), t('Weight'),);
//	$fieldtype_names = _node_specifications_get_fieldtype_names();

  $form['#tree'] = TRUE;
  $weights = array();
	foreach (node_specifications_load_multiple(FALSE) as $nsid => $specification) {
    	$info_markup = '';
    	$node_type = '';
    	if (!empty($specification->node_type)) {
        if ($node_type = node_type_load($specification->node_type)) {
          $node_type = $node_type->name;
        }
    	}
    	$vocabs = array();
    	if (!empty($specification->vid)) {
        $vocabs[$specification->vid] = array();
    	}
    	if (!empty($specification->tids)) {
    		foreach ($specification->tids as $tid) {
    			$term = taxonomy_term_load($tid);
    			if ($term) {
    				$vocabs[$term->vid][$term->tid] = $term->tid;
    			}
    		}
    	}
	    $_markup = array();
    	if ($vocabs) {
    		$vocabs_markup = array();
    		foreach ($vocabs as $vid => $tids) {
    			$vocab = taxonomy_vocabulary_load($vid);
    			if ($vocab) {
    				$_markup[$vid] =   t('Vocabulary') . ': ' . $vocab->name;
    				if (!empty($tids)) {
    					$terms = taxonomy_term_load_multiple($tids);
    					if ($terms) {
    						$_markup_terms = array();
    						foreach ($terms as $term) {
    							$_markup_terms[] = $term->name;
    						}
    						$_markup[$vid] .= '<br />' . t('Category') . ':' . implode(', ', $_markup_terms);
    					}
    				}
    				if ($specification->go_deep) {
    					$_markup[$vid] .= '<br />' . t('Include subterms');
    				}
    			}
    		}
    	}
    /****<< кусок ***/
//    m($specification);
    $uses_markup = t('Type of material') . ': ' . $node_type . '<br />' . implode('<br />', $_markup);

    $form1[$nsid]['name'] = array('#markup' => l($specification->name, $admin_path . $nsid . '/list'));
    $form1[$nsid]['uses'] =  array('#markup' => '<span style="font-size:13px">' . $uses_markup . '</span>');
    $form1[$nsid]['description'] = array('#markup' => $specification->description);
    $form1[$nsid]['operations'] = array(
      '#type' => 'item',
      '#title' => theme(
      			'links', array(
      				'links' => array(
      					'edit' => array('href' => $admin_path . $nsid . '/edit', 'title' => theme('pictogram', array('icon' => 'pencil')), 'attributes' => array('title' => t('Edit')), 'html' => TRUE),
      					'delete' => array('href' => $admin_path . $nsid . '/delete', 'title' => theme('pictogram', array('icon' => 'trash')), 'attributes' => array('title' => t('Delete')), 'html' => TRUE),
      				),
      				'attributes' => array('class' => 'links inline operations', 'style' => 'white-space:nowrap;')
      		)
      ),
    );
    $weight = empty($specification->weight) ? $nsid : $specification->weight;
    $form1[$nsid]['weight'] = array(
      '#type' => 'weight',
      '#title' => t('Weight for @title', array('@title' => $specification->name)),
      '#title_display' => 'invisible',
      '#delta' => 10,
      '#default_value' => $weight,
    );
    $weights[$nsid] = $weight; // для сортирвоки по весу ПЕРЕД выводом формы
  }
  // отсортируем окончательный список строк формы по весу
  asort($weights);
  //m($weights);
  foreach ($weights as $nsid => $weight) {
    $form[$nsid] = $form1[$nsid];
  }
  // Only make this form include a submit button and weight if more than one
  // vocabulary exists.
  if (count(node_specifications_load_multiple(FALSE)) > 0) {
    $form['actions'] = array('#type' => 'actions');
    $form['actions']['submit'] = array('#type' => 'submit', '#value' => t('Save'));
  }
	return $form;
}

// submit order of list of all specifications // Ruban
function node_specifications_overview_form_submit($form, &$form_state) {
  $w = 0;
  // in $form_state ['input'] is ordered, used this. but for data must use 'values'
  foreach ($form_state['input'] as $nsid => $values) {
    $w ++;
    if (!empty($values['weight'])) {
      db_query('UPDATE {node_specifications} SET weight = :weight WHERE nsid = :nsid', array(':weight' => $w, ':nsid' => $nsid));
    }
  }
	//	node_specifications_save($specification);
	drupal_set_message(t('The changes have been saved.'));
}

//@TODO
/**
 * Compare page
 *
 * Callback for: node/compare
 */
function node_specifications_compare() {
	$build = array();
	drupal_add_js(drupal_get_path('module', 'node_specifications') . '/js/node_specifications_compare_page.js');
	if (!empty($_COOKIE['compare_list'])) {
		$nids = array_unique(array_filter(explode(',', $_COOKIE['compare_list'])));
		$nodes = $nids ? node_load_multiple($nids) : array();
		drupal_add_css(drupal_get_path('module', 'node_specifications') . '/css/node_specifications.css');
		$rows = $header = array();

		//check that nodes have same node_type
		$node_types = array();
		foreach ($nodes as $idx => $node) {
      if (!node_access('view', $node)) {
        unset($nodes[$idx]);
      }
      else {
        $node_types[$node->type] = $node->type;
      }

      //for displaying specificaiotns without labels
      if (!empty($node->specifications)) {
        foreach ($node->specifications as &$specification) {
          $specification->as_extrafields = 1;
        }
      }
		}
		if (count($node_types) > 1) {
      drupal_set_message(t('Nodes must be same type'), 'error');
		}
		else {
      $node_type = reset($node_types);
			if (!empty($_GET['destination'])) {
        $back_url = drupal_parse_url($_GET['destination']);
			}

			$nodes_view = node_view_multiple($nodes, 'teaser');
			$nodes_view = $nodes_view['nodes'];

			$header = array('',);
			foreach ($nodes as $node) {
				$header[$node->nid] = '';
			}

			//Image row | in header
			$instances = field_info_instances('node', $node_type);
			foreach ($instances as $instance) {
				$field = field_info_field($instance['field_name']);
				if ($field['type'] === 'image') {
					foreach ($nodes as $node) {
						$nodes_view[$node->nid][$field['field_name']]['#title_display'] = 'invisible';
						$header[$node->nid] .= !empty($nodes_view[$node->nid][$field['field_name']]) ? render($nodes_view[$node->nid][$field['field_name']]) : '';
					}
					break;
				}
			}
			//title row | in header
			foreach ($nodes as $node) {
				$header[$node->nid] .= !empty($nodes_view[$node->nid]['title']) ? render($nodes_view[$node->nid]['title']) : '';
			}

			//spec fields rows
			$all_specs = array();
			//gather all specifications
			foreach ($nodes as $node) {
				if (!empty($node->specifications)) {
					foreach ($node->specifications as $spec) {
						$all_specs[$spec->nsid] = $spec;
					}
				}
			}
			if ($all_specs) {
				foreach ($all_specs as $nsid => $spec) {
					if (!empty($spec->fields)) {
						foreach ($spec->fields as $spec_field) {
							$index = $nsid . '/' . $spec_field['delta'];
              if (addons_extra_field_get_settings('node', $node_type, 'display', 'teaser', 'specifications', 'visibility_settings', $index)) {
                $rows[$index] = array(
                  array(
                    'data' => $spec_field['title'],
                    'header' => TRUE,
                  ),
                );
                foreach ($nodes as $node) {
                  if (!empty($nodes_view[$node->nid][$index])) {
                    $nodes_view[$node->nid][$index]['#label_display'] = 'hidden';
                    $data = $nodes_view[$node->nid][$index];
                  }
                  else {
                    $data = '&nbsp;';
                  }
                  $rows[$index][$node->nid]['data'] = $data;
                }
              }
						}
					}
				}
			}

			//price and buy_button row
			$rows['price'] = array(array('data' => t('Price'), 'header' => 1));
			foreach ($nodes as $node) {
        // price
        if (!empty($nodes_view[$node->nid]['price'])) {
          $rows['price'][$node->nid]['data'][] = $nodes_view[$node->nid]['price'];
        }
        // Buy button
        if (!empty($nodes_view[$node->nid]['buy_button'])) {
          $rows['price'][$node->nid]['data'][] = $nodes_view[$node->nid]['buy_button'];
        }
        if (empty($rows['price'][$node->nid]['data'])) {
          $rows['price'][$node->nid]['data'] = '&nbsp';
        }
			}

			$path = $_GET['q'];
			$query = drupal_http_build_query(drupal_get_query_parameters());
			if ($query != '') {
				$path .= '?' . $query;
			}

			//clear/delete row
			$clear_all = array(
				'#type' => 'link',
				'#title' => t('Clear'),
				'#href' => 'node/compare/clear',
				'#attributes' => array('class' => array('form-submit', 'cmp-manipulations', 'cmp-clear-all'), 'title' => t('Clear comparing list and return'),),
			);
			if (!empty($back_url)) {
				$clear_all['#options']['query']['destination'] = url($back_url['path'], array('query' => $back_url['query']));
			}
			$rows['delete'] = array(array('data' => render($clear_all), 'align' => 'center', 'header' => 1),);
			foreach ($nodes as $node) {
				$delete_link = array(
					'#type' => 'link',
					'#title' => 'X',
					'#href' => 'node/compare/remove/' . $node->nid,
					'#options' => array('query' => array('destination' => $path),),
					'#attributes' => array('class' => array('form-submit', 'cmp-manipulations'), 'title' => t('Remove from comparing'),),
				);
				$rows['delete'][$node->nid] = array('data' => render($delete_link), 'align' => 'center');
			}

			$build['compare_table_wrapper'] = array(
				'#prefix' => '<div id="compare-table-wrapper">', // needs for overflow-x:scroll css (@see node_specifications.css)
				'#suffix' => '</div>',
				'table' => array(
					'#theme' => 'table',
					'#header' => $header,
					'#rows' => $rows,
                    '#sticky' => FALSE,
					'#attributes' => array('class' => array('compare-table'),),
					'#empty' => t('Nothing to compare'),
				),
			);
		}
	}
	else {
      $build[]['#markup'] = '<p>' . t('Compare list is empty') . '</p>';
	}

	//back link
	if (!empty($back_url)) {
		$build['back_link'] = array(
			'#type' => 'link',
			'#prefix' => '<br /> &larr;',
			'#title' => t('Go back'),
			'#href' => $back_url['path'],
			'#options' => array('query' => $back_url['query']),
		);
	}

	return $build;
}

function node_specifications_filter_preset_form($form, &$form_state, $preset = NULL) {
  if (!$preset) {
    $preset = array(
      'id' => NULL,
      'title' => '',
      'base_uri' => '',
      'query_params' => '',
      'weight' => '',
    );

    if (!empty($_GET['destination'])) {
      $destination_uri = drupal_parse_url($_GET['destination']);
      if (!empty($destination_uri['path']) && !empty($destination_uri['query'])) {
        $preset['base_uri'] = $destination_uri['path'];

        $filtered_query = array();
        foreach (_node_specifications_get_filter_query_symbols() as $symbol) {
          if (!empty($destination_uri['query'][$symbol])) {
            $filtered_query[$symbol] = $destination_uri['query'][$symbol];
          }
        }
        if ($filtered_query) {
          $preset['query_params'] = drupal_http_build_query($filtered_query);
        }

      }
    }
  }
	$form['preset']['#tree'] = TRUE;

  $form['preset']['id'] = array(
    '#type' => 'value',
    '#value' => $preset['id'] ?? NULL,
  );

  $form['preset']['weight'] = array(
    '#type' => 'value',
    '#value' => $preset['weight'] ?? NULL,
  );

  $form['preset']['title'] = array(
    '#type' => 'textfield',
    '#title' => t('Title'),
    '#default_value' => $preset['title'] ?? '',
    '#required' => TRUE,
  );

  $form['preset']['content'] = array(
    '#type' => 'text_format',
    '#title' => t('Content'),
    '#default_value' => $preset['content'] ?? '',
  );

  $form['preset']['base_uri'] = array(
    '#type' => 'textfield',
    '#title' => t('Base uri'),
    '#default_value' => $preset['base_uri'] ?? '',
    '#required' => TRUE,
  );

  $form['preset']['query_params'] = array(
    '#type' => 'textfield',
    '#title' => t('Query params'),
    '#default_value' => $preset['query_params'] ?? '',
    '#required' => TRUE,
  );

	//seo stuff
	if (module_exists('seo')) {
		$q = db_select('seo_meta', 'sm')
			->fields('sm')
			->condition('entity_type', 'node_specification_filter_presets')
			->condition('id', $preset['id']);
		$seo_meta = $q->execute()->fetchAssoc();

		$form['seo'] = _seo_subform($seo_meta ?: array());
		$form['seo']['entry_num'] = array('#type' => 'value', '#value' => !empty($seo_meta['entry_num']) ? $seo_meta['entry_num'] : NULL);
	}
	//

	$form['#submit'][] = 'node_specifications_filter_preset_form_submit';
	$form['actions'] = array(
		'#type' => 'actions',
		'#tree' => FALSE,
		'submit' => array(
      '#type' => 'submit',
      '#value' => t('Save'),
    ),
		'delete' => array(
      '#type' => 'submit',
      '#value' => t('Delete'),
    ),
	);

	return $form;
}

function node_specifications_filter_preset_form_submit($form, &$form_state) {
	$values = $form_state['values'];
	if (!empty($values['op']) && !empty($values['preset'])) {
		$preset = $values['preset'];
		if (!empty($values['submit']) && $values['op'] === $values['submit']) {
			$preset = node_specifications_filter_preset_save($preset);
			if (!empty($preset['id'])) {
				drupal_set_message(t('Filter preset created'));
				//seo stuff
				if (!empty($values['seo']) && module_exists('seo')) {
					$uri = drupal_parse_url($preset['base_uri'] . '?' . $preset['query_params']);
					$seo = $values['seo'];
					$seo['entity_type'] = 'node_specification_filter_presets';
					$seo['uri'] = ltrim(url($uri['path'], array('query' => $uri['query'])), '/');
					$seo['bundle'] = '';
					$seo['id'] = $preset['id'];
					_seo_presave_items($seo);
					drupal_write_record('seo_meta', $seo, empty($seo['entry_num']) ? array() : 'entry_num');
				}
			}
		}
		elseif (!empty($values['delete']) && $values['op'] === $values['delete']) {
			node_specifications_filter_preset_delete($preset);
			drupal_set_message(t('Filter preset deleted'));
		}
	}
}

function node_specifications_hints_form($form, &$form_state, $specification) {
	if (empty($specification->fields)) {
		drupal_set_message(t('No fields in specification'), 'warning');
	}
	else {
		$form['#tree'] = TRUE;
		$form['#specification'] = $specification;
    // nsid - id группы хар., delta - id поля

		foreach ($specification->fields as $delta => $field) {
  		$form['hints'][$delta] = array(
  			'#type' => 'textarea',
  			'#title' => $field['title'],
  			'#default_value' => $field['hint'],
  		);
		}
	}

	$form['actions'] = array(
		'#type' => 'actions',
		'submit' => array('#type' => 'submit', '#value' => t('Save'),),
	);

	return $form;
}

function node_specifications_hints_form_submit($form, &$form_state) {
	if (!empty($form['#specification']) && ($specification = node_specifications_load($form['#specification']->nsid))) {
    $fields = $specification->fields;
    foreach ($form_state['values']['hints'] as $delta => $text) {
      $fields[$delta]['hint'] = trim($text);
    }
    node_specifications_save_fields($fields, $specification->nsid);
    drupal_set_message(t('The changes have been saved.'));
	}
}





