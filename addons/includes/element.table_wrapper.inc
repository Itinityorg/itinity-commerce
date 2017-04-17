<?php


/**
* Helper function for theme_table_wrapper() (see below)
*/
function _addons_element_to_table_rows($element, $wrapper_element) {
	$title = isset($element['#title']) ? $element['#title'] : '';
	$rows = array();
	if (empty($element['#type']) && !empty($element[0]['#field_name'])) {
		$title = '';
	}
	if ($title) {
		$label_cell = array(
			'label' => array(
				'#type' => 'item',
				'#markup' => theme(
					'form_element_label',
					array(
						'element' => array(
							'#id' => isset($element['#attributes']['id']) ? $element['#attributes']['id'] : (isset($element['#id']) ? $element['#id'] : NULL),
							'#title' => !empty($element['#title']) ? $element['#title'] . ':&nbsp;' : '',
							)
						+ $element
						+ array(
							'#title_display' => 'before',
							)
						)
					),
			),
		);
		if (!empty($element['#states'])) {
			$label_cell['label']['#states'] = $element['#states'];
			$label_cell['label']['#id'] = 'label-' . $element['#id'];
			drupal_process_states($label_cell['label']);
		}
		$row = array(
			'data' => array(
				array('class' => 'first-col', 'data' => $label_cell),
				array('data' => array('#title_display' => 'invisible') + $element),
			),
			);
		if (!empty($wrapper_element['#summary_col'])) {
			$row['data'][0]['header'] = TRUE;
		}
	}
	else {
		if (0 && !in_array($element['#type'], array('radios', 'checkboxes')) && ($childrens = element_get_visible_children($element))) {
			foreach ($childrens as $key) {
				foreach (_addons_element_to_table_rows($element[$key], $wrapper_element) as $r) {
					$rows[] = $r;
				}
			}
		}
		else {
			if ($element) {
				$row = array(
					'data' => array(
						array('class' => 'first-col', 'data' => $element, 'colspan' => 2,),
						),
				);
			}
		}
	}
	if (isset($row)) {
		$rows[] = $row;
	}
	return $rows;
}

/**
* Wrapper for groups of elements
* Generates table.
* Place each children element in personal rows in table. Split each children element onto label and field and place in personal cells.
* Usage example:
	$form['test_group'] = array(
		'#theme' => 'table_wrapper',
		'#title' => 'test', // Optional caption of table
		'#suppress_css' => TRUE, // Optional. Def is FALSE. Disable table decorations - supress default drupal styling for table (rows borders, even/odd highlight). Make table markup invisible.
		'#summary_row' => TRUE, // Optional. Def is FALSE. Apply 'header' property to row with last element
		'#type' => 'fieldset', // optional. Table tag will rendered inside it.
		'#collapsible' => TRUE, // For example. Standard property for fieldset, see #type
		'#tree' => TRUE, //  For example. Using other base properties of form elements.
		'element1' => array(
			'#title' => '1',
			'#type' => 'textfield',
			'#states' => array( // '#states' in splitted elements still supported
				'ivisible' => array( ... ... ... ),
			),
		),
		'element1' => array(
			'#title' => '2',
			'#type' => 'checkbox',
		),
);
*/
function theme_table_wrapper($variables) {
	$element = $variables['element'];
	$suppress_css = !empty($element['#suppress_css']);
	$rows = array();
	foreach (element_get_visible_children($element) as $key) {
		foreach (_addons_element_to_table_rows($element[$key], $element) as $r) {
			if ($suppress_css) {
				$r['no_striping'] = TRUE;
			}
			$rows[] = $r;
		}
	}
	if (!empty($element['#summary_row'])) {
		$last_idx = count($rows) - 1;
		foreach ($rows[$last_idx] as $idx => $row) {
			foreach ($row as $i => $r) {
				$rows[$last_idx][$idx][$i]['header'] = TRUE;
			}
		}
	}
	$classes = array('form-item table-wrapper');
	if ($suppress_css) {
		$classes[] = 'supress-css';
	}
	$classes = array_values(
		drupal_map_assoc($classes) +
		(
			!empty($element['#attributes']['class'])
			?
			(is_array($element['#attributes']['class']) ? drupal_map_assoc($element['#attributes']['class']) : array($element['#attributes']['class'] => $element['#attributes']['class']))
			:
			array()
		)
	);
	$attributes = array(
		'class' => $classes,
		'style' => array(),
		);
	foreach (array('width', 'height') as $prop) {
		if (isset($element['#' . $prop])) {
			$val = $element['#' . $prop];
			$attributes['style'][] = $prop . ':' . $val . '%';
		}
	}
	$attributes['style'][] = !empty($element['#attributes']['style']) ? $element['#attributes']['style'] : '';
	$attributes['id'] = isset($element['#id']) ? $element['#id'] : NULL;
	if (!empty($element['#draggable'])) {
		foreach ($rows as $idx => $row) {
			if ($row) {
				$rows[$idx]['class'][] = 'draggable';
				$rows[$idx]['data'][] = array(
					'data' => '<input type="hidden" class="drag-weight" />',
					'class' => array('element-invisible'),
				);
			}
		}
		drupal_add_tabledrag($element['#id'], 'order', 'sibling', 'drag-weight');
		if (empty($_COOKIE['Drupal_tableDrag_showWeight'])) {
			drupal_add_css('.tabledrag-toggle-weight {display: none;}', 'inline');
		}
	}

	return theme(
		'table',
		array(
			'caption' => isset($element['#title']) && !isset($element['#type']) ? '<div class="form-item"><label>' . $element['#title'] . '</label></div>' : '',
			//'header' => (empty($element['#type'])) && !empty($element['#title']) ? array(array('colspan' => 2, 'data' => array('#type' => 'item', '#title' => $element['#title']))) : NULL,
			'attributes' => $attributes,
			'rows' => $rows,
		)
	);
}


