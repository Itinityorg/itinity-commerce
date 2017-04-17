<?php



function _addons_process_date($element, &$form_state) {
	//unset out of range years
	foreach (range(1900, 1949) as $key) {
		unset($element['year']['#options'][$key]);
	}
	foreach (range(2031, 2050) as $key) {
		unset($element['year']['#options'][$key]);
	}
	foreach (array('day', 'year', 'month') as $idx) {
		if (isset($element[$idx]) && !empty($element[$idx]['#options'])) {
			$element[$idx]['#options'] = array(''=> '') + $element[$idx]['#options'];
		}
	}

	if (isset($element['#time']) && !empty($element['#time']) && is_array($element['#time'])) {
		if (in_array('hours', $element['#time'])) {
			$element['hours'] = array(
				'#type' => 'digit',
				'#title_display' => $element['year']['#title_display'],
				'#size' => 2,
				'#max' => 23,
				'#min' => 0,
				//'#options' =>  array('' => '') + drupal_map_assoc(range(0, 23), create_function('$x', 'return str_pad($x, 2, \'0\', STR_PAD_LEFT);')),
				'#weight' => -0.002,
				'#default_value' => isset($element['#default_value']['hours']) ? $element['#default_value']['hours'] : format_date(REQUEST_TIME, 'custom', 'G') + (in_array('minutes', $element['#time']) ? 0 : 1),//And Ivan Klut say approximate hours in a big way. And hours were approximated.
				//'#field_suffix' => t('Hours'),
				'#field_suffix' => (in_array('minutes', $element['#time'])) ? ' : ' : NULL,
				'#required' => TRUE,
			);
			if ($element['hours']['#default_value'] == 24) {
				$element['hours']['#default_value'] = 0;
			}
		}
		if (in_array('minutes', $element['#time'])) {
			$element['minutes'] = array(
				'#type' => 'digit',
				'#size' => 2,
				'#max' => 59,
				'#min' => 0,
				'#title_display' => $element['year']['#title_display'],
				//'#options' => array('' => '') + drupal_map_assoc(range(0, 59), create_function('$x', 'return str_pad($x, 2, \'0\', STR_PAD_LEFT);')),
				'#weight' => -0.001,
				'#default_value' =>	isset($element['#default_value']['minutes']) ? $element['#default_value']['minutes'] : format_date(REQUEST_TIME, 'custom', 'i'),
				//'#field_suffix' => t('Minutes'),
				'#field_suffix' => ' - ',
				'#required' => TRUE,
			);
		}
	}
	if (isset($element['#attributes']) && isset($element['#attributes']['class']) && in_array('datepicker', $element['#attributes']['class'])) {
		foreach (array('day', 'year', 'month') as $idx) {
			if (isset($element[$idx]) && !empty($element[$idx]['#options'])) {
				$element[$idx]['#attributes']['class'][] = $idx . ' js-hide';
			}
		}

		$mindate = mktime(23, 59, 59, 12, 13, 1950) * 1000; //min possible timestamp for 32bit php
		$maxdate = mktime(23, 59, 59, 12, 13, 2030) * 1000; //max possible

		static $js_added = FALSE;
		if (!$js_added) {
			$js_added = TRUE;
			drupal_add_library('system','ui.datepicker');
			drupal_add_js(drupal_get_path('module', 'addons').'/js_behaviors/datepickerInit.js');
			drupal_add_js(array(
				'datePicker' => array(
					'dayNamesMin' => array_map(get_t(), array('Su', 'Mo', 'Tu', 'We', 'Th', 'Fr', 'Sa')),
					'monthNames' => array_map(get_t(), array('January', 'February', 'March', 'April', 'May', 'June', 'July', 'August', 'September', 'October', 'November', 'December',)),
					'icon' => file_create_url(drupal_get_path('module', 'addons') . '/lib/datepicker/ico_cal.png'),
					'firstDay' => variable_get('date_first_day', 1),
					'minDate' => $mindate,
					'maxDate' => $maxdate,
				),
			), 'setting');
		}

		//drupal_add_js(array('datePicker' => array('elements' => array($element['#id'] => array()))), 'setting');

	}

	$element['year']['#weight'] = 0.01;

	return $element;
}

function _addons_date_validate($element) {
	$values = array_filter($element['#value']);
	if (!empty($values)) {
		if (!empty($values['month']) && !empty($values['day']) && !empty($values['year'])) {
			if (!checkdate($values['month'], $values['day'], $values['year'])) {
				form_error($element, t('The specified date is invalid.'));
			}
		}
		else {
			form_error($element, t('Fill in all fields or leave them blank.'));
		}
	}
}











