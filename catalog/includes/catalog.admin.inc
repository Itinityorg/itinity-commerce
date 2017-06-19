<?php


function _catalog_object_edit_form_ajax($form, $form_state) {
  return preg_match('/^block_(add_block_form|admin_configure)$/', $form_state['values']['form_id']) ? $form['settings'] : $form;
}


function catalog_object_edit_form($form, &$form_state, $catalog_object = NULL, $is_block = FALSE) {



  if (!$catalog_object) {
    $catalog_object = (object)array(
    'entity_type' => NULL,
    'bundle' => NULL,
    'cid' => NULL,
    'is_block' => $is_block,
    'status' => TRUE,
    'description' => NULL,
    'machine_name' => NULL,
    'format' => NULL,
    'title' => NULL,
    'vid' => 0,
    'tid' => 0,
    'orders' => array(),
    'filters' => array(),
    );
  }
  
  // Process ajax
  $orders = array();
  if (!empty($form_state['input']['orders'])) {
    foreach ($form_state['input']['orders'] as $key => $data) {
      if (isset($form_state['values']['orders'][$key])) {
        $orders[$key] = $form_state['values']['orders'][$key];
      }
    }
  }

  if (!empty($form_state['values']['orders'])) {
    $orders += $form_state['values']['orders'];
  }

  foreach ($orders as $key => $data) {
    $data['enabled'] = !empty($data['default']);
    $data['descending'] = !empty($data['default']) && $data['default'] === 'desc';
    unset($data['default']);
    $catalog_object->orders[$key] = $data;
  }


  
  if (!empty($form_state['values'])) {
    foreach ($form_state['values'] as $key => $data) {
      $catalog_object->$key = $data;
    }
  }

  if (!empty($form_state['values']['entity_type:bundle'])) {
    list($catalog_object->entity_type, $catalog_object->bundle) = explode(':', $form_state['values']['entity_type:bundle']);
  }
  elseif (isset($form_state['values']['entity_type:bundle'])) {
    $catalog_object->entity_type = $catalog_object->bundle = NULL;
    $catalog_object->vid = $catalog_object->tid = 0;
  }
  //m($form_state['values'], $catalog_object->entity_type, $catalog_object->bundle, $form_state['values']['entity_type:bundle']);
  

  if (!empty($catalog_object->is_block) && !empty($catalog_object->cid) && arg(0) . '/' . arg(1) === 'catalog/' . $catalog_object->cid) {
    drupal_goto('admin/structure/block/manage/catalog/' . $catalog_object->cid . '/configure', array(), 301);
  }
  
  if (!empty($catalog_object->title)) {
    drupal_set_title(t('List %name', array('%name' => $catalog_object->title)), PASS_THROUGH);
  }
  $wrapper_id = 'catalog-form-ajax-wrapper';
  $ajax = array(
    'wrapper' => $wrapper_id,
    'callback' => '_catalog_object_edit_form_ajax',
    'event' => 'change',
  );
  $form = array(
    '#prefix' => '<div id="' . $wrapper_id . '">',
    '#suffix' => '</div>',
  );


  if ($catalog_object->cid) {
    if (empty($catalog_object->is_block)) {
      field_attach_form('catalog_object', $catalog_object, $form, $form_state);
    }
    $form['#catalog_object'] = $catalog_object;
  }
  $form['cid'] = array(
    '#type' => 'hidden',
    '#value' => $catalog_object->cid,
  );

  $form['status'] = array(
    '#type' => !$catalog_object->status ? 'checkbox' : 'hidden',
    '#title' => t('Active'),
    !$catalog_object->status ? '#default_value' : '#value' => $catalog_object->status,
    '#weight' => -PHP_INT_MAX,
  );

  $form['title'] = array(
    'title' => array(
      '#title' => t('Content list title'),
      '#type' => 'textfield',
      '#required' => TRUE,
      '#default_value' => $catalog_object->title,
      '#translatable' => array(
        'object_type' => 'catalog',
        'object_id' => $catalog_object->cid,
        'field_id' => 'title',
        'module' => 'catalog',
      ),
    ),
    'machine_name' => array(
      '#type' => 'machine_name',
      '#maxlength' => 21,
      '#machine_name' => array(
        'exists' => 'catalog_object_machine_name_load',
        'source' => array('title', 'title'),
      ),
      '#default_value' => $catalog_object->machine_name,
    ),
  );

  $form['description'] = array(
    '#type' => 'text_format',
    '#title' => t('Description'),
    '#default_value' => $catalog_object->description,
    '#format' => $catalog_object->format,
    '#translatable' => array(
      'object_type' => 'catalog',
      'object_id' => $catalog_object->cid,
      'field_id' => 'description',
      'module' => 'catalog',
    ),
  );

  $form['filters']['select'] = array(
    '#type' => 'fieldset',
    '#title' => t('Content selection'),
  );


  $options = array('' => '');
  foreach (node_type_get_names() as $bundle => $name) {
    $options['node:' . $bundle] = $name . ' (' . drupal_strtolower(t('Content type')) . ')';
  }
  $options['user:user'] = t('User') . ' (' . drupal_strtolower(t('Profile')) . ')';

  $form['filters']['select']['entity_type:bundle'] = array(
    '#type' => 'select',
    '#title' => t('Output objects of type'),
    '#options' => $options,
    '#ajax' => $ajax,
    '#required' => TRUE,
    '#inline' => TRUE,
    '#default_value' => $catalog_object->entity_type . ':' . $catalog_object->bundle,
  );

  $allowed_vocabs = array();
  if ($catalog_object->entity_type && $catalog_object->bundle) {
    foreach (field_info_instances($catalog_object->entity_type, $catalog_object->bundle) as $field_name => $inst) {
      if (($info = field_info_field($field_name)) && $info['type'] === 'taxonomy_term_reference') {
        $allowed_vocabs[$info['settings']['allowed_values'][0]['vocabulary']] = $info['settings']['allowed_values'][0]['vocabulary'];
      }
    }
  }
  $form['filters']['select']['vid'] = array();

  $v_options = array();
  foreach (taxonomy_get_vocabularies() as $v) {
    if (!empty($allowed_vocabs[$v->machine_name])) {
      $v_options[$v->vid] = $v->name;
    }
  }

  $form['filters']['select']['vid'] = array(
    '#type'    => 'select',
    '#options'  => array(NULL => t('<None>'), ) + $v_options,
    '#title'  => '&nbsp;.&nbsp;.&nbsp;.&nbsp;' . t('by vocabulary'),
    '#default_value' => $v_options && isset($catalog_object->vid) && isset($v_options[$catalog_object->vid]) ? $catalog_object->vid : NULL,
    '#access' => (bool)$v_options,
    '#inline' => TRUE,
    '#ajax' => $ajax,
  );


  $t_options = array();
  $vid = !empty($form_state['values']['vid']) ? $form_state['values']['vid'] : (!empty($catalog_object->vid) ? $catalog_object->vid : NULL);
  if ($vid) {
    foreach (taxonomy_get_tree($vid) as $t) {
      $t_options[$t->tid] = str_repeat('-', $t->depth) . ' ' . $t->name;
    }
  }
  $form['filters']['select']['tid'] = array(
    '#type' => 'select',
    '#hierarchy_prefix' => '-',
    '#title' => '&nbsp;.&nbsp;.&nbsp;.&nbsp;.&nbsp;.&nbsp;.&nbsp;.&nbsp;' . t('from category'),
    '#options' => array(NULL => '<' . t('All') . '>') + $t_options,
    '#default_value' => $t_options && isset($catalog_object->tid) && isset($t_options[$catalog_object->tid]) ? $catalog_object->tid : NULL,
    '#access' => $v_options && $t_options,
    '#states' => array(
      'visible' => array(
        'select[name="vid"]' => array('filled' => TRUE),
      ),
    ),
    '#inline' => TRUE,
  );

    $form['filters']['depth'] = array(
      '#type' => 'select',
      '#inline' => TRUE,
      '#title' => t('Depth of a conclusion of objects on taxonomy'),
      '#options' => array(NULL => t('All levels')) + drupal_map_assoc(range(1, 4)),
      '#description' => t('Whether to display in a current category the objects attached to subcategories of the dictionary.'),
      '#default_value' => isset($catalog_object->depth) ? $catalog_object->depth : NULL,
      '#access' => !$is_block && $v_options,
      '#states' => array(
        'visible' => array(
          'select[name="vid"]' => array('filled' => TRUE),
        ),
      ),
    );





  $catalog_object_filters = !empty($catalog_object->filters) ? $catalog_object->filters : array();

  foreach (catalog_sql_info($catalog_object, 'filters') as $method => $data) {
    $form['filters']['filters'][$method] = array(
      '#type' => 'container',
      'enabled' => array(
        '#type' => 'checkbox',
        '#title' => $data['description'],
        '#default_value' => $catalog_object_filters ? !empty($catalog_object_filters[$method]['enabled']) : !empty($data['default']),

        '#prefix' => '<div class="container-inline" style="float:left;">',
        '#suffix' => '</div>',
      ),
      '#suffix' => '<div class="clearfix"></div>',
      '#attributes' => array(
        'style' => '',
      ),
      'exposed' => array(
        '#type' => 'hidden',
        '#value' => !empty($catalog_object_filters[$method]['exposed']),
      ),
      'collapsed' => array(
        '#type' => 'hidden',
        '#value' => !empty($catalog_object_filters[$method]['collapsed']),
      ),
      'settings' => !empty($data['settings form']) ? $data['settings form'] + array(
        '#type' => 'container',
        '#attributes' => array(
          'style' => 'display:inline-block;font-size:.8em;padding-bottom:.5em;padding-left:1em;margin-top:-.25em',
        ),
        '#states' => array(
          'visible' => array(
            'input[name="filters[' . $method . '][enabled]"]' => array('checked' => TRUE,),
          ),
        ),
      ): array(),
    );
  }
  if (!empty($form['filters']['filters'])) {
    $form['filters']['filters'] += array(
      '#type' => 'fieldset',
      '#title' => t('Additional restrictions'),
      '#tree' => TRUE,
    );
  }

  // ORDERS ELEMENTS with AJAX
  $form['filters']['orders'] = _catalog_edit_filter_orders_table($form_state, $catalog_object, $is_block);


  if ($is_block) { // Materials layout settings. If catalog object not is block see catalog_extra_field_formatter_settings_form()
    $form['block_settings'] = _catalog_block_extra_settings_form($form_state, $catalog_object);
  }


  $item = array();
  if (!empty($catalog_object->cid)) {
    if ($mlid = db_query_range('SELECT mlid FROM {menu_links} WHERE link_path = :path AND module = \'catalog\' ORDER BY mlid ASC', 0, 1,
      array(':path' => 'catalog/' . $catalog_object->cid,))->fetchField()
    ) {
      $item = menu_link_load($mlid);
    }
  }
  // Set default values.
  $item += array(
    'link_title' => '',
    'mlid' => 0,
    'plid' => 0,
    'menu_name' => variable_get('menu_parent_catalog_objects', 'navigation'),
    'weight' => 0,
    'options' => array(),
    'module' => 'catalog',
    'expanded' => 0,
    'hidden' => 0,
    'has_children' => 0,
    'customized' => 0,
  );
  // Find the depth limit for the parent select.
  if (!isset($item['parent_depth_limit'])) {
    $item['parent_depth_limit'] = _menu_parent_depth_limit($item);
  }
  $menu_subform = array('#node' => (object)(array('menu' => $item, 'type' => '')),);
  menu_form_node_form_alter($menu_subform, $form_state);
  unset($menu_subform['menu']['#weight'], $menu_subform['#node']);
  $menu_subform['menu']['link']['parent']['#options'] = menu_parent_options(menu_get_menus(), $item);
  $menu_subform['menu']['link']['parent']['#hierarchy_prefix'] = '--'; // @see _addons_process_select()
  //m($menu_subform['menu']['link']['parent']['#options']);
  foreach ($menu_subform['menu']['link']['parent']['#options'] as $idx => $data) {
    if ((empty($catalog_object->menu_name) || $idx !== $catalog_object->menu_name . ':' . $catalog_object->menu_mlid) && preg_match('/^(management|user-menu):[0-9]/', $idx)) {
      unset($menu_subform['menu']['link']['parent']['#options'][$idx]);
    }
  }
  $item['link']['generate_tree'] = !empty($catalog_object->menu_name) ? $catalog_object->menu_name . ':' . $catalog_object->menu_mlid : NULL;
  if (!empty($catalog_object->menu_name)) {
    $menu_subform['menu']['link']['parent']['#default_value'] = $catalog_object->menu_name . ':' . $catalog_object->menu_mlid;
  }
  elseif (!isset($menu_subform['menu']['link']['parent']['#options'][$menu_subform['menu']['link']['parent']['#default_value'] = $item['menu_name'] . ':' . $item['plid']])) {
    $menu_subform['menu']['link']['parent']['#default_value'] = $item['menu_name'] . ':0';
  }
  $menu_subform['menu']['link']['generate_tree'] = array(
    '#type' => 'checkbox',
    '#title' => t('Generate sub tree contains children objects'),
    '#default_value' => isset($catalog_object->menu_name) ? (bool)$catalog_object->menu_name : TRUE,
    '#states' => array(
      'visible' => array(
        'select[name="vid"]' => array('filled' => TRUE,),
      ),
    ),
  );

  $form['additional_settings'] = array(
    '#type' => 'vertical_tabs',
    '#weight' => 99,
  );

  $form['menu'] = $menu_subform['menu'];
  $form['menu']['#group'] = 'additional_settings';

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
    '#weight' => PHP_INT_MAX - 1,
  );

  if (!empty($catalog_object->cid)) {
    $form['delete'] = array(
      '#type' => 'submit',
      '#value' => t('Delete'),
      '#weight' => PHP_INT_MAX,
      '#href' => 'catalog/' . $catalog_object->cid . '/delete', // href attribute in buttons is supported by confirm_dialog module
    );
  }
  return $form;
}

