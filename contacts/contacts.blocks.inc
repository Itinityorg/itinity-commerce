<?php

/**
 * A block content function. @see contacts_block_view()
 */
function _contacts_block_contents() {
  
	$settings = variable_get('contacts_block_settings', array(
		'contact_information' => array(
			'enabled' => TRUE,
			'settings' => array(
				'trade_name' => array(
					'enabled' => TRUE,
				),
				'contacts' => array(
					'enabled' => TRUE,
					'format' => 'short',
				),
				'address' => array(
					'enabled' => TRUE,
					'format' => 'short',
				),
				/*'agreement' => array(
					'enabled' => FALSE,
				),*/
			),
		),
		'map' => array(),
		'contacts_form' => array(),
	));
	$contacts = contacts_load_settings('site_contacts');
	$content = array();
	$weights = array_flip(array_keys($settings));
  if (!empty($settings['map']['enabled'])) {
		if(
			isset($contacts['address']['gmap']['lat']) &&
			!empty($contacts['address']['gmap']['lat']) &&
			isset($contacts['address']['gmap']['lng']) &&
			!empty($contacts['address']['gmap']['lng'])
		) {
			$id = _contacts_add_map($contacts['address']['gmap']['lat'], $contacts['address']['gmap']['lng']);
			$content['wrapper']['map'] = array(
				'#weight' => $weights['map'],
				'#markup' => '<div id="' . $id . '"  style="height:300px;"></div>',
			);
		}
	}

	if (!empty($settings['contacts_form']['enabled'])) {
		module_load_include('inc', 'contacts', 'contacts.pages');
    $form = drupal_get_form('contacts_form');
		$content['wrapper']['form'] = $form;
		$content['wrapper']['form'] += array(
			'#prefix' => '',
			'#suffix' => '',
			'#weight' => $weights['contacts_form'],
		);
	}
	if (!empty($settings['contact_information']['enabled'])) {
		$info = $settings['contact_information']['settings'];
		$items = array();
		$content['wrapper']['contact_information'] = array(
			'#tree' => TRUE,
		);

		$info_weights = array_flip(array_keys($info));
		if (!empty($info['logotype']) && $info['logotype']['enabled'] && ($logo = theme_get_setting('logo'))) {
			$logo = '<img title="' . ($txt = check_plain(variable_get('site_name', t('Home')))) . '" alt="' . $txt . '" src="' . $logo . '" id="logo" />';
			$logo = drupal_is_front_page() && count($_GET) == 1 ? $logo : '<a href = "' . url('<front>', array('absolute' => TRUE,)) . '">' . $logo . '</a>';
			$content['wrapper']['contact_information']['logotype'] = array(
				'#markup' => $logo,
				'#prefix' => '<div class="contacts-logotype">',
				'#suffix' => '</div>',
				'#weight' => $info_weights['logotype'],
			);
		}

		if (!empty($info['copyright']) && $info['copyright']['enabled']) {
			$years = array(format_date(REQUEST_TIME, 'custom', 'Y'));
			if (!empty($info['copyright']['time'])) {
				$years[] = check_plain($info['copyright']['time']);
			}
			sort($years);
			$content['wrapper']['contact_information']['copyright'] = array(
				'#markup' => '&copy;&nbsp;' . implode('&nbsp;&ndash;&nbsp;', $years),
				'#prefix' => '<div class="contacts-copyright">',
				'#suffix' => '</div>',
				'#weight' => $info_weights['copyright'],
			);
		}

		if (!empty($info['trade_name']) && $info['trade_name']['enabled'] && !empty($contacts['names']['trade'])) {
			$content['wrapper']['contact_information']['trade_name'] = array(
				'#markup' => $contacts['names']['trade'],
				'#prefix' => '<div class="contacts-trade-name">',
				'#suffix' => '</div>',
				'#weight' => $info_weights['trade_name'],
			);
		}

		if (!empty($info['contacts']) && $info['contacts']['enabled'] && !empty($contacts['general'])) {
			$content['wrapper']['contact_information']['contacts'] = array(
				'#markup' => theme('contacts_general', array(
					'general' => $contacts['general'],
					'type' => empty($info['contacts']['format']) ? 'short' : 'full',
				)),
				'#prefix' => '<div class="contacts-general">',
				'#suffix' => '</div>',
				'#weight' => $info_weights['contacts'],
			);
		}

		if (!empty($info['address']) && $info['address']['enabled'] && !empty($contacts['address'])) {
			$content['wrapper']['contact_information']['address'] = array(
				'#markup' => theme('contacts_address', array(
					'address_elements' => $contacts['address'],
					'keys' => empty($info['address']['format']) ? array('country', 'city', 'street', 'building') : NULL,
				)),
				'#prefix' => '<div class="contacts-address">',
				'#suffix' => '</div>',
				'#weight' => $info_weights['address'],
			);
		}

		if (!empty($info['scheme']) && $info['scheme']['enabled'] && !empty($contacts['facade'])) {
			$file = file_load($contacts['facade']);
			if ($file && in_array(substr(strrchr($file->filename, '.'), 1), array('png', 'gif', 'jpg', 'jpeg')) ) {
				$content['wrapper']['contact_information']['facade'] = array(
					'#markup' => theme('image_style', array('style_name' => module_exists('features') /* @see features_image_default_styles() */ ? '128x96' : 'thumbnail', 'path' => $file->uri)),
					'#prefix' => '<div class="site-contacts-address-wrapper">',
					'#suffix' => '</div>',
					'#weight' => $info_weights['scheme'],
				);
			}
		}

		if (!empty($info['working_time']) && $info['working_time']['enabled']) {
			$content['wrapper']['contact_information']['working_time'] = array(
				'#markup' => theme('working_time', array(
					'working_time' => $contacts['additional']['working_time'],
					'detailed' => $contacts['additional']['detailed'],
				)),
				'#prefix' => '<div class="contacts-working-time">',
				'#suffix' => '</div>',
				'#weight' => $info_weights['working_time'],
			);
		}

		if (!empty($info['contact_persons']) && $info['contact_persons']['enabled'] && !empty($contacts['users'])) {
			$content['wrapper']['contact_information']['contact_persons'] = array(
				'#markup' => theme('contact_persons', array(
					'users' => $contacts['users'],
				)),
				'#prefix' => '<div class="contacts-contact-persons">',
				'#suffix' => '</div>',
				'#weight' => $info_weights['contact_persons'],
			);
		}
		$content['wrapper']['contact_information'] += array(
			'#prefix' => '',
			'#suffix' => '',
			'#weight' => $weights['contact_information'],
		);
    
    /*if (!empty($info['agreement']) && $info['agreement']['enabled']) {
			$content['wrapper']['contact_information']['agreement'] = array(
				'#markup' => l(t('User agreement'), 'agreement'),
				'#prefix' => '<div class="contacts-user-agreement">',
				'#suffix' => '</div>',
				'#weight' => $info_weights['agreement'],
			);
		}
		$content['wrapper']['contact_information'] += array(
			'#prefix' => '',
			'#suffix' => '',
			'#weight' => $weights['contact_information'],
		);*/
	}
  

  
  
  if (!empty($content['wrapper'])) {  
    $count = count(element_children($content['wrapper']));
    foreach (element_children($content['wrapper']) as $key) {
      $content['wrapper'][$key]['#prefix'] = '<div class="inline-block-wrapper-' . $count . '">';
      if (empty($content['wrapper'][$key]['#suffix'])) {
        $content['wrapper'][$key]['#suffix'] = '';
      }
      $content['wrapper'][$key]['#suffix'] .= '</div>';
    }
  }
  if ($content) {
    $content['#attached']['css'][] = drupal_get_path('module', 'contacts').'/css/contacts.css';
  }
  
	return $content;
}


