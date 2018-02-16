<?php

/**
 * Callback for /money/payment/%/%
 */
function money_payment_form($form, &$form_state, $operation, $operation_key, $sum = NULL, $source = NULL, $source_key = NULL) {
  $source_info = $source ? money_sources_info($source, $source_key) : NULL;
  $sum *= 1;
  $msgs = array();

  $recipient = money_operation_get_recipient($operation, $operation_key);
  $currency = $recipient['currency'] ?? variable_get('money_default_currency', 'RUB');
  $currency_info = money_currency_info($currency);

  if (!$source_info) {
    $available_sources = _money_available_sources($operation, $operation_key, $sum, TRUE);

    if (count($available_sources) == 1) {
      $source_info = reset($available_sources);
    }
  }

  $form['#money_source_info'] = $source_info;

  drupal_set_title(filter_xss(money_operations_info($operation, $operation_key, 'title'), array('em', 'a',)), PASS_THROUGH);
  $form['operation_descr'] = array(
    '#type' => 'item',
    '#title' => t('Operation'),
    '#markup' => money_operations_info($operation, $operation_key, 'title'),
    '#inline' => TRUE,
  );
  $form['operation'] = array(
    '#type' => 'value',
    '#value' => $operation,
  );

  // IKW ненужная информация на этой форме
  /*
 $recipient = money_operation_get_recipient($operation, $operation_key);
 $form['recipient'] = array(
   '#type' => 'item',
   '#title' => t('Recipient'),
   '#inline' => TRUE,
   '#markup' => money_sources_info($recipient['source'], $recipient['key'], 'title') . (!empty($recipient['description']) ? ' <small>(' . $recipient['description'] . ')</small>' : ''),
 );
 */

  $form['operation_key'] = array(
    '#type' => 'value',
    '#value' => $operation_key,
  );
  $form['sum'] = array(
    '#type' => 'digit',
    '#inline' => TRUE,
    '#title' => t('Sum to paid'),
    '#min' => $sum && ($operation != 'refuel') ? $sum : pow(10, -1 * $currency_info['decimals']),
    '#max' => $sum && ($operation != 'refuel') ? $sum : 99999999,
    '#step' => pow(10, -1 * $currency_info['decimals']),
    '#validate_step' => TRUE,
    '#default_value' => $sum ?? NULL,
    '#required' => TRUE,
    '#size' => $sum ? drupal_strlen(format_price($sum)) : 10,
  );

  $form['currency'] = array(
    '#type' => 'value',
    '#value' => $currency,
  );

  if ($form['sum']['#min'] == $form['sum']['#max']) {
    $form['sum']['#attributes']['class'][] = 'element-invisible';
    $form['sum']['#field_suffix'] = theme('price', array('price' => $sum, 'currency' => $currency));
  }
  else {
    $form['sum']['#field_suffix'] = format_currency($currency);
  }
  $submit_text = t('Continue');
  if (!$source_info || !$sum) { // $source not specified, sum not covered with user cash balance:
    $available_sources = _money_available_sources($operation, $operation_key, $sum, TRUE);
    $opts = array_column($available_sources, 'title', 'source');

    if (!$opts) {
      drupal_access_denied();
      drupal_exit();
    }

    $default_value = $source_info ? $source_info['source'] : NULL;
    $form['source'] = array(
      '#title' => t('Payment method'),
      '#inline' => TRUE,
      '#required' => TRUE,
      '#type' => 'radios',
      '#options' => $opts,
      '#default_value' => count($opts) == 1 ? key($opts) : $default_value,
    );

    if($source_info) { // hide radios if $source_info is set
      $form['source']['#prefix'] = "<div class='element-invisible'>";
      $form['source']['#suffix'] = "</div>";
    }

    $form['source_key'] = array(
      '#type' => 'value',
      '#value' => $source_info ? $source_info['key'] : NULL,
    );
  }
  else { // $source and $sum are specified:
    $fn = money_sources_info($source_info['source'], $source_info['key'], 'module') . '_payment_form_callback';
    if (function_exists($fn)) {
      $submit_text = t('Pay');
//      moved to money_form_money_payment_form_alter()
//      $form['data'] = array(
//        '#type' => 'item',
//        '#title' => t('Payment method: %name', array('%name' => $source_info['title'])),
//        '#tree' => TRUE,
//      ) + $fn($operation, $operation_key, $sum, $source_info);
    }
    if (!$source_info['external']) {
      $submit_text = t('Pay');

      // @see cash_account_form_money_payment_form_alter()
      if ($source_info['source'] != 'user') {
        $money = money_get_available($source_info, $operation, $operation_key);
        $form['source_descr'] = array(
          '#type' => 'item',
          '#title' => t('Available funds'),
          '#inline' => TRUE,
          '#markup' => theme('price', array('price' => $money)),
        );
      }
    }

    $form['source'] = array(
      '#type' => 'value',
      '#value' => $source_info['source'],
    );
    $form['source_key'] = array(
      '#type' => 'value',
      '#value' => $source_info['key'],
    );
  }
  $cancel_text = t('Cancel');
  if ($cancel_url = isset($_GET['destination']) ? $_GET['destination'] : NULL) {
    $cancel_url = drupal_parse_url($cancel_url);

    if (preg_match('/^money\/payment\//', $cancel_url['path'])) {
      $args = arg(NULL, $cancel_url['path']);
      if ($sum && (empty($args[4]) || $args[4] != $sum) && money_execute_access($operation, $operation_key, $sum, NULL, TRUE)) {
        $args[4] = $sum;

        $cancel_url['path'] = implode('/', $args);
      }
      $cancel_text = t('Back');
    }
  }
  else {
    $cancel_url = array('path' => NULL, 'query' => NULL);
  }
  $form['actions'] = array(
    '#type' => 'actions',
    'cancel' => array(
      '#type' => 'link',
      '#title' => $cancel_text,
      '#href' => $cancel_url['path'],
      '#options' => array(
        'query' => $cancel_url['query'],
      ),
      '#attributes' => array(
        'class' => array(
          'form-submit',
        ),
      ),
    ),
    'submit' => array(
      '#type' => 'submit',
      '#value' => $submit_text,
      '#states' => array(
        'visible' => array(
          'input[name="sum"]' => array('empty' => FALSE,),
        ),
      ),
    ),
  );
  if (!empty($form['source']['#options'])) {
    $form['actions']['submit']['#states']['visible']['input[name="source"]']['checked'] = TRUE;
  }

  return $form;
}


