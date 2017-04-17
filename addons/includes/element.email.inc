<?php


function _addons_theme_email($variables) {
	$variables += array(
		'attributes' => array(),
	);
	$variables['attributes'] += array(
		'title' => t('Email'),
		'class' => array(),
	);
	$variables['attributes']['class'][] = 'email';
	$variables['attributes']['href'] = 'mailto:' . $variables['email'];
	return $variables['email'] ? '<a ' . drupal_attributes($variables['attributes']) . '>' . check_plain($variables['email']) . '</a>' : '';
}


function addons_validate_email(&$element, &$form_state) {
	if ($element['#value']) {
		if (!utils_valid_email($element['#value'])){
			form_error($element, t('"%mail" is not a valid email address', array('%mail' => $element['#value'])));
		}
	}
}
