<?php


function theme_shop_cart_inblock_form($variables) {
  $form = &$variables['form'];
  $shop_cart = $form['#shop_cart'];
  $shop_cart_block_view = $form['#shop_cart_block_view'];
  if ($html['caption'] = drupal_render($form['caption'])) {
     $html['caption'] = '<div class="shop-cart-container">' . $html['caption'] . '</div>';
  }
 
  $rows = array();
  foreach (element_children($form['items']) as $idx) {
    if ($shop_cart_block_view !== 'editable') {
      $form['items'][$idx]['qty']['#attributes']['class'][] = 'element-hidden';
      $form['items'][$idx]['qty'] += array('#field_suffix' => '', );
      $form['items'][$idx]['qty']['#field_suffix'] .= $form['items'][$idx]['qty']['#default_value'];
    }

    $rows[$idx] = array(
      'id' => 'cart-item-' . str_replace(':', '-', $idx),
      'class' => array('cart-item',),
      'data' => array(
        '_title' => array(
          'data' => drupal_render($form['items'][$idx]['title']),
          'class' => array('col-title'),
        ),
        '_qty' => array(
          'data' => drupal_render($form['items'][$idx]['qty']),
          'class' => array('col-qty',),
        ),
        '_x' => array(
          'data' => 'x',
          'class' => array('col-x',),
        ),
        '_price' => array(
          'data' => theme('price', array('price' => $form['items'][$idx]['price']['#price'])),
          'class' => array('col-price',),
        ),
        '_delete' => array(
          'data' => drupal_render($form['items'][$idx]['delete']),
          'class' => array('col-delete',),
        ),
      ),
    );
    if ($shop_cart_block_view !== 'editable') {
      unset($rows[$idx]['data']['_delete']);
      if ($shop_cart_block_view !== 'detailed') {
        unset($rows[$idx]['data']['_x']);
        unset($rows[$idx]['data']['_price']);
      }
    }
  }
  if ($rows) {
    $rows['_summary'] = array(
      'data' => array(
        array(
          'colspan' => $shop_cart_block_view === 'simple' ? 1 : 3,
          'data' => '<strong>' . t('Total') . ': </strong>',
        ),
        array(
          'colspan' => $shop_cart_block_view === 'editable' ? 2 : 1,
          'data' => '<strong>' . $form['#total_sum'] . '</strong>',
        ),
      ),
    );
  }

  // items list:
  $html['list'] = '<div class="shop-cart-container' . ($shop_cart_block_view === 'none' || $shop_cart_block_view === 'summary' ? ' element-hidden' : '') . '">' . theme('table', array(
     'rows' => $rows,
     'header' => array(),
     'empty' => t('Cart is empty'),
     'attributes' => array('class' => array('shop-cart-items-list')),
  )) . '</div>';
  
  // buttons and etc
  
  if ($form['items'] && module_exists('order') && !variable_get('shop_cart_block_checkout_caption')) {
    $html['others']= '<div class="shop-cart-container element-hidden">' . drupal_render_children($form) . '</div>';
  }
  else {
    $html['others']= '<div class="shop-cart-container">' . drupal_render_children($form) . '</div>';
  }
  
  return implode('', $html);
}


