<?php


/**
 * Implements hook_block_configure().
 */
function _shop_block_configure($delta = '') {
  $form = array();
  if ($delta === 'cart') {
    $form['shop_cart_icon'] = array(
      '#type' => 'checkbox',
      '#title' => t('Display icon'),
      '#default_value' => variable_get('shop_cart_icon', FALSE),
      '#access' => module_exists('pictogram'),
    );

    $form['shop_cart_caption'] = array(
      '#type' => 'textfield',
      '#title' => t('Caption'),
      '#attributes' => array(
        'placeholder' => t('Without caption'),
      ),
      '#default_value' => variable_get('shop_cart_caption', NULL),
      '#size' => 60,
      '#maxlength' => 256,
    );

    $form['shop_cart_block_view'] = array(
      '#type' => 'select',
      '#title' => t('Shopping cart contents'),
      '#options' => array(
        'none' => t('Without content'), // display only total qty
        'summary' => t('Total sum'), // diplay total qty and total sum
        'simple' => t('List without prices'), // simple list with qtys (without prices) and total sum
        'detailed' => t('Detailed list'), // .. with total sum
        'editable' => t('Editable list'), // .. with total sum  
      ),
      '#default_value' => variable_get('shop_cart_block_view', 'detailed'),
    );
    /*
    DEPRECATED 12.12.2017:
    $form['shop_cart_block_checkout_caption'] = array(
      '#type' => 'textfield',
      '#maxlength' => 30,
      '#title' => t('Checkout order button'),
      '#default_value' => variable_get('shop_cart_block_checkout_caption', t('Checkout order')),
      '#access' => module_exists('order'),
    );*/

    /*layout*/
    $form['shop_cart_horizontal'] = array(
      '#type' => 'select',
      '#title' => t('Shopping cart laoyut'),
      '#options' => array(
        1 => t('Horizontal'),
        0 => t('Vertical'),
      ),
      '#default_value' => variable_get('shop_cart_horizontal', TRUE) ? 1 : 0,
    );
    $form = array(
      'shop_cart_settings' => array(
        '#type' => 'fieldset',
        '#title' => t('Shopping cart displaying'),
        $form,

      ),
    );
  }
  return $form;
}

/**
 * Implements hook_block_save().
 */
function _shop_block_save($delta = '', $edit = array()) {
  if ($delta == 'cart') {
    if ($edit['shop_cart_icon']) {
      variable_set('shop_cart_icon', TRUE);
    }
    else {
      variable_del('shop_cart_icon');
    }

    $edit['shop_cart_caption'] = trim($edit['shop_cart_caption']);
    if (!$edit['shop_cart_caption']) {
      variable_del('shop_cart_caption');
    }
    else {
      variable_set('shop_cart_caption', $edit['shop_cart_caption']);
    }

    if ($edit['shop_cart_block_view'] === 'detailed') {
      variable_del('shop_cart_block_view');
    }
    else {
      variable_set('shop_cart_block_view', $edit['shop_cart_block_view']);
    }
    /*
    DEPRECATED 12.12.2017:
    @TODO: function shop_update_7XXX() {variable_del('shop_cart_block_checkout_caption');}
    if (module_exists('order')) {
      $edit['shop_cart_block_checkout_caption'] = trim($edit['shop_cart_block_checkout_caption']);
      if ($edit['shop_cart_block_checkout_caption'] === t('Checkout order')) {
        variable_del('shop_cart_block_checkout_caption');
      }
      else {
        variable_set('shop_cart_block_checkout_caption', $edit['shop_cart_block_checkout_caption']);
      }
    }
    */
    
    if ($edit['shop_cart_horizontal']) { // default is TRUE
      variable_del('shop_cart_horizontal');
    }
    else {
      variable_set('shop_cart_horizontal', FALSE);
    }
  }
}


