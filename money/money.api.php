<?php

/**
 * INCOMPLETE API for money module
 */


/**
 * Implements hook_money_sources_info()
 * Define your money sources here.
 * Returns keyed array of money sources implemented in this module.
 *
 * @param $source - optional, can be passed or not. You can use it for dinamycally change result.
 * @param $source_key - optional, can be passed or not. You can use it for dinamycally change result.
 */
function hook_money_sources_info($source, $source_key) {
  return array(
    'my_source' => array(
      'title' => t('Virtual moneys'),
      'external' => TRUE, // Set to FALSE if your money source can recieve moneys
      'description' => t('An "printing press" accessible only for site administrators.'),
      'online' => TRUE, // Set to TRUE if payment proceeds through the Web or FALSE if not
    ),
  );
}


/**
 * Implements hook_payment_settings_form()
 * Settings form for your external money source called in money_config_form()
 */
function hook_payment_system_settings_form($source_id, $settings) {
  if ($source_id === 'my_source') {
    return array(
      'require_comment' => array(
        '#type' => 'checkbox',
        '#title' => t('Require comment'),
        '#default_value' => !empty($settings['require_comment']),
      ),
    );
  }
}


/**
 * Implements hook_money_operations_info()
 * Define your money operations here
 * Returns keyed array of money operations implemented in this module.
 *
 * @param $operation - optional
 * @param $operation_key - optional
 * @param $property - optional
 */
function hook_money_operations_info($operation = NULL, $operation_key = NULL, $property = NULL) {
  $result = array(
    'refuel' => array(
      'title' => t('Refuel personal balance'),
    ),
  );
  if ($operation === 'refuel' && $operation_key) {
    if ($operation_key === 'site') {
      $result['refuel']['title'] = t('Refuel site cash account');
    }
    else {
      $result['refuel']['title'] = t('Refuel personal balance');
    }
  }
  return $result;
}


/**
 * Alters hook_money_operations_info()
 *
 * @param $result
 *  Result of original hook_money_operations_info()
 * @param $context
 *  An associative array of incoming parameters of original hook_money_operations_info()
 *
 */
function hook_money_operations_info_alter(&$result, $context) {
  $operation = $context['operation'];
  $operation_key = $context['operation_key'];
  $property = $context['property'];
}


/**
 * Implements hook_money_execute_access()
 * Check access for given operation
 * If module supports desired operation, this hook must returns TRUE or FALSE
 *
 * @param $operation - desired operation machine name
 * @param $operation_key - additional unique key in operation context
 * @param $sum - sum to payment
 * @param $source_info - information about money source, may be empty
 * @param $before_insert - boolean that indicates what this checking runs before execution (e.g., user only wants see list of accessible payment methods)
 * @param &$messages - optional. Fill this array with your access-denied messages
 */
function hook_money_execute_access($operation, $operation_key, $sum, $source_info, $before_insert, &$messages) {
  if ($source_info['source'] === 'my_source') {
    if (!user_access('administer moneys')) {
      $messages[] = t('Only for site administrators');
      return FALSE;
    }
    else {
      return TRUE;
    }
  }
}


/**
 * Alters hook_money_execute_access()
 *
 * @param $result
 *  Result of original hook_money_execute_access()
 * @param $context
 *  An associative array of incoming parameters of original hook_money_execute_access()
 */
function hook_money_execute_access_alter(&$result, $context) {
  $operation = $context['operation'];
  $operation_key = $context['operation_key'];
  $sum = $context['sum'];
  $source_info = $context['source_info'];
  $before_insert = $context['before_insert'];
  $messages = $context['messages'];
}


/**
 * Implements hook_payment_form_callback()
 */
function hook_payment_form_callback($operation, $operation_key, $sum, $source_info) {
  if ($source_info['source'] === 'my_source') {
    if (!empty($source_info['settings']['require_comment'])) {
      return array(
        'comment' => array(
          '#type' => 'textarea',
          '#title' => t('Comment'),
        ),
      );
    }

  }
}


/**
 * Implements hook_money_operation_get_recipient()
 * Returns possible recipient for given operation.
 * Result may be string contains only recipient type or array contains recipient type + recipient id pair
 *
 * @param $operation - desired operation machine name
 * @param $operation_key - additional unique key in operation context
 */
function hook_money_operation_get_recipient($operation, $operation_key) {
  if ($operation === 'refuel') {
    if ($operation_key === 'site') {
      return 'site';
    }
    else {
      return array('user', $operation_key);
    }
  }
}


/**
 * Alters money_operation_get_recipient()
 *
 * @param $source_info
 *  Result of original money_operation_get_recipient()
 * @param $context
 *  An associative array of incoming parameters of original money_operation_get_recipient()
 */
function hook_money_operation_get_recipient_alter(&$source_info, $context) {
  $source = $source_info['source'];
  $source_key = $source_info['source_key'];
  $recipient = money_sources_info($source, $source_key);
}


/**
 * Alters money_payment_widget()
 *
 * @param $build
 *  Result of original hook_money_execute_access()
 * @param $context
 *  An associative array of incoming parameters of original hook_money_execute_access()
 */
function hook_money_payment_widget_alter(&$build, $context) {
  $operation = $context['operation'];
  $operation_key = $context['operation_key'];
  $sum = $context['sum'];
  $source = $context['source'];
  $error_messages = $context['error_messages'];
}


/**
 * Alters money_get_available()
 *
 * @param $available
 * @param $context
 */
function hook_money_get_available_alter(&$available, $context) {
  list($source_info, $operation, $operation_key) = array_values($context);
}


/**
 * @param $rows
 * @param $source_info
 * @param $money_transactions
 */
function hook_money_transactions_list_alter(&$rows, $source_info, $money_transactions) {

}






