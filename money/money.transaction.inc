<?php

/**
* This file always included by money.module
*/


/**
* Implements hook_entity_info()
*/
function money_entity_info() {
  return array(
    'money_transaction' => array(
      'label' => t('Payment transaction'),
      'base table' => 'money_transactions',
      'controller class' => 'MoneyTransactionController',
      'fieldable' => FALSE,
      'uri callback' => 'money_transaction_uri',
      'label callback' => 'money_transaction_label',
      'entity keys' => array(
        'id' => 'id',
      ),
      'bundles' => array(),
    ),
  );
}


/**
* Controller class for money_transaction entity
* @see money_entity_info()
*/
class MoneyTransactionController extends DrupalDefaultEntityController {
    function attachLoad(&$queried_transactions, $revision_id = FALSE) {
        foreach ($queried_transactions as &$transaction) {
          $transaction->data = $transaction->data ? unserialize($transaction->data) : array();
          foreach ($transaction->data as $key => $value) {
            if ($key && !isset($transaction->$key)) {
              $transaction->$key = $value;
            }
          }
          //unset($transaction->data);
          // ^^^ @TODO: Why i comment this line ???
        }
        parent::attachLoad($queried_transactions, $revision_id);
    }
}


/**
* label callback for money_transaction entity type
* @see money_entity_info()
*/
function money_transaction_label($money_transaction, $entity_type) {
    return t('Money transaction #@num', array('@num' => $money_transaction->id));
}


/**
* URI callback for money_transaction entity type
* @see money_entity_info()
*/
function money_transaction_uri($money_transaction) {
  return array('path' => 'money/transaction/' . $money_transaction->id);
}


/**
* Add missing fields to money_transaction object before saving
*/
function money_transaction_fill_defaults(&$money_transaction) {
  $money_transaction = $money_transaction ? (array)$money_transaction : array();
  if (empty($money_transaction['#defaults_filled'])) {
    $query = $_GET;
    unset($query['q'], $query['destination']);
    $money_transaction += array(
      'uri' => $_GET['q'] . ($query ? '?' . drupal_http_build_query($query) : ''),
      'request_uri' => request_uri(),
      'uid' => $GLOBALS['user']->uid,
      'timestamp' => REQUEST_TIME,
      'ip' => ip_address(),
      'status' => 1,
      'operation' => NULL,
      'operation_key' => NULL,
      'mail' => NULL,
      'source_key' => NULL,
    );
    if ((!isset($money_transaction['recipient']) && !isset($money_transaction['recipient_key'])) && ($recipient = money_operation_get_recipient($money_transaction['operation'], $money_transaction['operation_key']))) {
      if ($recipient = money_operation_get_recipient($money_transaction['operation'], $money_transaction['operation_key'])) {
        $money_transaction['recipient'] = $recipient['source'];
        $money_transaction['recipient_key'] = $recipient['key'];      
      }
      else {
        $money_transaction['recipient'] = NULL;
        $money_transaction['recipient_key'] = NULL;
      }
    }
    
    $money_transaction['#defaults_filled'] = TRUE;
  }
  $money_transaction = (object)$money_transaction;
}


/**
* Create new money_transaction object in memory (without saving in database). 
* You can pass to this function any array of fields.
*/
function money_new_transaction($money_transaction = NULL) {
  $money_transaction = (object)($money_transaction ? $money_transaction : NULL);
  module_invoke_all('money_transaction_prepare', $money_transaction);
  $money_transaction->{'#prepared'} = TRUE;
  money_transaction_fill_defaults($money_transaction);
  return $money_transaction;
}


/**
* Helper function
* Optional db lock that may called before money_transaction_save()
* Use it for manually lock database before money operations.
* Recomended for money hold and other multiple operations or check for existsing transactions before call money_transaction_save()
* example:
  
  // 0 sec passed:
  // Set database lock
  money_transaction_begin();
  // parallel thread will wait while this block executes
  if (!entity_load('money_transaction', FALSE, array('internal_document_id' => $internal_document_id, 'source' => 'robokassa',))) {
    // 0.01 sec passed:
    money_transaction_save($money_transaction);
  }
  // 0.02 sec passed:
  // release lock
  money_transaction_finish();

  
* @see money_transaction_save()
* @see money_transaction_finish()
*/
function money_transaction_begin() {
  $money_transaction_begin = &drupal_static(__FUNCTION__);
  if ($money_transaction_begin) {
    throw new Exception('Unclosed money_transaction process.');
  }
  if (!lock_acquire(__FUNCTION__)) {
    lock_wait(__FUNCTION__);
    money_transaction_begin();
  }
  $money_transaction_begin = TRUE;
}