/**
* Theme callback for shop_carts_block_form
*/
function theme_shop_carts_block_form($variables) {
  
  $form = &$variables['form'];
  $html = array();
  
  if ($html['caption'] = drupal_render($form['caption'])) {
    $html['caption'] = '<div class="shop-cart-container">' . '<h3 class="shop-cart-block-caption">' . $html['caption'] . '</h3>' . '</div>';
  }
  $rows = array();
  foreach (element_children($form['carts']) as $shop_id) {
    $cart = $form['carts'][$shop_id];
    foreach (element_children($cart) as $key) {
      $item = $cart[$key];
      foreach (element_get_visible_children($item) as $idx) {
        $classes = isset($item[$idx]['#attributes']['class']) ? (array)$item[$idx]['#attributes']['class'] : array();
        if (!$classes || !array_intersect(array('element-hidden', 'element-invisible'), $classes)) {
          $rows[$key]['data'][$idx]['data'] = drupal_render($form['carts'][$shop_id][$key][$idx]);
        }
      }    
    }
  }
  foreach ($rows as $key => $row) {
    $rows[$key]['class'][] = 'cart-item';
    $rows[$key]['id'][] = 'cart-item-' . preg_replace('/^(.*?):(.*?):.+/', '$1-$2', $key);
    foreach ($row['data'] as $idx => $data) {
      $rows[$key]['data'][$idx]['class'][] = 'col-' . $idx;
    }
  }
  if ($rows) {
    $rows['_summary'] = array(
      'data' => array(
        array(
          'colspan' => count(reset($rows)['data']) - 1,
          'data' => '<strong>' . t('Total') . ': </strong>',
        ),
        array(
          'data' => '<strong>' . theme('price', array('price' => $form['#total_sum'])) . '</strong>',
        ),
      ),
    );
    $html['carts'] = 
      '<div class="shop-cart-container' . ($form['#shop_cart_block_view'] === 'none' || $form['#shop_cart_block_view'] === 'summary' ? ' element-hidden' : '') . '">' .
      theme('table', array(
        'rows' => $rows,
        'empty' => t('Cart is empty'),
        'attributes' => array('class' => array('shop-cart-items-list',),),
      )) .
      '<div class="element-hidden">' . drupal_render_children($form['carts']) . '</div>' .
      '</div>';
  }
  $html['others']= '<div class="shop-cart-container">' . drupal_render_children($form) . '</div>';
  return implode("\n", $html);
}