function _contacts_block_configure($delta = '') {
	$form = array();
	if ($delta == 'contacts') {
		$form['content'] = array(
			'#type' => 'multifield_wrapper',
			'#default_value' => variable_get('contacts_block_settings', array(
				'contact_information' => array(),
				'map' => array(),
				'contacts_form' => array(),
			)),
			'#titles' => array(
				'contact_information' => array(
					'enabled' => t('Contact information'),
				),
				'map' => array(
					'enabled' => t('Map'),
				),
				'contacts_form' => array(
					'enabled' => t('Contacts form'),
				),
			),
			'#min' => 3,
			'#max' => 3,
			'enabled' => array(
				'#title' => t('Enabled'),
				'#type' => 'checkbox',
			),
			'settings' => array(
				'#type' => 'multifield_wrapper',
				'#default_value' => array(
					'logotype' => array(
						'enabled' => FALSE,
					),
					'copyright' => array(
						'enabled' => FALSE,
					),
					'trade_name' => array(
						'enabled' => FALSE,
					),
					'contacts' => array(
						'enabled' => FALSE,
					),
					'address' => array(
						'enabled' => FALSE,
					),
					'scheme' => array(
						'enabled' => FALSE,
					),
					'working_time' => array(
						'enabled' => FALSE,
					),
					'additional_info' => array(
						'enabled' => FALSE,
					),
					'contact_persons' => array(
						'enabled' => FALSE,
					),
					/*'agreement' => array(
						'enabled' => FALSE,
					),*/
				),
				'#titles' => array(
					'logotype' => array(
						'enabled' => t('Logotype'),
					),
					'copyright' => array(
						'enabled' => t('Copyright'),
					),
					'trade_name' => array(
						'enabled' => t('Trade name'),
					),
					'contacts' => array(
						'enabled' => t('Contacts'),
					),
					'address' => array(
						'enabled' => t('Address'),
					),
					'scheme' => array(
						'enabled' => t('Scheme'),
					),
					'working_time' => array(
						'enabled' => t('Working time'),
					),
					'additional_info' => array(
						'enabled' => t('Additional info'),
					),
					'contact_persons' => array(
						'enabled' => t('Contact persons'),
					),
					/*'agreement' => array(
						'enabled' => t('User agreement'),
					),*/
				),
				'enabled' => array(
					'#title' => t('Enabled'),
					'#type' => 'checkbox',
				),
				'format' => array(
					'#type' => 'radios',
					'#title' => t('Format'),
					'#options' => array(t('Short'), t('Full')),
				),
				'time' => array(
					'#type' => 'textfield',
					'#title' => t('Copyright time'),
				),
				'#min' => 10,
				'#max' => 10,
				'#post_process' => array('contacts_block_form_post_process_chilldren'),
			),
			'#post_process' => array('contacts_block_form_post_process'),
		);
	}
	return $form;
}

function contacts_block_form_post_process_chilldren(&$element, &$form_state, &$complete_form) {
  form_load_include($form_state, 'inc', 'contacts', 'contacts.blocks');
	foreach (array(
		'logotype',
		'trade_name',
		'contacts',
		'address',
		'scheme',
		'working_time',
		'additional_info',
		'contact_persons',
		/*'agreement',*/
	) as $key) {
		unset($element[$key]['time']);
	}
	foreach (array(
		'logotype',
		'trade_name',
		'copyright',
		'scheme',
		'working_time',
		'additional_info',
		'contact_persons',
    /*'agreement',*/
	) as $key) {
		unset($element[$key]['format']);
	}
}


function contacts_block_form_post_process(&$element, &$form_state, &$complete_form) {
  form_load_include($form_state, 'inc', 'contacts', 'contacts.blocks');
	foreach (array('map', 'contacts_form') as $key) {
		unset($element[$key]['settings']);
	}
}

