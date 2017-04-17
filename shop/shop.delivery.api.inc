<?php

/**
* Main function for gather information about existsing delivery methods and it properties.
* Returns full array or nested value if passed some params (key names)
* Accepts any count and of variables (key names)
* Usage example:
*  shop_delivery_info()  - get full information about delivery methods
*  Every item includes these options:
*   name  - real name of delivery method, needed for administratyive interfaces;
*   title - admin specified name of method, used for users. If this empty - name will used.
*   description - custom text for more information;
*   settings - system settings for method;
*   enabled - boolean indicates that method is enabled or not;
*   settings callback - name of function that returns settings form for delivery method;
*   widget callback - name of function that returns widget sub-form for user. In this form user can adjust selected method - specify address, select self delivery variant, and other;
*   summary callback - name of function that returns renderable array which contains information about selected delivery method and user specified options;
*   cost callback - name of function that returns price for delivery method.
*   required_time callback - name of function that calculates required time for delivery method
*
* Other examples:
*    shop_delivery_info('courier', 'enabled') - returns enabled/disabled status for courier delivery method
*    shop_delivery_info('courier', 'name') - get default name for courier delivery method. This used for administrative interface.
*    shop_delivery_info('courier', 'title') - get admin defined name for courier delivery method. This used for display to users.
*    shop_delivery_info('self', 'settings', 'variants') - get all pickup variants of self-delivery
*
*/
function shop_delivery_info() {
  $result = &drupal_static(__FUNCTION__);
  if (!isset($result)) {
    $result = array();
    $shop_delivery_info = variable_get('shop_delivery_info', array());
    // Arrange by weight @see sortable fieldsets in shop_settings_delivery_form()
    foreach ($shop_delivery_info as $key => $data) {
      $result[$key] = array();
    }
    foreach (module_implements('shop_delivery_info') as $m) {
      $fn = $m . '_shop_delivery_info';
      if ($info = $fn()) {
        foreach ($info as $key => $data) {
          $result[$key]['module'] = $m;
          foreach (['settings', 'description', 'enabled', 'title', 'payment_allow'] as $n) {
            if (isset($shop_delivery_info[$key][$n])) {
              $result[$key][$n] = $shop_delivery_info[$key][$n];
            }
          }
          $result[$key] += $data;
          $result[$key] += array(
            'name' => $key,
            'description' => NULL,
            'settings' => array(),
            'enabled' => FALSE,
            'settings callback' => NULL,
            'widget callback' => NULL,
            'summary callback' => NULL,
            'cost callback' => NULL,
            'required_time callback' => NULL,
          );
          $result[$key]['title'] = !empty($result[$key]['title']) ? $result[$key]['title'] : $result[$key]['name'];
        }
      }
    }
    drupal_alter(__FUNCTION__, $result); // Invoke global hook_shop_delivery_info_alter(&$info)
    $result = array_filter($result);    
  }
  return drupal_array_get_nested_value($result, func_get_args());
}



/**
 * Calculates required time for selected delivery method by user choosed parameters
 */
function shop_delivery_calc_required_time($delivery_method, $delivery_params = array()) {
  if ($callback = shop_delivery_info($delivery_method, 'required_time callback')) {
    return $callback($delivery_params);
  }
}


/**
 * Calculates cost of specified delivery method
 * Invokes delivery method 'cost callback' and returns price
 */
function shop_delivery_cost($delivery_method, $delivery_params, $order_sum) {
  if ($fn = shop_delivery_info($delivery_method, 'cost callback')) {
    $cost = $fn($delivery_params, $order_sum);
    drupal_alter(__FUNCTION__, $cost, $delivery_method, $delivery_params, $order_sum);
    return $cost;
  }
}


/**
* Returns administrative settings form for needed delivery method
*/
function shop_delivery_settings_form($delivery_method) {
  $form = NULL;
  if ($fn = shop_delivery_info($delivery_method, 'settings callback')) {
    $form = $fn(shop_delivery_info($delivery_method, 'settings'));
    drupal_alter(__FUNCTION__, $form, $delivery_method);
  }
  return $form ? $form : array();
}


/**
* Build summary information about user-chosen delivery method and params
* Invokes delivery method 'summary callback' and returns renderable array for given params with complete information
*/
function shop_delivery_summary($delivery_method, $delivery_params = NULL) {
  if ($fn = shop_delivery_info($delivery_method, 'summary callback')) {
    $summary = $fn($delivery_params);
    drupal_alter(__FUNCTION__, $summary, $delivery_method, $delivery_params);
    return $summary;
  }
}