/**
* Shopping cart form displayed in block @see _shop_block_view()
*/ 
function _shop_cart_inblock_form($form, &$form_state) {
  $shop_cart = shop_cart();
  form_load_include($form_state, 'inc', 'shop', 'shop.cart.block');
  
  $form += array(
    '#theme' => 'shop_cart_inblock_form',
    '#shop_cart' => $shop_cart,
    '#shop_cart_block_view' => variable_get('shop_cart_block_view', 'detailed'),
    '#shop_cart_icon' => variable_get('shop_cart_icon', FALSE),
    '#shop_cart_caption' => trim(variable_get('shop_cart_caption', NULL)),
    '#shop_cart_horizontal' => variable_get('shop_cart_horizontal', TRUE),
  );
  
  /* 
  $shop_cart_block_view variants:
    'none' => t('Without content'), // display only total qty in <sup> tag over cart icon|caption
    'summary' => t('Total sum'), // display total qty in <sup> tag over cart icon|caption and total sum
    'simple' => t('List without prices'), // list of goods with qtys without prices but with total sum in bottom
    'detailed' => t('Detailed list'), //  list of goods with qtys, prices and total sum in bottom
    'editable' => t('Editable list'), // list of goods with editable qtys, prices, delete buttons and total sum in bottom
  */

  $form['#attributes']['class'][] = 'shop-cart-wrapper';
  $form['#attributes']['class'][] = variable_get('shop_cart_horizontal', TRUE) ? 'cart-horizontal-mode' : 'cart-vertical-mode';

  $form['#attached']['js'][] = drupal_get_path('module', 'shop') . '/js/shop_cart.block.js';
  $form['#attached']['js'][] = array(
    'data' => array(
      'shop_cart' => array(
        'block_uri' => url('js/shop-cart-ajax', array('absolute' => TRUE, 'query' => array('get' => $_GET,),)),
        'payment_button_form_uri' => url('js/shop-cart-payment-button-ajax', array('absolute' => TRUE, 'query' => array('get' => $_GET,),)),
      ),
    ),
    'type' => 'setting',
  );
  
  $total_sum = shop_build_price('shop_cart', $shop_cart);
  unset($total_sum['delivery']);
  $total_sum = array_sum($total_sum);
  $form['#total_sum'] = $total_sum ? theme('price', array('price' => $total_sum)) : '';
  
  $caption = $form['#shop_cart_caption'] ? ($shop_cart->is_empty() ? check_plain($form['#shop_cart_caption']) : l($form['#shop_cart_caption'], 'shop/cart', array('html' => TRUE, 'query' => drupal_get_destination()))) : '';
  
  
  $qty = $sum = '';
  if ($form['#shop_cart_block_view'] === 'none' || $form['#shop_cart_block_view'] === 'summary') {
    if ($form['#shop_cart_icon'] || $form['#shop_cart_caption']) {
      $qty = '&nbsp;<sup>' . l(count($shop_cart->items), 'shop/cart', array('html' => TRUE, 'query' => drupal_get_destination())) . '</sup>';
      $sum = $form['#shop_cart_block_view'] === 'summary' && $form['#total_sum'] ? '&nbsp;' . $form['#total_sum']  : '';
    }
    elseif (count($shop_cart->items)) {
      if ($form['#shop_cart_block_view'] === 'summary' && $form['#total_sum']) {
        $qty = format_plural(count($shop_cart->items), '1 good on !sum', '@count goods on !sum', array('!sum' => $form['#total_sum']));
      }
      else { // 'none':
        $qty = format_plural(count($shop_cart->items), '1 good', '@count goods', array());
      }
    }
    else {
      $qty = t('Cart is empty');
    }

  }
  $icon = $form['#shop_cart_icon'] ? (l(theme('pictogram', array('icon' => 'basket')), 'shop/cart', array('html' => TRUE, 'query' => drupal_get_destination())) . ($form['#shop_cart_caption'] ? '&nbsp;' : '')) : '';
  if ($icon || $caption || $qty || $sum) {
    $form['caption'] = array(
      '#type' => 'html_tag',
      '#tag' => 'h3',
      '#attributes' => array(
        'class' => 'shop-cart-block-caption',
      ),
      '#value' => $icon . $caption . ($qty || $sum ? '<small>' . $qty  . $sum . '</small>' : ''),
    );
  }
  else {
    $form['caption'] = array();
  }

  
  
  $form['items'] = array();
  foreach ($shop_cart->items as $idx => $item) {
    $qty = $item->qty * 1;
    $form['items'][$idx] = array(

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
       // '#buttons' => TRUE,
        '#min' => 0,
        '#size' => min(drupal_strlen($qty), 4),
        '#step' => 1,
        '#float' => shop_get_info('balances', 'settings', 'qty_fraction'),
        '#attributes' => array(
          'title' => t('Qty'),
        ),
      ),
      'price' => array(
        '#price' => $item->price,
      ),
      'delete' => array(
        '#type' => 'submit',
        '#value' => 'X',
        '#name' => $item->entity_type . '-' . $item->entity_id . '-' . $item->snapshot_id . '-delete',
        '#idx' => $idx,
        '#attributes' => array(
          'title' => t('Delete'),
        ),
      ),
    );
  }
  //if ($form['items']) {
    $form['actions'] = array(
      '#type' => 'actions',
      'submit' => array(
        '#type' => 'submit',
        '#value' => t('Save'),
        '#attributes' => array(
          'class' => array('js-hide',),
        ),
      ),
    );
    if ($form['items'] && module_exists('order')) {
      $shop_cart_block_checkout_caption = trim(filter_xss_admin(variable_get('shop_cart_block_checkout_caption', t('Checkout order'))));
      $form['actions']['checkout'] = array(
        '#type' => 'link',
        '#title' => $shop_cart_block_checkout_caption,
        '#href' => 'shop/cart',
        '#access' => !!$shop_cart_block_checkout_caption,
        '#options' => array(
          'attributes' => array(
            'class' => array('form-submit', 'default-submit',),
          ),
          'html' => TRUE,
          //'query' => drupal_get_destination(),
        ),
      );
    }
  //}
  $form['items'] += array(
    '#tree' => TRUE,
  );
  $form['add'] = array(
    '#type' => 'textfield',
    '#prefix' => '<div class="element-hidden">', // Only for shop-cart.block.js
    '#suffix' => '</div>',
  );
  return $form;
}


