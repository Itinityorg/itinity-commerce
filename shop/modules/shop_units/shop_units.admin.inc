<?php

 /**
 * Admin pages
 */

/**
* Callback for /admin/config/regional/units
*/
function shop_units_settings_form($form, &$form_state) {
	foreach (shop_units_categories() as $category_id => $category) {
		$units = shop_units_load(NULL, array('category_id' => $category_id,));
		$default_values = array();
		foreach ($units as $unit_id => $unit) {
			$default_values[] = array(
				'unit_id'   => $unit->unit_id,
				'enabled'   => $unit->enabled,
				'title'     => t($unit->title),
				'symbol'    => $unit->symbol,
			);
		}

		$form[$category_id] = array(
			'#type'          => 'multifield_wrapper',
			'#draggable'     => FALSE,
			'#default_value' => $default_values,
			'#title'         => $category->title,
			//structure
			'enabled'        => array(
				'#type'  => 'checkbox',
				'#title' => t('Enabled'),
			),
			'unit_id'        => array(
				'#type'      => 'textfield',
				'#title'     => t('Identifier'),
				'#maxlength' => 10,
				'#size'      => 10,
				'#required'  => TRUE,
			),
			'title'          => array(
				'#type'     => 'textfield',
				'#title'    => t('Title'),
				'#size'     => 25,
				'#required' => TRUE,
			),
			'symbol'         => array(
				'#type'      => 'textfield',
				'#title'     => t('Symbol') . ' (' . drupal_strtolower(t('English')) . ')',
				'#maxlength' => 10,
				'#size'      => 10,
				'#required' => TRUE,
				'#after_build' => array('_shop_units_settings_symbol_after_build'),
			),
		);
	}
	$form['submit'] = array(
		'#type'  => 'submit',
		'#value' => t('Save'),
	);
	$form['#validate'][] = 'shop_units_settings_form_validate';
	return $form;
}


function _shop_units_settings_symbol_after_build($element) {
	global $language;
	if ($element['#value'] && $language->language !== 'en') {
		$element += array('#field_suffix' => '');
		$element['#field_suffix'] .= ' <em>(' . $language->language . ': ' . t($element['#value']) . ')</em>';
	}
	return $element;
}


function shop_units_settings_form_validate($form, &$form_state) {
	$found_units = array();
	foreach ($form_state['values'] as $category_id => $units) {
		if ($units && is_array($units)) {
			foreach ($units as $idx => $unit) {
				if (isset($found_units[$unit['unit_id']])) {
					form_error($form[$category_id][$idx], t('Duplicate error. Unit %id is already defined in category %category.', array('%id' => $unit['unit_id'], '%category' => $form[$found_units[$unit['unit_id']]]['#title'])));
				}
				else {
					$found_units[$unit['unit_id']] = $category_id;
				}
			}
		}
	}
}


function shop_units_settings_form_submit($form, &$form_state) {
	form_state_values_clean($form_state);
	
	$category_ids = array();
	$db_insert = db_insert('shop_units')->fields(array('unit_id', 'category_id', 'title', 'symbol', 'enabled', 'weight',));
	$insert = FALSE;
	foreach ($form_state['values'] as $category_id => $units) {
		$category_ids[$category_id] = $category_id;
		$weight = 0;
		if ($units && is_array($units)) {
			foreach ($units as $unit) {
				$insert = TRUE;
				$db_insert->values(array(
					'unit_id' => $unit['unit_id'],
					'category_id' => $category_id,
					'title' => $unit['title'],
					'symbol' => $unit['symbol'],
					'enabled' => $unit['enabled'],
					'weight' => $weight++,
				));
			}
		}
	}

	db_delete('shop_units')->execute();
	if ($insert) {
		$db_insert->execute();
	}
	cache_clear_all('shop_units', 'cache');
	drupal_static_reset('shop_units_load');
	drupal_set_message(t('All changes saved'));
}










