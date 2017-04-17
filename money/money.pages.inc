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
    $form['refuel'] = money_payment_widget('refuel', !empty($source_info['key']) ? $source_info['key'] : $source_info['source']);
    $form['refuel']['#title'] = t('Refuel personal balance');
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


function money_transactions_list($source_info) {
  $params = drupal_get_query_parameters();

  $build = array();

  $header = array(
    'id' => array('data' => t('#, date, time'), 'field' => 'mt.id', 'sort' => 'desc',),
    'uid' => array('data' => t('User'), 'field' => 'username'),
    'operation' => array('data' => t('Operation'), 'field' => 'mt.operation_info_snapshot'),
    'source' => array('data' => t('Source'), 'field' => 'mt.source',),
    'arrow' => '&rarr;',
    'recipient' => array('data' => t('Recipient'), 'field' => 'mt.recipient',),
    'sum' => array('data' => t('Sum'), 'field' => 'mt.sum'),
    'uri' => array('data' => t('Page')),
    'balance' => array('data' => t('Balance'), 'field' => 'balance',),
  );
  if (!$source_info || $source_info['external']) {
    unset($header['balance']);
  }
  if (!user_access('administer moneys')) {
    unset($header['uri']);
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
    $query->condition('mt.timestamp', $params['timestamp']['max'] + (3600 * 24), '<=');
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

  foreach ($query->execute() as $r) {
    $money_transaction = money_transaction_load($r->id);
    $link = '-';

    if ($money_transaction->uri && ($m = menu_get_item($money_transaction->uri))) {
      $link = url($m['href']) . '<br /><small><em>' . l('"' . trim($m['title'], '"') . '"', $m['href']) . '</em></small>';
    }
    else {
      $link = url($money_transaction->uri);
    }

    $op_title = $money_transaction->operation ? money_operations_info($money_transaction->operation, $money_transaction->operation_key, 'title') : NULL;
    $op_title = $op_title ? $op_title : $money_transaction->operation_info_snapshot;

    $source_title = $money_transaction->source ? money_sources_info($money_transaction->source, $money_transaction->source_key, 'title') : NULL;
    $source_title = $source_title ? $source_title : $money_transaction->source . ($money_transaction->source_key ? ':' . $money_transaction->source_key : '');

    $recipient_title = $money_transaction->recipient ? money_sources_info($money_transaction->recipient, $money_transaction->recipient_key, 'title') : '-';

    if (arg(0) . '/' . arg(1) . '/' . arg(2) . '/' . arg(3) === 'admin/config/money/transactions') {
      if ($money_transaction->source && !$money_transaction->source_key) {
        $source_title = l($source_title, 'admin/config/money/transactions/' . $money_transaction->source, array('html' => TRUE));
      }
      if ($money_transaction->recipient && !$money_transaction->recipient_key) {
        $recipient_title = l($recipient_title, 'admin/config/money/transactions/' . $money_transaction->recipient, array('html' => TRUE));
      }
    }

    $is_negative = $source_info ? $money_transaction->source === $source_info['source'] && $money_transaction->source_key == $source_info['key'] : NULL;

    $sum = $money_transaction->sum * 1;

    $rows[$money_transaction->id] = array(
      'data' => array(
        'id' => array(
          'data' => l($money_transaction->id, entity_uri('money_transaction', $money_transaction)['path'], array('attributes' => array('class' => array('ajax-popup',)))) . ', ' . format_date($money_transaction->timestamp),
        ),
        'uid' => array(
          'data' => '',
        ),
        'operation' => array(
          'data' => $op_title,
        ),
        'source' => array(
          'data' => $source_title,
        ),
        'arrow' => array(
          'data' => $header['arrow'],
        ),
        'recipient' => array(
          'data' => $recipient_title ? $recipient_title : $money_transaction->recipient,
        ),
        'sum' => array(
          'data' => ($source_info && !$source_info['external']) ? theme('price', array('price' => $sum)) : format_price($sum),
          'class' => !$sum ? array('neutral') : ($is_negative ? array('negative') : array('positive')),
        ),
      ),
    );

    if (user_access('administer moneys')) {
      $rows[$money_transaction->id]['data']['uri'] = array(
        'data' => $link,
      );
      if ($r->user_exists) {
        $rows[$money_transaction->id]['data']['uid']['data'] = theme('entity_link', array('entity_type' => 'user', 'entity' => $r->uid, 'attributes' => array('class' => array('popup',))));
      }
      else {
        $rows[$money_transaction->id]['data']['uid']['data'] = $r->mail ? theme('email', array('email' => $r->mail)) . ' (' . ($r->uid ? 'uid:' . $r->uid : t('Guest')) . ')' : ($r->uid ? 'uid:' . $r->uid : t('Guest'));
      }
    }
    else {
      unset($rows[$money_transaction->id]['data']['uid']);
    }

    if ($source_info && !$source_info['external']) {
      if (
        ($money_transaction->source === $source_info['source'] && $money_transaction->source_key == $source_info['key'])
        ||
        ($money_transaction->recipient === $source_info['source'] && $money_transaction->recipient_key == $source_info['key'])
      ) {

        $class = array('balance');
        $class[] = !$sum ? 'neutral' : ($is_negative ? 'negative' : 'positive');
        $rows[$money_transaction->id]['data']['balance']['class'] = $class;
        $rows[$money_transaction->id]['data']['balance']['data'] = format_price($r->balance);
      }
      else {
        $rows[$money_transaction->id]['data']['balance']['data'] = '&nbsp;';
      }

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