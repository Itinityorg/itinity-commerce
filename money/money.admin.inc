<?php

/**
 * Form callback for /admin/config/money
 */
function money_config_form($form, &$form_state) {
  if (arg(0) . '/' . arg(1) . '/' . arg(2) === 'admin/config/money') {
    drupal_set_title(t('Commerce'));
  }

  $def = variable_get('money_default_currency', 'RUB');
  $opts = money_currency_info(NULL, 'full');
//  asort($opts);
  $opts = isset($opts[$def]) ? array($def => $opts[$def]) + $opts : $opts;

  $form['money_default_currency'] = array(
    '#title' => t('Default currency'),
    '#inline' => TRUE,
    '#type' => 'select',
    '#options' => $opts,
    '#default_value' => $def,
    '#disabled' => TRUE, // in construction
  );

  $form['actions'] = array(
    '#type' => 'actions',
    'submit' => array(
      '#type' => 'submit',
      '#value' => t('Apply'),
    ),
  );

  return $form;
}


function money_config_form_submit($form, &$form_state) {
  variable_set('money_default_currency', $form_state['values']['money_default_currency']);
  drupal_set_message(t('The changes have been saved.'));
}



function money_payment_system_fieldset_pre_render($element) {
  $element['#title'] .= drupal_render($element['enabled']);
  return $element;
}
/**
 * Form callback for /admin/config/money/payment
 */
function money_payment_config_form($form, &$form_state) {
  if (arg(0) . '/' . arg(1) . '/' . arg(2) . '/' . arg(3) === 'admin/config/money/payment') {
    drupal_set_title(t('Payment'));
  }
  $form['#attached']['css'][] = drupal_get_path('module', 'money') . '/css/money.admin.css';
  $form['#attached']['js'][] = drupal_get_path('module', 'money') . '/js/money.admin.js';
  $form['#attached']['library'][] = array('system', 'ui.sortable');
  $money_payment_systems_settings = variable_get('money_payment_systems_settings', array());
  foreach ($money_payment_systems_settings as $key => $data) {
    $form['money_payment_systems_settings'][$key] = array();
  }
  $GLOBALS['conf']['money_payment_systems_settings'] = array();
  foreach (money_sources_info() as $source_id => $info) {
    $form['money_payment_systems_settings'][$info['source']] = array(
      '#pre_render' => array('money_payment_system_fieldset_pre_render',),
      '#type' => 'fieldset',
      '#title' => theme('pictogram', array('icon' => 'resize-vertical')) . ' ' . $info['title'],
      '#attributes' => array(
        'class' => array('money-payment-sortable',),
      ),
      'enabled' => array(
        '#type' => 'checkbox',
        '#title' => t('Enabled'),
        '#default_value' => isset($money_payment_systems_settings[$info['source']]['enabled']) ? $money_payment_systems_settings[$info['source']]['enabled'] : $info['enabled'],
        '#description' => $info['description'],
        '#disabled' => !$info['external'],
        //'#parents' => array('money_payment_systems_settings', $info['source'], 'enabled',),
      ),
      'settings' => array(
        'title' => array(
          '#type' => 'textfield',
          '#title' => t('Name'),
          '#inline' => TRUE,
          '#size' => module_invoke('utils', 'client_device_type') === 'computer' ? 80 : NULL,
          '#default_value' => isset($money_payment_systems_settings[$info['source']]['title']) && $money_payment_systems_settings[$info['source']]['title'] !== $info['title'] ? $money_payment_systems_settings[$info['source']]['title'] : NULL,
          '#attributes' => array(
            'placeholder' => $info['title'],
            'title' => t('Leave empty for defaults'),
          ),
          '#parents' => array(
            'money_payment_systems_settings',
            $info['source'],
            'title',
          ),
        ),
        'description' => array(
          '#type' => 'textfield',
          '#title' => t('Description'),
          '#inline' => TRUE,
          '#size' => module_invoke('utils', 'client_device_type') === 'computer' ? 80 : NULL,
          '#default_value' => isset($money_payment_systems_settings[$info['source']]['description']) && $money_payment_systems_settings[$info['source']]['description'] !== $info['description'] ? $money_payment_systems_settings[$info['source']]['description'] : NULL,
          '#attributes' => array(
            'placeholder' => strip_tags($info['description']),
            'title' => t('Leave empty for defaults'),
          ),
          '#parents' => array(
            'money_payment_systems_settings',
            $info['source'],
            'description',
          ),
        ),
      ),
    );
    $fn = $info['module'] . '_payment_system_settings_form';

    if (function_exists($fn)) {
      $settings = array();

      if (isset($money_payment_systems_settings[$info['source']]['settings'])) {
        $settings = $money_payment_systems_settings[$info['source']]['settings'];
      }
      elseif (isset($info['settings'])) {
        $settings = $info['settings'];
      }

      if ($settings_form = $fn($source_id, $settings)) {
        $form['money_payment_systems_settings'][$info['source']]['settings'] += $settings_form;
      }
    }
  }
  $GLOBALS['conf']['money_payment_systems_settings'] = $money_payment_systems_settings;
  $form['money_payment_systems_settings'] = array_filter($form['money_payment_systems_settings']);
  if ($form['money_payment_systems_settings']) {
    $form['money_payment_systems_settings'] += array(
      '#tree' => TRUE,
    );
  }
  
  $form['actions'] = array(
    '#type' => 'actions',
    'submit' => array(
      '#type' => 'submit',
      '#value' => t('Apply'),
    ),
  );

  return $form;
}


function money_payment_config_form_submit($form, &$form_state) {
  $settings = array();
  foreach ($form_state['input']['money_payment_systems_settings'] as $key => $data) { // Get order of sortable elements from ['input']
    if (isset($form_state['values']['money_payment_systems_settings'][$key])) {
      $settings[$key] = $form_state['values']['money_payment_systems_settings'][$key];
    }
  }
  
  $settings += variable_get('money_payment_systems_settings', array());
  variable_set('money_payment_systems_settings', $settings);
  drupal_set_message(t('The changes have been saved.'));
}


/**
 * @todo delete?
 */
function admin_money_transactions_overview_filter_ajax_callback($form, &$form_state) {
  return $form['wrapper'];
}