function catalog_object_edit_form_validate($form, &$form_state) {
  if (isset($form_state['values']['op']) && !empty($form['delete']['#value'])) {
    if ($form_state['values']['op'] === $form['delete']['#value']) {
      // Skip validation on delete
      return;
    }
  }
  if (empty($form_state['values']['is_block'])) {
    $path_error = FALSE;
    $alias = drupal_strtolower(trim($form_state['values']['machine_name']));
    if (preg_match('/^[\/\\\]*catalog([\/\\\]|$)/', $alias) || path_is_admin($alias) || menu_get_item($alias)) {
      $path_error = t('The path %path is already reserved by system.', array('%path' => $alias));
    }
    elseif (file_exists($alias) || file_exists(DRUPAL_ROOT . '/' . $alias)) {
      $path_error = t('The path %path is already reserved by system.', array('%path' => $alias));
    }
    elseif ($path = path_load(array('alias' => $alias))) {
      if (!empty($form_state['values']['cid'])) {
        if ($path['source'] !== 'catalog/' . $form_state['values']['cid']) {
          $path_error = t('The alias is already in use in %uri.', array('%uri' => $path['source']));
        }
      }
      else {
        $path_error = t('The alias is already in use in %uri.', array('%uri' => $path['source']));
      }
    }
    if ($path_error) {
      form_set_error('machine_name', $path_error);
    }
    if (!empty($form_state['values']['cid'])) {
      entity_form_field_validate('catalog_object', $form, $form_state);
    }
  }
  _catalog_edit_form_state_fix($form, $form_state);
}

