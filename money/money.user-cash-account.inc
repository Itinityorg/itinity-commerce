<?php

/**
* Callback for /user/%user/cash-account
*/
function money_cash_account_page($account) {
  $build = array();
  
  $build['balance'] = array(
    '#type' => 'item',
    '#title' => t('Balance'),
    'summary' => array(
      '#type' => 'item',
      '#title' => t('Sum'),
      '#inline' => TRUE,
      'sum' => array(
        '#theme' => 'price',
        '#price' => 100,
      ),
    ),
  );
  
  return $build;
}


/**
* Callback for /user/%user/cash-account/refuel
*/
function money_refuel_cash_account_page($account) {
  $money_transaction = (object)array(
    'transaction_type' => 'refuel',
    'transaction_target' => 'user:' . $account->uid,
  );
  module_load_include('inc', 'money', 'money.payment-form');
  return drupal_get_form('money_payment_form', $money_transaction);
}

function money_refuel_cash_account_form_submit($form, &$form_state) {
  
}
























