<?php

/**
* Main function for gather information about existsing delivery methods and it properties.
* Returns full array or nested value if passed some params (key names after $node param)
* @param $node - context object. May be empty, or good, or organisation
* Accepts any count and of variables (key names)
* Usage example:
*  shop_delivery_info()  - get full information about delivery methods
*  Every item includes these options:
*   name  - real name of delivery method, needed for administration interfaces;
*   title - admin specified name of method, used for users. If this empty - name will used.
*   description - custom text for more information;
*   settings - system settings for method;
*   enabled - boolean indicates that method is enabled or not;
*   settings callback - name of function that returns settings form for delivery method;
*   settings summary callback - summary information about delivery method settings
*   user widget callback - name of function that returns widget sub-form for user. In this form user can adjust selected method - specify address, select self delivery variant, and other;
*   user summary callback - name of function that returns renderable array which contains information about selected delivery method and user specified options;
*   cost callback - name of function that returns price for delivery method.
*   required time callback - name of function that calculates required time for delivery method
*
* Other examples:
*    shop_delivery_info('courier', 'enabled') - returns enabled/disabled status for courier delivery method
*    shop_delivery_info('courier', 'name') - get default name for courier delivery method. This used for administrative interface.
*    shop_delivery_info('courier', 'title') - get admin defined name for courier delivery method. This used for display to users.
*    shop_delivery_info('self', 'settings', 'variants') - get all pickup variants of self-delivery
*
*/
function shop_delivery_info($node = NULL) {
  //$result = &drupal_static(__FUNCTION__);
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
            'settings summary callback' => NULL,
            'user widget callback' => NULL,
            'user summary callback' => NULL,
            'cost callback' => NULL,
            'required time callback' => NULL,
          );
          $result[$key]['title'] = !empty($result[$key]['title']) ? $result[$key]['title'] : $result[$key]['name'];
        }
      }
    }
  }
  drupal_alter(__FUNCTION__, $result, $node); // Invoke global hook_shop_delivery_info_alter($node, &$info)
  $result = array_filter($result);    
  $args = func_get_args();
  unset($args[0]);
  return drupal_array_get_nested_value($result, $args);
}


/**
* Save shop_delivery_info configuration into database.
*/
function shop_delivery_info_save($node, &$shop_delivery_info) {
  foreach (module_implements('shop_delivery_info_presave') as $m) {
    $fn = $m . '_shop_delivery_info_presave';
    $fn($node, $shop_delivery_info);
  }
  drupal_static_reset('shop_delivery_info');
  if (!$node) { // save as global settings @see 
    variable_set('shop_delivery_info', $shop_delivery_info);
  }
  foreach (module_implements('shop_delivery_info_save') as $m) {
    $fn = $m . '_shop_delivery_info_save';
    $fn($node, $shop_delivery_info);
  }
  drupal_static_reset('shop_delivery_info');
}


/**
 * Calculates required time for selected delivery method by user choosed parameters
 */
function shop_delivery_required_time($node = NULL, $delivery_method, $delivery_params = array()) {
  if ($fn = shop_delivery_info($node, $delivery_method, 'required time callback')) {
    $time = $fn($node, $delivery_params, shop_delivery_info($node, $delivery_method, 'settings'));
    // Invoke hook_shop_delivery_required_time_alter()
    drupal_alter(__FUNCTION__, $time, $node, $delivery_method, $delivery_params);
    return $time;
  }
}


/**
 * Calculates cost of specified delivery method
 * Invokes delivery method 'cost callback' and returns price
 */
function shop_delivery_cost($node = NULL, $delivery_method, $delivery_params, $order_sum) {
  $cost = 0;
  if ($fn = shop_delivery_info($node, $delivery_method, 'cost callback')) {
    $cost = $fn($node, $delivery_params, $order_sum, shop_delivery_info($node, $delivery_method, 'settings'));
    // Invoke hook_shop_delivery_cost_alter()
    
    //drupal_alter(__FUNCTION__, $cost, $node, $delivery_method, $delivery_params, $order_sum);
    
    // We calls drupal_alter() not directly here because drupal_alter() accepts only 4 variables maximum.
    foreach (module_implements($hook = __FUNCTION__ . '_alter') as $m) {
      $fn = $m . '_' . $hook;
      $c = $cost;
      $fn($cost, $node, $delivery_method, $delivery_params, $order_sum);
    }
  }
  return $cost;
}


