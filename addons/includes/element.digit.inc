<?php


function _addons_process_digit($element, &$form_state) {
  if (!isset($element['#max'])) {
    $element['#max'] = PHP_INT_MAX;
  }
	$element += array(
		'#float' => NULL,
		'#min' => 0,
		'#step' => NULL, // Increment step
		'#validate_step' => FALSE, // Filled value must be a multiple of #step property. E.g is step = 0.5, then value can be = 1|1.5|2... but value = 2.3 will denied.
		'#cycle' => FALSE, //When value > max, set value to min. When value < min, set value to max.
		'#attributes' => array(),
    '#buttons' => FALSE, // use plus-minus buttons.
    '#buttons_position' => 'horizontal', // [+][-] buttons layot. 'horizontal' = [-][input][+]; 'vertical' = [input][±]. Default is horizontal.
	);
	if (!isset($element['#float'])) {
		if (strpos($element['#step'] . $element['#min'] . $element['#max'], '.') !== FALSE) {
			$element['#float'] = TRUE;
		}
	}
  $element['#buttons_position'] = in_array($element['#buttons_position'], array('horizontal', 'vertical',), TRUE) ? $element['#buttons_position'] : 'horizontal';
	$element['#min'] *= 1;
	$element['#max'] *= 1;
	if (!$element['#float']) {
		$element['#min'] = round($element['#min']);
		$element['#max'] = round($element['#max']);
	}
	if (!empty($element['#ajax']) && !isset($element['#ajax']['event'])) { // Simple ajax patch (ajax.inc will ignore additional elements without 'event' flag)
		$element['#ajax']['event'] = 'blur'; //place standard 'blur' - it's hardcoded in ajax.inc for textfields
	}
	if (!isset($element['#size']) || $element['#size'] > 20) {
		$element['#size'] = $element['#float'] ? 8 : max(strlen('' . $element['#min']) + ($element['#min'] < 0 ? 1 : 0), strlen(''.$element['#max']) + ($element['#max'] < 0 ? 1 : 0));
		$element['#size'] += round($element['#size'] / 3);
	}

	if (!$element['#float']) {
		//$element['#maxlength'] = max(strlen(''.$element['#min']), strlen(''.$element['#max']));
	}
	if ($element['#required'] && !$element['#value']) {
		//	$element['#value'] = '0';
	}
	if (!isset($element['#step'])) {
		$element['#step'] = abs(round(($element['#max'] - $element['#min']) / 20));
	}
	elseif ($element['#step'] && !$element['#float']) {
    $element['#step'] = round($element['#step']);
	}
  $element['#step'] = $element['#step'] ? $element['#step'] : 1;
  if ($element['#value'] == $element['#min'] && $element['#value'] == $element['#max']) {
		$element['#attributes']['readonly'] = 'readonly';
	}
	if (!array_key_exists('#description', $element)) {
		t('Specify value from @min to @max.', array('@min' => $element['#min'], '@max' => $element['#max']))
		//.($element['#float'] ? ' '.t('Float values are allowed.') : '')
		;
	}
  
  if ($element['#buttons']) {
		$element['#attributes']['class'][] = 'use-buttons';
		$options = array(
			'min' => $element['#min'],
			'max' => $element['#max'],
			'step' => '' . $element['#step'],
			'cycle' => $element['#cycle'],
			'float' => !empty($element['#float']),
		);
		$element['#post_render'][] = '_addons_digit_post_render';
	}
  
  
  $element['#attached']['js'][] = drupal_get_path('module', 'addons').'/js_behaviors/addons.digit.js';
  $element['#attributes']['data-min'] = $element['#min'];
  $element['#attributes']['data-max'] = $element['#max'];
  $element['#attributes']['data-step'] = $element['#step'];
  $element['#attributes']['data-validate-step'] = $element['#validate_step'];
  $element['#attributes']['data-float'] = $element['#float'];
  
	$element['#attributes']['autocomplete'] = 'off';
	$element['#attributes']['class'][] = 'form-digit';
  if ($element['#required']) {
    $element['#attributes']['required'] = 'required';
  }
	return $element;
}


function _addons_digit_post_render($html, $element) {
	return preg_replace('/(<input[^>]+>)/u', 
  '<span class="digit-element-buttons-wrapper' . ($element['#buttons_position'] === 'vertical' ? ' vertical-view' : '') . '">' . 
    '<input type="text" value="-" class="form-text digit-button digit-minus" title="' . check_plain(t('Reduce')) . '" readonly="readonly">' . 
      '$1' . 
    '<input type="text" value="+" class="form-text digit-button digit-plus" title="' . check_plain(t('Increase')) . '" readonly="readonly">' . 
  '</span>', 
  $html);
}


function addons_validate_digit(&$element, &$form_state) {
	if ($element['#value'] === '') {
		$element['#value'] = NULL;
		form_set_value($element, NULL, $form_state);
	}
	elseif (!isset($element['#access']) || $element['#access']) {
			// Replace comma to dots; remove non valid chars.
			$v = preg_replace('/[^0-9\.-]/u', '', str_replace(',', '.', $element['#value']));
			if ($v === '') {
				form_error($element, t('Invalid input format in !field field.', array('!field' => isset($element['#title']) ? '<strong>"' . check_plain($element['#title']) . '"</strong>' : '', )));
			}
			// remove left zeros
			$v = preg_replace('/^0+/u', '', $v);
			$v = preg_replace('/\.0+$/u', '', $v);
			$v = $v ? $v : 0;
			if ($v && !filter_var($v, $element['#float'] ? FILTER_VALIDATE_FLOAT : FILTER_VALIDATE_INT)) {
				form_error($element, t('Invalid input format in !field field.', array('!field' => isset($element['#title']) ? '<strong>"' . check_plain($element['#title']) . '"</strong>' : '', )));
			}
			else {
				$has_errors = FALSE;
				if ($v < $element['#min']) {
					$has_errors = TRUE;
					form_error($element, t('Value of !field field must be no less than %min.', array('!field' => '<strong>"' . check_plain($element['#title']) . '"</strong>', '%min' => $element['#min'])));
				}
				if ($v > $element['#max']) {
					$has_errors = TRUE;
					form_error($element, t('Value of !field field must be no greater than %max.', array('!field' => '<strong>"' . check_plain($element['#title']) . '"</strong>', '%max' => $element['#max'])));
				}
				if (!empty($element['#validate_step']) && !empty($element['#step']) && strpos(($v / $element['#step']) * 1, '.') !== FALSE) {
					$has_errors = TRUE;
					form_error($element, t('Value of !field field must be be a multiple of %m.', array('!field' => '<strong>"' . check_plain($element['#title']) . '"</strong>', '%m' => $element['#step'])));
				}
				/*if ($element['#required'] && !$v && $element['#min']) {
					$has_errors = TRUE;
					form_error($element, t('!name field is required.', array('!name' => $element['#title'])));
				}*/
				if (!$has_errors) {
					$v = preg_replace('/^\./u', '0.', $v);
					$v = trim($v, '.');
          $element['#value'] = $v;
					form_set_value($element, $v, $form_state);
				}
			}
	}

}