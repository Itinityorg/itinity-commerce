<?php

function multifield_my_form($form, &$form_state, $test_num = 1) {
	form_load_include($form_state, 'inc', 'multifield' , 'multifield.test_form');
	$test1 = array(
	'#prefix' => '<div id="form-ajax-wrapper">',
	'#suffix' => '</div>',
		'test' => array(
			'#title' => 'colors',
			'#type' => 'multifield_wrapper',
			'#elements_to_cells' => TRUE,
			//'#use_ajax' => FALSE,
			'#default_value' => array(
				0 => array(
						'a_active' => '#123456',
					),
			),
			'all' => array(
				'#title' => 'Text',
				'#type' => 'digit',
				//'#description' => '',
			),
			'a' => array(
				'#title' => 'Link',
				'#type' => 'digit',
				//'#description' => '',
			),
			'a_hover' => array(
				'#title' => 'Hovered link',
				'#type' => 'digit',
				//'#description' => '',
			),
			'a_active' => array(
				'#title' => 'Active link',
				'#type' => 'colorpicker',
				//'#description' => '',
				'#ajax' => array(
					'callback' => 'temizator_main_form_ajax_callback',
					'wrapper' => 'form-ajax-wrapper',
					'progress' => array(
						'type' => 'throbber',
						'message' => t('Please wait'),
					),
					'event' => 'change',
					),
			),
			
		),
	);
	$test2['test'] = array (
  '#type' => 'multifield_wrapper',
	'#title' => 'aaaaaaaa',
  '#default_value' => 
  array (
	0 => array(
		's' => '#fff',
	),
	1 => array(
		's' => '#fff',
	),
  ),
  '#titles' => array(
	0 => array(
		's' => 'ssss',
	),
  ),
  's' => 
  array (
	'#type' => 'colorpicker',
	'#is_background' => true,
	'#use_hash' => true,
	'#title' => 'bg1',
  ),
);
	$test3['test'] = array(
		'#type' => 'multifield_wrapper',
		'#collapsible' => TRUE,
		'#description' => 'multifield description',
		'#title' => 'test',
		'#default_value' => array(
			30 => array(
				'a' => 30,
				'b' => array(
					2, 4, 5
				),
			),
			1 => array(
				'a' => 'abc',
				'b' => array(
					1,
				),
			),
			12 => array(
				'b' => '12!',
				'c' => array(
					2 => array(
						'x' => 'ddd',
						'z' => array(
							1 => array(
								'd' => array(
									'day' => 6
								),
							),
							2 => array(
								'd' => array(
									'month' => 6
								),
							),
							3 => array(
								'd' => array(
									'year' => 1986,
									'day' => 4,
									'month' => 3,
								),
							),
						),
					),
				),
			),
			'skjdksjdksa' => array(
			
			),
		),
		'a' => array(
			'#type' => 'textfield',
			'#size' => 12,
			'#required' => TRUE,
			'#default_value' => '',
			'#title' => 'field a',
		),
		'b' => array(
			'#type' => 'checkboxes',
			'#title' => 'field b',
			'#options' => array(1, 2,),
		),
		'c' => array(
			'#type' => 'multifield_wrapper',
			'#title' => 'sub multifield',
			'x' => array(
				'#type' => 'textfield',
				'#size' => 12,
			),
			'z' => array(
				'#type' => 'multifield_wrapper',
				'#collapsible' => TRUE,
				'#title' => 'dates',
				'#min' => 2,
				'#max' => 4,
				'd' => array(
					'#type' => 'date',
				),
			),
		),
	);
	if (isset(${'test' . $test_num})) {
		$form += ${'test' . $test_num};
	}
	$form['asdf'] = array(
		'#type' => 'textfield',
		'#required' => TRUE,
		'#title' => 'sdsdsds',
	);
	$form['submit'] = array(
		'#type'		=> 'submit',
		'#value'	=> t('Save'),
	);
	return $form;
}

function multifield_my_form_validate($form, &$form_state) {
	
}

function multifield_my_form_submit($form, &$form_state) {
	m($form_state['values']);
}



function multifield_form_alter_test(&$form, &$form_state, $form_id) {
	if ($form_id != 'multifield_my_form' && $form_id != 'search_block_form' && $form_id != 'devel_switch_user_form') {
		$subform = multifield_my_form($form, $form_state);
		$form['mmm'] = $subform['test'];
		$form['mmm']['#weight'] = -PHP_INT_MAX;
		return;
		$form['mmmm'] = array(
			'#type' => 'multifield_wrapper',
			'#default_value' => array(
				0 => array(
					'a' => 'aaa',
				),
			),
			//'#title' => 'mmmm',
			'a' => array(
				'#title' => 'aaaa',
				'#type' => 'textfield',
			),
			'zzz' => array(
				'#type' => 'multifield_wrapper',
				'#default_value' => array(
					0 => array(
						'sss' => 'ssssssssss',
					),
				),
				'sss' => array(
					'#title' => 'dd',
					'#type' => 'textfield',
				),
			),
		);
	}
}