/**
* Returns administrative settings form for needed delivery method
*/
function shop_delivery_settings_form($node = NULL, $delivery_method) {
  $form = NULL;
  if ($fn = shop_delivery_info($node, $delivery_method, 'settings callback')) {
    $form = $fn($node, shop_delivery_info($node, $delivery_method, 'settings'));
    // Invoke hook_shop_delivery_settings_form_alter()
    drupal_alter(__FUNCTION__, $form, $node, $delivery_method);
  }

  return $form ? $form : array();
}



/**
* Returns summary informationform about administrative settings of specified delivery method
*/
function shop_delivery_settings_summary($node, $delivery_method) {
  $build = NULL;
  if ($fn = shop_delivery_info($node, $delivery_method, 'settings summary callback')) {
    $build = $fn($node, shop_delivery_info($node, $delivery_method, 'settings'));
    // Invoke hook_shop_delivery_settings_summary_alter()
    drupal_alter(__FUNCTION__, $build, $node, $delivery_method);
  }
  return $build ? $build : array();  
}


/**
* Build summary information about user-chosen delivery method and params
* Invokes delivery method 'user summary callback' and returns renderable array for given params with complete information
*/
function shop_delivery_user_summary($node, $delivery_method, $delivery_params = NULL) {
  $build = NULL;
  if ($fn = shop_delivery_info($node, $delivery_method, 'user summary callback')) {
    $build = $fn($node, $delivery_params, shop_delivery_info($node, $delivery_method, 'settings'));
    // Invoke hook_shop_delivery_user_summary_alter()
    drupal_alter(__FUNCTION__, $build, $node, $delivery_method, $delivery_params);
  }
  return $build ?: array();
}


/**
* Build form widget for user-chosen delivery method.
* Invokes delivery method 'user widget callback' and returns array of form elements
*/
function shop_delivery_user_widget($node, $delivery_method, $delivery_params = NULL, &$complete_form = NULL, &$form_state = NULL) {
  $form = array();
  if ($fn = shop_delivery_info($node, $delivery_method, 'user widget callback')) {
    $form = $fn($node, $delivery_params, shop_delivery_info($node, $delivery_method, 'settings'), $complete_form, $form_state);
    if ($form) {
      $form['#node'] = $node;
      // Invoke hook_shop_delivery_user_widget_alter()
      // We calls drupal_alter() not directly here because drupal_alter() accepts only 4 variables maximum.
      foreach (module_implements($hook = __FUNCTION__ . '_alter') as $m) {
        $fn = $m . '_' . $hook;
        $fn($form, $node, $delivery_method, $delivery_params, $complete_form, $form_state);
      }
    }

  }
  return $form;
}




