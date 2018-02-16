<?php
/**
 * Created by PhpStorm.
 * User: mpatrin
 * Date: 05.08.16
 * Time: 11:33
 */


function money_transactions_overview_page($source = NULL) {
  return array(
    'overview' => drupal_get_form('money_source_transactions_overview_form', $source),
  );
}


function money_source_transactions_overview_form($form, &$form_state, $source_info = NULL) {
  if ($source_info) {
    drupal_set_title(t('Money transactions of !source', array('!source' => '<em>' . $source_info['title'] . '</em>')), PASS_THROUGH);
    $form['balance'] = empty($source_info['external']) ? money_balance_widget($source_info) : NULL;
  }

  $form['q'] = array(
    '#type' => 'hidden',
    '#value' => ($_GET['q'] !== 'system/ajax') ? $_GET['q'] : $form_state['values']['q'],
  );

  $form_state['input'] = $_GET;
  $_GET['q'] = $form['q']['#value'];

  if (!empty($form_state['values']['current_query'])) {
    $current_query = drupal_parse_url($form_state['values']['current_query']);
    $_GET = array('q' => $_GET['q']) + $current_query['query'];
  }

  unset($form_state['values']['current_query']);

  if (isset($form_state['triggering_element']['#parents'][0]) && $form_state['triggering_element']['#parents'][0] === 'filter') {
    money_transactions_filter_form_submit($form, $form_state);
  }

  $ajax_content_id = 'transactions-overview-ajax-wrapper';

  $form['content'] = array(
    '#attached' => array(
      'js' => array(drupal_get_path('module', 'money') . '/js/admin_money_transactions_overview_form.js'),
    ),
    '#type' => 'container',
    '#id' => $ajax_content_id,
    'filter' => money_transactions_filter_form($form, $form_state, $source_info),
    'list' => money_transactions_list($source_info),
    '#weight' => 10,
  );

  $ajax = array(
    'callback' => 'money_source_transactions_overview_form_ajax_callback',
    'wrapper' => $ajax_content_id,
    'event' => 'change',
    'progress' => array(),
  );

  $form['content']['filter']['timestamp']['min']['#ajax'] = $ajax;
  $form['content']['filter']['timestamp']['max']['#ajax'] = $ajax;
  $form['content']['filter']['sum']['min']['#ajax'] = $ajax;
  $form['content']['filter']['sum']['max']['#ajax'] = $ajax;
  $form['content']['filter']['source']['from']['#ajax'] = $ajax;
  $form['content']['filter']['source']['to']['#ajax'] = $ajax;

  $q = $_GET;
  unset($q['q']);
  $current_query = url($_GET['q'], array('query' => $q, 'absolute' => TRUE,));
  $form_state['input']['current_query'] = $current_query;

  $form['content']['current_query'] = array(
    '#type' => 'textfield',
    '#maxlength' => 1024,
    '#size' => 80,
    '#ajax' => $ajax,
    '#default_value' => $current_query,
    '#attributes' => array(
      'class' => array('element-invisible',),
    ),
  );

  $form['source_info'] = array(
    '#type' => 'value',
    '#value' => $source_info,
  );

  return $form;
}


function money_source_transactions_overview_form_ajax_callback($form, &$form_state) {
  return $form['content'];
}


