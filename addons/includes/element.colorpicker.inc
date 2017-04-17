<?php

/**
* Colorpicker element with Spectrum jQuery plugin.
* Provides textfield with color selector popup window and additional validation.
* Options:
	#allow_alpha - boolean. Display opacity slider and returns value in rgba() format where opacity is < 1
	#palette_group - custom string identifier. This enable diplay palette area and group items in palette by this identifier.
	#use_hash - boolean. If true, element returns color value with '#' symbol in beginning escept alpha color formst (rgb())
*/
function _addons_process_colorpicker($element, &$form_state) {
	$element += array(
		'#allow_alpha' => NULL, // Use rgba. 
		'#palette_group' => NULL, // Identifier which used for local storage in client browser. Needed for palette.
		'#maxlength' => 30,
		
		'#use_hash' => NULL, // Bool. Using '#' symbol in hex color submission. Optional. When property not specified, and #default_value(#value) of element started with "#", this will set automaticaly to TRUE.
	);
	$element['#size'] = 10;
	if (!isset($element['#use_hash']) && $element['#value'] && substr($element['#value'], 0, 1) === '#') {
		$element['#use_hash'] = TRUE;
	}
	// Settings for colorpicker spectrum linbrary
	$element['#attributes']['data-palette-group'] = $element['#palette_group'] ? 'colorpicker-' . $element['#palette_group'] . '-' . $form_state['complete form']['#form_id'] : NULL;
	$element['#attributes']['data-alpha'] = !empty($element['#allow_alpha']) ? 'true' : 'false';
	
	$element['#maxlength'] = max(30, $element['#maxlength']); // Increase maxlength to 30, needed for rgba format
	
	$element['#attributes']['class'][] = 'colorpicker';

	// Static copy of js-template for better performance. We initialize colorpicker only by clicking on this button:
	$element['#attributes']['class'][] = 'js-hide';
	
	$element += array(
		'#field_suffix' => '',
	);
	$bg = $element['#value'];
	if (!preg_match('/(^\s*#?[0-9a-z][0-9a-z][0-9a-z])|(^\s*rgba?\(\s*\d+\s*,\s*\d+\s*,\s*\d+\s*(,\s*(([01]?\.\d+)|[01]))?\s*)?\)/ii', $bg)) {
		$bg = '';
	}
	else {
		$bg = preg_replace('/^\s*([0-9a-f])/i', '#$1', $bg);
	}
	$element['#field_suffix'] = 
        '<div class="sp-replacer form-submit">' . 
            '<div class="sp-preview"><div class="sp-preview-inner' . ($element['#value'] ? '' : ' sp-clear-display') . '" style="background-color:' . $bg . ';"></div></div>' .
            '<div class="sp-dd">&#9660;</div>' .
        '</div>' . 
				$element['#field_suffix'];



	$element['#attached']['js'][] = drupal_get_path('module', 'addons') . '/lib/spectrum_colorpicker_1.5.1/spectrum.js';
	$element['#attached']['css'][] = drupal_get_path('module', 'addons') . '/lib/spectrum_colorpicker_1.5.1/spectrum.css';
	
	
	$element['#attached']['js'][] = drupal_get_path('module', 'addons') . '/js_behaviors/colorpicker_field.js';	
	$element['#attached']['css'][] = drupal_get_path('module', 'addons') . '/css/colorpicker_field.css';	
	
	return $element;
}


function addons_validate_colorpicker(&$element, &$form_state) {
	if ($v = trim($element['#value'])) {
		if (substr($v, 0, 1) === '#') {
			if (!($v = substr($v, 1))) { //Remove hash
				form_set_value($element, '', $form_state); // Clear value and pass empty element to standard 'required' validation
			}
		}
		if ($v) {
			// Is HEX value ?:
			if (preg_match('/^[0-9a-f]/i', $v)) {
				if (strtolower(str_pad(dechex(hexdec($v)), 6, '0', STR_PAD_LEFT)) != strtolower(str_pad($v, 6, '0', STR_PAD_LEFT))) {
					form_error($element, t('Invalid input format in !field field.', array('!field' => '<strong>"'.check_plain($element['#title']).'"</strong>')));
				}
				else {
					if ($v < 0 && $v > 0xFFFFFF) {
						form_error($element, t('Invalid color value in !field field. Use HTML specified HEX-value (e.g. "#AADD00", "#FF0000")', array('!field' => '<strong>"'.check_plain($element['#title']).'"</strong>',)));
					}
					else {
						form_set_value($element, ($element['#use_hash'] ? '#' : '') . (($v = str_pad(dechex(hexdec($v)), 6, '0', STR_PAD_LEFT)) ? $v : '000000'), $form_state);
					}
				}				
			}
			else {
				// Is rgb(a) value?:
				if (!preg_match('/^rgba?\s*\((?:\s*(?<r>\d+)\s*,\s*(?<g>\d+)\s*,\s*(?<b>\d+)\s*)(?:,\s*(?<a>(?:[01]?\.\d+)|[01])\s*)?\)$/i', $element['#value'], $m) || $m['r'] > 255 || $m['g'] > 255 || $m['b'] > 255 || (isset($m['a']) && $m['a'] > 1)) {
					form_error($element, t('Invalid input format in !field field.', array('!field' => '<strong>"' . check_plain($element['#title']) . '"</strong>')));
				}
				else {
					if (!$element['#allow_alpha'] || !isset($m['a']) || $m['a'] >= 1) {
						unset($m['a']);
						// Save as hex
						form_set_value($element, ($element['#use_hash'] ? '#' : '') . (str_pad(dechex($m['r']), 2, '0', STR_PAD_LEFT) . str_pad(dechex($m['g']), 2, '0', STR_PAD_LEFT) . str_pad(dechex($m['b']), 2, '0', STR_PAD_LEFT)), $form_state);
					}
					else {
						// Save as rgba
						form_set_value($element, 'rgba(' . $m['r'] . ', ' . $m['g'] . ', ' . $m['b'] . ', ' . $m['a'] . ')', $form_state);
					}
				}
			}
		}

	}
}