/**
* Helper function for convert/reorder form_state structure to real catalog object structure.
*/
function _catalog_edit_form_state_fix($form, &$form_state) {
  // reorder draggable orders by input
  $orders = array();
  foreach (array('input', 'values') as $set_it) {
    $form_state[$set_it]['orders'] = isset($form_state[$set_it]['orders']) ? $form_state[$set_it]['orders']: array();
  }
  foreach ($form_state['input']['orders'] as $key => $data) {
    if (isset($form_state['values']['orders'][$key])) {
      $orders[$key] = $form_state['values']['orders'][$key];
    }
  }
  // Convert form elements structure to real orders structure
  $form_state['values']['orders'] = $orders + $form_state['values']['orders'];
  foreach ($form_state['values']['orders'] as $key => $data) {
    $data['enabled'] = !empty($data['default']);
    $data['descending'] = !empty($data['default']) && $data['default'] === 'desc';
    unset($data['default']);
    $form_state['values']['orders'][$key] = $data;
  }
  if (!empty($form_state['values']['entity_type:bundle'])) {
    list($form_state['values']['entity_type'], $form_state['values']['bundle']) = explode(':', $form_state['values']['entity_type:bundle']);
  }
}


/**
* edit catalog object form submission
*/
function catalog_object_edit_form_submit($form, &$form_state) {
  $form_state['values']['format'] = isset($form_state['values']['description']['format']) ? $form_state['values']['description']['format'] : NULL;
  $form_state['values']['description'] = isset($form_state['values']['description']['value']) ? $form_state['values']['description']['value'] : NULL;
  if (!empty($form['delete']['#value']) && $form_state['values']['op'] === $form['delete']['#value']) {
    $destination = array();
    if (isset($_GET['destination'])) {
      $destination = drupal_get_destination();
      unset($_GET['destination']);
    }
    $form_state['redirect'] = array('catalog/' . $form_state['values']['cid'] . '/delete', array('query' => $destination));
    return;
  }
  form_state_values_clean($form_state);

  $catalog_object = (object)$form_state['values'];

  $catalog_object->is_block = !empty($catalog_object->is_block);
  if ($catalog_object->is_block) {
    $catalog_object->title = $form_state['values']['info'];
  }
  //!!
  
  if (!empty($catalog_object->cid) && !$catalog_object->is_block) {
    entity_form_submit_build_entity('catalog_object', $catalog_object, $form, $form_state);
  }



  if (!$catalog_object->is_block && !empty($form_state['values']['menu']['parent'])) {
    list($catalog_object->menu['menu_name'], $catalog_object->menu['plid']) = explode(':', $form_state['values']['menu']['parent']);
  }
  
  $catalog_object->menu_name = '';
  if (!$catalog_object->is_block && $catalog_object->menu['generate_tree'] && $catalog_object->menu['parent'] && $catalog_object->menu['enabled']) {
    list($catalog_object->menu_name, $catalog_object->menu_mlid) = explode(':', $catalog_object->menu['parent']) + array('', 0,);
  }
  
  if (!empty($form_state['values']['cid'])) {
    $catalog_object->original = entity_load_unchanged('catalog_object', $form_state['values']['cid']);
    if (!empty($catalog_object->original->orders)) {
      foreach ($catalog_object->original->orders as $key => $data) {
        if (isset($data['exposed'])) {
          $catalog_object->orders[$key]['exposed'] = $data['exposed'];
        }
      }
    }

    if ($catalog_object->original->filters && $catalog_object->filters) {
        // Keep original order of filters @see catalog_object_filtersorting_form()
        $filters = array();
        foreach (array_keys($catalog_object->original->filters) as $key) {
          if (!empty($catalog_object->filters[$key])) {
            $filters[$key] = $catalog_object->filters[$key];
          }
        }
        $catalog_object->filters = $filters + $catalog_object->filters;
    }
  }

  if ($catalog_object = catalog_object_save($catalog_object)) {
    $form_state['values']['cid'] = $catalog_object->cid;
    if (!$catalog_object->is_block) {
      $form_state['redirect'] = array('catalog/' . $catalog_object->cid);
    }
    return $catalog_object;
  }
}


