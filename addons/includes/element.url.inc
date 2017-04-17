<?php



/// NOT USED FUNCTION?
function _addons_theme_url($variables) {
	return $variables['url'] ? l($variables['url'], $variables['url']) : '';
	/*
	$url = preg_match('/^(?:ftp|https?):\/\/.*$/', $variables['url']) ? $variables['url'] : 'http://' . $variables['url'];
	return $variables['url'] ? '<a class="ext" href="' . $url . '">' . check_plain($variables['url']) . '</a><span class="ext"></span>' : '';
	*/
}



function addons_validate_url(&$element, &$form_state) {
	if ($element['#value']) {
		if (!utils_valid_url($element['#value'])) {
			form_error($element, t('"%url" is not a valid url', array('%url' => $element['#value'])));
		}
	}
}
