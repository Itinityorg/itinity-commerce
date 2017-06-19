<?php


function catalog_object_load_exposed_presets($catalog_object, $type, $account = NULL) {
  $result = array();
  if (system_list('module_enabled')['catalog']->schema_version >= 7063) { // @TODO: delete this condition after update 7063 applied to production
    if (!$account) {
      $account = $GLOBALS['user'];
    }
    $query = db_select('catalog_exposed', 'ce')->fields('ce')->condition('ce.cid', $catalog_object->cid)->condition(db_or()->condition('ce.uid', $account->uid)->condition('ce.uid', 0));
    if ($type) {
      $query->condition('ce.type', $type);
    }
    foreach ($query->execute() as $r) {
      $r->data = unserialize($r->data);
      $result[$r->eid] = $r;
    }
  }
  return $result;
}


function catalog_object_delete_exposed_presets($exposed_id = NULL, $catalog_object = NULL, $type = NULL, $account = NULL) {
  $query = db_delete('catalog_exposed');
  if ($exposed_id) {
    $query->condition('eid', $exposed_id);
  }
  else {
    if (!$account) {
      $account = $GLOBALS['user'];
    }
    $query->condition('uid', $account->uid);
  }
  if ($catalog_object) {
    $query->condition('cid', $catalog_object->cid);
  }
  if ($type) {
    $query->condition('type', $type);
  }

}


/**
* Contains routines for building order/filter elements and processing user queries
*/

/**
* Aggregate links for exposed 'order by' user control panel (or table header items).
*/
function catalog_object_get_sorting_links($catalog_object) {
  $results = &drupal_static(__FUNCTION__);
  $cid = implode(':', array($catalog_object->cid, $catalog_object->vid, $catalog_object->tid, md5(serialize($catalog_object->orders))));
  if (!isset($results[$cid])) {
    $results[$cid] = array();
    $field_info_instances = field_info_instances($catalog_object->entity_type, $catalog_object->bundle);
    $field_info_extra_fields = field_info_extra_fields($catalog_object->entity_type, $catalog_object->bundle, 'display');
    $field_extra_fields_get_display = field_extra_fields_get_display($catalog_object->entity_type, $catalog_object->bundle, 'teaser');
    $orders = catalog_sql_info($catalog_object, 'orders');
    foreach ($catalog_object->orders as $order_method => $data) {
      if (!empty($catalog_object->orders[$order_method]['exposed']) && !empty($orders[$order_method])) {
        $link = array();
        if (!empty($field_info_instances[$order_method]) || !empty($field_info_extra_fields[$order_method])) {
          if (!empty($field_info_instances[$order_method])) {
            if ($field_info_instances[$order_method]['display']['teaser']['type'] != 'hidden') {
              $link['text'] = $field_info_instances[$order_method]['label'];
            }
          }
          elseif (!empty($field_info_extra_fields[$order_method])) {
            $link['text'] = isset($field_extra_fields_get_display[$order_method]['label']) ? $field_extra_fields_get_display[$order_method]['label'] : $field_info_extra_fields[$order_method]['label'];
          }
        }
        elseif (!empty($orders[$order_method]['label'])) {
          $link['text'] = $orders[$order_method]['label'];
        }
        if ($link) {
          $query = $_GET;
          unset($query['q'], $query['catalog_commands_' . $catalog_object->cid]['orders']);
          $query['catalog_commands_' . $catalog_object->cid]['orders'][$order_method] = 'asc';
          if (!empty($catalog_object->orders[$order_method]['enabled']) && empty($catalog_object->orders[$order_method]['descending'])) {
            $query['catalog_commands_' . $catalog_object->cid]['orders'][$order_method] = 'desc';
          }

          $link += array(
            'path' => $_GET['q'],
            'options' => array(
              'html' => TRUE,
              'attributes' => array('class' => array()),
              'query' => $query,
            ),
          );
          $link['text'] = trim(check_plain($link['text']));
          if (!empty($catalog_object->orders[$order_method]['enabled'])) {
            if (preg_match('/\sDESC$/i', $orders[$order_method]['by'])) { // reverse logic
              $link['text'] .= theme('tablesort_indicator', array('style' => empty($catalog_object->orders[$order_method]['descending']) ? 'asc' : 'desc'));
            }
            else {
              $link['text'] .= theme('tablesort_indicator', array('style' => !empty($catalog_object->orders[$order_method]['descending']) ? 'asc' : 'desc'));
            }
            $link['options']['attributes']['class'][] = 'active';
          }
          $results[$cid][$order_method] = $link;
        }
      }
    }
  }
  return $results[$cid];
}


