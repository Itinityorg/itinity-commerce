<?php


/**
 * Calculate price
 * Recommended to use for displaying prices or final prices calculations in invoices, orders, etc.
 * DO NOT use this for saving it in database!
 *
 * @param $context_type - type of object which contains price information.
 * @param $context_object - main subject which contains price or other information for calculation
 * @param $account - optional user account for which the price to be calculated.
 * @param $required_key - optional key to specify part of summary array to return.
 * Returns final price for object in array style where $required_key is not specified; otherwise function returns scalar value of specified part in summary price array or FALSE if not exists.
 */
function shop_build_price($context, $context_object, $account = NULL, $required_key = NULL) {
  if (!isset($account)) {
    $account = $GLOBALS['user'];
  }
  elseif (is_scalar($account)) {
    if (!$account || !($account = user_load($account))) {
      $account = drupal_anonymous_user();
    }
  }
  $summary = array();
  foreach (module_implements('shop_build_price') as $m) {
    $fn = $m . '_shop_build_price';
    // call hook_shop_build_price()
    if ($p = $fn($context, $context_object, $account)) {
      foreach ($p as $idx => $v) {
        $summary += array($idx => 0);
        $summary[$idx] += $v;
      }
    }
  }

  // call shop_build_price_alter()
  drupal_alter('shop_build_price', $summary, $context, $context_object, $account);

  $discounts = module_invoke_all('shop_discount', $summary, $context, $context_object, $account);

  if ($discounts) {
   // TODO:  asort($discounts, SORT_NUMERIC );
   // TODO:  $summary[key($discounts)] = reset($discounts);
    $min_discount_value = min($discounts); // найти наименьшее значение
    $min_discount_key = array_search($min_discount_value, $discounts); // и модуль, который его предоставляет
    $summary[$min_discount_key] = $min_discount_value;
  }

  if ($rounding = shop_get_info('general', 'rounding')) {
    foreach ($summary as $key => $value) {
      switch ($rounding) {
        case "ceil":
          $summary[$key] = ceil($value);
          break;
        case "floor":
          $summary[$key] = floor($value);
          break;
        case "round":
          $summary[$key] = round($value);
          break;
      }
    }
  }

  return $required_key ? (isset($summary[$required_key]) ? $summary[$required_key] : FALSE) : $summary;
}


/**
 * Implements hook_shop_build_price()
 */
function product_shop_build_price($context, $context_object, $account) {
  if ($context === 'product') {
    if (module_exists('multicurrency')) {
      return array('price' => multicurrency_convert($context_object->price, $context_object->currency, shop_get_info('currency', 'default')));
    }
    return array('price' => $context_object->price);
  }
}


/**
 * Implements hook_shop_build_price()
 */
function order_shop_build_price($context, $context_object, $account = NULL) {
  $result = array();
  if ($context === 'shop_cart_item') {
    $result['price'] = $context_object->price;
    if (module_exists('multicurrency')) {
      $result['price'] = multicurrency_convert($result['price'], $context_object->currency, shop_get_info('currency', 'default'));
    }
  }
  elseif ($context === 'shop_cart') {
    $result['total_sum'] = $context_object->total_sum;
    if ($context_object->order && ($order = node_load($context_object->order))) {
      $account = $account ? $account : shop_get_customer($context_object);
      $result = order_shop_build_price('order', $order, $account);
    }
    else {
      if ($method = $context_object->data('delivery_method')) {
        $params = $context_object->data('delivery_params');
        $params = isset($params[$method]) ? $params[$method] : NULL;

        if ($context_object->frozen) {
          $result['delivery'] = $context_object->delivery_price; // get frozen delivery cost
        }
        else {
          $result['delivery'] = shop_delivery_cost($method, $params, isset($result['total_sum']) ? $result['total_sum'] : NULL);
        }
      }
      if($order_discount = $context_object->data('order_discount')) {
        $result['order_discount'] = -$context_object->data('order_discount');
      }
    }
  }
  elseif ($context === 'order') {
    $shop_cart = shop_cart(NULL, $context_object, NULL, TRUE);
    $currency = shop_get_info('currency', 'default');
    $precision = money_currency_info($currency, 'decimals');
    $result['total_sum'] = round($shop_cart->total_sum, $precision);
    if ($context_object->frozen) {
      $result['delivery'] = $context_object->delivery_price; // get frozen delivery cost
    }
    elseif (!empty($context_object->delivery_method)) {
      $result['delivery'] = shop_delivery_cost($context_object->delivery_method, isset($context_object->delivery_params) ? $context_object->delivery_params : array(), isset($result['total_sum']) ? $result['total_sum'] : NULL);
    }
  }
  return $result;
}


