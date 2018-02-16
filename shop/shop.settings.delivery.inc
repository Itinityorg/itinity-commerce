<?php

function shop_settings_delivery_method_form($form, &$form_state, $method) {
  if ($info = shop_delivery_info(NULL, $method)) {
    drupal_set_title(t('Settings for %name delivery method', array('%name' => $info['name'])), PASS_THROUGH);
    $form['#delivery_method'] = $method;
    if ($info['settings callback'] && ($settings_form = shop_delivery_settings_form(NULL, $method))) {
      $form['settings'] = array(
        '#tree' => TRUE,
        '#type' => 'item', // 'item' enables #element_validate support for method settings form
      ) + $settings_form;
    }
    $payment_allow_opts = array();
    foreach (money_sources_info() as $source_id => $source_info) {
      if ($source_info['external'] && $source_info['enabled']) {
        $payment_allow_opts[$source_id] = $source_info['title'];
      }
    }
    if ($payment_allow_opts) {
      $form['payment_allow'] = array(
        '#type' => 'select',
        '#multiple' => TRUE,
        '#title' => t('Allow payment methods'),
        '#options' => array(NULL => t('Any')) + $payment_allow_opts,
        '#default_value' => !empty($info['payment_allow']) ? array_filter($info['payment_allow']) : array(),
        '#access' => !module_exists('cash_account'),
      );
    }
    $form['actions'] = array(
      '#type' => 'actions',
      'submit' => array(
        '#type' => 'submit',
        '#value' => t('Apply'),
      ),
      'cancel' => array(
        '#type' => 'link',
        '#title' => t('Cancel'),
        '#href' => 'admin/config/money/delivery',
      ),
    );
    return $form;
  }
  else {
    drupal_not_found();
    drupal_exit();
  }
}

function shop_settings_delivery_method_form_validate($form, &$form_state) {
  // TODO
}

function shop_settings_delivery_method_form_submit($form, &$form_state) {
  
  $shop_delivery_info = variable_get('shop_delivery_info', array());
  if (isset($form_state['values']['settings'])) {
    $shop_delivery_info[$form['#delivery_method']]['settings'] = $form_state['values']['settings'];
  }
  $shop_delivery_info[$form['#delivery_method']]['payment_allow'] = isset($form_state['values']['payment_allow']) ? $form_state['values']['payment_allow'] : array();
  shop_delivery_info_save(NULL, $shop_delivery_info);
  /*variable_set('shop_delivery_info', $shop_delivery_info);
  drupal_static_reset('shop_delivery_info');*/
  drupal_set_message(t('Settings for %name delivery method was updated.', array('%name' => shop_delivery_info(NULL, $form['#delivery_method'], 'name'))));
}