/**
 * Get the sub-query from url that assigned to specified catalog object
 * @param $catalog_object - loaded catalog object
 * @param null $mode - 'orders|f' (@TODO: WTF 'f' is 'filters'?)
 * @return array
 */
function catalog_object_get_query_url($catalog_object, $mode = NULL) {
  $result = isset($_GET['catalog_commands_' . $catalog_object->cid]) ? $_GET['catalog_commands_' . $catalog_object->cid] : array();
  return !$mode ? $result : (isset($result[$mode]) ? $result[$mode] : array());
}


/**
* Get user query via $_GET and apply it to current catalog object properties
*/
function catalog_object_apply_query_url(&$catalog_object, $query = array()) {

  if (!isset($query['orders'])) {
    $query['orders'] = catalog_object_get_query_url($catalog_object, 'orders');
  }
  // Validate user query - check for existing and allowed (exposed) by admin
  foreach ($query['orders'] as $order_method => $sort) {
    if (
      !catalog_sql_info($catalog_object, 'orders', $order_method)
      ||
      (
        (empty($catalog_object->materials_settings['mode']) || $catalog_object->materials_settings['mode'] !== 'table')
        &&
        empty($catalog_object->orders[$order_method]['exposed'])
      )
    ) {
      unset($query['orders'][$order_method]);
    }
  }
  if ($query['orders']) {
    // Disable all previous enabled ordering methods where valid user query is detected
    foreach (array_keys($catalog_object->orders) as $order_method) {
      $catalog_object->orders[$order_method]['enabled'] = FALSE;
    }
    // Enable ordering methods by user query
    foreach ($query['orders'] as $order_method => $sort) {
      $catalog_object->orders[$order_method]['enabled'] = TRUE;
      $catalog_object->orders[$order_method]['descending'] = $sort === 'desc';
      break; // Prevent from using multiple ordering methods. One query - one order applied
      // TODO: consider the use of multiple orders in future, and optimize code & url system where this idea will refused
    }
  }
  // TODO: process exposed filters
  if (!isset($query['f'])) {
    $query['f'] = catalog_object_get_query_url($catalog_object, 'f');
  } // instead filter => f!
  if ($query['f']) { // Enable filtering methods by user query
    foreach ($query['f'] as $filter_method => $filter) {
      if (isset($filter['enabled'])) {
        $catalog_object->filters[$filter_method]['enabled'] = $filter['enabled'];
      }
      else {
        $catalog_object->filters[$filter_method]['settings'] = $filter;
        $catalog_object->filters[$filter_method]['enabled'] = 1;
      }
    }
  }

}