/**
* Helper function
* Unlock database locked in money_transaction_begin() context
*/
function money_transaction_finish() {
  $money_transaction_begin = &drupal_static('money_transaction_begin');
  if ($money_transaction_begin) {
    lock_release('money_transaction_begin');
    drupal_static_reset('money_transaction_begin');
  }
}


/**
* Create (or update existsing) money transaction entity
* Return TRUE|FALSE;
* You can use passed &$messages reference for display errors or successful result message
*/
function money_transaction_save($money_transaction, &$messages = NULL) {
  $result = FALSE;
  $messages = $messages ? $messages : array();
  $money_transaction_begin = &drupal_static('money_transaction_begin');
  
  // If we not in money_transaction_begin() context, try to lock database here:
  if (!$money_transaction_begin) {
    if (!lock_acquire('money_transaction_begin')) {
      // Another request is execute money transaction.
      lock_wait('money_transaction_begin');
      return money_transaction_save($money_transaction, $messages);
    }
  }


  $transaction = db_transaction();
  try {
    if (empty($money_transaction->{'#prepared'})) {
      module_invoke_all('money_transaction_prepare', $money_transaction);
    }
    else {
      unset($money_transaction->{'#prepared'});
    }
    money_transaction_fill_defaults($money_transaction);
    unset($money_transaction->{'#defaults_filled'});
    if ($money_transaction->uid && !isset($money_transaction->mail)) {
      if ($acc = user_load($money_transaction->uid)) {
        $money_transaction->mail = $acc->mail;
      }
    }
    // Load the stored entity, if any.
    if (!empty($money_transaction->id) && !isset($money_transaction->original)) {
      $money_transaction->original = entity_load_unchanged('money_transaction', $money_transaction->id);
    }
    $money_transaction->is_new = empty($money_transaction->id);
    // Allow modules to alter the money_transaction before saving.
    module_invoke_all('money_transaction_presave', $money_transaction);
    module_invoke_all('entity_presave', $money_transaction, 'money_transaction');
    if (!$money_transaction->operation || !$money_transaction->operation_key || !isset($money_transaction->sum)) {
      $messages[] = t('Not enough options');
    }
    elseif (!money_execute_access($money_transaction->operation, $money_transaction->operation_key, $money_transaction->sum, money_sources_info($money_transaction->source, $money_transaction->source_key), TRUE, $m)) {
      $messages[] = t('Access denied: !errors', array('!errors' => implode('; ', $m)));
    }
    else {
      // Pack unknown fields into data array
      $data = isset($money_transaction->data) ? (array)$money_transaction->data : array();
      unset($money_transaction->data);
      $known_fields = drupal_schema_fields_sql('money_transactions');
      $known_fields[] = 'is_new';
      $known_fields[] = 'original';
      foreach ($money_transaction as $key => $val) {
        if (!in_array($key, $known_fields, TRUE) && strpos($key, '#') !== 0) {
          $data[$key] = $val;
        }
      }
      $money_transaction->data = $data ? serialize($data) : NULL;
      $money_transaction->uri = drupal_substr($money_transaction->uri, 0, 255);
      $money_transaction->request_uri = drupal_substr($money_transaction->request_uri, 0, 255);
      $source_info = money_sources_info($money_transaction->source, $money_transaction->source_key);
      if ($money_transaction->is_new) {//
        $money_transaction->operation_info_snapshot = money_operations_info($money_transaction->operation, $money_transaction->operation_key, 'title');
        $money_transaction->source_balance_snapshot = $money_transaction->recipient_balance_snapshot = 0;
        if ($source_info) {
          $money_transaction->source_balance_snapshot = money_get_balance($source_info) - $money_transaction->sum;
        }
        if ($recipient_info = money_sources_info($money_transaction->recipient, isset($money_transaction->recipient_key) ? $money_transaction->recipient_key : NULL)) {
          $money_transaction->recipient_balance_snapshot = money_get_balance($recipient_info) + $money_transaction->sum;
        }
      }
      money_reset_static();
      $result = drupal_write_record('money_transactions', $money_transaction, $money_transaction->is_new ? array() : 'id');
      unset($money_transaction->data);
      if (!$result) {
        $messages[] = t('Can not save into database');
       
      }
      elseif ($money_transaction->is_new && $source_info) {
        money_unhold($money_transaction->operation, $money_transaction->operation_key, $source_info);       
      }
      // Clear the static loading cache.
      entity_get_controller('money_transaction')->resetCache(array($money_transaction->id));

      // Ignore slave server temporarily to give time for the
      // saved money_transaction to be propagated to the slave.
      db_ignore_slave();

      $op = $money_transaction->is_new ? 'insert' : 'update';
      // Tell the other modules a money_transaction has been submitted.
      module_invoke_all('money_transaction_' . $op, $money_transaction, $result);
      module_invoke_all('entity_' . $op, $money_transaction, 'money_transaction');
      _money_transaction_trigger($money_transaction, 'money_transaction_' . $money_transaction->operation . '_' . $op);
    }
    unset($money_transaction->original);
  }
  catch (Exception $e) {
    $transaction->rollback();
    watchdog_exception('money_transaction', $e);
    $messages[] = t('Database error');
    if (!$money_transaction_begin) {
      lock_release('money_transaction_begin');
    }
    //throw $e;
    return FALSE;
  }
  if (!$money_transaction_begin) {
    lock_release('money_transaction_begin');
  }
  if ($money_transaction->source === 'user') {
    module_invoke('admin_menu', 'flush_caches', $money_transaction->source_key);
  }
  if (!$result) {
    watchdog('money', 'Money transaction can not be executed. <br />Transaction: !transaction <br />Error messages: !errors', array(
    '!transaction' => '<pre>' . var_export((array)$money_transaction, TRUE) . '</pre>',
    '!errors' => $messages ? '<br />' . implode('<br />', array_map('check_plain', $messages)) : '-',
    ), WATCHDOG_ALERT);
  }
  elseif ($money_transaction->is_new && $money_transaction->operation) {
    if ($title = money_operations_info($money_transaction->operation, $money_transaction->operation_key, 'title')) {
      $messages[] = t('Operation <strong>"!op"</strong> for the sum <strong>!sum</strong> successfully completed.', array(
        '!op' => l(filter_xss_admin($title), entity_uri('money_transaction', $money_transaction)['path'], array('html' => TRUE, 'attributes' => array('class' => array('ajax-popup')))), 
        '!sum' => theme('price', array('price' => $money_transaction->sum,))));
    }
  }

  return $result;
}