function money_transactions_filter_form($form, &$form_state, $source_info) {
  $sources = array();
  $query = db_select('money_transactions', 'mt')->fields('mt', array('source'));
  if ($source_info) {
    if ($source_info['key']) {
      $query->condition(db_or()
        ->condition(db_and()->condition('mt.source', $source_info['source'])->condition('mt.source_key', $source_info['key']))
        ->condition(db_and()->condition('mt.recipient', $source_info['source'])->condition('mt.recipient_key', $source_info['key']))
      );
    }
    else {
      $query->condition(db_or()->condition('mt.source', $source_info['source'])->condition('mt.recipient', $source_info['source']));
    }
  }
  $query->groupBy('mt.source');
  foreach ($query->execute()->fetchCol() as $s) {
    $sources[$s] = $s;
    if ($t = money_sources_info($s, NULL, 'title')) {
      $sources[$s] = $t;
    }
  }

  $recipients = array();
  $query = db_select('money_transactions', 'mt')->fields('mt', array('recipient'));
  if ($source_info) {
    if ($source_info['key']) {
      $query->condition(db_or()
        ->condition(db_and()->condition('mt.source', $source_info['source'])->condition('mt.source_key', $source_info['key']))
        ->condition(db_and()->condition('mt.recipient', $source_info['source'])->condition('mt.recipient_key', $source_info['key']))
      );
    }
    else {
      $query->condition(db_or()->condition('mt.source', $source_info['source'])->condition('mt.recipient', $source_info['source']));
    }

  }
  $query->groupBy('mt.source');
  foreach ($query->execute()->fetchCol() as $s) {
    $recipients[$s] = $s;
    if ($t = money_sources_info($s, NULL, 'title')) {
      $recipients[$s] = $t;
    }
  }

  $filter = array(
    '#attached' => array(
      'css' => array(drupal_get_path('module', 'money') . '/css/admin_money_transactions_overview_form.css'),
    ),
    '#type' => 'item',
    '#title' => t('Money operations overview'),
    '#tree' => TRUE,
    '#attributes' => array(
      'class' => array('money-transactions-filter',),
    ),
    'timestamp' => array(
      '#type' => 'container',
      '#attributes' => array(
        'class' => array('container-inline', 'filter-group',),
      ),
      'min' => array(
        '#type' => 'datetime',
        '#default_value' => isset($_GET['timestamp']['min']) ? $_GET['timestamp']['min'] : NULL,
        '#title' => t('from', array(), array('context' => 'date-interval')),
        '#min' => 'Jul 2015',
        '#max' => '+1 day',
        '#group' => 'money-transactions-filter',
      ),
      'max' => array(
        '#type' => 'datetime',
        '#default_value' => isset($_GET['timestamp']['max']) ? $_GET['timestamp']['max'] : NULL,
        '#title' => t('to', array(), array('context' => 'date-interval')),
        '#min' => 'Jul 2015',
        '#max' => '+1 day',
        '#group' => 'money-transactions-filter',
      ),
    ),
    'sum' => array(
      '#type' => 'container',
      '#attributes' => array(
        'class' => array('container-inline', 'filter-group',),
      ),
      'min' => array(
        '#type' => 'digit',
        '#title' => t('Sum'),
        '#inline' => TRUE,
        '#attributes' => array(
          'placeholder' => t('from'),
        ),
        '#size' => 8,
        '#min' => 0.01,
        '#default_value' => isset($_GET['sum']['min']) ? $_GET['sum']['min'] : NULL,
      ),
      'separator' => array('#markup' => ' - ',),
      'max' => array(
        '#type' => 'digit',
        '#attributes' => array(
          'placeholder' => t('to'),
        ),
        '#size' => 8,
        '#min' => 0.01,
        '#default_value' => isset($_GET['sum']['max']) ? $_GET['sum']['max'] : NULL,
      ),
    ),
    'source' => array(
      '#type' => 'container',
      '#attributes' => array(
        'class' => array('container-inline', 'filter-group',),
      ),
      'from' => array(
        '#type' => 'select',
        '#default_value' => isset($_GET['source']['from']) ? $_GET['source']['from'] : NULL,
        '#options' => array(NULL => t('Any source')) + $sources,
        '#attributes' => array('title' => t('Source'),),
      ),
      'separator' => array('#markup' => ' ' . (module_exists('pictogram') ? theme('pictogram', array('icon' => 'right')) : '&rarr;') . ' ',),
      'to' => array(
        '#type' => 'select',
        '#default_value' => isset($_GET['source']['to']) ? $_GET['source']['to'] : NULL,
        '#options' => array(NULL => t('Any recipient')) + $recipients,
        '#attributes' => array('title' => t('Recipient'),),
      ),
    ),
    'actions' => array(
      '#parents' => array(),
      '#type' => 'actions',
      'submit' => array(
        '#type' => 'submit',
        '#value' => t('Filter'),
        '#submit' => array('money_transactions_filter_form_submit',),
        '#attributes' => array(
          'class' => array('js-hide',),
        ),
      ),
      'reset' => array(
        '#type' => 'submit',
        '#value' => t('Reset filter'),
        '#submit' => array('money_transactions_filter_form_reset',),
        '#attributes' => array(
          'class' => array('reset', 'js-hide',),
        ),
      ),
    ),
  );

  return $filter;
}


