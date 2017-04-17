<?php

function addons_test_form($form, &$form_state) {
	
	$form['tabs'] = array(
		'#tree' => FALSE,
		'tab1' => array(
			'#title' => 'tab #1',
			'#type' => 'fieldset',
			'#use_tabs' => TRUE,
			'checkbox' => array(
				'#type' => 'checkbox',
				'#title' => 'checkbox in beginning of tab #1',
			),
			'select' => array(
				'#type' => 'select',
				'#title' => 'hierarhical select box',
				'#hierarchy_prefix' => '--',
				'#options' => array(
					' - None -',
					'1', 
					'2', 
					'--2.1',
					'--2.2',
					'----2.2.1',
					'----2.2.2',
					'--2.3',
					'3',
					'--3.1',
					'----3.1.1',
					'------3.1.1.1',
					'------3.1.1.2',
					'----3.1.2',
					'--3.2',
					'4',
					'--4.1',
					'--4.2',
					'----4.2.1',
					'----4.2.2',
					'------4.2.2.1',
					'5',
					'6',
					'7',
					'8',
					'9',
					'10',
				),
			),
			'tab1_1' => array(
				'#title' => 'tab #1_1',
				'#type' => 'fieldset',
				'#use_tabs' => TRUE,
				'#value' => 'markup test: lorem ipsum',
			),
			'tab1_2' => array(
				'#description' => 'tab #1_2 description',
				'#title' => 'tab #1_2',
				'#type' => 'fieldset',
				'#use_tabs' => TRUE,
				'textarea' => array(
					'#type' => 'textarea',
					'#title' => 'textarea',
				),
			),
			'checkbox2' => array(
				'#type' => 'checkbox',
				'#title' => 'checkbox in footer of tab #1',
			),
		),
		'tab2' => array(
			'#title' => 'tab #2',
			'#type' => 'fieldset',
			'#use_tabs' => TRUE,
			'radios' => array(
				'#title' => 'radios',
				'#type' => 'radios',
				'#options' => range(0,5),
			),
		),
		'tab3' => array(
			'#title' => 'tab #3',
			'#type' => 'fieldset',
			'#use_tabs' => TRUE,
			//'#active' => TRUE,
			'tab_3_1' => array(
				'#title' => 'empty tab #3_1',
				'#type' => 'fieldset',
				'#use_tabs' => TRUE,
			),
			'tab_3_2' => array(
				'#title' => 'tab #3_2',
				//'#active' => TRUE,
				'#type' => 'fieldset',
				'#use_tabs' => TRUE,
				'textfield' => array(
					'#type' => 'textfield',
					'#title' => 'textfield',
				),
			),
		),
		'tab4' => array(
			'#title' => 'tab #4',
			'#type' => 'fieldset',
			'#use_tabs' => TRUE,
			'date1' => array(
				'#inline' => TRUE,
				'#type' => 'date',
				'#title' => 'date 1',
				'#attributes' => array('class' => array('datepicker'),),
			),
			'date2' => array(
				'#type' => 'date',
				'#title' => 'date 2',
			),
			'date3' => array(
				'#type' => 'date',
				'#title' => 'date 3',
			),
			'tab_4_1' => array(
				'#title' => 'tab #4_1 nested',
				'#type' => 'fieldset',
				'#use_tabs' => TRUE,

				'tab_4_1_1' => array(
					'#title' => 'tab #4_1_1 nested',
					'#type' => 'fieldset',
					'#use_tabs' => TRUE,
					'textfield' => array(
						'#type' => 'textfield',
						'#required' => TRUE,
						'#title' => 'textfield',
					),
				),
				'tab_4_1_2' => array(
					'#title' => 'tab #4_1_2 nested',
					'#type' => 'fieldset',
					'#use_tabs' => TRUE,
				),
			),
			'tab_4_2' => array(
				'#title' => 'tab #4_2',
				'#type' => 'fieldset',
				'#use_tabs' => TRUE,
			),
		),
	);
	$form['submit'] = array(
		'#type' => 'submit',
		'#value' => 'Submit',
	);
	return $form;
}

function addons_test_form_submit($form, &$form_state) {
	m($form_state['values']);
}









