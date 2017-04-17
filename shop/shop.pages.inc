<?php

/**
* Callback for /shop/delivery-conditions-overview
*/
function shop_delivery_conditions_overview() {
  $build = array();
  foreach (shop_delivery_info() as $method => $data) {
    if ($data['enabled']) {
      $build[$method] = array(
        '#type' => 'fieldset',
        '#title' => $data['name'],
        '#description' => $data['description'],
      );
      if (!empty($data['settings']['variants'])) {
        foreach ($data['settings']['variants'] as $idx => $item) {
          $cost = shop_delivery_cost($method, array('variant' => $idx), 0);
          $build[$method][$idx] = array(
            '#type' => 'item',
            '#title' => shop_delivery_summary($method, array('variant' => $idx)) . ' - ' . ($cost ? theme('price', array('price' => $cost)) : drupal_strtolower(t('Free', array(), array('context' => 'cost')))),
          );
          if (module_exists('timetables') && !empty($item['timetable'])) {
            $build[$method][$idx]['timetable'] = array(
              '#theme' => 'week_timetable',
              '#timetable' => $item['timetable'],
              '#skip_holidays' => FALSE,
              '#aggregate_days' => TRUE,
              //'#aggregate_intervals' => TRUE,
            );
          }
        }
      }
    }
  }
  
  return $build;
}