function money_transactions_filter_form_submit($form, &$form_state) {
  foreach ($form_state['values']['filter'] as $key => $item) {
    if ($item = is_array($item) ? array_filter($item) : $item) {
      $_GET[$key] = $item;
    }
    else {
      unset($_GET[$key]);
    }
  }
  unset($_GET['destination']);
}


function money_transactions_filter_form_reset($form, &$form_state) {
  foreach ($form_state['values']['filter'] as $key => $item) {
    unset($_GET[$key]);
  }
  unset($_GET['destination'], $_GET['order'], $_GET['sort'], $_GET['page']);
}


function money_transactions_list($source_info = NULL) {
  $params = drupal_get_query_parameters();

  $build = array();

  $header = array(
    'datetime' => array('data' => t('date/time'), 'field' => 'mt.id', 'sort' => 'desc',),
    'id' => array('data' => t('#'), 'field' => 'mt.timestamp', 'sort' => 'desc',),
    'operation' => array('data' => t('Operation'), 'field' => 'mt.operation_info_snapshot'),
    'source' => array('data' => t('Source'), 'field' => 'mt.source',),
    'arrow' => '&rarr;',
    'recipient' => array('data' => t('Recipient'), 'field' => 'mt.recipient',),
    'sum' => array('data' => t('Sum'), 'field' => 'mt.sum'),
  );

  if (module_exists('multicurrency')) {
    unset($header['sum']);
    $header['sum_s'] = array('data' => t('Sum<br/><small>source&nbsp;currency</small>'), 'field' => 'mt.sum');
    $header['sum_r'] = array('data' => t('Sum<br/><small>recipient&nbsp;currency</small>'), 'field' => 'mt.sum');
  }

  $header += array(
    'balance' => array('data' => t('Balance', array(), array('context' => 'accounting')), 'field' => 'balance',),
    'uid' => array('data' => t('Proceeded by'), 'field' => 'username'),
  );

  if (!$source_info || $source_info['external']) {
    unset($header['balance']);
  }

  if (!user_access('administer moneys')) {
    unset($header['uid']);
  }

  $limit = 30;
  $pager_element = PagerDefault::$maxElement;
  $query = db_select('money_transactions', 'mt')
    ->extend('PagerDefault')
    ->extend('TableSort')
    ->fields('mt')
    ->limit($limit)
    ->element($pager_element)
    ->orderByHeader($header);

  if (module_exists('social_profile')) {
    $query->addExpression('IF (u.lfm, u.lfm, u.name)', 'username');
  }
  else {
    $query->addExpression('u.name', 'username');
  }

  if ($source_info) {
    if ($source_info['key']) {
      $db_or = db_or()
        ->condition(db_and()->condition('mt.source', $source_info['source'])->condition('mt.source_key', $source_info['key']))
        ->condition(db_and()->condition('mt.recipient', $source_info['source'])->condition('mt.recipient_key', $source_info['key']));
    }
    else {
      $db_or = db_or()->condition('mt.source', $source_info['source'])->condition('mt.recipient', $source_info['source']);
    }

    $query->condition($db_or);
  }
  if (!empty($params['timestamp']['min'])) {
    $query->condition('mt.timestamp', $params['timestamp']['min'], '>=');
  }
  if (!empty($params['timestamp']['max'])) {
    $query->condition('mt.timestamp', $params['timestamp']['max'] + (3600 * 24), '<');
  }
  if (!empty($params['source']['from'])) {
    $query->condition('mt.source', $params['source']['from']);
  }
  if (!empty($params['source']['to'])) {
    $query->condition('mt.recipient', $params['source']['to']);
  }
  if (!empty($params['sum']['min'])) {
    $query->condition('mt.sum', $params['sum']['min'], '>=');
  }
  if (!empty($params['sum']['max'])) {
    $query->condition('mt.sum', $params['sum']['max'], '<=');
  }
  $query->leftJoin('users', 'u', 'u.uid = mt.uid');
  $query->addExpression('IF (u.uid, 1, 0)', 'user_exists');
  if ($source_info && !$source_info['external']) {
    $query->addExpression('IF(mt.recipient = :r AND mt.recipient_key = :rk, mt.recipient_balance_snapshot, mt.source_balance_snapshot)', 'balance', array(':r' => $source_info['source'], ':rk' => $source_info['key']));
  }

  $rows = array();
  $money_transactions = array();
  $period_profit = array();

  $query->addTag('money_transactions_list');

  foreach ($query->execute() as $r) {
    $money_transaction = money_transaction_load($r->id);
    $money_transactions[$r->id] = $money_transaction;

    if ($source_info) {
      if ($source_info['source'] == $money_transaction->recipient) {
        $period_profit[$money_transaction->currency] = ($period_profit[$money_transaction->currency] ?? 0) + $r->sum;
      }
      else {
        $period_profit[$money_transaction->currency] = ($period_profit[$money_transaction->currency] ?? 0) - $r->sum;
      }
    }

    $op_title = _money_transactions_list_operation_title_helper($money_transaction);
    $source_title = _money_transactions_list_source_title_helper($money_transaction, 'source');
    $recipient_title = _money_transactions_list_source_title_helper($money_transaction, 'recipient');

    $is_negative = $source_info ? $money_transaction->source === $source_info['source'] && $money_transaction->source_key == $source_info['key'] : NULL;

    $rows[$money_transaction->id]['data']['datetime'] = array(
      'data' => format_date($money_transaction->timestamp),
    );
    $rows[$money_transaction->id]['data']['id'] = array(
      'data' => l($money_transaction->id, entity_uri('money_transaction', $money_transaction)['path'], array('attributes' => array('class' => array('ajax-popup',)))),
      'class' => array('rteright'),
    );

    $rows[$money_transaction->id]['data']['operation'] = array(
      'data' => $op_title,
    );
    $rows[$money_transaction->id]['data']['source'] = array(
      'data' => $source_title,
    );

    $rows[$money_transaction->id]['data']['arrow'] = array(
      'data' => $header['arrow'],
      'class' => array('rtecenter'),
    );
    $rows[$money_transaction->id]['data']['recipient'] = array(
      'data' => $recipient_title ? $recipient_title : $money_transaction->recipient,
    );


    $sum = $money_transaction->sum * 1;
    if (module_exists('multicurrency')) {
      $class = array('rteright', (!$sum || !$source_info ? 'neutral' : ($is_negative ? 'negative' : 'positive')));

      $sum_s = _multicurrency_convert_with_rates($sum, 1, $money_transaction->source_currency_rate);
      $rows[$money_transaction->id]['data']['sum_s'] = array(
        'data' => theme('price', array('price' => $sum_s, 'currency' => $money_transaction->source_currency)),
        'class' => $class,
      );

      $sum_r = _multicurrency_convert_with_rates($sum, 1, $money_transaction->recipient_currency_rate);
      $rows[$money_transaction->id]['data']['sum_r'] = array(
        'data' => theme('price', array('price' => $sum_r, 'currency' => $money_transaction->recipient_currency)),
        'class' => $class,
      );
    }
    else {
      $rows[$money_transaction->id]['data']['sum'] = array(
        'data' => theme('price', array('price' => $sum, 'currency' => $money_transaction->currency)),
        'class' => array('rteright', (!$sum || !$source_info ? 'neutral' : ($is_negative ? 'negative' : 'positive'))),
      );
    }


    if ($source_info && !$source_info['external']) {
      if (
        ($money_transaction->source === $source_info['source'] && $money_transaction->source_key == $source_info['key'])
        ||
        ($money_transaction->recipient === $source_info['source'] && $money_transaction->recipient_key == $source_info['key'])
      ) {

        $class = array('balance', 'rteright');
        $rows[$money_transaction->id]['data']['balance']['class'] = $class;
        $rows[$money_transaction->id]['data']['balance']['data'] = theme('price', array('price' => $r->balance, 'currency' => $money_transaction->currency));
      }
      else {
        $rows[$money_transaction->id]['data']['balance']['data'] = '&nbsp;';
      }
    }

    if (user_access('administer moneys')) {
      if ($r->user_exists) {
        $rows[$money_transaction->id]['data']['uid']['data'] = theme('entity_link', array('entity_type' => 'user', 'entity' => $r->uid, 'attributes' => array('class' => array('popup',))));
      }
      else {
        $rows[$money_transaction->id]['data']['uid']['data'] = $r->mail ? theme('email', array('email' => $r->mail)) . ' (' . ($r->uid ? 'uid:' . $r->uid : t('Guest')) . ')' : ($r->uid ? 'uid:' . $r->uid : t('Automatically'));
      }
    }
    else {
      unset($rows[$money_transaction->id]['data']['uid']);
    }

  }

  drupal_alter('money_transactions_list', $rows, $source_info, $money_transactions);

  $build['period_profit'] = array(
    '#type' => 'item',
    '#title' => t('Period profit'),
    '#access' => !empty($source_info),
  );

  if (count($period_profit) == 1) {
    $currency = key($period_profit);
    $sum = $period_profit[$currency];
    $build['period_profit'] += array(
      '#inline' => TRUE,
      '#markup' => theme('price', array('price' => $sum, 'currency' => $currency)),
    );
  }
  else {
    foreach ($period_profit as $currency => $sum) {
      $build['period_profit'][$currency] = array(
        '#type' => 'item',
        '#inline' => TRUE,
        '#title' => money_currency_info($currency, 'full'),
        '#markup' => theme('price', array('price' => $sum, 'currency' => $currency)),
      );
    }
  }

  $build['list'] = array(
    '#theme' => 'table',
    '#attributes' => array(
      'style' => 'width: 100%',
      'class' => array('money-transaction-list',),
    ),
    '#caption' => '<hr />',
    '#header' => $header,
    '#rows' => $rows,
    '#empty' => t('Operations not found'),
  );
  if ($rows) {
    $build['pager'] = array(
      '#theme' => 'pager',
      '#element' => $pager_element,
    );
  }
  else {
    PagerDefault::$maxElement--;
    unset($GLOBALS['pager_page_array'][$pager_element], $GLOBALS['pager_total'][$pager_element], $GLOBALS['pager_total_items'][$pager_element], $GLOBALS['pager_limits'][$pager_element]);
  }
  return $build;
}


function _money_transactions_list_operation_title_helper($money_transaction) {
  $money_transaction = is_array($money_transaction) ? (object) $money_transaction : $money_transaction;
  $op_info = money_operations_info($money_transaction->operation, $money_transaction->operation_key);
  return $op_info['overview']['title'] ?? $op_info['title'] ?? $money_transaction->operation_info_snapshot ?? t('unknown');
}


/**
 * @param $money_transaction
 * @param $key = 'source' || 'recipient'
 */
function _money_transactions_list_source_title_helper($money_transaction, $key) {
  $money_transaction = is_array($money_transaction) ? (object) $money_transaction : $money_transaction;
  $source = $money_transaction->{$key};
  $source_key = $key == 'source' ? $money_transaction->source_key : $money_transaction->recipient_key;

  $source_info = money_sources_info($source, $source_key);

  $title = $source_info['overview']['title'] ?? $source_info['title'] ?? ($source . (!$source_key ?: ":{$source_key}"));
  $description = money_payment_systems_settings($source, 'title') ?? $source_info['overview']['description'] ?? $source_info['description'] ?? NULL;

  return $title . ($description ? "<br/><small>{$description}</small>" : '');
}