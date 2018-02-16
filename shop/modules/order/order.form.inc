<?php


/**
* Shopping cart form.
* Can be used as independent form on special pages (shop/cart), and as part of parent form (e.g. order_form)
* Contains goods controls and delivery method radios.
*/
function order_shop_cart_form($form, &$form_state, $shop_cart, $order = NULL) {

  $use_ajax = TRUE; // for test
  $no_commit = $shop_cart->no_commit; // temporary suppress commit shopping cart
  $shop_cart->no_commit = TRUE;
  $form['#shop_cart'] = $shop_cart;
  if ($use_ajax) {
    // Ajax crutches
    if (!empty($form_state['values']['shop_cart']['add']) && isset($form_state['triggering_element']['#name']) && $form_state['triggering_element']['#name'] === 'shop_cart:save') {
      if (!empty($form_state['values']['shop_cart']['add'])) {
        $shop_cart->add('node', $nid = $form_state['values']['shop_cart']['add']);
        unset($form_state['input']['shop_cart']['add']);
        $snapshot_bundle = isset($order->nid) ? 'cart_item_ordered_snapshot' : 'cart_item_snapshot';
        $snapshot = snapshot_create($snapshot_bundle, 'node', $nid);
        $key = \Shop\Cart::key('node', $nid, $snapshot->sid);
        $form_state['input']['shop_cart']['items'][$key] = (array) $shop_cart->items[$key];
      }
    }
    foreach ($shop_cart->items as $idx => $item) {
      if (isset($form_state['triggering_element']['#name']) && $form_state['triggering_element']['#name'] === 'shop_cart:delete:' . $idx) {
        $shop_cart->delete($item->entity_type, $item->entity_id, $item->snapshot_id);
      }
      elseif (isset($form_state['values']['shop_cart']['items'][$idx]['qty'])) {
        $shop_cart->set($item->entity_type, $item->entity_id, $item->snapshot_id, $form_state['values']['shop_cart']['items'][$idx]['qty']);
      }
    }
  }

  $readonly = !empty($order->is_complete);
  static $form_idx = 0;
  $form_idx ++;
  $wrapper_id = 'order-shop-cart-form-wrapper-' . str_replace(':', '-', $shop_cart->currentKey()) . '-' . $form_idx . (!empty($order->nid) ? '-' . $order->nid : '');

  $form['#wrapper_id'] = $wrapper_id;

	///$form['#attached']['js'][] = array('type' => 'setting', 'data' => array('currencyFormat' => shop_currency_format(), 'currencyDefault' => shop_get_info('currency', 'default')));
	///$form['#attached']['js'][] = drupal_get_path('module', 'order') . '/js/order.shop-cart.form.js';
	$form['#attached']['css'][] = drupal_get_path('module', 'order') . '/css/order.shop-cart.form.css';
  $form = array(
    '#tree' => TRUE,
    'shop_name' => array(
      '#markup' => '<h4 class="shop-name">' . t('Seller') . ': ' . l(shop_list_shops($shop_cart->shop_id)['name'], shop_list_shops($shop_cart->shop_id)['uri']) . '</h4>',
      '#access' => FALSE,
    ),
    '#parents' => array('shop_cart',), // force set parents because this form can be used in other forms
    'items' => array(),
    '#readonly' => $readonly,
    '#theme' => 'shop_cart_form',
    '#type' => 'container', // needed where cart rendered in other form context e.g. order node form
    'shop_id' => array(
      '#type' => 'hidden',
      '#value' => $shop_cart->shop_id,
    ),
    'uid' => array(
      '#type' => 'hidden',
      '#value' => $shop_cart->uid,
    ),
    'order' => array(
      '#type' => 'hidden',
      '#value' => !empty($order->nid) ? $order->nid : NULL,
    ),
  ) + $form;

  $form += array(
    '#prefix' => '',
    '#suffix' => '',
  );
  $form['#prefix'] .= '<div id="' . $wrapper_id . '">';
  $form['#suffix'] = '</div>' . $form['#suffix'];
  $form['#attributes']['class'][] = 'shop-cart-sub-form' . ($readonly ? ' readonly' : '');

  $qtys = shop_units_options();

  foreach ($shop_cart->items as $idx => $item) {
    $entities = entity_load($item->entity_type, array($item->entity_id));
    $entity = $entities[$item->entity_id] ?? NULL;
    $original_price = NULL;

    /**
     * If order is frozen (shop cart is frozen)
     * load $original_price from snapshot
     */
    if ($shop_cart->frozen) {
      $item_snapshot = snapshot_load($item->snapshot_id);
      $original_price = $item_snapshot->data['price']['price'] ?? NULL;
    }

    $image = $entity ? product_get_image($entity, $item->entity_type) : '';
    $original_price = $original_price ?? $item->price_array['price'] ?? NULL;

//		$item_buy_allowed = !isset($entity->qty) || !shop_get_info('balances', 'settings', 'track') || $entity->goods_on_request || $entity->qty > 0;

    $form['items'][$idx] = array(
      'entity_type' => array(
        '#type' => 'hidden',
        '#value' => $item->entity_type,
      ),
      'entity_id' => array(
        '#type' => 'hidden',
        '#value' => $item->entity_id,
      ),
      'snapshot_id' => array(
        '#type' => 'hidden',
        '#value' => $item->snapshot_id,
      ),
      'currency' => array(
        '#type' => 'hidden',
        '#value' => $item->currency ? $item->currency : shop_get_info('currency',
          'default'),
      ),
      'original_price' => array(
        '#type' => 'hidden',
        '#value' => $original_price,
      ),
      'price' => array(
        '#type' => 'hidden',
        '#value' => $item->price,
      ),
      'qty_type' => array(
        '#type' => 'hidden',
        '#value' => $item->qty_type,
      ),
      'qty' => array(),
      'image' => array(
        '#markup' => $image ? l($image, $item->entity_uri,
          array('html' => TRUE)) : '&nbsp;',
      ),
      'title' => array(
        '#type' => 'value',
        '#value' => $item->title,
        '#prefix' => l($item->title, $item->entity_uri),
        '#suffix' => '',
      ),
    );
    if ($readonly) {
      $form['items'][$idx]['qty'] = array(
        '#type' => 'value',
        '#value' =>  $item->qty,
        '#suffix' => $item->qty . ' ' . ($qtys[$item->qty_type] ?? $item->qty_type),
      );
    }
    else {
      $qty = $item->qty * 1;
      if ($use_ajax && isset($form_state['values']['shop_cart']['items'][$idx]['qty'])) {
        $qty = $form_state['values']['shop_cart']['items'][$idx]['qty'];
      }
      $in_stock_qty = !isset($entity->qty) ? PHP_INT_MAX : max($entity->qty, 0);
      $max = !shop_get_info('balances', 'settings', 'track') || $entity->goods_on_request ? PHP_INT_MAX : $in_stock_qty;
      
      $form['items'][$idx]['qty'] = array(
        '#type' => 'digit',
        '#buttons' => TRUE,
        '#float' => shop_get_info('balances', 'settings', 'qty_fraction') && !empty($item->qty_fraction) && is_float($item->qty_fraction * 1),
// old: '#' . ($item_buy_allowed ? 'default_' : '') . 'value' => $qty,
        '#default_value' => $qty,
        '#step' => shop_get_info('balances', 'settings', 'qty_fraction') && !empty($item->qty_fraction) && $item->qty_fraction * 1 ? $item->qty_fraction * 1 : 1,
        '#min' => $max ? shop_get_info('balances', 'settings', 'qty_fraction') && !empty($item->qty_fraction) && $item->qty_fraction * 1 ? $item->qty_fraction * 1 : 1 : 0,
          '#max' => $max,
          '#description' => '',
          '#size' => 3,
          '#field_suffix' => $qtys[$item->qty_type] ?? $item->qty_type,
        '#suffix' => ($qty > $in_stock_qty && $entity->goods_on_request) ? t('Available on request') : '',
        '#title' => t('Quantity'),
        '#title_display' => 'invisible',
        '#ajax' => $use_ajax ? array(
          'wrapper' => $wrapper_id,
          'callback' => 'order_shop_cart_form_ajax',
          'event' => 'change',
          'trigger_as' => array('name' => 'shop_cart:save',),
          'progress' => array(),
        ) : array(),
      );
      $form['items'][$idx]['delete'] = array(
        '#type' => 'submit',
        '#value' => module_exists('pictogram') ? pictogram('cancel-1') : 'X',
        '#name' => 'shop_cart:delete:' . $idx,
        '#submit' => array('order_shop_cart_form_submit',),
        '#access' => !$readonly && (empty($order->nid) || count($shop_cart->items) > 1),
        '#ajax' => $use_ajax ? array(
          'wrapper' => $wrapper_id,
          'callback' => 'order_shop_cart_form_ajax',
          'event' => 'click',
          'progress' => array(),
        ) : array(),
        '#attributes' => array(
          'class' => array('pictogram', 'delete-button', ),
        ),
        '#limit_validation_errors' => array(array('shop_cart')),
      );
    }
  }
  if (!$readonly && user_access('administer moneys')) {
    $form['add'] = array(
      '#type' => 'entity_selector',
      '#selector_entity_type' => 'node',
      '#selector_bundles' => array_keys(array_filter(shop_get_info('entity_types', 'node'))),
      '#selector_shop_cart_shop_id' => $shop_cart->shop_id,
      '#selector_shop_cart_uid' => $shop_cart->uid,
      '#selector_shop_cart_order' => $shop_cart->order_node->nid ?? NULL,
      '#title' => t('Add new product'),
      '#ajax' => $use_ajax ? array(
        'wrapper' => $wrapper_id,
        'callback' => 'order_shop_cart_form_ajax',
        'event' => 'change',
        'trigger_as' => array('name' => 'shop_cart:save',),
        'progress' => array(),
      ) : array(),
    );
  }

  
  $order_discount = $form_state['values']['shop_cart']['order_discount']['order_discount'] ?? $order->order_discount ?? 0;
  $shop_cart->data('order_discount', $order_discount);

  $order_discount_text = $form_state['values']['shop_cart']['order_discount']['order_discount_text'] ?? $order->order_discount_text ?? NULL;
  $shop_cart->data('order_discount_text', $order_discount_text);

  // shop_bonus CRUTCH
  $debit_bonus = $form_state['values']['shop_cart']['debit_bonus'] ?? $order->debit_bonus ?? NULL;
  $shop_cart->data('debit_bonus', $debit_bonus);

  $shop_cart_delivery_method = $shop_cart->data('delivery_method');

  $shop_cart_delivery_params = (!empty($order->nid) && $shop_cart_delivery_method)
    ? array($shop_cart_delivery_method => $order->delivery_params)
    : $shop_cart->data('delivery_params');


  $delivery_options = array();
  foreach (shop_delivery_info(order_shop_delivery_get_entity($order, $shop_cart)) as $key => $info) {
    if ($info['enabled']) {
      $delivery_options[$key] = $info['title'];
    }
  }

  if (isset($form_state['values']['shop_cart']['delivery']['method'])) {
    $delivery_options_default = $form_state['values']['shop_cart']['delivery']['method'];
  }
  elseif (empty($order->nid) && $shop_cart_delivery_method && shop_delivery_info(order_shop_delivery_get_entity($order, $shop_cart), $shop_cart_delivery_method, 'enabled')) {
    $delivery_options_default = $shop_cart_delivery_method;
  }
  else {
    $delivery_options_default = isset($delivery_options[$shop_cart_delivery_method]) ? $shop_cart_delivery_method : key($delivery_options);
  }


  if (isset($form_state['values']['shop_cart']['delivery']['params'][$delivery_options_default])) {
    $delivery_params = $form_state['values']['shop_cart']['delivery']['params'][$delivery_options_default];
  }
  elseif (isset($shop_cart_delivery_params[$delivery_options_default])) {
    $delivery_params = $shop_cart_delivery_params[$delivery_options_default];
  }
  elseif ($variants = shop_delivery_info(order_shop_delivery_get_entity($order, $shop_cart), $delivery_options_default, 'settings', 'variants')) {
    $delivery_params['variant'] = key($variants);
  }
  else {
    $delivery_params = NULL;
  }

  /*
  $delivery_options_default = !empty($order->nid) ? $order->delivery_method : key($delivery_options);

  /*

  if (empty($order->nid) && $shop_cart_delivery_method && shop_delivery_info($order, $shop_cart_delivery_method, 'enabled')) {
    $delivery_options_default = $shop_cart_delivery_method;
  }

  if (isset($form_state['values']['shop_cart']['delivery']['method'])) {
    $delivery_options_default = $form_state['values']['shop_cart']['delivery']['method'];
  }

  $params = isset($form_state['values']['shop_cart']['delivery']['params'][$delivery_options_default]) ? $form_state['values']['shop_cart']['delivery']['params'][$delivery_options_default] : NULL;
  $params = !$params && isset($shop_cart_delivery_params[$delivery_options_default]) ? $shop_cart_delivery_params[$delivery_options_default] : $params;
  */

  $shop_cart->data('delivery_method', $delivery_options_default);
  $shop_cart->data('delivery_params', array($delivery_options_default => $delivery_params));

  $o = $shop_cart->order_node;
  $shop_cart->order_node = NULL; // ajax CRUTCHES! @see order_shop_build_price() -->> if ($context_object->order_node...
  $shop_cart->frozen = FALSE; // crutch
  
  $total_sum = shop_build_price('shop_cart', $shop_cart, $shop_cart->uid);
  $shop_cart->order_node = $o;

  $form['#total_sum'] = $total_sum;
  $form['total_sum'] = array(
    '#type' => 'value',
    '#value' => $total_sum['total_sum'],
  );

  $shop_cart->data('debit_bonus', 0);
  $shop_cart->data('order_discount', 0);
  $shop_cart->data('order_discount_text', '');

  if ($delivery_options) {
    $form['delivery'] = array(
      '#tree' => TRUE,
      '#type' => 'item',
      '#title' => t('Order receipt'),
      '#title_display' => 'invisible',
      'method' => array(
        '#type' => 'radios',
        '#title' => t('Delivery method'),
        '#title_display' => 'invisible',
        '#options' => $delivery_options,
        '#inline' => TRUE,
        '#required' => TRUE,
        '#default_value' => $delivery_options_default,
        '#ajax' => $use_ajax ? array(
          'wrapper' => $wrapper_id,
          'callback' => 'order_shop_cart_form_ajax',
          'progress' => array(),
          'trigger_as' => array('name' => 'shop_cart:save',),
          'path' => 'system/ajax/delivery',
        ) : array(),
        '#element_validate' => array('shop_cart_delivery_element_validate', ),
        'params' => array(),
      ),
    );

    if ($order) {
      $form['delivery']['#type'] = 'fieldset';
      unset($form['delivery']['#title_display']);
      $form['delivery']['#collapsible'] = TRUE;

      if ($_GET['q'] === 'system/ajax/delivery') {
        $form['delivery']['#collapsed'] = FALSE;
      }
      elseif ($delivery_options_default) {
        $form['delivery']['#collapsed'] = TRUE;
        if ($t = shop_delivery_info(order_shop_delivery_get_entity($order, $shop_cart), $delivery_options_default, 'title')) {
          $form['delivery']['#title'] .= ': ' . $t;
        }
      }
      else {
        $form['delivery']['#title'] .= ': ' . drupal_strtolower(t('- Select -'));
        $form['delivery']['#collapsible'] = FALSE;
      }
    }
    $max_delivery_time = 0;

    $order_created = $order->created ?? REQUEST_TIME;
    $variant_id = $delivery_params['variant'] ?? NULL;
   
    $variant = isset($variant_id) ? shop_delivery_info(order_shop_delivery_get_entity($order, $shop_cart), $delivery_options_default, 'settings', 'variants', $variant_id) : NULL;
    if ($variant && $variant_id) {
      foreach ($shop_cart->items as $idx => $item) {
        if ($e = entity_load($item->entity_type, array($item->entity_id))) {
          $entity = $e[$item->entity_id];
          $delivery_time = _product_get_delivery_time($entity, $item->qty, $delivery_options_default, $variant, $variant_id, $order_created, FALSE);
          $max_delivery_time = max($max_delivery_time, $delivery_time);
        }
      }
      $delivery_params['delivery_time'] = $max_delivery_time; // for passing to user summary callback
    }

//    $form['delivery']['delivery_time'] = array(
//      '#type' => 'item',
//      '#title' => t('Delivery date'),
//      '#markup' => format_date($max_delivery_time, 'custom', 'd.m.Y'),
//      '#inline' => TRUE,
//      '#access' => $max_delivery_time,
//    );

    foreach (shop_delivery_info(order_shop_delivery_get_entity($order, $shop_cart)) as $delivery_method => $item) {
      if ($item['enabled']) {
        $form['delivery']['descriptions'][$delivery_method] = array(
          '#type' => 'item',
          '#title' => '',
          '#markup' => '<small>' . filter_xss_admin($item['description']) . '</small>',
          '#states' => array(
            'visible' => array(
              'input[name="shop_cart[delivery][method]"]' => array('value' => $delivery_method),
            ),
          ),
        );
        if ($subform = shop_delivery_user_widget(order_shop_delivery_get_entity($order, $shop_cart), $delivery_method, $delivery_params, $form, $form_state)) {
          if ($use_ajax) {
            // Apply ajax to every element in sub form
            $f = function($e) use (&$f, $wrapper_id) {
              if (is_array($e)) {
                if (!empty($e['#type']) && !isset($e['#ajax']) && ($info = element_info($e['#type'])) && !empty($info['#input']) && !element_children($e)) {
                  $e['#ajax'] = array(
                    'wrapper' => $wrapper_id,
                    'callback' => 'order_shop_cart_form_ajax',
                    'progress' => array('type' => 'throbber', 'message' => ''),
                    'trigger_as' => array('name' => 'shop_cart:save',),
                    'event' => 'change',
                    'path' => 'system/ajax/delivery',
                  );
                }
                $e = array_map($f, $e);
              }
              return $e;
            };
            $subform = $f($subform);
          }

          $form['delivery']['params'][$delivery_method] = array(
            '#type' => 'container',
          ) + $subform;

          if ($delivery_method !== $delivery_options_default) {
            $form['delivery']['params'][$delivery_method]['#attributes']['class'][] = 'element-hidden';
          }
          if (!$use_ajax) {
            $form['delivery']['params'][$key]['#states']['visible'] = array(
              'input[name="shop_cart[delivery][method]"]' => array('value' => $delivery_method),
            );
          }
        }
      }
    }

    $form['delivery_summary'] = array(
      '#type' => 'item',
      //IKW: remove 'Delivery information' title
      '#title' => node_is_page($order) ? t('Delivery information') : NULL,
      'summary' => $delivery_options_default ? shop_delivery_user_summary($order, $delivery_options_default, $delivery_params) : NULL,
    );
    $form['delivery_summary']['#access'] = !!$form['delivery_summary']['summary'];

    $d = $total_sum['delivery'] ?? NULL;

    if ($d === 0) {
      $value = t('Free', array(), array('context' => 'order'));
    } elseif ($d === NULL) {
      $value = t('Unknown');
    } else {
      $value = theme('price', array('price' => $total_sum['delivery']));
    }

    $form['order_delivery_sum'] = array(
      '#type' => 'html_tag',
      '#tag' => 'h4',
      '#attributes' => array(
        'class' => array(
          'order-price-delivery',
        ),
      ),
      '#value' => t('Delivery') . ': ' . $value,
    );
  }

  $form['order_total_sum'] = array(
    '#type' => 'html_tag',
    '#tag' => 'h4',
    '#attributes' => array(
      'class' => array(
        'order-price-total',
      ),
    ),
    '#value' => t('In total') . ': ' . theme('price', array('price' => array_sum($total_sum))),
    '#weight' => 100,
  );

  // Ajax issues fix 
  // @see also order_shop_cart_form_submit(); organisations_shop_form_order_node_form_alter()
  $form['current_uri'] = array(
    '#type' => 'value',
    '#value' => $form_state['values']['shop_cart']['current_uri'] ?? $_GET,
  );


  $form['actions'] = array(
    '#type' => 'actions',
    'save' => array(
      '#type' => !empty($order->nid) ? 'button' : 'submit',
      '#value' => t('Save'),
      '#name' => 'shop_cart:save',
      '#limit_validation_errors' => array(array('shop_cart')),
      '#ajax' => $use_ajax ? array(
        'wrapper' => $wrapper_id,
        'callback' => 'order_shop_cart_form_ajax',
        'event' => 'click',
        'progress' => array(),
      ) : array(),
      '#submit' => array('order_shop_cart_form_submit',),
    ),
    'order_no_registration' => array(
      '#type' => 'submit',
      '#value' => t('Checkout without registration'),
      '#name' => 'shop_cart:order_no_registration',
      '#access' => !$order && !user_is_logged_in() && user_access('submit orders'),
      '#submit' => array('order_shop_cart_form_submit',),
      '#weight' => 10,
    ),
    'order' => array(
      '#type' => 'submit',
      '#value' => t('Checkout order'),
      '#name' => 'shop_cart:order',
      '#access' => !$order,
      '#submit' => array('order_shop_cart_form_submit',),
    ),
  );
  if ($order || $use_ajax) {
    $form['actions']['save']['#attributes']['class'][] = 'js-hide';
  }
  $shop_cart->no_commit = $no_commit;
  return array('shop_cart' => $form);
}


