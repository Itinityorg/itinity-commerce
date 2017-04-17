<?php



/**
* Menu callback for /admin/content/contact/agreement
*/
function social_profile_admin_terms_of_service_form($form, &$form_state) {
	global $language;
  $form['preview'] = array(
    '#type' => 'container',
    '#attributes' => array(
      'style' => 'text-align: right;',
    ),
    'link' => array(
      '#type' => 'link',
      '#title' => t('User agreement'),
      '#href' => 'agreement',
      '#attributes' => array(
        'class' => array('ajax-popup',),
        'title' => t('Preview'),
        'target' => '_blank',
      ),    
    ),
  );
  
	$social_profile_terms_of_service = variable_get('social_profile_terms_of_service', array('texts' => array(), 'required' => FALSE,));
	$form['social_profile_terms_of_service'] = array(
		'#tree' => TRUE,
		'required' => array(
			'#type' => 'checkbox',
			'#title' => t('Require confirmation from the user'),
			'#default_value' => $social_profile_terms_of_service['required'],
		),
	);
	foreach (language_list('language') as $langcode => $lang) {
		$form['social_profile_terms_of_service']['texts'][$langcode] = array(
			'#type' => 'text_format',
			'#title' => t('Text of agreement'),
			'#access' => $language->language === $langcode,
			'#default_value' => isset($social_profile_terms_of_service['texts'][$langcode]['value']) ? $social_profile_terms_of_service['texts'][$langcode]['value'] : NULL,
			'#format' => isset($social_profile_terms_of_service['texts'][$langcode]['format']) ? $social_profile_terms_of_service['texts'][$langcode]['format'] : NULL,
			'#description' => drupal_multilingual() ? t('This text is for @lang (%native) language. If multilingual is enabled, please switch to different language for translate text to specific version.', array(
        '@lang' => $lang->name, 
        '%native' => $lang->native,
      )) : '',
		);
	}
	$form['actions'] = array(
		'#type' => 'actions',
		'submit' => array(
			'#type' => 'submit',
			'#value' => t('Save'),
		),
		'reset' => array(
			'#type' => 'submit',
			'#value' => t('Clear accounts data'),
			'#attributes' => array(
				'title' => t('Delete confirmed users flags.'),
			),
			'#submit' => array('social_profile_admin_terms_of_service_form_reset_submit'),
			'#states' => array(
				'visible' => array(
					'input[name="social_profile_terms_of_service[required]"]' => array('checked' => TRUE,),
				),
			),
			'#href' => module_exists('contacts') ? 'admin/content/contact/agreement/reset' : 'admin/config/people/accounts/term/reset', // Popup dialogs support, @see confirm_dialog.module
		),
	);
	return $form;
}


function social_profile_admin_terms_of_service_form_reset_submit($form, &$form_state) {
	$form_state['redirect'] = module_exists('contacts') ? 'admin/content/contact/agreement/reset' : 'admin/config/people/accounts/term/reset';
}


function social_profile_admin_terms_of_service_form_validate($form, &$form_state) {
	global $language;
	foreach ($form_state['values']['social_profile_terms_of_service']['texts'] as $langcode => $data) {
		if ($langcode == $language->language && !trim($data['value'])) {
			form_error($form['social_profile_terms_of_service']['texts'][$langcode], t('The field %field is required.', array('%field' => $form['social_profile_terms_of_service']['texts'][$langcode]['#title'])));
		}
	}
}


function social_profile_admin_terms_of_service_form_submit($form, &$form_state) {
	$social_profile_terms_of_service = variable_get('social_profile_terms_of_service', array('required' => FALSE,));
	if ($social_profile_terms_of_service['required'] != $form_state['values']['social_profile_terms_of_service']['required']) {
		variable_set('menu_rebuild_needed', TRUE);
	}
	variable_set('social_profile_terms_of_service', $form_state['values']['social_profile_terms_of_service']);

	drupal_set_message(t('The configuration options have been saved.'));
}


/**
* Menu callback for /admin/content/contact/agreement/reset or /admin/config/people/accounts/term/reset
* @see social_profile_admin_terms_of_service_form_reset_submit()
*/
function social_profile_admin_terms_of_service_reset_confirm_form($form, &$form_state) {
	return confirm_form(
		$form,
		t('Are you sure want to delete all stored confirmations in users profiles?'),
		module_exists('contacts') ? 'admin/content/contact/agreement' : 'admin/config/people/accounts/term',
		t('This action cannot be undone.') . '<br />'.
		t('This action most in demand if text of agreement has been heavily modified. Do not use this operation too often.')
		);
}


/**
* Uncheck all aggreements in user accounts
*/
function social_profile_admin_terms_of_service_reset_confirm_form_submit($form, &$form_state) {
	if ($form_state['values']['confirm']) {
		$form_state['redirect'] = $form['actions']['cancel']['#href'];
		$processed = 0;
		foreach (db_query('SELECT uid, data FROM {users} WHERE uid') as $r) {
			if (!empty($r->data) && ($r->data = @unserialize($r->data)) && isset($r->data['social_profile_terms_agreed'])) {
				unset($r->data['social_profile_terms_agreed']);
				$processed ++;
				db_query('UPDATE {users} SET data = :data WHERE uid = :uid', array(':data' => serialize($r->data), ':uid' => $r->uid));
			}
		}
		if ($processed) {
			drupal_set_message(t('Agreement flags was deleted from @count accounts.', array('@count' => $processed)));
		} else {
			drupal_set_message(t('Accounts with confirmed terms of service was not found in database.'));
		}
	}
}










