<?php


//Slider form element
/**
 * Processes slider element
 */
function addons_process_slider($element) {
	if ($element['#max'] == $element['#min']) {
		return $element;
	}

	drupal_add_library('system', 'ui.slider');
	$module_path = drupal_get_path('module', 'addons');
	drupal_add_js($module_path . '/js_behaviors/' . 'slider.js');
	drupal_add_css($module_path . '/css/' . 'slider.css');

	$element['#tree'] = TRUE;
	$element['#prefix'] = '<div class="slider">';
	$element['#suffix'] = '</div>';

	$element['t0'] = array(
		'#markup' => '<table><tr><td>'
	);
	// Generate left input for slider
	$element['left'] = array(
		'#type' => 'textfield',
		'#title' => $element['#left'],
		'#default_value' => !empty($element['#left_value']) ? $element['#left_value'] : $element['#min'],
		'#size' => $element['#size'],
		'#attributes' => array('class' => array('slider-field', 'left')),
		'#prefix' => '<div class="container-inline">',
		'#suffix' => '</div>',
	);
	$element['t1'] = array('#markup' => '</td><td align="right">');

	// Generate right input for slider
	$element['right'] = array(
		'#type' => 'textfield',
		'#title' => $element['#right'],
		'#default_value' => !empty($element['#right_value']) ? $element['#right_value'] : $element['#max'],
		'#size' => $element['#size'],
		'#attributes' => array('class' => array('slider-field', 'right')),
		'#field_suffix' => $element['#field_suffix'],
		'#prefix' => '<div class="container-inline">',
		'#suffix' => '</div>',
	);

	$element['t2'] = array('#markup' => '</td></tr></table>');

	// Create markup for slider container
	$element['container']['#markup'] = '<div class="slider-container"></div>';

	$element['min_val'] = array(
		'#type' => 'hidden',
		'#value' => $element['#min'],
		'#attributes' => array('class' => array('slider-min'),),
	);
	$element['max_val'] = array(
		'#type' => 'hidden',
		'#value' => $element['#max'],
		'#attributes' => array('class' => array('slider-max'),),
	);

	foreach ($element['#element_validate'] as $validate_function) {
		call_user_func($validate_function, $element);
	}

	return $element;
}

function addons_validate_slider($element) {
	if (isset($element['#left_value']) && isset($element['#right_value'])) {
		if (!is_numeric($element['#left_value']) || !is_numeric($element['#right_value'])) {
			form_error($element, t('The value should be a valid integer number'));
		}
		elseif ($element['#left_value'] > $element['#right_value']) {
			form_error($element, t("Left value can't be larger than right value"));
		}
	}
}