/*
Colorpicker type element.
Provides textfield with color selector popup window and additional validation.
Used properties:
	#use_hash = TRUE|FALSE. Using '#' symbol in hex color storage. Optional. When property not specified, and #default_value(#value) of element started with "#", this will set automaticaly to TRUE.
	#is_background = TRUE|FALSE. Color settings is for background or foreground? For inverse colors of backround or text in element inline css.
 **/
function DEPRECATED_____addons_process_colorpicker___($element, &$form_state) {
	$element += array(
		'#use_hash' => FALSE,
		'#is_background' => FALSE,
		'#description' => t('Color value in HTML format (e.c. "%sample").', array('%sample' => (!empty($element['#use_hash']) ? '#' : '').'33FF00')),
	);
	if (!empty($element['#ajax']) && !isset($element['#ajax']['event'])) { // Simple ajax patch (ajax.inc will ignore additional elements without 'event' flag)
		$element['#ajax']['event'] = 'blur'; //place standard 'blur' - it's hardcoded in ajax.inc for textfields
	}
	if (!$element['#use_hash'] && $element['#value'] && substr($element['#value'], 0, 1) === '#') {
		$element['#use_hash'] = TRUE;
	}
	if ($element['#use_hash']) {
		$element['#attributes']['class'][] = 'use-hash';
	}
	if ($element['#is_background']) {
		$element['#attributes']['class'][] = 'is-background';
	}

	if ($element['#value']) {
		$c1 = utils_hex_to_rgb($element['#value']);
		//$c  = round(($c[0] + $c[1] + $c[2]) / 3);
		if (array_sum($c1) < 384 /* 128 x 3 */) {
			$c1 = '#FFF';
		} else {
			$c1 = '#000';
		}
		$c2 = ($element['#use_hash'] ? '' : '#') . $element['#value'];
		$element['#attributes']['style'][] =
		'color:'.($element['#is_background'] ? $c1 : $c2).';'.
		'background-color:'.($element['#is_background'] ? $c2 : $c1).';'.
		'';
		if (!$element['#is_background']) {
			$element['#attributes']['style'][] = 'font-weight:700;';
		}
	}
	$element['#size'] = $element['#use_hash'] ? 7 : 6;
	$element['#maxlength'] = $element['#size'];



	//TODO: проблема с css. Если элемент вызывается в контексте ахах, то стили пролетают (в отличие от скриптов!!!). Хотя по идее, вроде бы, наверное, должно бы равномерно быть..
	$js = array();
	if ($element['#use_hash']) {
		$js['h'] = 1;
	}
	if ($element['#is_background']) {
		$js['b'] = 1;
	}
	drupal_add_js(array('colorpickers' => array($element['#id'] => $js)), array('type' => 'setting',));
	drupal_add_js(drupal_get_path('module', 'addons').'/lib/colorpicker/js/colorpicker.js');
	drupal_add_js(drupal_get_path('module', 'addons').'/js_behaviors/colorpicker_field.js');

	return $element;
}

/*
Implementation of colorpicker element validation.
 **/
function DEPRECATED___addons_validate_colorpicker(&$element, &$form_state) {
	if ($v = trim($element['#value'])) {
		if (substr($v, 0, 1) === '#') {
			if (!($v = substr($v, 1))) { //Remove hash
				form_set_value($element, '', $form_state); // Clear value and pass empty element to standard 'required' validation
			}
		}
		if ($v) {
			if (strtolower(str_pad(dechex(hexdec($v)), 6, '0', STR_PAD_LEFT)) != strtolower($v)) {
				form_error($element, t('Invalid input format in !field field.', array('!field' => '<strong>"'.check_plain($element['#title']).'"</strong>')));
			} else {
				if ($v < 0 && $v > PHP_INT_MAX) {
					form_error($element, t('Invalid color value in !field field. Use HTML specified HEX-value (e.g. "#AADD00", "#FF0000")', array('!field' => '<strong>"'.check_plain($element['#title']).'"</strong>',)));
				} else {
					form_set_value($element, ($element['#use_hash'] ? '#' : '').(($v = str_pad(dechex(hexdec($v)), 6, '0', STR_PAD_LEFT))? $v : '000000'), $form_state);
				}
			}
		}

	}
	///form_error($element, 'under construction');
}