/**
* Ajax callback for shopping cart form
*/
function order_shop_cart_form_ajax($form, &$form_state) {
  $parents = array();
  foreach (array_reverse($form_state['triggering_element']['#array_parents']) as $p) {
    if ($p === 'shop_cart' || $parents) {
      $parents[] =  $p;
    }
  }
  $shop_cart_form = array('messages' => array('#theme' => 'status_messages')) + drupal_array_get_nested_value($form, array_reverse($parents));

  return $shop_cart_form;
}


/**
* Custom validation for delivery sub-forms
* Skip validation of delivery params elements in two cases -
* 1 - shop cart is not ordered; 2 - shop cart ordered but sub-forms not belongs to selected delivery method
*/
function shop_cart_delivery_element_validate($element, &$form_state) {
  $array_parents = $element['#array_parents']; // Lookup by parents because shop cart form can generated in context of other form(s)
  array_pop($array_parents);
  $array_parents[] = 'params';
  if ($delivery_params_form = drupal_array_get_nested_value($form_state['complete form'], $array_parents)) {
    $is_critical = strpos($form_state['triggering_element']['#name'], 'shop_cart:') !== 0;
    foreach ($form_state['values']['shop_cart']['delivery']['params'] as $key => $item) {
      if (!$is_critical || $key !== $form_state['values']['shop_cart']['delivery']['method']) {
        $f = function ($e) use (&$f)  {
          if (is_array($e)) {
            if (!empty($e['#input'])) {
              $e['#validated'] = TRUE;
            }
            $e = array_map($f, $e);
          }
          return $e;
        };
        $delivery_params_form[$key] = $f($delivery_params_form[$key]);
      }
    }
    drupal_array_set_nested_value($form_state['complete form'], $array_parents, $delivery_params_form);
  }

  $order_created = !empty($form_state['values']['created']) ? $form_state['values']['created'] : REQUEST_TIME;

  $shop_cart = $form_state['values']['shop_cart'];

  $method = $shop_cart['delivery']['method'] ?? NULL;
  $variant_id = $shop_cart['delivery']['params'][$method]['variant'] ?? NULL;
  $variant = isset($variant_id) ? shop_delivery_info(NULL/* < fix it*/, $method, 'settings', 'variants', $variant_id) : NULL;
  if (isset($form_state['values']['shop_cart']['delivery']['params'][$method])) {
    drupal_alter('shop_delivery_params', $method, $form_state['values']['shop_cart']['delivery']['params'][$method]);
  }
  if (isset($form_state['input']['shop_cart']['delivery']['params'][$method])) {
    drupal_alter('shop_delivery_params', $method, $form_state['input']['shop_cart']['delivery']['params'][$method]);
  }
  
  if ($method && $variant_id && $variant) {
    $max_delivery_time = 0;
    if (!empty($shop_cart['items'])) {
      foreach ($shop_cart['items'] as $idx => $item) {
        if ($e = entity_load($item['entity_type'], array($item['entity_id']))) {
          $entity = $e[$item['entity_id']];
          $delivery_time = _product_get_delivery_time($entity, $item['qty'], $method, $variant, $variant_id, $order_created, FALSE);
          $max_delivery_time = max($max_delivery_time, $delivery_time);
        }
      }
    }

    $form_state['values']['shop_cart']['delivery']['params'][$method]['delivery_time'] = $max_delivery_time;
  }
}



