<?php

function _addons_theme_skype($variables) {
	$variables += array(
		'attributes' => array(),
	);
	$variables['attributes'] += array(
		'title' => t('Skype'),
		'class' => array(),
	);
	$variables['attributes']['class'][] = 'skype';
	$variables['attributes']['href'] = 'skype:' . $variables['skype'] . '?chat';
	return '<a' . drupal_attributes($variables['attributes']) . '>' . check_plain($variables['skype']) . '</a>';
}

function addons_validate_skype(&$element, &$form_state) {
	if($element['#value']) {
		//do nothing
	}
}