/**
* Build form widget for user-chosen delivery method.
* Invokes delivery method 'widget callback' and returns array of form elements
*/
function shop_delivery_form_widget($delivery_method, $delivery_params = NULL, &$complete_form = NULL, &$form_state = NULL) {
  $form = array();
  if ($fn = shop_delivery_info($delivery_method, 'widget callback')) {
    $form = $fn($delivery_params, $complete_form, $form_state);
    drupal_alter(__FUNCTION__, $form, $delivery_method, $delivery_params, $complete_form, $form_state);
  }
  return $form;
}




/**
* Implements hook_shop_delivery_info()
*/
function shop_shop_delivery_info() {

  $result['courier'] = array(
    'name' => t('Courier'),
    'settings' => array(
      'variants' => array(
        'default' => array(
          'name' => t('On city'),
          'price' => 0,
          'threshold' => 0,
        ),
      ),
    ),
    'settings callback' => '_shop_delivery_courier_settings_form',
    'widget callback' => '_shop_delivery_courier_widget_form',
    'summary callback' => '_shop_delivery_courier_summary',
    'cost callback' => '_shop_delivery_courier_calc_cost',  
    'required_time callback' => '_shop_delivery_courier_calc_required_time',
  );


  if (module_exists('contacts') && ($site_contacts = contacts_load_settings('site_contacts')) && $site_contacts['address']) {
    $self_default_variant_description = strip_tags(theme('contacts_address', array(
      'address_elements' => $site_contacts['address'], 
      'separator' => ', ',
      'keys' => array(
        'country',
        'city',
        'street',
        'building',
      ),
    )));
  }

  $result['self'] = array(
    'name' => t('Self-delivery'),
    'settings' => array(
      'variants' => array(
        'default' => array(
          'name' => t('From store'),
          'description' => isset($self_default_variant_description) ? $self_default_variant_description : NULL,
        ),
      ),
    ),
    'settings callback' => '_shop_delivery_self_settings_form',
    'widget callback' => '_shop_delivery_self_widget_form',
    'summary callback' => '_shop_delivery_self_summary',  
    'required_time callback' => '_shop_delivery_self_calc_required_time',
  );
  return $result;
}


/**
* Implements hook_shop_delivery_info_alter()
*/
function shop_shop_delivery_info_alter(&$info) {
  if ($info['courier']['enabled']) {
    $info['courier']['enabled'] = FALSE;
    if (!empty($info['courier']['settings']['variants'])) {
      foreach ($info['courier']['settings']['variants'] as $v) {
        if (!empty($v['name'])) {
          $info['courier']['enabled'] = TRUE;
          break;
        }
      }
    }
  }

  if ($info['self']['enabled']) {
    $info['self']['enabled'] = FALSE;
    if (!empty($info['self']['settings']['variants'])) {
      foreach ($info['self']['settings']['variants'] as $v) {
        if (!empty($v['name']) || !empty($v['description'])) {
          $info['self']['enabled'] = TRUE;
          break;
        }
      }
    }
  }

}


/**
* Settings callback for courier delivery method @see shop_shop_delivery_info()
*/
function _shop_delivery_courier_settings_form($settings) {
 // m($settings['variants']);
  $form['variants'] = array(
    '#type' => 'multifield_wrapper',
    '#elements_to_cells' => FALSE,
    '#title' => t('Variants'),
    '#default_value' => !empty($settings['variants']) ? $settings['variants'] : array(),
    '#visible_type' => 'item',
    'name' => array(
      '#title' => t('Name'),
      '#type' => 'textfield',
    ),
    'm0' => array('#markup' => '<div class="container-inline">',),
    'price' => array(
      '#title' => t('Cost of delivery'),
      '#type' => 'digit',
    ),
    'threshold' => array(
      '#title' => t('To the order sum'),
      '#type' => 'digit',
    ),
    'm1' => array('#markup' => '</div>',),
    'required_time' => array(
      '#type' => 'digit',
      '#title' => t('Required time, hours'),
      '#inline' => TRUE,
      '#step' => 1,
      '#min' => 0,
      '#max' => 24 * 356,
      '#float' => FALSE,
    ),
   /* 'description' => array(
      '#type' => 'textarea',
      '#title' => t('Description'),
      '#rows' => 3,
      '#resizable' => FALSE,
    ),*/
  );
  if (module_exists('timetables')) {
    $form['variants']['timetable'] = array(
      '#title' => t('Delivery hours'),
      '#type' => 'schedule',
    );
  }
  return $form;
}


/**
* Callback for 'required_time callback' property, @see hook_shop_delivery_info(), @see shop_delivery_calc_required_time()
*/
function _shop_delivery_courier_calc_required_time($delivery_params) {
  if ($required_time = isset($delivery_params['variant']) ? shop_delivery_info('courier', 'settings', 'variants', $delivery_params['variant'], 'required_time') : NULL) {
    return $required_time * 3600;
  }
  
}