/**
 * Implements hook_shop_build_price_alter()
 */
//function new_price_shop_build_price_alter(&$summary, $context, $context_object, $account) {
//  if ($context === 'product') {
//    if (isset($context_object->new_price) && $context_object->new_price * 1) {// && $context_object->new_price < $context_object->price) {
//      $new_price = $context_object->new_price;
//      if (module_exists('multicurrency')) {
//        $new_price = multicurrency_convert($new_price, $context_object->currency, shop_get_info('currency', 'default'));
//      }
//      $summary['new_price_fix'] = -($summary['price'] - $new_price);
//    }
//  }
//}


/**
 * Implements hook_shop_discount()
 */
function new_price_shop_discount($summary, $context, $context_object, $account) {
  if ($context === 'product') {
    if (isset($context_object->new_price) && $context_object->new_price * 1) {// && $context_object->new_price < $context_object->price) {
      $new_price = $context_object->new_price;
      if (module_exists('multicurrency')) {
        $new_price = multicurrency_convert($new_price, $context_object->currency, shop_get_info('currency', 'default'));
      }
      return array('new_price_fix' => -($summary['price'] - $new_price));
    }
  }
}


//function progressive_discounts_shop_build_price_alter(&$summary, $context, $context_object, $account) {
//  if (($context === 'shop_cart' || ($context === 'order' && $context_object && !$context_object->is_paid)) && ($progressive_discounts = shop_get_info('progressive_discounts'))) {
//    foreach ($progressive_discounts as $item) {
//      $_progressive_discounts[$item['price']] = $item['percent'];
//    }
//    ksort($_progressive_discounts);
//    $subtract = 0;
//    foreach ($_progressive_discounts as $cost => $p) {
//      if ($summary['total_sum'] > $cost) {
//        $subtract = ($summary['total_sum'] / 100) * $p;
//        $summary['progressive_discount'] = -$subtract;
//      }
//    }
//  }
//}


/**
 * Implements hook_shop_discount()
 */
function progressive_discounts_shop_discount($summary, $context, $context_object, $account) {
  if (($context === 'shop_cart' || ($context === 'order' && $context_object && !$context_object->is_paid)) && ($progressive_discounts = shop_get_info('progressive_discounts'))) {
    foreach ($progressive_discounts as $item) {
      $_progressive_discounts[$item['price']] = $item['percent'];
    }
    ksort($_progressive_discounts);
    $subtract = 0;
    $progressive_discount = 0;
    foreach ($_progressive_discounts as $cost => $p) {
      if ($summary['total_sum'] > $cost) {
        $subtract = ($summary['total_sum'] / 100) * $p;
        $progressive_discount = -$subtract;
      }
    }
    if (!empty($progressive_discount)) {
      return array('progressive_discount' => $progressive_discount);
    }
  }
}


/**
 * Implements hook_shop_build_price_alter()
 */
//function personal_discounts_shop_build_price_alter(&$summary, $context, $context_object, $account) {
//  if ($context == 'product' && !empty($account->personal_discount) && !empty($summary['price'])) {
//    $summary['personal_discounts'] = -($summary['price'] * $account->personal_discount / 100);
//  }
//}


/**
 * Implements hook_shop_discount()
 */
function personal_discounts_shop_discount($summary, $context, $context_object, $account) {
  if ($context == 'product' && !empty($account->personal_discount) && !empty($summary['price'])) {
    return array('personal_discounts' => -($summary['price'] * $account->personal_discount / 100));
  }
}


/**
 * Implements hook_shop_build_price_alter()
 */
function order_shop_build_price_alter(&$summary, $context, $context_object, $account) {
  if ($context == 'shop_cart' || $context == 'order') {
    $order = ($context == 'shop_cart' && !empty($context_object->order)) ? node_load($context_object->order) : $context_object;

    if (!empty($order->order_discount)) {
      $summary['order_discount'] = -$order->order_discount;
    }
  }
}