<?php
/**
 * @file
 * Shop settings form.
 */

 /**
 * Implement hook_form().
 */
function shop_settings_form($form, &$form_state) {

	drupal_add_css(drupal_get_path('module', 'shop') .'/css/shop_settings.css');

	$tabs = array_keys(shop_get_info());
	rsort($tabs); // IKW: сначала типы товаров, потом учет остатков

	foreach ($tabs as $tab) {
		$subform = _shop_settings_subform($tab);

		if (empty($subform)) {
			continue;
		}

		$form[$tab] = array(
				'#type' => 'fieldset',
				'#title' => t(drupal_ucfirst(str_replace('_', ' ', $tab))), // 'entity_types' => t('Entity types')
				'#tree' => TRUE,
			)
			+ $subform;
		utils_fill_form_defaults($form[$tab], shop_get_info($tab));
	}
	$form['actions'] = array(
		'#type' => 'actions',
		'submit' => array(
			'#type'		=> 'submit',
			'#value'	=> t('Save'),
		),
	);
	return $form;
}


/**
 * Getting the form structure of settings for types.
 */
function _shop_settings_subform($tab_name) {

	$types = array();
	switch ($tab_name) {
		case 'entity_types':
			$form['description'] = array(
				'#type'				=> 'item',
				'#title'		=> t('Select entity types that be selling on this site.'),
			);
			foreach (shop_get_supported_entities() as $entity_type => $info) {
				foreach ($info['bundles'] as $bundle_id => $bundle) {
          $locked = FALSE;
          if ($entity_type === 'node') {
            $nt = node_type_load($bundle_id);
            if ($nt->orig_type === 'product') {
              $locked = TRUE;
            }
          }
					$form[$entity_type][$bundle_id] = array(
						'#type' => 'checkbox',
						'#title' => $bundle['label'] . ($info['label'] != $bundle['label'] ? ' <em>(' . drupal_strtolower($info['label']) . ')</em>' : ''),
						'#return_value' => $bundle_id, // Backward compatibility
            '#disabled' => $locked,
					);

				}
			}
		break;
		case 'balances':
			$form['settings']['track'] = array(
				'#type'		=> 'checkbox',
				'#title'	=> t('Keep track of balances'),
			);
			$form['settings'][]['#markup'] = '<div style="padding-left:1em">'; // IKW: [17:44:07] ivanklut: и ещё можно появляющиеся настройки сделать со сдвигом вправо - типа иерархия
			$form['settings']['product_debit_condition'] = array(
				'#type'		=> 'select',
				'#title'	=> t('Time write-off of goods'),
				'#options'	=> array(
					'created' => t('Order is created'), // inherit from delivery_prepared
					'completed' => t('Order is completed'), // inherit from delivered. Detect is last status from statuses is set
					'paid' => t('Order is paid'), // leave as is
				),
				'#states' => array(
					'visible' => array(
						'input[name="balances[settings][track]"]' => array('checked' => TRUE),
					),
				),
			);
			$form['settings']['accept_requests'] = array(
				'#type' => 'checkbox',
				'#title' => t('Accept bids for products with a zero balance'),
				'#states' => array(
					'visible' => array(
						'input[name="balances[settings][track]"]' => array('checked' => TRUE),
					),
				),
				'#access' => FALSE, // IKW: temporary disable "Accept bids for products with a zero balance" @see also product_update_7016()
			);
			$form['settings'][]['#markup'] = '</div>'; // IKW: [17:44:07] ivanklut: и ещё можно появляющиеся настройки сделать со сдвигом вправо - типа иерархия

      $form['settings']['goods_on_request'] = array(
        '#type' => 'checkbox',
        '#title' => t('Sell goods on request'),
        '#description' => '',
      );
      $form['settings'][]['#markup'] = '<div style="padding-left:1em">';
      $form['settings']['supply_time'] = array(
        '#type' => 'digit',
        '#size' => 3,
        '#title' => t('Supply time'),
        '#description' => t('Default product supply time. This value can be overwritten in product item form. This value will be summed to delivery time.'),
        '#field_suffix' => t('days'),
        '#states' => array(
          'visible' => array(
            'input[name="balances[settings][goods_on_request]"]' => array('checked' => TRUE),
          ),
        ),
      );
      $form['settings'][]['#markup'] = '</div>';

      $form['settings']['qty_fraction'] = array(
				'#type' => 'checkbox',
				'#title' => t('Enable fractional quantity'),
				'#description' => t('Enables in product editing form additional setting for adjustment granularity of quantity.'),
			);
		break;
		default:
			$form = array();
	}
	return $form;
}


/**
 * Saving all forms changes.
 * Getting current settings and rewrite existing in form keys.
 */
function shop_settings_form_submit($form, &$form_state) {
	form_state_values_clean($form_state);
	foreach ($form_state['values'] as $type => $data) {
		if (is_array($data)) {
			unset($data['types']); // remove vertical tab artefact
		}
	}
	$shop_info = shop_get_info();
	shop_update_info($form_state['values']);
	drupal_set_message(t('The configuration options have been saved.'));
}


/**
 * IKW: Не будем плодить страницы настроек, пусть чекбокс
 * "Оплата без регистрации" будет в разделе "Коммерция" - "Конфигурация"
 *
 * mpatrin: эта форма еще понадобится, не удаляю.
 * @see shop_form_money_config_form_alter()
 */
/*
function shop_general_settings_form($form, $form_state) {
  $form = array();

  // admin/people/permissions#edit-submit-orders
  $form['submit_orders'] = array(
    '#type' => 'checkbox',
    '#title' => t('Order without registration'),
    '#description' => t("'Submit orders' permission for anonymous users"),
    '#default_value' => user_access('submit orders', drupal_anonymous_user()),
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  return $form;
}


function shop_general_settings_form_submit($form, &$form_state) {
  if (!empty($form_state['values']['submit_orders'])) {
    user_role_grant_permissions(DRUPAL_ANONYMOUS_RID, array('submit orders'));
  }
  else {
    user_role_revoke_permissions(DRUPAL_ANONYMOUS_RID, array('submit orders'));
  }

  drupal_set_message(t('Configuration saved.'));
} */
