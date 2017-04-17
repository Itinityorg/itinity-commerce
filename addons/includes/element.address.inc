<?php


function _addons_process_address($element, &$form_state, $complete_form) {
	$element['#tree'] = TRUE;
	$element += array(
		'#geocode' => TRUE,
		'#prefix' => '',
		'#suffix' => '',
	);
	$wrapper_id = $element['#id'] . '-ajax-address-wrapper';
	$element['#prefix'] .= '<div id="' . $wrapper_id . '">';
	$element['#suffix'] = '</div>' . $element['#suffix'];

	$address_elements = array(
		'country' => t('Country'),
		'state' => t('Territory, area, township'),
		'city' => t('City (settlement)'),
		'postcode' => t('Postcode'),
		'street' => t('Street'),
		'building' => t('Building'),
		'office' => t('Office'),
		'details' => t('Details'),
	);
	foreach ($address_elements as $name => $title) {
		$element[$name] = array(
			'#type' => 'textfield',
			'#title' => $title,
			'#prefix' => '<div id="' . $element['#id'] . '-' . $name . '-wrapper" class="address-info">',
			'#suffix' => '</div>',
			'#default_value' => isset($element['#default_value'][$name]) ? $element['#default_value'][$name] : '',
			'#size' => 30,
			'#ajax' => isset($element['#ajax']) ? $element['#ajax'] : NULL,
		);
	}

	$element['#attached']['js'][] = array(
		'type' => 'external',
//		'data' => '//maps.google.com/maps/api/js?sensor=false',
		'data' => '//api-maps.yandex.ru/2.1/?lang=ru-RU',
	);

	$element['#attached']['css'][] = drupal_get_path('module', 'addons') . '/css/address.css';

	$element['#attached']['js'][] = drupal_get_path('module', 'addons') . '/js_behaviors/address_field.js';
	$element['#attached']['js'][] = array(
		'type' => 'setting',
		'data' => array(
			'addressElements' => array(
				$element['#id'] => array(
					'map' => !empty($element['#map']),
					'geocode'	=> !empty($element['#geocode']),
				),
			),
		),
	);

	$element['gmap'] = array(
		'#tree' => TRUE,
		'#weight'	=> 1,
		'map'	=> array(
			'#type' => 'item',
			'#title' => t('Location on the map'),
			'#markup' => '<div class="gmap-element-container-map' . (!empty($element['#map']) ? '' : ' element-hidden') . '" style="height:500px;"></div>',
		),
		'lat'	=> array(
			'#type' => 'textfield',
			'#default_value'	=> isset($element['#default_value']['gmap']['lat']) ? $element['#default_value']['gmap']['lat'] : '',
			'#attributes'	=> array(
				'class'	=> array('js-hide','latitude',)
			),
		),
		'lng' => array(
			'#type' => 'textfield',
			'#default_value' => isset($element['#default_value']['gmap']['lng']) ? $element['#default_value']['gmap']['lng'] : '',
			'#attributes'	=> array(
				'class'	=> array('js-hide','longitude',)
			),
		),
	);
	$element['#title_display'] = 'invisible';
	$element['#value'] = '';  // Remove 'Array' string from fieldset
	$element['#theme_wrappers'][] = 'fieldset';
	return $element;
}

/*function _addons_address_element_js_callback($form, $form_state) {
	//return drupal_array_get_nested_value($form, $form_state['triggering_element']['#ajax']['wrapped_on_to']);
}*/



function addons_validate_address(&$element, &$form_state) {
	/*if ($longitude > 180 || $longitude < -180) {
		form_set_error('longitude', t("Longitude error value."));
	}
	if ($latitude > 90 || $latitude < -90) {
		form_set_error('latitude', t('Latitude error value.'));
	}*/
}