function shop_settings_delivery_form_delivery_fieldset_post_render($element) {
  $element['#title'] .= drupal_render($element['enabled']);
  return $element;
}
/**
* Callback for /admin/config/money/delivery
*/
function shop_settings_delivery_form($form, &$form_state) {
	$form['#attached']['js'][] = drupal_get_path('module', 'shop') . '/js/settings_delivery.js';
	$form['#attached']['css'][] = drupal_get_path('module', 'shop') . '/css/shop_settings.css';
  $form['#attached']['library'][] = array('system', 'ui.sortable');
  $form['delivery']['#tree'] = TRUE;
  
  
  foreach (shop_delivery_info() as $key => $info) {
    $form['delivery'][$key] = array(
      '#type' => 'fieldset',
      '#pre_render' => array('shop_settings_delivery_form_delivery_fieldset_post_render'),
      '#attributes' => array(
        'class' => array('shop-delivery-sortable'),
      ),
      '#title' => theme('pictogram', array('icon' => 'resize-vertical')) . ' ' . $info['name'],
      '#description' => $info['description'],
      'enabled' => array(
        '#type' => 'checkbox',
        '#title' => t('Enabled'),
        '#default_value' => $info['enabled'],
      ),
      'title' => array(
        '#type' => 'textfield',
        '#maxlength' => 128,
        '#inline' => TRUE,
        '#title' => t('Name'),
        '#default_value' => $info['title'] == $info['name'] ? NULL : $info['title'],
        '#attributes' => array(
          'placeholder' => $info['name'],
        ),
        '#states' => array(
          'visible' => array(
            'input[name="delivery[' . $key . '][enabled]"]' => array('checked' => TRUE,),
          ),
        ),
      ),
    );
    //if ($info['settings callback'] && ($settings_form = shop_delivery_settings_form($key))) {
      $form['delivery'][$key]['settings'] = array(
        '#type' => 'container',
        //'#children' => 'admin/config/money/delivery/' . $key,
        'config' => array(
          '#type' => 'link',
          '#title' => t('Settings'),
          '#href' => 'admin/config/money/delivery/' . $key,
          '#options' => array(
            'query' => drupal_get_destination(),
          ),
          '#attributes' => array(
            'class' => array('ajax-popup',),
          ),
        ),
        '#states' => array(
          'visible' => array(
            'input[name="delivery[' . $key . '][enabled]"]' => array('checked' => TRUE,),
          ),
        ),
      );// + $settings_form;
    //}
  }
  
  $form['actions'] = array(
    '#type' => 'actions',
    'submit' => array(
      '#type' => 'submit',
      '#value' => t('Save'),
    ),
  );
  return $form;
}

/**
* Validation callback for shop_settings_delivery_form
*/
function shop_settings_delivery_form_validate($form, &$form_state) {
  // Skip validation of disabled methods
  // Clear form errors that was generated by disabled delivery methods elements
///////////////    deprecated    if ($form_set_error = &drupal_static('form_set_error', array())) {
///////////////    deprecated      foreach ($form_state['values']['delivery'] as $key => $data) {
///////////////    deprecated        if (!$data['enabled']) {
///////////////    deprecated          foreach ($form_set_error as $k => $m) {
///////////////    deprecated            if (strpos($k, 'delivery][' . $key . ']') !== FALSE) {
///////////////    deprecated              unset($form_set_error[$k]);
///////////////    deprecated              if (!empty($_SESSION['messages']['error'])) {
///////////////    deprecated                foreach ($_SESSION['messages']['error'] as $idx => $t) {
///////////////    deprecated                  if ($t === $m) {
///////////////    deprecated                    unset($_SESSION['messages']['error'][$idx]);
///////////////    deprecated                    $_SESSION['messages']['error'] = array_filter($_SESSION['messages']['error']);
///////////////    deprecated                    $_SESSION['messages'] = array_filter($_SESSION['messages']);
///////////////    deprecated                  }
///////////////    deprecated                }
///////////////    deprecated              }
///////////////    deprecated            }
///////////////    deprecated          }
///////////////    deprecated        }
///////////////    deprecated      }
///////////////    deprecated    }

}

function shop_settings_delivery_form_submit($form, &$form_state) {
  $data = array();
  foreach (array_keys($form_state['input']['delivery']) as $key) { // Order submitted items by weight @see shop-delivery-sortable class in shop_settings_delivery_form()
    if (isset($form_state['values']['delivery'][$key])) {
      $data[$key] = $form_state['values']['delivery'][$key];
    }
  }
  foreach ($data as $key => $info) {
    if (trim(drupal_strtolower($info['title'])) === trim(drupal_strtolower(shop_delivery_info(NULL, $key, 'name')))) {
      unset($data[$key]['title']);
    }
  }
  $shop_delivery_info = variable_get('shop_delivery_info', array());
  foreach ($data as $key => $item) {
    $data[$key] += isset($shop_delivery_info[$key]) ? $shop_delivery_info[$key] : array();
  }
  variable_set('shop_delivery_info', $data);
  drupal_static_reset('shop_delivery_info');
  drupal_set_message(t('The configuration options have been saved.'));
}
