/**
 * Overview existing catalog objects
 */
function catalog_overview_form($form, &$form_state) {
  foreach (catalog_object_load_multiple(FALSE, array('is_block' => 0)) as $catalog_object) {
    $form['catalog_objects'][$catalog_object->cid] = array(
      'view' => array(
        '#type' => 'link',
        '#title' => $catalog_object->title,
        '#href' => 'catalog/' . $catalog_object->cid,
      ),
      'edit' => array(
        '#type' => 'link',
        '#title' => t('Edit'),
        '#href' => 'catalog/' . $catalog_object->cid . '/edit',
      ),
      'delete' => array(
        '#type' => 'link',
        '#title' => t('Delete'),
        '#href' => 'catalog/' . $catalog_object->cid . '/delete',
      ),
      'status' => array(
        '#type' => 'checkbox',
        '#default_value' => $catalog_object->status,
      ),
    );
  }
  if (!empty($form['catalog_objects'])) {
    $form['catalog_objects']['#tree'] = TRUE;
    $form['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Save'),
    );
  }
  return $form;
}


function theme_catalog_overview_form($variables) {
  $form = $variables['form'];
  $header = array(
    t('Content list'), 
    array('data' => t('Operations'), 'colspan' => 2), 
    t('Active'),
  );
  $rows = array();
  foreach (element_children($form['catalog_objects']) as $key) {
    $row = array();
    foreach (element_children($form['catalog_objects'][$key]) as $key2) {
      $row[] = drupal_render($form['catalog_objects'][$key][$key2]);
    }
    $rows[] = array('data' => $row,);
  }
  return theme('table', array(
      'header' => $header, 
      'rows' => $rows,
      'empty' => t('No available content lists. <a href="@link">Add content list</a>.', array('@link' => url('admin/structure/catalog/add'))),
      'attributes' => array('id' => 'catalog',),
    )) . drupal_render_children($form);
}

