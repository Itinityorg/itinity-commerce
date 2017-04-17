<?php



function _addons_process_contacts($element, &$form_state, $complete_form) {

	$options = _addons_contacts_get_defined_options();

	if(isset($element['#value']['type'])) {
		if(!in_array($element['#value']['type'], array_keys($options))) {
			unset($element['#value']['type']);
		}
	}

	$element += array(
		'#prefix' => '',
		'#suffix' => '',
	);
	$wrapper_id = $element['#id'].'-ajax-contacts-wrapper';

	$element['#prefix'] .= '<div id="' . $wrapper_id . '" class="container-inline">';
	$element['#suffix'] = '</div>' . $element['#suffix'];
	$element['#tree'] = TRUE;
	$element['#type'] = 'item';

	drupal_get_messages('error', TRUE); // wtf??

	$element['#value']['type'] =
		!empty($form_state['triggering_element']['#value'])
		&&
		!empty($form_state['triggering_element']['#_contacts_selector'])
		&&
		$form_state['triggering_element']['#_contacts_selector'] == $element['#name']
		?
			$form_state['triggering_element']['#value']
		:
		(
			isset($element['#value']['type'])
			?
			$element['#value']['type']
			:
			'textfield'
		);
	$element['#value'] += array(
		'type' => '',
		'data' => '',
	);
	$ajax = array(
		'event' => 'change',
		'callback' => '_addons_contacts_js_callback',
		'wrapper' => $wrapper_id,
		'wrapped_on_to'	=> $element['#array_parents'],
	);
	$element += array(
		'type' => array(
			'#type' => 'select',
			'#options' => _addons_contacts_get_defined_options(),
			'#value' => $element['#value']['type'],
			'#_contacts_selector'	=> $element['#name'],
			'#ajax' => $ajax,
		),
		'data' => array(
			'#value' => $element['#value']['data'],
			'#type' => $element['#value']['type'],
			'#size' => 30,
			//only for infoarena
			/*'#suffix'	=> $element['#value']['type'] != 'textfield' ? theme($element['#value']['type'], array($element['#value']['type'] => $element['#value']['data'])) : check_plain($element['#value']['data']),*/
			///'#ajax' => $ajax,
		),
	);
	if ($element['#preview']) {
		$element['data']['#suffix'] = $element['#value']['type'] != 'textfield' ? theme($element['#value']['type'], array($element['#value']['type'] => $element['#value']['data'])) : check_plain($element['#value']['data']);
	}
	$element['#value']['type'] = $element['data']['#type'];
	return $element;
}


//Define elements included in contacts
function _addons_contacts_get_defined_options() {

	$options = array(
		'phone' => t('Phone'),
		'url' => t('URL'),
		'skype' => t('Skype'),
		'icq' => t('ICQ'),
		'email' => t('Email'),
		'textfield' => t('Other'),
	);

	return $options;
}

function _addons_contacts_js_callback($form, $form_state) {

	return drupal_array_get_nested_value($form, $form_state['triggering_element']['#ajax']['wrapped_on_to']);
}

function addons_validate_contacts(&$element, &$form_state) {
}
