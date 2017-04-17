<?php


/**
* 
*/
function _addons_process_internal_uri($element, &$form_state) {
	$element += array(
		'#description' => t('Internal URL on this site. Type relative system path or alias.'),
		'#allow_fragments' => TRUE, // Allow '#' in url
		'#allow_query' => TRUE, // Allow '?params=' in url
		);
	return $element;
}


/**
* 
*/
function addons_validate_internal_uri(&$element, &$form_state) {
	if ($element['#allow_fragments']) {
		$element['#value'] = preg_replace('/(\/#|#).*/i', '', $element['#value']);
	}
	if ($element['#allow_query']) {
		$element['#value'] = preg_replace('/(\/\?|\?).*/i', '', $element['#value']);
	}
	$element['#value'] = preg_replace('/[a-z]+\:\/\/[^\/]+\//i', '', $element['#value']);

	form_set_value($element, $element['#value'], $form_state);
	if ($element['#value'] !== '') {
		$src = FALSE;
		if (!menu_get_item($element['#value']) && (!($src = drupal_lookup_path('source', $element['#value'])) || !menu_get_item($src))) {
			form_error($element, t('URL "%url" not found on this site', array('%url' => $element['#value'],)));
		} else {
			if ($src) {
				form_set_value($element, $src, $form_state);
			}
		}

	}
}
