<?php
/**
	The test area
	works only when ADV_ELEMENTS_TEST_MODE is set to TRUE.
**/
function adv_elements_menu() {
	return array(
		'adv_elements_test_form' => array(
			'title' => 'adv_elements_test_form',
			'page callback' => 'drupal_get_form',
			'page arguments' => array('adv_elements_test_form'),
			'access callback' => TRUE,			
		),
	);
}

function adv_elements_test_form($form, &$form_state) {
	$form['selectbox1'] = array(
		'#type' => 'select',
		'#inline' => TRUE,
		'#title' => 'dddd',
		'#multiple' => TRUE,
		'#options' => array(1,2,3,4,5,6),
		'#default_value' => array(3),
	);
	$form['selectbox2'] = array(
		'#type' => 'select',
		'#title' => 'dddd',
		'#multiple' => TRUE,
		'#options' => array(1,2,3,4,5,6, 'zzz' => array(1,2,3,4,)),
	);
	$form['tp'] = array(
		'#type' => 'timepicker',
		'#title' => 'tttt',
		'#default_value' => variable_get('adv_elements_test_form_time'),
	);
	$form['file1'] = array(
		'#title' => 'managed_file',
		'#type' => 'managed_file',
	);
	$form['file2'] = array(
		'#title' => 'file',
		'#type' => 'file',
	);
	$form['actions'] = array(
		'#type' => 'actions',
		'submit' => array(
			'#type' => 'submit',
			'#value' => 'OK',
		),
	);
	return $form;
}

function adv_elements_test_form_submit($form, &$form_state) {
	variable_set('adv_elements_test_form_time', $form_state['values']['tp']);
}