/**
* Shopping carts form inside block
* @see shop_block_view()
*/
function shop_carts_block_form($form, &$form_state) {
  
  form_load_include($form_state, 'inc', 'shop', 'shop.cart.block');

  /* 
  $shop_cart_block_view variants @see _shop_block_configure():
    'none' => t('Without content'), // display only total qty in <sup> tag over cart icon|caption
    'summary' => t('Total sum'), // display total qty in <sup> tag over cart icon|caption and only total sum
    'simple' => t('List without prices'), // list of goods with qtys without prices but with total sum in bottom
    'detailed' => t('Detailed list'), //  list of goods with qtys, prices and total sum in bottom
    'editable' => t('Editable list'), // list of goods with editable qtys, prices, delete buttons and total sum in bottom
  */
  $form += array(
    '#shop_cart_block_view' => variable_get('shop_cart_block_view', 'detailed'),
    '#shop_cart_icon' => variable_get('shop_cart_icon', FALSE),
    '#shop_cart_caption' => trim(variable_get('shop_cart_caption', NULL)),
    '#shop_cart_horizontal' => variable_get('shop_cart_horizontal', TRUE),
  );

  $form['#attributes']['class'][] = 'shop-cart-wrapper';
  $form['#attributes']['class'][] = $form['#shop_cart_horizontal'] ? 'cart-horizontal-mode' : 'cart-vertical-mode';


  $form['caption'] = array(
    '#access' => $form['#shop_cart_block_view'] === 'summary' || $form['#shop_cart_caption'] || $form['#shop_cart_icon'],
    'icon' => array(
      '#access' => $form['#shop_cart_icon'],
      '#type' => 'link',
      '#title' => theme('pictogram', array('icon' => 'basket')),
      '#href' => 'shop/cart',
      '#options' => array(
        'query' => drupal_get_destination(),
        'html' => TRUE,
        'attributes' => array(
          'class' => array('shop-cart-icon',),
        ),
      ),    
    ),
    'space' => array(
      '#markup' => '&nbsp;',
      '#access' => $form['#shop_cart_caption'] && $form['#shop_cart_icon']
    ),
    'text' => array(
      '#access' => !!$form['#shop_cart_caption'],
      '#type' => 'link',
      '#title' => $form['#shop_cart_caption'],
      '#href' => 'shop/cart',
      '#options' => array(
        'query' => drupal_get_destination(),
        'html' => TRUE,
        'attributes' => array(
          'class' => array('shop-cart-text',),
        ),
      ),    
    ),
    'qty' => array(
      '#type' => 'html_tag',
      '#tag' => 'small',
      '#access' => $form['#shop_cart_caption'] || $form['#shop_cart_icon'],
    ),
    'sum' => array(
      '#type' => 'html_tag',
      '#tag' => 'small',
      '#access' => $form['#shop_cart_block_view'] === 'summary',
    ),
  );

  $form['carts'] = array(
    '#tree' => TRUE,
  );
 
  
  $total_sum = $total_count = 0;
  foreach (shop_cart_multiple() as $shop_id => $shop_cart) {
    $total_count += count($shop_cart->items);
    
    $sum = shop_build_price('shop_cart', $shop_cart);
    unset($sum['delivery']);
    $total_sum += array_sum($sum);
    foreach ($shop_cart->items as $idx => $item) {
      $qty = $item->qty * 1;
      $form['carts'][$shop_id][$idx] = array(
        '#type' => 'container',
        '#inline' => TRUE,
        'title' => array(
          '#theme' => 'entity_link',
          '#entity_type' => $item->entity_type,
          '#entity' => $item->entity_id,
          '#text' => truncate_utf8($item->title, $form['#shop_cart_block_view'] === 'editable' ? 20 : 25, TRUE, TRUE, 3),
          '#attributes' => array(
            'title' => $item->title,
          ),
        ),
        'qty' => array(
          '#type' => 'digit',
          '#title' => 'qty',
          '#default_value' => $qty,
          '#title_display' => 'invisible',
          '#min' => 0,
          '#size' => min(drupal_strlen($qty), 4),
          '#step' => 1,
          '#float' => shop_get_info('balances', 'settings', 'qty_fraction'),
          '#attributes' => array(
            'title' => t('Qty'),
          ),
        ),
        'qty_readonly' => array(
          '#markup' => $qty,
          '#access' => $form['#shop_cart_block_view'] !== 'editable',
        ),
        'x' => array(
          '#markup' => ' x ',
          '#access' => ($form['#shop_cart_block_view'] === 'editable' || $form['#shop_cart_block_view'] === 'detailed') && $form['#shop_cart_block_view'] !== 'simple',
        ),
        'price' => array(
          '#theme' => 'price',
          '#price' => $item->price,
          '#access' => $form['#shop_cart_block_view'] !== 'simple',
        ),
        'delete' => array(
          '#type' => 'submit',
          '#value' => 'X',
          '#name' => $item->entity_type . '-' . $item->entity_id . '-' . $item->snapshot_id . '-delete',
          '#idx' => $idx,
          '#attributes' => array(
            'title' => t('Delete'),
          ),
          '#access' => $form['#shop_cart_block_view'] === 'editable',
        ),
      );
      if ($form['#shop_cart_block_view'] !== 'editable') {
        $form['carts'][$shop_id][$idx]['qty']['#attributes']['class'][] = 'element-hidden';
      }
    }
  }
  
  $form['caption']['qty']['#value'] = '<sup>' . l($total_count, 'shop/cart', array('query' => drupal_get_destination())) . '</sup>';
  if (!$form['caption']['qty']['#access']) {
    $form['caption']['sum']['#value'] = $total_count . '&nbsp;x&nbsp;' . theme('price', array('price' => $total_sum));
  }
  elseif ($total_sum) {
    $form['caption']['sum']['#value'] = theme('price', array('price' => $total_sum));
  }
  else {
    $form['caption']['sum']['#access'] = FALSE;
  }
 
  
  $form['#total_sum'] = $total_sum;
  $form['actions'] = array(
    '#type' => 'actions',
    'submit' => array(
      '#type' => 'submit',
      '#value' => t('Save'),
      '#attributes' => array('class' => array('js-hide',)),
    ),
    'add' => array(
      '#type' => 'textfield',
      '#prefix' => '<div class="element-hidden">', // Only for shop-cart.block.js
      '#suffix' => '</div>',
    ),
  );
  
  $form['#attributes']['class'][] = $total_count ? 'cart-not-empty' : 'cart-empty';
  
  // TODO: AJAX: 
  $form['#attached']['js'][] = drupal_get_path('module', 'shop') . '/js/shop_cart.block.js';
  $form['#attributes']['data-buy_button_form_url'] = url('js/shop-cart-buy-button-ajax', array('absolute' => TRUE, 'query' => array('get' => $_GET,),));
  $form['#attributes']['data-reload_block_url'] = url('js/shop-cart-ajax', array('absolute' => TRUE, 'query' => array('get' => $_GET,),));
  
  return $form;
}


