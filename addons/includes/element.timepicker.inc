<?php




// DEPRECATED 










function _addons_process_timepicker($element, &$form_state) {
	$element += array(
		'#attributes' => array(),
	);
	$element['#attributes'] += array('placeholder' => t('HH:MM'));
	$element['#attributes']['class'][] = 'timepicker';
	$element['#attached']['js'][] = drupal_get_path('module', 'addons') . '/js_behaviors/date_time_pickers.js';

	foreach (array('#default_value', '#value',) as $key) {
		if (!empty($element[$key])) {
			if (is_numeric($element[$key])) {
				$element[$key] = format_date($element[$key], 'custom', 'H:i');
			}
		}
	}

	return $element;
}

function _addons_validate_timepicker(&$element, &$form_state) {
	$value = 0;
	if (!empty($element['#value'])) {
		if (!preg_match('/^\s*(2[0-3]|[0-1]?[0-9])\s*:\s*([0-5]?[0-9])$/', $element['#value'], $matches)) {
			form_error($element, t('Incorrect time value, time format is HH:MM'));
		}
		else {
			$value = ($matches[1] * 3600) + ($matches[2] * 60);
		}
	}
	form_set_value($element, $value, $form_state);
}