/**
* Submit callback for shopping cart form.
*/
function order_shop_cart_form_submit($form, &$form_state) {
  $shop_cart = shop_cart($form_state['values']['shop_cart']['shop_id'], $form_state['values']['shop_cart']['uid'], $form_state['values']['shop_cart']['order'], TRUE);
  if (isset($form_state['values']['shop_cart']['delivery']['method'])) {
    $method = $form_state['values']['shop_cart']['delivery']['method'];
    if ($shop_cart->data('delivery_method') !== $method) {
      $shop_cart->data('delivery_method', $method);
    }
    $shop_cart_delivery_params = $shop_cart->data('delivery_params');
    $shop_cart_delivery_params ? $shop_cart_delivery_params : array();
    $old_params = $shop_cart_delivery_params[$method] ?? NULL;
    $new_params = $form_state['values']['shop_cart']['delivery']['params'][$method] ?? NULL;
    
    //drupal_alter('shop_delivery_params', $method, $new_params);
    
    if (serialize($old_params) !== serialize($new_params)) {
      $shop_cart_delivery_params[$method] = $new_params;
      $shop_cart->data('delivery_params', $shop_cart_delivery_params);
    }
  }
  
  if (!empty($form_state['values']['shop_cart']['add']) && ($n = node_load($form_state['values']['shop_cart']['add']))) {
    $shop_cart->add('node', $n->nid);
    if ($shop_cart->item('node', $n->nid)) {
      drupal_set_message(t('An item %title has been added to shop cart.', array('%title' => $n->title)));
    }
  }

  if (!empty($form_state['values']['shop_cart']['items'])) {
    foreach ($form_state['values']['shop_cart']['items'] as $idx => $item) {
      if (!empty($form_state['values']['shop_cart:delete:' . $idx])) {
        if (empty($form_state['values']['shop_cart']['order']) || count($shop_cart->items) > 1) {
          if ($e = entity_load($item['entity_type'], array($item['entity_id']))) {
            drupal_set_message(t('An item %title has been removed from shop cart.', array('%title' => entity_label($item['entity_type'], reset($e)))), 'status',  FALSE);
          }
          $shop_cart->delete($item['entity_type'], $item['entity_id'], $item['snapshot_id']);
        }
        else {
          form_set_error('shop_cart[items][' . $item['entity_type'] . ':' . $idx, t('Cart is empty, You can not checkout order without goods.'));
        }

      }
      elseif (!$shop_cart->item($item['entity_type'], $item['entity_id'], $item['snapshot_id'])) {
        $shop_cart->set($item['entity_type'], $item['entity_id'], $item['snapshot_id'], $item['qty']);
        if ($shop_cart->item($item['entity_type'], $item['entity_id'], $item['snapshot_id'])) {
          if ($e = entity_load($item['entity_type'], array($item['entity_id']))) {
            drupal_set_message(t('An item %title has been added to shop cart.', array('%title' => entity_label($item['entity_type'], reset($e)))), 'status',  FALSE);
          }
        }
      }
      elseif ($shop_cart->item($item['entity_type'], $item['entity_id'], $item['snapshot_id'])->qty != $item['qty']) {
        $shop_cart->set($item['entity_type'], $item['entity_id'], $item['snapshot_id'], $item['qty']);
        drupal_set_message(t('Shopping cart has been updated'), 'status', FALSE);
      }
    }
  }

  // Ajax crutches
  if (arg(0) . '/' . arg(1) === 'system/ajax') {
    $form_state['rebuild'] = TRUE;
    $form_state['input']['shop_cart']['add'] = '';
    $form_state['input']['shop_cart']['items'] = array();
  }
  elseif (!empty($form_state['values']['shop_cart:save'])) {
    // Stay on current page when 'save' button clicked in non-ajax context
    $get = $form_state['values']['shop_cart']['current_uri'];//$_GET;
    unset($get['q']);
    $_GET['destination'] = $_GET['q'] . '?' . drupal_http_build_query($get);
  }
  elseif (!empty($form_state['values']['shop_cart:order_no_registration'])) {
    $_GET['destination'] = 'shop/cart/' . $shop_cart->shop_id . '/order';
  }
  // Redirect anonymous to login or register page when 'order' button clicked
  elseif (!user_is_logged_in()) {
    $get = $_GET;
    unset($get['q']);
    $destination = array('destination' => 'shop/cart/' . $shop_cart->shop_id . '/order' . ($get ? '?' . drupal_http_build_query($get) : ''));
    $_GET['destination'] = 'user' . (empty($_COOKIE['Drupal_visitor_mail']) ? '/register' : '') . '?' . drupal_http_build_query($destination);
  }
  elseif (!empty($form_state['values']['shop_cart:order'])) {
    $get = $_GET;
    unset($get['q']);
    $_GET['destination'] = 'shop/cart/' . $shop_cart->shop_id . '/order' . ($get ? '?' . drupal_http_build_query($get) : '');
  }
}

