<?php

function node_specifications_generate_form($form, &$form_state) {
	$form['node_type'] = array(
		'#type'    => 'select',
		'#title'   => t('Type'),
		'#options' => node_type_get_names(),
	);

	$specs = array();
	foreach (node_specifications_get_names() as $spec) {
		$specs[$spec->nsid] = $spec->name;
	}

	$form['sp_set'] = array(
		'#type'    => 'checkboxes',
		'#title'   => t('Specifications'),
		'#options' => $specs,
	);

	$form['submit'] = array(
		'#type'   => 'submit',
		'#value'  => t('GO'),
		'#submit' => array('node_specifications_generate_form_submit'),
	);

	return $form;
}

function node_specifications_generate_form_submit($form, &$form_state) {
	$specs = array();
	if (!empty($form_state['values']['sp_set'])) {
		$specs = node_specifications_load_multiple($form_state['values']['sp_set']);
	}

	$nids = array();
	$operations = array();
	if (!empty($form_state['values']['node_type'])) {
		$nids = db_select('node')
			->fields('node', array('nid'))
			->condition('type', $form_state['values']['node_type'])
			->execute()->fetchCol();
	}

	foreach ($nids as $nid) {
		$operations[] = array('node_sepecification_generate_batch', array($nid, $specs));
	}

	batch_set(array(
		'operations' => $operations,
		'file'       => drupal_get_path('module', 'node_specifications') . '/inc/node_specifications.test.inc',
	));
}

function node_sepecification_generate_batch($nid, $specs, &$context) {
	$node = node_load($nid);

	$node->specifications_data = array();
	foreach ($specs as $spec_type) {
		foreach ($spec_type->fields as $delta => $field) {

			switch ($field['field_type']) {
				case 'number_integer':
					$node->specifications_data[$field['nsid']][$delta] = mt_rand(0, 10000);
					break;
				case 'list_text':
				case 'color_select':
					$vars = array_keys($field['vars']);

					if ($field['multiple']) {
						$node->specifications_data[$field['nsid']][$delta] = array();
						foreach ($vars as $var) {
							if (mt_rand(0, 1)) {
								$node->specifications_data[$field['nsid']][$delta] += array($var => $var);
							}
						}
					}
					else {
						$node->specifications_data[$field['nsid']][$delta] = mt_rand(array_shift($vars), array_pop($vars));
					}

					break;
				case 'text':
					$node->specifications_data[$field['nsid']][$delta] = _ns_generate_word(25);
					break;
			}
		}
	}
	$context['results'][] = $node->nid . ' : ' . check_plain($node->title);
	$context['message'] = t('Loading node "@title"', array('@title' => $node->title));

	if (!$node->price) {
		$node->price = mt_rand(0, 100000);
	}
	if (!$node->qty) {
		$node->price = mt_rand(0, 10);
	}

	node_save($node);
}

function _ns_generate_word($length) {

	mt_srand((double) microtime() * 1000000);

	$vowels = array('a', 'e', 'i', 'o', 'u');
	$cons = array(
		'b', 'c', 'd', 'g', 'h', 'j', 'k', 'l', 'm', 'n', 'p', 'r', 's', 't', 'u', 'v', 'w', 'tr',
		'cr', 'br', 'fr', 'th', 'dr', 'ch', 'ph', 'wr', 'st', 'sp', 'sw', 'pr', 'sl', 'cl', 'sh'
	);

	$num_vowels = count($vowels);
	$num_cons = count($cons);
	$word = '';

	while (strlen($word) < $length) {
		$word .= $cons[mt_rand(0, $num_cons - 1)] . $vowels[mt_rand(0, $num_vowels - 1)];
	}

	return substr($word, 0, $length);
}