function catalog_overview_form_submit($form, &$form_state) {
  $catalog_objects = catalog_object_load_multiple(array_keys($form_state['values']['catalog_objects']));
  foreach ($catalog_objects as $catalog_object) {
    if ($catalog_object->status != $form_state['values']['catalog_objects'][$catalog_object->cid]['status']) {
      $catalog_object->status = !$catalog_object->status;
      catalog_object_save($catalog_object);
    }
  }
}


function catalog_object_delete_form($form, &$form_state, $catalog_object) {
  $form['#catalog_object'] = $catalog_object;
  return confirm_form($form, t('Are you sure want delete the custom content list %title?', array('%title' => $catalog_object->title)), 'catalog/' . $catalog_object->cid);
}


function catalog_object_delete_form_submit($form, &$form_state) {
  $catalog_object = $form['#catalog_object'];
  catalog_object_delete($catalog_object->cid);
  $t_args = array('%title' => $catalog_object->title,);
  drupal_set_message(t('The content list %title has been deleted.', $t_args));
  watchdog('catalog', 'Deleted content list %title.', $t_args, WATCHDOG_NOTICE);
  $form_state['redirect'] = 'admin/structure/catalog';
}


/**
*  filter catalog object form submission menu catalog/%catalog_object/filter
*/
//
function catalog_object_filtersorting_form($form, &$form_state, $catalog_object = NULL, $is_block = FALSE) {

  $catalog_object_filters = !empty($catalog_object->filters) ? $catalog_object->filters : array();
  // set initialy order of elements in array by existing settings
  $orders_elements = array();
  foreach ($catalog_object_filters as $method => $data) {
    $orders_elements[$method] = array();
  }

  foreach (catalog_sql_info($catalog_object, 'filters') as $method => $data) {
    $enabled = $catalog_object->cid ? !empty($catalog_object_orders[$method]['enabled']) : $data['default'];
    $orders_elements[$method] = array(
      '#type' => 'item',
      '#title' => $data['description'],
      '#inline' => TRUE,
      'exposed' => array(
        '#type' => 'checkbox',
        '#default_value' => !empty($catalog_object_filters[$method]['exposed']),
      ),
      'collapsed' => array(
        '#type' => 'checkbox',
        '#default_value' => !empty($catalog_object_filters[$method]['collapsed']),
      ),
    );
  }

  if ($orders_elements = array_filter($orders_elements)) {
    $form['filters'] = array(
      '#type' => 'fieldset',
      '#title' => t('Filter for user'),
      'filters' => $orders_elements + array(
        '#tree' => TRUE,
        '#theme' => 'catalog_admin_filtersorting_table',
        '#is_block' => $is_block,
      ),
    );
    $form['filters']['form_expanded'] = array(
      '#type' => 'checkbox',
      '#title' => t('Form is expanded'),
      '#default_value' => !empty($catalog_object_filters['form_expanded']),
      '#weight' => - 1,
    );
  }
      // ORDERS NOW
  $catalog_object_orders = !empty($catalog_object->orders) ? $catalog_object->orders : array();
  // set initialy order of elements in array by existing settings
  $orders_elements = array();
  foreach ($catalog_object_orders as $method => $data) {
    $orders_elements[$method] = array();
  }

  foreach (catalog_sql_info($catalog_object, 'orders') as $method => $data) {
    $enabled = $catalog_object->cid ? !empty($catalog_object_orders[$method]['enabled']) : $data['default'];
    $is_default = !empty($catalog_object_orders[$method]['enabled'])?'<small><strong> ('.t('Default').')</strong></small>':'';
    $orders_elements[$method] = array(
      '#type' => 'item',
      '#title' => $data['description'] . $is_default,
      '#inline' => TRUE,
      'exposed' => array(
        '#type' => 'checkbox',
        '#title' => t('Sorting allowed to user'),
        '#default_value' => !empty($catalog_object_orders[$method]['exposed']),
      ),
    );
  }
  if ($orders_elements = array_filter($orders_elements)) {
    $form['orders'] = array(
      '#type' => 'fieldset',
      '#title' => t('Sorting for user'),
      'orders' => $orders_elements + array(
        '#tree' => TRUE,
        '#theme' => 'catalog_admin_filtersorting_table',
      ),
    );
  }
  $form['cid'] = array( '#type' => 'hidden', '#value' => $catalog_object->cid, );
  $form['bundle'] = array( '#type' => 'hidden', '#value' => $catalog_object->bundle, );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
    '#weight' => PHP_INT_MAX - 1,
  );
  return $form;
}