/*
*  this filter FORM for show at top of current catalog content (used in catalog.pages.inc )
*/
function catalog_filter_exposed_form($form, &$form_state, $catalog_object = NULL, $is_block = FALSE) {
  
  $methods = array();
  $catalog_object_filters = !empty($catalog_object->filters) ? $catalog_object->filters : array();

  foreach ($catalog_object_filters as $method => $data) {
    if (!empty($catalog_object_filters[$method]['exposed'])) {
       $form['filters'][$method] = array(); // ordered
    }
  }
  foreach (catalog_sql_info($catalog_object, 'filters') as $method => $data) {
    // only for exposed it for user
    if (!empty($catalog_object_filters[$method]['exposed'])) {
      $methods[$method] = array(
        '#type' => 'fieldset',
        '#title' => $data['description'],
        '#collapsible' => !empty($catalog_object_filters[$method]['collapsed']),
        '#collapsed' => !empty($catalog_object_filters[$method]['collapsed']),
        '#attributes' => array(
          'style' => 'display: inline-block; vertical-align: top;',
        ),
        'enabled' => empty($data['settings form']) ? array(
          '#type' => 'checkbox',
          '#title' => t('Yes'),//t('Enabled'),
          '#default_value' => $catalog_object_filters ? !empty($catalog_object_filters[$method]['enabled']) : !empty($data['default']),
          '#prefix' => '<div class="container-inline">',
          '#suffix' => '</div>',
        ): array(),
        'settings' => !empty($data['settings form']) ? $data['settings form'] + array(
          '#type' => 'container',
          '#attributes' => array(
            'style' => 'display:inline-block;font-size:.8em;padding-bottom:.5em;padding-left:1em;margin-top:-.25em',
          ),
        ): array(),
      );
    }
  }
  if (!empty($methods)) {
    $form['filters'] = array(
      array('#markup' => '<div class="container-inline">'),
      '#type' => 'fieldset',
      '#tree' => TRUE,
      '#inline' => TRUE,

    );

    foreach ($catalog_object_filters as $method => $data) {
      if (!empty($catalog_object_filters[$method]['exposed']) && isset($methods[$method])) {
        $form['filters'][$method] = $methods[$method]; // ordered
      }
    }

    $form['filters'] += array('#markup' => '</div>');
    $form['cid'] = array( '#type' => 'hidden', '#value' => $catalog_object->cid, );
    $form['filters']['submit'] = array(
      '#prefix' => '<div class="clearfix"></div>',
      '#type'  => 'submit',
      '#value' => t('Apply'), // t('Filter', array(), array('context' => 'verb',)),
    );
    $form['filters']['reset'] = array(
      '#type' => 'submit',
      '#value' => t('Reset'),
      '#attributes' => array('class' => array('reset-submit')),
    );

    $exposed_filters = catalog_object_load_exposed_presets($catalog_object, 'filters');

    /*
    *   now work with user filter sets
    */
    global $user;
    $uid = isset($user->uid) ? $user->uid : 0;
    if (isset($_GET['catalog_filter_link_del']) && $uid) {
      // @todo: make normal menu item callback with access via permissions and token
      if (isset($_GET['user'])) {
        // if command to delete global filter set (user 0), check current user rights for it
        if ((empty($_GET['user']) && catalog_access('update', $catalog_object)) || ($uid . '' === $_GET['user'] . '') ) {
          _catalog_user_filters_sets_delete($catalog_object->cid, $_GET['user'], array($_GET['catalog_filter_link_del']));
        }
      }
      drupal_goto(drupal_get_path_alias());
    }

    $hrefs = array();

    $filter_set = isset($_GET['catalog_commands_' . $catalog_object->cid]['f']) ? $_GET['catalog_commands_'. $catalog_object->cid]['f'] : array();
    $all_count = _catalog_filter_exposed_set_entities_count( $catalog_object);
    $hrefs[] = empty($filter_set) ? '<strong>' . t('All') . '</strong>'.$all_count :
        l(t('All'), drupal_get_path_alias(), array ('attributes' => array('class' => 'filter-link-all'))).$all_count;


      // global & user sets links
    foreach ((($uid) ? array (0, $uid) : array(0)) as $link_uid) {
      $sets = (isset($catalog_object->filters['user'][$link_uid]['sets'])) ? $catalog_object->filters['user'][$link_uid]['sets'] : array();
      foreach ($sets as $name => $set) {
        $show_count = _catalog_filter_exposed_set_entities_count($catalog_object, $set);
        if ($filter_set && ($set == $filter_set)) {
          $del_link = '';
          if ((!$link_uid && catalog_access('update', $catalog_object)) || $link_uid) {
            $del_link = l(' x', $_GET['q'],
              array(
                'query' => array('catalog_filter_link_del' => $name, 'user' => $link_uid),
                'attributes' =>  array(
                  'class' => array('filter-remember-link-delete',),
                  'title' => t('delete'),
                ),
              )
            );
          }
          $hrefs[] = '<strong>' . $name . '</strong>' . $show_count . $del_link;
          $filter_set = NULL; // нет нужды сохран€ть, он уже есть в пресете
        }
        else {
          $hrefs[] = l($name, drupal_get_path_alias(), array(
            'query' =>  array('catalog_commands_' . $catalog_object->cid => array('f' => $set,),),
            'attributes' =>  array('class' => 'filter-remember-link'),
          )) . $show_count;
        }
      }
    }
    $hrefs[] = l(t('Filter'), drupal_get_path_alias(), array ('attributes' =>
          array('class' => array('filter-link ' . (empty($catalog_object_filters['form_expanded']) ? 'collapsed' : '')))));

    $form['#tree'] = TRUE;
    $form['catalog_filter_links'] = array(
      '#type' => 'container',
      '#weight' => PHP_INT_MIN,
      '#children' => implode(' &nbsp; ', $hrefs),
      '#attributes' => array(
        'class' => array(
          'catalog-filter-links',
          'container-inline',
        ),
      ),
    );

    //@see status_form_alter()
    $form['#attached'] = array(
      'js' => array(
        drupal_get_path('module', 'catalog') . '/catalog.filter.exposed.js',
      ),
      'css' => array(
        drupal_get_path('module', 'catalog') . '/catalog.filter.exposed.css',
      ),
    );

    // кнопку сохранить показывать юзерам, только если кастом фильтр установлен и не сохранен в сете
    if ($filter_set && $user) {
      $form['filters']['remember'] = array(
        '#prefix' => '&nbsp;&nbsp;',
        '#type' => 'checkbox',
        '#title' => t('Remember'),
      );
      $form['filters'] += array(
        'link_name' => array(
          '#type' => 'textfield',
          '#title' => t('Label'),
          '#default_value' => '',
          '#size' => 15,
          '#maxlength' => 30,
          '#states' => array(
            'visible' => array(':input[name="filters[remember]"]' => array('checked' => TRUE),),
          ),
        ),
        'link_forAll' => (catalog_access('update', $catalog_object)) ? array(
          '#prefix' => '&nbsp;',
          '#type' => 'checkbox',
          '#title' => t('For all'),
          '#states' => array(
            'visible' => array(':input[name="filters[remember]"]' => array('checked' => TRUE),),
          ),
        ) : array(),
        'save' => array(
          '#type' => 'submit',
          '#value' => t('Save'),
          '#states' => array(
            'visible' => array(':input[name="filters[remember]"]' => array('checked' => TRUE),),
          ),
        ),
      );
    }
    // тут костыльчик дл€ свертывани€ по умолчанию (нужен ибо form expand управл€етс€ в catalog.filter.exposed.js)
    if (empty($catalog_object_filters['form_expanded']) && empty($filter_set)) {
      drupal_add_css('#edit-filters {display: none;}', 'inline');
    }
  }

  return $form;
}