/**
 * Implements hook_form().
 */
function order_form($node, &$form_state) {
  global $user; // ?

  $is_new = empty($node->nid);

  $GLOBALS['conf']['cache'] = 0;


  $shop_cart = shop_cart(NULL, NULL, $node);


  if (isset($form_state['values']['customer'])) {
    $customer = !empty($form_state['values']['customer']) ? user_load($form_state['values']['customer']) : drupal_anonymous_user();
  }
  else {
    $customer = shop_get_customer($node);
  }

  $currency = shop_get_info('currency', 'default');
  $currency_info = money_currency_info($currency);

  $form = array();
  $form['#control_changes'] = FALSE; // @see features_form_alter(), addons_form_alter()

  form_load_include($form_state, 'inc', 'node', 'node.pages');// Fix AJAX problems with node_form_validate() calls

  if (arg(0) . '/' . arg(1) . '/' . arg(2) . '/' . arg(3) . '/' . arg(4) === 'shop/cart/' . $shop_cart->shop_id . '/checkout/' . drupal_hmac_base64('shop-cart-checkout', $shop_cart->currentKey() . drupal_get_private_key() . drupal_get_hash_salt())) { // @see order_drupal_goto_alter()
    $form['#attached']['js'][] = array(
      'data' => array('shop_cart_autosubmit' => TRUE),
      'type' => 'setting',
    );

    $form['#attributes']['class'][] = 'js-hide';
  }

  if ($is_new) {
    /*foreach ($shop_cart->data() as $field_name => $val) {
        if (!isset($node->{$field_name}) && !in_array($field_name, array('changed', 'created', 'uid', 'nid', 'vid', 'menu', 'shop_cart', 'log', 'date', 'form_build_id', 'form_token', 'form_id', 'path', 'op', 'validated', 'is_new', 'timestamp', 'tnid', 'translate', 'language',), TRUE)) {
            $node->{$field_name} = $val;
        }
    }*/
  }
  if (!$is_new && arg(0) . '/' . arg(1) . '/' . arg(2) === 'node/' . $node->nid . '/edit') {
    drupal_set_title(t('Edit %title', array('%title' => $node->title)), PASS_THROUGH);
  }

  /////// ORDER_CONTENT extra-field
  $form['order_content'] = array(
      '#type' => 'item',
      '#id' => 'order-form-order-content',
    ) + order_shop_cart_form(array(), $form_state, $shop_cart, $node);


  $price_summary = $form['order_content']['shop_cart']['#total_sum'];
  unset($price_summary['order_discount']);

  $order_discount = !empty($node->order_discount) ? $node->order_discount : ($shop_cart->data('order_discount') ? $shop_cart->data('order_discount') : NULL);
  $order_discount = !empty($currency_info['decimals']) && empty(shop_get_info('general', 'rounding')) ? round($order_discount) : $order_discount;

  $order_discount_text = !empty($node->order_discount_text) ? $node->order_discount_text :
    ($shop_cart->data('order_discount_text') ? $shop_cart->data('order_discount_text') : NULL);

  $wrapper_id = $form['order_content']['shop_cart']['#wrapper_id'];

  /////// ORDER_DISCOUNT extra-field
  $form['order_content']['shop_cart']['order_discount'] = array(
    '#prefix' => '<div class="order-price-total">',
    '#suffix' => '</div>',
    '#weight' => 0.007,

    '#type' => 'container',
    '#access' => user_access('administer moneys'),
    '#disabled' => !empty($node->is_paid),

    'order_discount' => array(
      '#type' => 'digit',
      '#title' => t('Order discount'),
      // VSW sum-1 http://itinity.ru/comment/5719#comment-5719
      '#description' => '0 - ' . theme('price', array('price' => array_sum($price_summary) - 1, 'currency' => $currency)),
      '#float' => !empty($currency_info['decimals']) && empty(shop_get_info('general', 'rounding')),
      '#min' => 0,
      '#max' => array_sum($price_summary) - 1,
      '#size' => 8,
      '#default_value' => $order_discount,
      '#field_suffix' => $currency_info['reduction'],
      '#ajax' => array(
        'wrapper' => $wrapper_id,
        'callback' => 'order_shop_cart_form_ajax',
        'event' => 'change',
        'trigger_as' => array('name' => 'shop_cart:save',),
        'progress' => array(),
      ),
      '#access' => array_sum(($price_summary)) > 1,
    ),
    'order_discount_text' => array(
      '#type' => 'textfield',
      '#title' => t('Order discount text'),
      '#default_value' => $order_discount_text,
      '#maxlength' => 255,
      '#ajax' => array(
        'wrapper' => $wrapper_id,
        'callback' => 'order_shop_cart_form_ajax',
        'event' => 'change',
        'trigger_as' => array('name' => 'shop_cart:save',),
        'progress' => array(),
      ),
    ),
  );

  // implement IKW: An customer field can be guest (0) or current user if not specified, or any user uid
  /////// CUSTOMER extra-field
  $form['customer'] = array(
    '#type' => 'entity_selector',
    '#access' => user_access('administer moneys'),
    '#selector_entity_type' => 'user',
    '#title' => t('Customer'),
    '#default_value' => !$is_new ? $node->customer : (isset($customer->uid) ? $customer->uid : NULL),
  );

	/// ORDER_INFORMATION extra-field
	$form['order_information'] = array(
		'#type' => 'item',
	);

  $required = addons_extra_field_get_settings('node', 'order', 'form', NULL, 'order_information', 'required');
  $required = $required ?? drupal_map_assoc(array('phone'));

  $enabled = addons_extra_field_get_settings('node', 'order', 'form', NULL, 'order_information', 'enabled');
  $enabled = $enabled ?? drupal_map_assoc(array('lfm', 'phone', 'mail'));

	$form['order_information']['recipient_fields'] = array(
		'#tree' => TRUE,
		'#type' => 'item',
		'#id' => 'recipient-fields-wrapper',
		'#title' => '',
		'lfm' => array(
			'#type' => 'textfield',
      '#title' => t('Recipient'),
      '#access' => !empty($enabled['lfm']),
      '#required' => !empty($required['lfm']),
			'#description' => t('Type first name and last name or organisation name'),
      '#attributes' => array('class' => array('form-text-lfm'))
		),
		'mail' => array(
			'#type' => 'email',
			'#title' => t('E-mail'),
      '#access' => !empty($enabled['mail']),
      '#required' => !empty($required['mail']),
		),
    'phone' => array(
      '#type' => 'phone',
      '#title' => t('Phone'),
      '#access' => !empty($enabled['phone']),
      '#required' => !empty($required['phone']),
    ),
	);

	// IKW: При создании заказа пользователем с Ролями: Админ, Агент, Менеджер не заполнять поле получатель по умолчанию
  if (empty($node->nid) && !user_access('administer moneys')) {
    foreach (array('lfm', 'phone', 'mail') as $field) {
      if (!empty($node->recipient_fields[$field])) {
        $form['order_information']['recipient_fields'][$field]['#default_value'] = $node->recipient_fields[$field];
      }
      elseif (!empty($customer->{$field})) {
        $form['order_information']['recipient_fields'][$field]['#default_value'] = $customer->{$field};
      }
      elseif (!empty($_COOKIE['Drupal_visitor_' . $field])) {
        $form['order_information']['recipient_fields'][$field]['#default_value'] = $_COOKIE['Drupal_visitor_' . $field];
      }
    }
  }
  else {
    foreach (array('lfm', 'phone', 'mail') as $field) {
      if (!empty($node->recipient_fields[$field])) {
        $form['order_information']['recipient_fields'][$field]['#default_value'] = $node->recipient_fields[$field];
      }
    }
  }

  $form['customer']['#ajax'] = array(
    'wrapper' => $form['order_information']['recipient_fields']['#id'],
    'callback' => '_order_form_customer_ajax',
    'event' => 'change',
  );

	$form['#after_build'][] = '_order_order_node_form_after_build';

	return $form;
}