function _shop_cart_inblock_form_submit($form, &$form_state) {
  if ($_GET['q'] === 'js/shop-cart-ajax') {
    $form_state['no_redirect'] = TRUE;
  }
  $form_state['values'] += array('items' => array());
  $can_be_float = shop_get_info('balances', 'settings', 'qty_fraction');
  if (!empty($form_state['values']['add']) && preg_match('/^([a-zA_Z0-9_\-]+)\:(\d+)\:(\d+' . ($can_be_float ? '(\.\d+)?' : '') . ')$/', $form_state['values']['add'], $m)) { // node:123:1
    $snapshot = snapshot_create('cart_item_snapshot', $m[1], $m[2]);
    $key = Shop\Cart::key($m[1], $m[2], $snapshot->sid);
    $form_state['values']['items'] += array($key => array('qty' => 0));
    $form_state['values']['items'][$key]['qty'] += $m[3];
  }

  $shop_cart = shop_cart();
  if (preg_match('/^(.+?)-(\d+)-(\d+)-delete$/', $form_state['triggering_element']['#name'], $m)) {
    $key = Shop\Cart::key($m[1], $m[2], $m[3]);
    if (!empty($shop_cart->items[$key])) {
      drupal_set_message(t('An item %title has been removed from shop cart.', array('%title' => $shop_cart->items[$key]->title)));
      $shop_cart->delete($m[1], $m[2], $m[3]);
    }
  }
  else {
    foreach ($form_state['values']['items'] as $idx => $item) {
      if (($is_added = !isset($shop_cart->items[$idx])) || $shop_cart->items[$idx]->qty != $item['qty']) {
        list($entity_type, $entity_id) = explode(':', $idx);
        $shop_cart->set($entity_type, $entity_id, NULL, $item['qty']);
//        if ($is_added) {
//         @todo fix Notice: Trying to get property of non-object в функции _shop_cart_inblock_form_submit()
//          drupal_set_message(t('An item %title has been added to shop cart.', array('%title' => $shop_cart->items[$idx]->title)));
//        }
//        else {
          drupal_set_message(t('Shopping cart has been updated'), 'status', FALSE);
//        }
      }

    }
  }
}