/**
* Load single money transaction object from database by id
*/
function money_transaction_load($id, $reset = FALSE) {
  $money_transactions = money_transaction_load_multiple(array($id), array(), $reset);
  return $money_transactions ? $money_transactions[$id] : FALSE;
}


/**
* Load array of money transaction objects from database by passed ids or/and conditiions
*/
function money_transaction_load_multiple($ids = array(), $conditions = array(), $reset = FALSE) {
  return entity_load('money_transaction', $ids, $conditions, $reset);
}


/**
* Build renderable array for view money_transaction object
*/
function money_transaction_view($money_transaction) {
  
  // Remove previously built content, if exists.
  $money_transaction->content = array();

  entity_prepare_view('money_transaction', array($money_transaction->id => $money_transaction));
  $money_transaction->content += field_attach_view('money_transaction', $money_transaction, 'full');

  
  $money_transaction->content['id'] = array(
    '#type' => 'item',
    '#title' => t('Num'),
    '#markup' => $money_transaction->id,
    '#inline' => TRUE,
  );
  $money_transaction->content['timestamp'] = array(
    '#type' => 'item',
    '#title' => t('Date, time'),
    '#markup' => format_date($money_transaction->timestamp),
    '#inline' => TRUE,
  );

  $txt = money_operations_info($money_transaction->operation, $money_transaction->operation_key, 'title');
  $txt = $txt ? filter_xss($txt) : $money_transaction->operation;
  $money_transaction->content['operation'] = array(
    '#type' => 'item',
    '#title' => t('Operation'),
    '#markup' => $txt,
    '#inline' => TRUE,
  );
  
  if ($money_transaction->uri) {
    $title = $money_transaction->uri;
    if (!preg_match('/^[a-z]+:/i', $money_transaction->uri) && ($menu_item = menu_get_item($money_transaction->uri))) {
      $title = $menu_item['title'];
    }
    $money_transaction->content['uri'] = array(
      '#type' => 'item',
      '#title' => t('Page'),
      '#markup' => l($title, $money_transaction->uri),
      '#inline' => TRUE,
    );
  }

  
  
  $txt = money_sources_info($money_transaction->source, $money_transaction->source_key, 'title');
  $money_transaction->content['source'] = array(
    '#type' => 'item',
    '#title' => t('Source'),
    '#markup' => $txt ? $txt : $money_transaction->source,
    '#inline' => TRUE,
  );
  $txt = money_sources_info($money_transaction->recipient, $money_transaction->recipient_key, 'title');
  $money_transaction->content['recipient'] = array(
    '#type' => 'item',
    '#title' => t('Recipient'),
    '#markup' => $txt ? $txt : $money_transaction->recipient,
    '#inline' => TRUE,
  );
  $money_transaction->content['sum'] = array(
    '#type' => 'item',
    '#title' => t('Sum'),
    '#markup' => $money_transaction->sum,
    '#inline' => TRUE,
  );
  
  $acc = $money_transaction->uid ? user_load($money_transaction->uid) : NULL;
  $txt = $acc ? theme('username', array('account' => $acc)) : theme('email', array('email' => $money_transaction->mail));
  $money_transaction->content['user'] = array(
    '#type' => 'item',
    '#title' => t('User'),
    '#markup' => $txt,
    '#inline' => TRUE,
  );
  
  if (user_access('administer moneys')) {
    $money_transaction->content['admin-data'] = array(
      '#type' => 'fieldset',
      '#title' => 'Service data',
      '#collapsible' => TRUE,
      '#attributes' => array(
        'class' => array('collapsible'),
      ),
      '#attached' => array(
        'library' => array(
          array('system', 'drupal.collapse'),
        ),
      ),
      'source_balance_snapshot' => array(
        '#type' => 'item',
        '#title' => t('Source balance after operation'),
        '#inline' => TRUE,
        '#markup' => theme('price', array('price' => $money_transaction->source_balance_snapshot,)),
      ),
      'recipient_balance_snapshot' => array(
        '#type' => 'item',
        '#title' => t('Recipient balance after operation'),
        '#inline' => TRUE,
        '#markup' => theme('price', array('price' => $money_transaction->recipient_balance_snapshot,)),
      ),
      'ip' => array(
        '#type' => 'item',
        '#inline' => TRUE,
        '#title' => 'IP',
        '#markup' => check_plain($money_transaction->ip),
      ),
      'request_uri' => array(
        '#type' => 'item',
        '#inline' => TRUE,
        '#title' => 'URL',
        '#markup' => check_plain($money_transaction->request_uri),
      ),
      'internal_document_id' => array(
        '#type' => 'item',
        '#inline' => TRUE,
        '#title' => t('Internal document ID'),
        '#markup' => check_plain($money_transaction->internal_document_id),
      ),
    );
    $money_transaction->content['admin-data']['data'] = _money_transaction_render_data($money_transaction->data);
    if ($money_transaction->content['admin-data']['data']) {
      $money_transaction->content['admin-data']['data'] += array(
        '#type' => 'fieldset',
        '#collapsible' => TRUE,
        '#collapsed' => TRUE,
        '#title' => t('Additional data'),
      );
    }
  }
  
  
  // Allow modules to make their own additions to the money_transaction.
  module_invoke_all('money_transaction_view', $money_transaction);
  module_invoke_all('entity_view', $money_transaction, 'money_transaction', 'full', NULL);
  
  $build = $money_transaction->content;
  // We don't need duplicate rendering info in node->content.
  unset($money_transaction->content);
  
  $build += array(
    '#money_transaction' => $money_transaction,
  );

  
  // Allow modules to modify the structured node.
  $type = 'money_transaction';
  drupal_alter(array('money_transaction_view', 'entity_view'), $build, $type);
  
  return $build;
}


function _money_transaction_render_data($data, $reset = TRUE) {
  $result = array();
  foreach ((array)$data as $key => $val) {
    $result[$key] = array(
      '#type' => 'item',
      '#title' => check_plain($key),
      '#inline' => is_scalar($val),
    );
    if (is_scalar($val)) {
      $result[$key]['#markup'] = check_plain($val);
    }
    else {
      $result[$key] += _money_transaction_render_data($val, FALSE);
    }
  }
  return $result;
}