function catalog_filter_exposed_form_submit($form, &$form_state) {
  if ($catalog_object = catalog_object_load($form_state['values']['cid'])) {//current(entity_load('catalog_object', array($form_state['values']['cid'])))) {
    $param = array();
    // if not reset - convert to get params and redirect
    if ($form_state['values']['op'] === t('Apply')) {
      foreach ($form_state['values']['filters'] as $method => $data) {
        if (isset($data['settings']) && is_array($data['settings'])) {
          // empty settings do not send in param
          foreach ($data['settings'] as $element => $vals) {
            if (!is_array($vals)) {
              if (empty($vals)) {
                unset ($data['settings'][$element]);
              }
            }
            else {
              foreach ($vals as $key => $val) {
                if (empty($val)) {
                  unset($data['settings'][$element][$key]);
                }
              }
            }
            // TODO!!! если есть пассивый фильтр по этой выборке то надо передать дисабл если мы его отменили
            //if (empty($data['settings'][$element])) $data['settings']['enabled'] = 0;
          }
        }
        if (!isset($data['settings']) && isset($data['enabled'])) {
          $data['settings']['enabled'] = $data['enabled'];
        }
        // remove 'settings' level in get param
        if (!empty($data['settings'])) {
          $param[$method] = $data['settings'];
        }
      }
      $form_state['redirect'] = array(
        $_GET['q'],//'catalog/' . $catalog_object->cid,
        array(
          'query' =>
            array(
              'catalog_commands_' . $catalog_object->cid => array('f' => $param,),
            )
        )
      );
    }
    elseif ($form_state['values']['op'] === t('Save')) {
      // [user][%][sets]['some-one-name'] = array()); param from GET without catalog and filters wrappers
      if (isset($_GET['catalog_commands_' . $catalog_object->cid]['f'])) {
        global $user;
        $get_it = db_select('catalog_objects', 'co')
          ->fields('co', array('filters'))
          ->condition('cid', $catalog_object->cid)
          ->execute();
        $filters = unserialize($get_it->fetchAssoc()['filters']);
        $name = ($form_state['values']['filters']['link_name']) ? $form_state['values']['filters']['link_name'] : '***';
        $filters['user'][!empty($form_state['values']['filters']['link_forAll']) ? 0 : $user->uid]['sets'][$name] = $_GET['catalog_commands_' . $catalog_object->cid]['f'];
        db_update('catalog_objects')
          ->fields(array('filters' => serialize($filters)))
          ->condition('cid', $catalog_object->cid)
          ->execute();
      }
    }
    else {
      $form_state['redirect'] = array('catalog/' . $catalog_object->cid);
    }
  }

}