/**
* After build order node form. @see order_form()
* Hide unnecessary form elements.
*/
function _order_order_node_form_after_build($form, &$form_state) {
	$form['path']['#access'] = $form['menu']['#access'] = FALSE;
	if (empty($form['nid']['#value'])) {
		$form['#pre_render'][] = '_order_node_form_pre_render';
	}
	return $form;
}


function _order_node_form_pre_render($form) {
	$form['additional_settings'] += array(
		'#prefix' => '',
		'#suffix' => '',
	);
	$form['additional_settings']['#prefix'] = '<div class="element-hidden">' . $form['additional_settings']['#prefix'];
	$form['additional_settings']['#suffix'] .= '</div>';
	$current = '';
	foreach (element_children($form, TRUE) as $key) {
		if (in_array($key, array('order_content', 'order_information',), TRUE)) {
			$current = $key;
		}
		elseif ($current) {
			$form[$current][$key] = $form[$key];
			unset($form[$key]);
		}
	}
	return $form;
}


/**
 * AJAX callback for customer field. @see order_form()
 */
function _order_form_customer_ajax($form, &$form_state) {
  // node.pages.inc.
  $uid = $form_state['values']['customer'];
  $customer = $uid && ctype_digit($uid . '') ? user_load($uid) : NULL;

  $recipient_fields = &$form['order_information']['recipient_fields'];
  $recipient_fields['lfm']['#value'] = $customer ? format_username($customer) : '';
  $recipient_fields['phone']['#value'] = !empty($customer->phone) ? $customer->phone : '';
  $recipient_fields['mail']['#value'] = $customer ? $customer->mail : '';

  return $form['order_information']['recipient_fields'];
}