/**
 * Implements hook_block_view().
 */
function _shop_block_view($delta = '') {
	global $user;
	$block = array();
	if ($delta === 'cart') {
			$GLOBALS['conf']['cache'] = 0; // Force disable page caching for correct cart displaying.
			if (!preg_match('/^(user\/(\d+\/)?)?shop\/cart/', $_GET['q'])) { // Hide on *shop/cart pages

        $shop_cart = shop_cart();
        $block['subject'] = $shop_cart->is_empty() ? t('Cart') : l(t('Cart'), 'shop/cart', array('query' => drupal_get_destination()));
        $block['content']['shop_cart'] = drupal_get_form('_shop_cart_inblock_form');
        return $block;

        /*DEPRECATED:
        $out[] = theme('shop_cart', array(
          'shop_cart' => $shop_cart,
          'mode' => $shop_cart_block_view,
          'shop_cart_sum_purchases' => variable_get('shop_cart_sum_purchases', FALSE),
        ));*/

        /*
        $block['content'] = array(
          '#attached' => array(
            'js' => array(
              array(
                'data' => drupal_get_path('module', 'shop') . '/js/shop_cart.block.js',
                'type' => 'file',
              ),
              array(
                'data' => array(
                  'shop_cart_block' => array(
                    'ajax_uri' => url('js/shop_cart_add', array('absolute' => TRUE, 'query' => array('get' => $_GET,),)),
                  ),
                ),
                'type' => 'setting',
              ),
            ),
          ),
          'cart' => array(
            '#prefix' => '<div class="shop-cart-wrapper ' . (variable_get('shop_cart_horizontal', TRUE) ? 'cart-horizontal-mode' : 'cart-vertical-mode') . '">',
            '#suffix' => '</div>',
            'items' => array(
              '#markup' => implode(variable_get('shop_cart_horizontal', TRUE) ? ' ' : '<br />', $out),
            ),
          ),
        );*/

        /*
        if (!$shop_cart->is_empty() && module_exists('order')) {
          $block['content']['cart']['checkout']['#markup'] = ($shop_cart_block_view !== 'summary' ? '<div>&nbsp;</div>' : ' '). l(variable_get('shop_cart_block_checkout_caption', t('Checkout order')), 'shop/cart/order', array(
            'html' => TRUE,
            'attributes' => array(
              'class' => array('form-submit', 'default-submit',),
            ),
            'query' => drupal_get_destination(),
            ));
        }*/
      }
    }
    return $block;
}

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
        // NEW:
        'none' => t('Without content'), // display only total qty
        'summary' => t('Total sum'), // diplay total qty and total sum
        'simple' => t('List without prices'), // simple list with qtys (without prices) and total sum
        'detailed' => t('Detailed list'), // .. with total sum
        'editable' => t('Editable list'), // .. with total sum  
        /*
        OLD:
        'none' => t('No'),
        'min_digit' => t('Quantity of goods digits'),  
        'min' => t('Quantity of goods words'),
        'list_without_prices' => t('Shopping list of products without prices'),
        'default' => t('Shopping list of products and prices with sum of purchases'), // @see theme_shop_cart()
        //'max' => t('Detailed table'),*/
      ),
      '#default_value' => variable_get('shop_cart_block_view', 'detailed'),
    );