/**
* Widget callback for courier delivery method @see shop_shop_delivery_info()
*/
function _shop_delivery_courier_widget_form($delivery_params, &$complete_form, &$form_state) {
  $total_sum = isset($complete_form['#total_sum']['total_sum']) ? $complete_form['#total_sum']['total_sum'] : NULL;

  if ($variants = shop_delivery_info('courier', 'settings', 'variants')) {
    $opts = array_filter($variants, function($item) {
      return !empty($item['name']);
    });
    if ($opts) {
      $radios_limit = 5;
      if (count($opts) > $radios_limit) {
        foreach ($opts as $idx => $data) {
          $opts[$idx] = $data['name'];
          if (!empty($data['price'])) {
            $opts[$idx] .= ' - ' . format_price($data['price']) . ' ' . format_currency(variable_get('money_default_currency', 'RUB'));
            if (!empty($data['threshold'])) {
              $opts[$idx] .= ' ' . t('to the order sum @sum', array('@sum' => format_price($data['threshold']) . ' ' . format_currency(variable_get('money_default_currency', 'RUB'))));
            }
          }
        }
      }
      else {
        foreach ($opts as $idx => $data) {
          $opts[$idx] = $data['name'];
          $descr = array();
          if (isset($data['price'])) {
            $descr[] = $data['price'] == 0 ? t('Free', array(), array('context' => 'shop')) : t('Price is !price', array('!price' => theme('price', array('price' => $data['price']))));
            if (!empty($data['threshold'])) {
              if (isset($total_sum) && $total_sum > $data['threshold']) {
                $descr = array(t('Free', array(), array('context' => 'shop')));
              }
              else {
                // $form_state['build_info']['args'][0]->total_sum
                $descr[] = t('For free delivery you can buy goods for !sum', array('!sum' => theme('price', array('price' => $data['threshold']))));
              }

            }
          }

          if (!empty($data['description'])) {
            $descr[] = $data['description'];
          }
          if ($descr) {
            $opts[$idx] .= '<br /><span class="description">' . implode(' ', $descr) . '</span>';
          }
        }
      }
      //if (module_exists('timetables')) {
      //  if (!function_exists('timetables_find_by_time')) {
      //    function timetables_find_by_time($timetable, $current_time = NULL) {
      //      $tz = variable_get('date_default_timezone', @date_default_timezone_get());
      //      $current_day_of_week = format_date(REQUEST_TIME, 'custom', 'w', $tz);
      //      $relative_now = (format_date(REQUEST_TIME, 'custom', 'H', $tz) * 3600) + (format_date(REQUEST_TIME, 'custom', 'i', $tz) * 60) + format_date(REQUEST_TIME, 'custom', 's', $tz);
      //      $timetable = array_filter(timetable_week_aggregate_intervals($variant['timetable'], $current_day_of_week));
      //    }
      //  }
      //  
      //  foreach ($opts as $idx => $t) {
      //    if (!empty($variants[$idx]['timetable'])) {
      //      if ($timetable = array_filter(timetable_week_aggregate_intervals($variants[$idx]['timetable']))) {
      //        m('TODO', $timetable);
      //      }
      //    }
      //  }
      //}
      return array(
        'variant' => array(
          '#type' => count($opts) > $radios_limit ? 'select' : 'radios',
          '#title' => '',
          '#options' => $opts,
          '#default_value' => isset($delivery_params['variant']) && isset($opts[$delivery_params['variant']]) ? $delivery_params['variant'] : key($opts),
        ),
        'address' => array(
          '#type' => 'textarea',
          '#rows' => 3,
          '#resizable' => FALSE,
          '#title' => t('Address'),
          '#required' => TRUE,
          '#default_value' => isset($delivery_params['address']) ? $delivery_params['address'] : NULL,
          '#states' => array(
            'invisible' => array(
              'input[name="shop_cart[delivery][params][courier][variant]"]' => array('empty' => TRUE,),
            ),
          ),
        ),
      );
    }    
  }

}

/**
* Summary callback for courier delivery method @see shop_shop_delivery_info()
*/
function _shop_delivery_courier_summary($delivery_params) {
  $text = array();
  if (isset($delivery_params['variant']) && $n = shop_delivery_info('courier', 'settings', 'variants' , $delivery_params['variant'], 'name')) {
    $text[] = check_plain(shop_delivery_info('courier', 'settings', 'variants' , $delivery_params['variant'], 'name'));
  }
  if (!empty($delivery_params['address'])) {
    $text[] = t('Address') . ': ' . check_plain($delivery_params['address']);
  }

  if(!empty($delivery_params['delivery_time'])) {
    $text[] = t('Delivery date') . ': ' . format_date($delivery_params['delivery_time'], 'custom', 'd.m.Y');
  }

  return implode('<br />', $text);
}