function _money_hold_page($operation, $operation_key, $sum = NULL, $source = NULL, $source_key = NULL) {
  $source = money_sources_info($source, $source_key);
  money_hold($operation, $operation_key, $sum, $source);
  drupal_goto(drupal_get_destination());
}


function _money_unhold_page($operation, $operation_key, $source = NULL, $source_key = NULL) {
  $attributes = drupal_get_query_parameters();
  $token = $attributes['token'] ?? NULL;


  if (drupal_valid_token($token, "money-unhold-{$operation}-{$operation_key}-{$source}-{$source_key}")) {
    $source_info = money_sources_info($source, $source_key);
    money_unhold($operation, $operation_key, $source_info);
  }
  else {
    drupal_set_message(t('Invalid token!'), 'error');
  }

  drupal_goto(drupal_get_destination());
}

/**
 * Implements hook_form_BASE_FORM_ID_alter().
 */
function money_form_money_payment_form_alter(&$form, &$form_state, $form_id) {
  list($operation, $operation_key, $sum) = $form_state['build_info']['args'];

  $source_info = $form['#money_source_info'];

  if ($source_info) {
    if (!money_execute_access($operation, $operation_key, $sum, $source_info, TRUE, $msgs)) {
      foreach ($msgs as $m) {
        drupal_set_message($m, 'error');
      }
      drupal_access_denied();
      drupal_exit();
    }
    else {
      $fn = money_sources_info($source_info['source'], $source_info['key'], 'module') . '_payment_form_callback';
      if (function_exists($fn)) {
        $form += array('data' => array());
        $form['data']['#tree'] = TRUE;
        $form['data'] += array(
            '#type' => 'item',
            '#title' => t('Payment method: !name', array('!name' => $source_info['title'])),
          ) + call_user_func($fn, $operation, $operation_key, $sum, $source_info);
      }
    }
  }
}


function money_payment_form_validate($form, &$form_state) {
  $source_info = money_sources_info($form_state['values']['source'], isset($form_state['values']['source_key']) ? $form_state['values']['source_key'] : NULL);
  if (!$source_info || !money_execute_access($form_state['values']['operation'], $form_state['values']['operation_key'], $form_state['values']['sum'], $source_info, TRUE, $m)) {
    form_set_error('sum', implode('<br />', $m));
    form_set_error('source', '');
    form_set_error('actions', '');
  }
}

function money_payment_form_submit($form, &$form_state) {

  $fn = money_sources_info($form_state['values']['source'], $form_state['values']['source_key'] ?? NULL, 'module') . '_payment_form_callback';

  // User choosed source and press Continue and source payment form callback is exists:
  if (isset($form['source']['#options'][$form_state['values']['source']]) && function_exists($fn)) {
    $op = $form_state['values']['operation'];
    $op_key = $form_state['values']['operation_key'];
    $sum = $form_state['values']['sum'];
    $source = $form_state['values']['source'];
    $source_key = $form_state['values']['source_key'];

    $redirect = "money/payment/{$op}/{$op_key}/{$sum}/{$source}/{$source_key}";
    $destination = $_GET['destination'] ?? '';
    unset($_GET['destination']);

    drupal_goto($redirect, array('query' => array('destination' => $destination)));
  }
  else {
    money_transaction_begin();
    $money_transaction = money_new_transaction(array(
      'operation' => $form_state['values']['operation'],
      'operation_key' => $form_state['values']['operation_key'],
      'sum' => $form_state['values']['sum'],
      'currency' => $form_state['values']['currency'] ?? variable_get('money_default_currency', 'RUB'),
      'source' => $form_state['values']['source'],
      'source_key' => $form_state['values']['source_key'] ?? NULL,
      'data' => $form_state['values']['data'] ?? NULL,
    ));
    $result = money_transaction_save($money_transaction, $m);
    money_transaction_finish();
    drupal_set_message(implode('<br />', $m), $result ? 'status' : 'error');

    if (isset($_GET['destination'])) {
      $dest = $_GET['destination'];
      if (preg_match('/^money\/payment\/.*?[&\?]destination=(.*?)($|&|#)/', $dest, $m)) {
        $dest = rawurldecode($m[1]);
      }
      $_GET['destination'] = $dest;
    }
  }
}