/**
* saving exposed status and order of methods filters & orders that allow for user  // SRuban
*/
function catalog_object_filtersorting_form_submit($form, &$form_state) {
  $catalog_object = current(entity_load('catalog_object', array($form_state['values']['cid'])));
  $orders_elements = array();
  $set_filter_methods = array();
  foreach ((isset($catalog_object->filters) ? $catalog_object->filters : array()) as $method => $data) {
    if (!empty($data['exposed'])) {
      $set_filter_methods[] = $method;
    }
  }

  foreach (array('filters', 'orders') as $type) {
    // in $form_state ['input'] is ordered, used this. but for data must use 'values'
    foreach ($form_state['input'][$type] as $method => $data) {
      $orders_elements[$type][$method] =  isset($catalog_object->{$type}[$method])
        ? array_merge ($catalog_object->{$type}[$method], $form_state['values'][$type][$method]) : $form_state['values'][$type][$method];
    }
    $catalog_object->{$type} = $orders_elements[$type];
  }
  // check difference new and old enabled methods setting - if someone now disabled, drop users sets with it
  $drop_filter_methods = array();
  foreach ($orders_elements['filters'] as $method => $data) {
    if (empty($data['exposed']) && in_array($method, $set_filter_methods)) {
      $drop_filter_methods[] = $method;
    }
  }
  $filters = unserialize(db_select('catalog_objects', 'co')->fields('co', array('filters'))->condition('cid', $catalog_object->cid)->execute()->fetchAssoc()['filters']);
  if ($drop_filter_methods)  {
    if (!empty($filters['user'])) {
      foreach ($filters['user'] as $uid => $item) {
        foreach ($item['sets'] as $name => $data) {
          if (array_intersect(array_keys($data), $drop_filter_methods)) {
            unset($filters['user'][$uid]['sets'][$name]);
          }
        }
      }
    }
  }
  $catalog_object->filters['user'] = isset($filters['user']) ? $filters['user'] : array(); // for resave all users sets states
  $catalog_object->filters['form_expanded'] = !empty($form_state['values']['form_expanded']); // move flag to filters for save!
  if ($catalog_object = catalog_object_save($catalog_object)) {
    $form_state['values']['cid'] = $catalog_object->cid;
    if (!$catalog_object->is_block) {
      $form_state['redirect'] = array('catalog/' . $catalog_object->cid);
    }
    return $catalog_object;
  }
}