// get entities_count for some one set ( $set arguments) or default
function _catalog_filter_exposed_set_entities_count($catalog_object, $set = array()) {
  $c_o = clone $catalog_object; // clone for none change for $catalog_object
    // add (activate)  also passive (admin) filters
  if ($set) {
    foreach ($c_o->filters as $method => $val) {
      if (is_array($val) && !empty($val['enabled'])) {
//         $set = array_merge (is_array($val)? $val : array(), isset($set[$method]) ? $set[$method] : array());
      }
    }
  }
  $c_o->filters = array();

  catalog_object_apply_query_url($c_o, array('f' => $set));
  $count = count(_catalog_select_entities($c_o, FALSE, TRUE));
  return $count ? '<sup><small>' . $count . '</small></sup>' : '';
}


/**
* delete (names in $set_names) catalog ($cid) filter sets, created by user ($uid). if empty names - remove all
* Attention! user access logic to do, you must set before starting it / catalog_access('update', $catalog_object) etc.
*/
function _catalog_user_filters_sets_delete($cid, $uid = 0, $set_names = array()) {
  // find and clear
  $get_it = db_select('catalog_objects', 'co')->fields('co', array('filters'))->condition('cid', $cid)->execute();
  $filters = unserialize($get_it->fetchAssoc()['filters']);
  if (isset($filters['user'][$uid])) {
    if ($set_names) {
      foreach ($set_names as $name) {
        unset($filters['user'][$uid]['sets'][$name]);
      }
    }
    else {
      unset($filters['user'][$uid]);
    }
    db_update('catalog_objects')->fields(array('filters' => serialize($filters)))->condition('cid', $cid)->execute();
    return TRUE;
  }
  return FALSE;
}


/**
* hook_user_delete() - for clear all catalogs filter sets created by this user
*/
function catalog_user_delete($account) {
  catalog_object_delete_exposed_presets(NULL, NULL, NULL, $account);
}
