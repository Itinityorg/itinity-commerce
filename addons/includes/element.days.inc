<?php



function _addons_process_days($element, &$form_state, $complete_form) {
	$element += array(
		'#title' => '',
		'#inline'	=> TRUE,
		'#one' => FALSE,
	);

	if ($element['#inline']) {
		$element += array('#prefix' => '', '#suffix' => '',);
		$element['#prefix'] .= '<div class="container-inline">';
		$element['#suffix'] = '</div> ' .$element['#suffix'];
	}

	if ($element['#one']) {
		$element['#type'] = 'radios';
	}

	$element['#options'] = array(
		1	=> t('Mon'),
		2	=> t('Tue'),
		3	=> t('Wed'),
		4	=> t('Thu'),
		5	=> t('Fri'),
		6	=> t('Sat'),
		0	=> t('Sun'),
	);
	return $element;
}

function addons_validate_days(&$element, &$form_state) {
	$values = drupal_array_get_nested_value($form_state['values'], $element['#array_parents']);
	form_set_value($element, array_filter($values, 'is_string'), $form_state);
}