/**
* Submit callback for shop_carts_block_form
*/
function shop_carts_block_form_submit($form, &$form_state) {
  if (arg(0) . '/' . arg(1) === 'js/shop-cart-ajax') {
    $form_state['no_redirect'] = TRUE;
  }
  if (preg_match('/^(.+?)-(\d+)-(\d+)-delete$/', $form_state['triggering_element']['#name'], $m)) {
    if ($e = entity_load($m[1], array($m[2]))) {
      $entity = reset($e);
      if (isset($entity->shop_id)) {
        $shop_cart = shop_cart($entity->shop_id);
        $shop_cart->delete($m[1], $m[2], $m[3]);
        drupal_set_message(t('An item %title has been removed from shop cart.', array('%title' => entity_label($m[1], $entity))));
      }
    }
  }
  else {
    if ($form_state['values']['add']) {
      list($entity_type, $entity_id, $qty) = explode(':', $form_state['values']['add']);
      if ($entity_type && $entity_id && ($e = entity_load($entity_type, array($entity_id)))) {
        $entity = reset($e);
        if (isset($entity->shop_id) && shop_list_shops($entity->shop_id)) {
          $snapshot = snapshot_create('cart_item_snapshot', $entity_type, $entity_id);
          $key = Shop\Cart::key($entity_type, $entity_id, $snapshot->sid);
          if (!isset($form_state['values']['carts'][$entity->shop_id][$key]['qty'])) {
            $form_state['values']['carts'][$entity->shop_id][$key]['qty'] = 0;
          }
          $form_state['values']['carts'][$entity->shop_id][$key]['qty'] += $qty ? $qty : 1;
        }      
      }
    }
    if (!empty($form_state['values']['carts'])) {
      foreach ($form_state['values']['carts'] as $shop_id => $items) {
        $shop_cart = shop_cart($shop_id);
        foreach ($items as $key => $data) {
          list($t, $id, $sid) = explode(':', $key);
          $shop_cart->set($t, $id, $sid, $data['qty']);
        }
      }
    }

    drupal_set_message(t('Shopping cart has been updated'), 'status', FALSE);
  }
}


// AJAX routines for shop cart block and buy buttons in product nodes
// @see shop_cart.block.js

/**
* Page deliver callback for /js/shop-cart***
*/
function shop_cart_ajax_deliver($page_callback_result) {
  drupal_add_http_header('Content-Type', 'application/json; charset=utf-8');
  // Print the response.
  print ajax_render(ajax_prepare_response($page_callback_result));
  // Perform end-of-request tasks.
  ajax_footer();
}


/**
* Helper for proper render forms in /js/shop-cart*** context
*/
function _shop_cart_ajax_override_request_uri() {
  drupal_static_reset('element_info');
  drupal_static_reset('arg');
  drupal_static_reset('drupal_get_destination');
  $_GET = $_GET['get'];
	element_info('form'); // initialize static
	$element_info = &drupal_static('element_info');
	$element_info['form']['#action'] = url($_GET['q'], array('query' => $_GET));
}


/**
* Callback for /js/shop-cart-ajax
*/
function _shop_cart_ajax() {
  if (isset($_POST['form_build_id'])) {
    $messages_backup = $_SESSION['messages'] ?? NULL;
    $_SESSION['messages'] = array();
    $prev_state = array();
    foreach (shop_cart_multiple() as $shop_id => $shop_cart) {
      foreach ($shop_cart->items as $item) {
        $prev_state[$item->entity_type . ':' . $item->entity_id] = ($item->qty * 1) . '';
      }
    }
    drupal_get_form('shop_carts_block_form');
    _shop_cart_ajax_override_request_uri();
    unset($_POST['form_id']); // avoid submission processing in drupal_get_form()
    $blocks = _block_get_renderable_array(_block_render_blocks(array(block_load('shop', 'cart'))));
    $new_state = array();
    foreach (shop_cart_multiple() as $shop_id => $shop_cart) {
      foreach ($shop_cart->items as $item) {
        $new_state[$item->entity_type . ':' . $item->entity_id] = ($item->qty * 1) . '';
      }      
    }
    $touched = array();
    foreach ($prev_state as $idx => $qty) {
      if (!isset($new_state[$idx]) || $new_state[$idx] !== $qty) {
        $touched[$idx] = explode(':', $idx);
      }
    }
    foreach ($new_state as $idx => $qty) {
      if (!isset($prev_state[$idx]) || $prev_state[$idx] !== $qty) {
        $touched[$idx] = explode(':', $idx);
      }
    }
    $messages = theme('status_messages');
    if ($messages_backup) {
      $_SESSION['messages'] = $messages_backup;
    }
    return array(
      '#type' => 'ajax',
      '#commands' => array(
        'block' => drupal_render($blocks['shop_cart']),
        'messages' => $messages,
        'touched' => $touched,
      ),
    );    
  }
  return '';
}


/**
* Callback for /js/shop-cart-buy-button-ajax
*/
function _shop_cart_buy_button_ajax() {
  if (!empty($_POST['entity_type']) && !empty($_POST['view_mode']) && !empty($_POST['entity_id']) && ($e = entity_load($_POST['entity_type'], array($_POST['entity_id'])))) {
    
    _shop_cart_ajax_override_request_uri();

    unset($_POST['form_id']); // avoid submission processing in drupal_get_form()
    $form = drupal_get_form('product_buy_button_form', $_POST['entity_type'], reset($e), $_POST['view_mode'], !empty($_POST['is_secondary']));

    return array(
      '#type' => 'ajax',
      '#commands' => array(
        'form' => drupal_render($form),
        'messages' => theme('status_messages'),
      ),
    );
  }
  return '';
}