/**
* Implements hook_form_alter().
*/
function order_form_alter(&$form, &$form_state, $form_id) {
  if ($form_id == 'order_node_form') {
    if (!isset($form['actions']['delete']['#access']) || $form['actions']['delete']['#access']) {
      $form['actions']['delete']['#access'] = user_access('administer moneys');
    }
    $form['language'] = array(
      '#type' => 'value',
      '#value' => LANGUAGE_NONE,
    );
    if (empty($form['#node']->nid)) {
      $form['actions']['submit']['#value'] = t('Confirm order');
      //@see personal_data_agreement module
      $form['#personal_data_agreement'] = array('group_name' => 'actions', 'button_name' => 'submit');
      /* IKW: $form['actions']['cancel'] = array(
        '#type' => 'link',
        '#title' => t('Cancel'),
        '#href' => 'shop/cart',
        '#attributes' => array(
          'class' => array('form-submit',),
        ),
      );*/
    }

    if (($idx = array_search('node_form_submit', $form['actions']['submit']['#submit'], TRUE)) === FALSE) {
      array_unshift($form['actions']['submit']['#submit'], '_order_node_form_submit');
    }
    else {
      $form['actions']['submit']['#submit'][$idx] = '_order_node_form_submit';
    }
  }
}


/**
* Wrapper for standard node_form_submit() overriden in order_form_alter()
*/
function _order_node_form_submit($form, &$form_state) {
  $is_new = empty($form['#node']->nid);
	$shop_cart = shop_cart(NULL, NULL, $form['#node']);
  if ($is_new) {
    // Do not order items with zero qty, but save it to new cart @see end of function
    $keep_items = $shop_cart->items;
    $shop_cart->items = array();
    foreach ($form_state['values']['shop_cart']['items'] as $key => $item) {
      if ($item['qty']) {
        unset($keep_items[$key]);
        $shop_cart->set($item['entity_type'], $item['entity_id'], $item['snapshot_id'], $item['qty']);//items[$key] = (object)$item;
      }
    }
  }
  else {
    order_shop_cart_form_submit($form, $form_state);
  }
  if ($shop_cart->is_empty()) {
    form_error($form['order_content'], t('Cart is empty, You can not checkout order without goods.'));
  }
  elseif (!$is_new || node_access('create', 'order')) {
    if (isset($form_state['values']['shop_cart']['delivery']['method'])) {
      $form_state['values']['delivery_method'] = $form_state['values']['shop_cart']['delivery']['method'];
      if (isset($form_state['values']['shop_cart']['delivery']['params'][$form_state['values']['delivery_method']])) {
        $form_state['values']['delivery_params'] = $form_state['values']['shop_cart']['delivery']['params'][$form_state['values']['delivery_method']];
      }
      else {
        $form_state['values']['delivery_params'] = NULL;
      }
    }

    if (isset($form_state['values']['shop_cart']['order_discount'])) {
      $form_state['values']['order_discount'] = $form_state['values']['shop_cart']['order_discount']['order_discount'];
      $form_state['values']['order_discount_text'] = $form_state['values']['shop_cart']['order_discount']['order_discount_text'];
    }

    $form_state['values']['customer'] = $form_state['values']['customer'] === '' ? NULL : $form_state['values']['customer'];

    node_form_submit($form, $form_state);
    
    if ($form_state['nid'] && ($node = node_load($form_state['nid']))) {
      
      // bugfix. ['vid'] is NULL here, by this reason with i18n_node module node will re-saved incorrectly
      $form_state['values']['vid'] = empty($form_state['values']['vid']) ? $node->vid : $form_state['values']['vid'];
      $form_state['vid'] = empty($form_state['vid']) ? $node->vid : $form_state['vid'];
      
      
      //Without cache clear the node field cache will be incorrect (empty).
      cache_clear_all("field:node:{$node->nid}", 'cache_field', TRUE);
      if ($is_new) {
        if (!empty($_SESSION['messages']['status'])) {
            // IKW: dont want see standard drupal message at order insertion
            // Remove message from queue:
          $standard_drupal_notify_text = t('@type %title has been created.', array('@type' => node_type_get_name('order'), '%title' => $node->title)); // @see node_form_submit() -->> drupal_set_message(t('@type %title has been created.', $t_args));
          foreach ($_SESSION['messages']['status'] as $idx => $message_text) {
            if ($message_text === $standard_drupal_notify_text) {
              unset($_SESSION['messages']['status'][$idx]);
              break;
            }
          }
        }
        drupal_set_message(t('Your order has been created and assigned the number <a href="@url">@num</a>.', array(
          '@url' => url('node/' . $node->nid),
          '@num' => $node->order_num,
        )));
        if (!user_is_logged_in() && order_grant_temporary_access($node, NULL, FALSE)) {
          drupal_set_message(t('By security reasons access to order will provided for @time. To follow order execution, You must request for temporary access. For Your convenience, please <a class="ajax-popup" href="@reg-url">register</a> if You have not bought from us, or <a class="ajax-popup" href="@login-url">log in</a> if You already registede in our shop.', array(
            '@time' => format_interval(ORDER_ANNONYMOUS_ACCESS_TIMEOUT),
            '@reg-url' => url('user/register'),
            '@login-url' => url('user'),
          )));
        }
        if (node_access('view', $node)) {
          $uri = entity_uri('node', $node);
          $form_state['redirect'] = $_GET['destination'] = $uri['path'];
        }
      }
    }
  }
  elseif (!user_is_logged_in()) {
    if (!empty($form_state['values']['recipient_fields']['mail']) && empty($_COOKIE['Drupal_visitor_mail'])) {
      user_cookie_save(array('mail' => $_COOKIE['Drupal_visitor_mail'] = $form_state['values']['recipient_fields']['mail']));
    }
    if (!user_access('submit orders')) {
      $_GET['destination'] = 'user' . (empty($_COOKIE['Drupal_visitor_mail']) ? '/register' : '') . '?destination=shop/cart/' . $shop_cart->shop_id . '/checkout/' . drupal_hmac_base64('shop-cart-checkout', $shop_cart->currentKey() . drupal_get_private_key() . drupal_get_hash_salt());
    }
  }

  if ($is_new && $keep_items) {
    $new_shop_cart = shop_cart($form['#node']->shop_id, $shop_cart->uid);
    foreach ($keep_items as $key => $item) {
      $new_shop_cart->set($item->entity_type, $item->entity_id, $item->snapshot_id, $item->qty);
    }
  }
}

function order_shop_delivery_get_entity($order, $shop_cart) {
  if ($order === NULL && module_exists('organisations_shop') && ($org = organisation_from_shop_id($shop_cart->shop_id, TRUE))) {
    return $org;
  }

  return $order;
}