/**
* Implements hook_shop_delivery_info()
* Define two own basic delivery methods - courier & self (pickup points)
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
    'settings summary callback' =>  '_shop_delivery_courier_settings_summary',
    'user widget callback' => '_shop_delivery_courier_user_widget',
    'user summary callback' => '_shop_delivery_courier_user_summary',
    'cost callback' => '_shop_delivery_courier_calc_cost',  
    'required time callback' => '_shop_delivery_courier_required_time',
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
          'description' => $self_default_variant_description ?? NULL,
        ),
      ),
    ),
    'settings callback' => '_shop_delivery_self_settings_form',
    'settings summary callback' =>  '_shop_delivery_self_settings_summary',
    'user widget callback' => '_shop_delivery_self_user_widget',
    'user summary callback' => '_shop_delivery_self_user_summary',  
    'required time callback' => '_shop_delivery_self_required_time',
  );
  return $result;
}


/**
* Implements hook_shop_delivery_info_alter()
* 
*/
function shop_shop_delivery_info_alter(&$info, $node) {
  if ($info['courier']['enabled']) {
    // Automatically disable courier delivery method if he not contains variants with name
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
    // Automatically disable self delivery method if he not contains variants with name and description
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
function _shop_delivery_courier_settings_form($node, $delivery_method_settings) {
 // m($delivery_method_settings['variants']);
  $form['variants'] = array(
    '#type' => 'multifield_wrapper',
    '#elements_to_cells' => FALSE,
    '#title' => t('Delivery region'),
    '#default_value' => !empty($delivery_method_settings['variants']) ? $delivery_method_settings['variants'] : array(),
    '#visible_type' => 'item',
    '#add_button' => t('Add region'),
    'name' => array(
      '#title' => t('Name'),
      '#type' => 'textfield',
    ),
    'm0' => array('#markup' => '<div class="container-inline">',),
    'price' => array(
      '#title' => t('Cost of delivery'),
      '#type' => 'digit',
      '#min' => 0,
    ),
    'threshold' => array(
      '#title' => t('To the order sum'),
      '#type' => 'digit',
      '#min' => 0,
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
* 'settings summary callback' for courier delivery method. @see shop_delivery_settings_summary()
*/
function _shop_delivery_courier_settings_summary($node, $delivery_method_settings) {
  $build = array();
  if (!empty($delivery_method_settings['variants'])) {
    foreach ($delivery_method_settings['variants'] as $idx => $item) {
      $build[$idx] = array(
        '#type' => 'container',
        '#inline' => TRUE,
        'name' => array(
          '#type' => 'html_tag',
          '#tag' => 'strong',
          '#value' => !empty($item['name']) ? check_plain($item['name']) : NULL,
          '#access' => !empty($item['name']),        
        ),
        'price' => array(
          '#type' => 'item',
          '#title' => t('Price'),
          '#inline' => TRUE,
          '#title_display' => 'invisible',
          'price' => array(
            '#theme' => 'price',
            '#price' => $item['price'] ?? NULL,
          ),        
        ),
        'threshold' => array(
          '#type' => 'conainer',
          '#inline' => TRUE,
          'price' => array(
            '#theme' => 'price',
            '#prefix' => drupal_strtolower(t('To the order sum')) . ' ' ,
            '#price' => $item['threshold'] ?? NULL,
          ),
          '#access' => !empty($item['threshold']),        
        ),
        'required_time' => array(
          '#type' => 'item',
          '#title' => t('Required time, hours'),
          '#markup' => $item['required_time'] ?? NULL,
          '#access' => !empty($item['required_time']),        
        ),
      );
    }    
  }

  return $build;
}

/**
* Callback for 'required time callback' property, @see hook_shop_delivery_info(), @see shop_delivery_required_time()
*/
function _shop_delivery_courier_required_time($node, $delivery_params, $delivery_method_settings) {
  if ($required_time = isset($delivery_params['variant']) ? ($delivery_method_settings['variants'][$delivery_params['variant']]['required_time'] ?? NULL) : NULL) {
    return $required_time * 3600;
  }
  
}


/**
* user widget callback for courier delivery method @see shop_shop_delivery_info()
*/
function _shop_delivery_courier_user_widget($node, $delivery_params, $delivery_method_settings, &$complete_form, &$form_state) {
  $total_sum = $complete_form['#total_sum']['total_sum'] ?? NULL;

  if ($variants = $delivery_method_settings['variants'] ?? NULL) {//shop_delivery_info('courier', 'settings', 'variants')) {
    $opts = array_filter($variants, function($item) {
      return !empty($item['name']);
    });
    if ($opts) {
      $radios_limit = 0;
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
          '#type' => 'textfield', //'textarea',
          '#inline' => TRUE,
          '#maxlength' => 255,
          '#size' => 90,
          //'#rows' => 3,
          //'#resizable' => FALSE,
          '#title' => t('Address'),
          '#required' => TRUE,
          '#default_value' => $delivery_params['address'] ?? NULL,
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
* user summary callback for courier delivery method @see shop_shop_delivery_info()
*/
function _shop_delivery_courier_user_summary($node, $delivery_params, $delivery_method_settings) {
  $build = array();
  if (isset($delivery_params['variant']) && ($n = $delivery_method_settings['variants'][$delivery_params['variant']]['name'] ?? NULL)) {//shop_delivery_info('courier', 'settings', 'variants' , $delivery_params['variant'], 'name'))) {
    $build['name'] = array(
      '#type' => 'container',
      '#children' => check_plain($n),
    );
  }
  if (!empty($delivery_params['address'])) {
    if (is_array($delivery_params['address']) && module_exists('geo2_delivery')) {
      $delivery_params['address'] = geo2_delivery_address_format($delivery_params);
    }
    $build['address'] = array(
      '#type' => 'item',
      //IKW: remove 'Address' title from delivery summary in checkout order steps.
      '#title' => node_is_page($node) ? t('Address') : NULL,
      '#inline' => TRUE,
      '#markup' => check_plain($delivery_params['address']),
    );
  }

  if (!empty($delivery_params['delivery_time'])) { // 'delivery_time' defined only order_shop_cart_form()
    $build['delivery_date'] = array(
      '#type' => 'item',
      '#title' => t('Delivery date'),
      '#inline' => TRUE,
      '#markup' => format_date($delivery_params['delivery_time'], 'custom', 'd.m.Y'),
    );
  }

  return $build;
}


/**
* Cost callback for courier delivery method @see shop_shop_delivery_info()
*/
function _shop_delivery_courier_calc_cost($node, $delivery_params, $required_sum, $delivery_method_settings) {
  //m(func_get_args());
  if (isset($delivery_params['variant'])) {// && shop_delivery_info('courier', 'settings', 'variants', $delivery_params['variant'], 'name')) {
    $price = $delivery_method_settings['variants'][$delivery_params['variant']]['price'] ?? NULL;

    if ($price === NULL || $price === 0) {
      return $price;
    }

    $threshold = $delivery_method_settings['variants'][$delivery_params['variant']]['threshold'] ?? NULL;//shop_delivery_info('courier', 'settings', 'variants', $delivery_params['variant'], 'threshold');

    if (isset($delivery_method_settings['variants'][$delivery_params['variant']]['threshold'])) {
      if ($required_sum < $threshold) {
        return $price;
      }

      return 0;
    }

    return $price;
  }
}


/**
* Settings callback for self-delivery method @see shop_shop_delivery_info()
*/
function _shop_delivery_self_settings_form($node, $settings) {
  
  $form['variants'] = array(
    '#type' => 'multifield_wrapper',
    '#title' => t('Points of self-delivery'),
    '#default_value' => (!empty($settings['variants']) ? $settings['variants'] : array()),
    '#visible_type' => 'item',
    '#add_button' => t('Add issuing point'),
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
* 'settings summary callback' for self delivery method. @see shop_delivery_settings_summary()
*/
function _shop_delivery_self_settings_summary($node, $delivery_method_settings) {
  $build = array();
  if (!empty($delivery_method_settings['variants'])) {
    foreach ($delivery_method_settings['variants'] as $idx => $item) {
      if (!empty($item['name']) || !empty($item['description']) || !empty($item['required_time']) || (module_exists('timetables') && !empty($item['timetable']))) {
        $build['#access'] = TRUE;
      }
      elseif (!isset($build['#access'])) {
        $build['#access'] = FALSE;
      }
      $build[$idx] = array(
        '#type' => 'container',
        '#inline' => TRUE,
      );
      $build[$idx]['name'] = array(
        '#type' => 'html_tag',
        '#tag' => 'strong',
        '#inline' => TRUE,
        '#value' => !empty($item['name']) ? check_plain($item['name']) : NULL,
        '#access' => !empty($item['name']),
      );
      $build[$idx]['description'] = array(
        '#type' => 'html_tag',
        '#tag' => 'div',
        '#inline' => TRUE,
        '#value' => !empty($item['description']) ? check_plain($item['description']) : NULL,
        '#access' => !empty($item['description']),
      );
      /*
      ITW: POKA VREMYA DOSTAVKI NERABOTAIT NE ZAMORACHIVAISA S CHASAMI
      $build[$idx]['required_time'] = array(
        '#type' => 'html_tag',
        '#tag' => 'div',
        '#inline' => TRUE,
        '#value' => !empty($item['required_time']) ? ' - ' . format_plural($item['required_time'], '1 hour', '@count hours') : NULL,
        '#access' => !empty($item['required_time']),
      );*/
      if (module_exists('timetables')) {
        $build[$idx]['timetable'] = array(
          '#theme' => 'week_timetable',
          '#timetable' => $item['timetable'],
          '#skip_holidays' => FALSE,
          '#aggregate_days' => TRUE,
          '#access' => !empty($item['timetable']),
        );
      }
    }    
  }

  return $build;
}

/**
* user widget callback for self-delivery method @see shop_shop_delivery_info()
*/
function _shop_delivery_self_user_widget($node, $delivery_params, $delivery_method_settings, &$complete_form, &$form_state) {
  $opts = array();
  if (!empty($delivery_method_settings['variants'])) {
    foreach ($delivery_method_settings['variants'] as $idx => $data) {
      if (!empty($data['name']) || !empty($data['description'])) {
        $opts[$idx] = !empty($data['name']) ? $data['name'] : $data['description'];
        if (!empty($data['name']) && !empty($data['description'])) {
          $opts[$idx] .= '<br /><small class="description">' . $data['description'] . '</small>';
        }
      }
    }
  }

  //if ($opts) {
    return array(
      'variant' => array(
        '#type' => 'radios',
        '#title' => '',
        '#options' => $opts,
        '#access' => !!$opts,
        '#default_value' => isset($delivery_params['variant']) ? $delivery_params['variant'] : key($opts),
      ),
    );
  //}
}

/**
* Callback for 'required time callback' property, @see hook_shop_delivery_info(), @see shop_delivery_required_time()
*/
function _shop_delivery_self_required_time($node, $delivery_params, $delivery_method_settings) {
  if ($required_time = isset($delivery_params['variant']) ? ($delivery_method_settings['variants'][$delivery_params['variant']]['required_time'] ?? NULL) : NULL) {
    return $required_time * 3600;
  }
}

/**
 * user summary callback for self-delivery method @see shop_shop_delivery_info()
 */
function _shop_delivery_self_user_summary($node, $delivery_params, $delivery_method_settings) {
 // $text = array();
  $build = array();
  if ($name = $delivery_method_settings['variants'][$delivery_params['variant']]['name'] ?? NULL) {//shop_delivery_info('self', 'settings', 'variants', $delivery_params['variant'], 'name')) {
    $build['name'] = array(
      '#type' => 'container',
      '#children' => check_plain($name),
    );
  }
  $address = NULL;

  if ($delivery_params['variant'] === 'default') {
    if ($site_contacts = contacts_load_settings('site_contacts')) {
      $address = theme('contacts_address', array('address_elements' => $site_contacts['address'], 'separator' => ', '));
    }
  }
  else {
    $address = $delivery_method_settings['variants'][$delivery_params['variant']]['description'] ?? NULL;//shop_delivery_info('self', 'settings', 'variants', $delivery_params['variant'], 'description');
  }
  if ($address) {
    $build['address'] = array(
      '#type' => 'item',
      '#title' => t('Address'),
      '#inline' => TRUE,
      '#markup' => check_plain($address),
    );
  }

  if (!empty($delivery_params['delivery_time'])) { // 'delivery_time' defined only order_shop_cart_form()
    $build['delivery_date'] = array(
      '#type' => 'item',
      '#title' => t('Ready date'),
      '#inline' => TRUE,
      '#markup' => format_date($delivery_params['delivery_time'], 'custom', 'd.m.Y'),
    );
  }
  
  return $build;
}