/* DEPRECATED
    $form['shop_cart_sum_purchases'] = array(
      '#type' => 'checkbox',
      '#title' => t('Sum of purchases'),
      '#default_value' => variable_get('shop_cart_sum_purchases', FALSE),
      '#states' => array(
        'invisible' => array(
          array(
            'select[name="shop_cart_block_view"]' => array('value' => 'detailed'),
          ),
        ),
      ),
    );*/

    $form['shop_cart_block_checkout_caption'] = array(
      '#type' => 'textfield',
      '#maxlength' => 30,
      '#title' => t('Checkout order button'),
      '#default_value' => variable_get('shop_cart_block_checkout_caption', t('Checkout order')),
      '#access' => module_exists('order'),
    );

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
    // DEPRECATED $form['shop_cart_block_hide_empty'] = array(
    // DEPRECATED   '#type' => 'checkbox',
    // DEPRECATED   '#title' => t('Do not display block without products'),
    // DEPRECATED   '#default_value' => variable_get('shop_cart_block_hide_empty', TRUE),
    // DEPRECATED );
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
    }/*
    if ($edit['shop_cart_sum_purchases']) {
      variable_set('shop_cart_sum_purchases', TRUE);
    } 
    else {
      variable_del('shop_cart_sum_purchases');
    }
*/
    if (module_exists('order')) {
      $edit['shop_cart_block_checkout_caption'] = trim($edit['shop_cart_block_checkout_caption']);
      if ($edit['shop_cart_block_checkout_caption'] === t('Checkout order')) {
        variable_del('shop_cart_block_checkout_caption');
      }
      else {
        variable_set('shop_cart_block_checkout_caption', $edit['shop_cart_block_checkout_caption']);
      }
    }

    if ($edit['shop_cart_horizontal']) {
      variable_del('shop_cart_horizontal');
    }
    else {
      variable_set('shop_cart_horizontal', FALSE);
    }


   // DEPRECATED if ($edit['shop_cart_block_hide_empty']) {
   // DEPRECATED   variable_del('shop_cart_block_hide_empty');
   // DEPRECATED }
   // DEPRECATED else {
   // DEPRECATED   variable_set('shop_cart_block_hide_empty', FALSE);
   // DEPRECATED }
  }
}



// AJAX routines for shop cart block and buy buttons in product nodes
// @see shop_cart.block.js

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
* Callback for /js/js/shop-cart-ajax
*/
function _shop_cart_ajax() {
  $shop_cart = shop_cart();
  $old_items = unserialize(serialize($shop_cart->items)); // Destroy sub-objects references. Without it every item in $old_items will equals to item in $new_items.
  drupal_get_form('_shop_cart_inblock_form');
  $new_items = $shop_cart->items;
  // Collect information about that was modifed/added or deleted
  // This needed for update all dependent buy buttons on page
  // @see shop_cart.block.js
  $touched = array();
  foreach ($old_items as $idx => $item) {
    if (!isset($new_items[$idx]) || ($new_items[$idx]->qty * 1 !== $item->qty * 1)) {
      $touched[$idx] = array($item->entity_type, $item->entity_id);
    }
  }
  foreach ($new_items as $idx => $item) {
    if (!isset($old_items[$idx]) || ($old_items[$idx]->qty * 1 !== $item->qty * 1)) {
      $touched[$idx] = array($item->entity_type, $item->entity_id);
    }
  }
  _shop_cart_ajax_override_request_uri();
  unset($_POST['form_id']); // avoid submission processing in drupal_get_form()
  $blocks = _block_get_renderable_array(_block_render_blocks(array(block_load('shop','cart'))));

  return array(
    '#type' => 'ajax',
    '#commands' => array(
      'block' => drupal_render($blocks['shop_cart']),
      'messages' => theme('status_messages'),
      'touched' => $touched,
    ),
  );
}


function _shop_cart_payment_button_ajax() {
  if (!empty($_POST['entity_type']) && !empty($_POST['view_mode']) && !empty($_POST['entity_id']) && ($e = entity_load($_POST['entity_type'], array($_POST['entity_id'])))) {

    _shop_cart_ajax_override_request_uri();

    unset($_POST['form_id']); // avoid submission processing in drupal_get_form()
    $form = drupal_get_form('product_buy_button_form', $_POST['entity_type'], reset($e), $_POST['view_mode'], isset($_POST['is_secondary']) ? $_POST['is_secondary'] : FALSE);

    return array(
      '#type' => 'ajax',
      '#commands' => array(
        'form' => drupal_render($form),
        'messages' => theme('status_messages'),
      ),
    );
  }

}