function _catalog_edit_filter_orders_table($form_state, $catalog_object, $is_block) {
  $methods_disable_options = array();

  $catalog_object_orders = !empty($catalog_object->orders) ? $catalog_object->orders : array();
  // set initialy order of elements in array by existing settings
  $orders_elements = array();
  foreach ($catalog_object_orders as $method => $data) {
    $orders_elements[$method] = array();
  }
  foreach (catalog_sql_info($catalog_object, 'orders') as $method => $data) {
    // modify if it send by ajax from it elements
    // add from select by ajax
    if (!empty($form_state['values']['orders_add']) && $form_state['values']['orders_add'] === $method) {
      $enabled = TRUE;
    }
    elseif (!empty($form_state['values']['orders'])) {
      $enabled = !empty($form_state['values']['orders'][$method]['enabled']);
    }
    else {
      // if first step - default
      $enabled = $catalog_object->cid ? !empty($catalog_object_orders[$method]['enabled']) : $data['default'];
    }

    $default_value = $enabled ? (!empty($catalog_object_orders[$method]['descending']) ? 'desc' : 'asc') : NULL;
    if  ($default_value) {
      $orders_elements[$method] = array(
        '#type' => 'item',
        '#title' => $data['description'],
        '#inline' => TRUE,
        'default' => array(
          '#type' => 'select',
          '#title' => t('Sorting order'),
          '#options' => array(
            '' => t('None'),
            'asc' => isset($data['label_asc'])? $data['label_asc']: t('Normal'),
            'desc' => isset($data['label_desc'])? $data['label_desc']: t('Reverse order'),
          ),
          '#default_value' => $default_value,
          '#ajax' => array(
            'wrapper' => 'filter-orders-wrapper',
            'event' => 'change',
            'method' => 'replace',
            'callback' => '_catalog_filter_orders_ajax',
          ),
        ),
        'exposed' => array(
          '#type' => 'hidden',
          '#value' => !empty($catalog_object_orders[$method]['exposed']) && !$is_block,
        ),
      );
    }
    else {
      $methods_disable_options[$method] = strip_tags(str_replace('<br />', ' - ', $data['description']));
    }
  }
  $form = array(
    '#type' => 'fieldset',
    '#title' => t('Content sorting by default'),
    '#prefix' => '<div id="filter-orders-wrapper">',
    '#suffix' => '</div>',
  );
  if ($orders_elements = array_filter($orders_elements)) {
    $form['orders'] = $orders_elements + array(
        '#tree' => TRUE,
        '#theme' => 'catalog_admin_orders_table',
        '#is_block' => $is_block,
    );
  }
  if ($methods_disable_options) {
    $form['orders_add'] = array(
        '#type' => 'select',
        '#options' => array('' => '+ ' . t('Add sort criterion')) + $methods_disable_options,
        '#ajax' => array(
        'wrapper' => 'filter-orders-wrapper',
        'event' => 'change',
        'method' => 'replace',
        'callback' => '_catalog_filter_orders_ajax',
      ),
    );
  }
  return $form;
}

function _catalog_filter_orders_ajax($form, $form_state) {
  // if block add settings group
  return isset($form['filters']['orders']) ? $form['filters']['orders']: $form['settings']['filters']['orders'];
}
