/**
* Cost callback for courier delivery method @see shop_shop_delivery_info()
*/
function _shop_delivery_courier_calc_cost($delivery_params, $required_sum) {
  
  if (isset($delivery_params['variant']) && shop_delivery_info('courier', 'settings', 'variants', $delivery_params['variant'], 'name')) {
    if ($price = shop_delivery_info('courier', 'settings', 'variants', $delivery_params['variant'], 'price')) {
      $def_curr = shop_get_info('currency', 'default');
      $threshold = shop_delivery_info('courier', 'settings', 'variants', $delivery_params['variant'], 'threshold');
      if (!$threshold || !$required_sum || $required_sum < $threshold) {
        return $price;
      }
    }
  }
}


/**
* Settings callback for self-delivery method @see shop_shop_delivery_info()
*/
function _shop_delivery_self_settings_form($settings) {
  $form['variants'] = array(
    '#type' => 'multifield_wrapper',
    '#title' => t('Points of self-delivery'),
    '#default_value' => (!empty($settings['variants']) ? $settings['variants'] : array()),
    '#visible_type' => 'item',
    '#elements_to_cells' => FALSE,
    'm0' => array('#markup' => '<div class="container-inline">'),
    'name' => array(
      '#type' => 'textfield',
      '#rows' => 3,
      '#resizable' => FALSE,
      '#title' => t('Name'),
      '#required' => TRUE,
      '#maxlength' => 500,
    ),
    'description' => array(
      '#type' => 'textfield',
      '#resizable' => FALSE,
      '#title' => t('Address'),
      '#rows' => 3,
    ),
    'required_time' => array(
      '#type' => 'digit',
      '#title' => t('Required time, hours'),
      '#inline' => TRUE,
      '#step' => 1,
      '#min' => 0,
      '#max' => 24 * 356,
      '#float' => FALSE,
    ),
    'm1' => array('#markup' => '</div>',),
  );
  if (module_exists('timetables')) {
    $form['variants']['timetable'] = array(
      '#title' => t('Opening hours'),
      '#type' => 'schedule',
    );
  }
  return $form;
}


/**
* Widget callback for self-delivery method @see shop_shop_delivery_info()
*/
function _shop_delivery_self_widget_form($delivery_params, &$complete_form, &$form_state) {
  $opts = array();
  foreach (shop_delivery_info('self', 'settings', 'variants') as $idx => $data) {
    if (!empty($data['name']) || !empty($data['description'])) {
      $opts[$idx] = !empty($data['name']) ? $data['name'] : $data['description'];
      if (!empty($data['name']) && !empty($data['description'])) {
        $opts[$idx] .= '<br /><small class="description">' . $data['description'] . '</small>';
      }
    }
  }
  if ($opts) {
    return array(
      'variant' => array(
        '#type' => 'radios',
        '#title' => '',
        '#options' => $opts,
        '#default_value' => isset($delivery_params['variant']) ? $delivery_params['variant'] : key($opts),
      ),
    );
  }
}

/**
* Callback for 'required_time callback' property, @see hook_shop_delivery_info(), @see shop_delivery_calc_required_time()
*/
function _shop_delivery_self_calc_required_time($delivery_params) {
  if ($required_time = isset($delivery_params['variant']) ? shop_delivery_info('self', 'settings', 'variants', $delivery_params['variant'], 'required_time') : NULL) {
    return $required_time * 3600;
  }
}

/**
 * Summary callback for self-delivery method @see shop_shop_delivery_info()
 */
function _shop_delivery_self_summary($delivery_params) {
  $text = array();

  $text[] = check_plain(shop_delivery_info('self', 'settings', 'variants', $delivery_params['variant'], 'name'));

  if ($delivery_params['variant'] === 'default') {
    if ($site_contacts = contacts_load_settings('site_contacts')) {
      $text[] = t('Address') . ': ' . theme('contacts_address', array('address_elements' => $site_contacts['address'], 'separator' => ', '));
    }
  }
  else {
    $text[] = t('Address') . ': ' . check_plain(shop_delivery_info('self', 'settings', 'variants', $delivery_params['variant'], 'description'));
  }

  if (!empty($delivery_params['delivery_time'])) {
    $text[] = t('Date') . ': ' . format_date($delivery_params['delivery_time'], 'custom', 'd.m.Y');
  }

  return implode('<br />', $text);
}








