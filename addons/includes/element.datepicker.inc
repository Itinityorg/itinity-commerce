<?php


// DEPRECATED 



function _addons_process_datepicker($element, &$form_state) {
	$element['#attributes'] += array('class' => array('datepicker'), 'placeholder' => t('DD.MM.YYYY'));
	$element['#attached']['library'][] = array('system', 'ui.datepicker');
	$element['#attached']['js'][] = drupal_get_path('module', 'addons') . '/js_behaviors/date_time_pickers.js';

	foreach (array('#default_value', '#value',) as $key) {
		if (!empty($element[$key])) {
			if (is_numeric($element[$key])) {
				$element[$key] = format_date($element[$key], 'custom', 'd.m.Y');
			}
		}
	}

	return $element;
}

function _addons_validate_datepicker(&$element, &$form_state) {
	$value = 0;

	if (!empty($element['#value'])) {
		$value = strtotime($element['#value']);
		if (!$value) {
			form_error($element, t('The specified date is invalid.'));
		}
	}
	form_set_value($element, $value, $form_state);
}
