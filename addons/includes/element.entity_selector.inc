<?php

define('ADDONS_ENTITY_SELECTOR_COOKIE_MAX_RECENTS', 10);

/**
 * entity_selector type element.
 * Provides textfield autocomplete nodes/users/etc entities chooser.
 * Used properties:
 *  #selector_entity_type - Type of entity. Required.
 *  #selector_bundles - Optional restrictive filter. Can be array of bundles or simple bundle string.
 *  #selector_roles - Only for entity_type is USER. Optional restrictive filter. Can be array of roles or simple role ID
 *  #return_field - name of key in entity for set value returned when form submitted. By default - entity id key name (nid, uid or tid). If #return_field is 'url', entity uri will used.
 *  #selector_conditions - Other restrictive filters which will inserted to database select query. e.g. array('status' => 1). Optional.
 *
 *  #selector_order_by - order autocomplete items by special method(s). Array or string.
 *          Supported methods:
 *             'recents' - stores in cookies last choosed entities and place it at top of autocomplete list
 *             'base.field_name' - any column name of base table of entity. E.g., for node it may be 'base.title', 'base.created')
 *         Default is array of 'base.created' for node entity type, 'recents' and 'base.title ('label' column name from entity_get_info())
 *  #allow_edit_title - if TRUE and entity type supports label keys, then selected entity will saved with new title/label (if it was changed) at element validation stage. Default is FALSE.
 *  #selector_allow_add - generate add entity button that opens creation form in new tab and automatically puts new entity id in field. Default is TRUE.
 **/
function _addons_process_entity_selector($element, &$form_state) {
  $element += array(
    '#selector_bundles' => NULL,
    '#size' => 58,
    '#selector_roles' => NULL,
    '#selector_allow_edit_title' => FALSE,
    '#selector_allow_add' => TRUE,
    '#selector_use_entity_picture' => TRUE,
    '#selector_order_by' => NULL,
    '#attributes' => array(),
    '#field_suffix' => '',
    '#suffix' => '',
  );
  $entity_info = entity_get_info($element['#selector_entity_type']);
  if (empty($element['#selector_order_by'])) {
    $element['#selector_order_by'] = array();
    $element['#selector_order_by'][] = 'recents';
    if (!empty($entity_info['entity keys']['label'])) {
      $element['#selector_order_by'][] = 'base.' . $entity_info['entity keys']['label'] . ' ASC';
    }
    elseif ($element['#selector_entity_type'] === 'user') {
      $element['#selector_order_by'][] = module_exists('social_profile') ? 'base.lfm ASC' : 'base.name ASC';
    }
    if ($element['#selector_entity_type'] === 'node') {
      $element['#selector_order_by'][] = 'base.created';
    }

  }
  else {
    $element['#selector_order_by'] = (array) $element['#selector_order_by'];
  }
  $element['#selector_order_by'] = drupal_map_assoc(array_filter($element['#selector_order_by']));
  $element['#maxlength'] = 255;
  $element['#selector_bundles'] = drupal_map_assoc(array_filter((array) $element['#selector_bundles']));
  if (!array_key_exists('#title', $element)) {
    $b = reset($element['#selector_bundles']);
    if (count($element['#selector_bundles']) == 1 && isset($entity_info['bundles'][$b]['label'])) {
      $element['#title'] = $entity_info['bundles'][$b]['label'];
    }
    elseif (isset($entity_info['label'])) {
      $element['#title'] = $entity_info['label'];
    }
  }
  $element['#selector_roles'] = array_filter((array) $element['#selector_roles']);
  if (!empty($element['#ajax']) && !isset($element['#ajax']['event'])) { // Simple ajax patch (ajax.inc will ignore additional elements without 'event' flag)
    $element['#ajax']['event'] = 'change';
  }

  $selector_params = array('element_name' => $element['#name']);
  foreach ($element as $key => $data) {
    if ($key[0] === '#' && preg_match('/^#selector_(.+)$/', $key, $m)) {
      $selector_params[$m[1]] = $data;
    }
  }
  $element['#selector_params'] = $selector_params;
  $element['#autocomplete_path'] = 'addons/entity_selector/autocomplete/' . str_replace('/', '------', base64_encode(serialize($selector_params)));
  $element['#autocomplete_path'] .= '/' . drupal_get_token($element['#autocomplete_path']);

  $default_img = $element['#selector_use_entity_picture'] ? _addons_entity_selector_get_entity_default_image($element['#selector_entity_type']) : file_create_url(drupal_get_path('module', 'addons') . '/img/preview.png');

  $entity = NULL;
  foreach (array('#default_value', '#value',) as $key) {
    if (!empty($element[$key])) {
      if (ctype_digit('' . $element[$key])) {
        if ($e = entity_load($element['#selector_entity_type'], array($element[$key]))) {
          $entity = reset($e);
          $label = entity_label($element['#selector_entity_type'], $entity);
          $element[$key] = $label . ' [:' . $entity_info['entity keys']['id'] . ':' . $element[$key] . ':]';
          if ($element['#selector_use_entity_picture']) {
            $img = _addons_entity_selector_get_entity_default_image($element['#selector_entity_type'], $entity);
          }
        }
        else {
          $element[$key] = t('--- NOT EXISTING OBJECT #@num ---', array('@num' => $element[$key]));
        }
      }
      else {
        if ($element['#selector_use_entity_picture'] && !isset($img) && preg_match('/\[\:' . $entity_info['entity keys']['id'] . '\:(\d+):\]/', $element[$key], $m)) {
          if ($e = entity_load($element['#selector_entity_type'], array($m[1]))) {
            $entity = reset($e);
            $img = _addons_entity_selector_get_entity_default_image($element['#selector_entity_type'], $entity);
          }
        }
      }
    }
    else {
      $element[$key] = NULL;
    }
  }
  if ($element['#selector_entity_type'] === 'user' && !user_access('access user profiles')) {
    if (empty($element['#required']) && (!isset($element['#access']) || $element['#access'])) {
      $element['#attributes']['readonly'] = 'readonly';
    }
  }
  $element['#attributes']['data-url'] = NULL;//$element['#attributes']['data-edit-url'] = NULL;

  if ($entity) {
    $entity_type = $element['#selector_entity_type'];
    list($entity_id, ,) = entity_extract_ids($entity_type, $entity);
    $uri = entity_uri($entity_type, $entity);
    $element['#attributes']['data-url'] = url($uri['path']);
    $element['#attributes']['data-selector-entity-type'] = $entity_type;
    if(!empty($element['#view_mode'])) {
      $element['#attributes']['data-selector-view-mode'] = $element['#view_mode'];
    }
    $element['#selector_entity_id'] = $entity_id;
  }

  $links = array();
  $actions = array('view' => 'eye', 'edit' => 'pencil',);

  foreach ($actions as $action => $icon) {
    $key = $action . '-link';

    $links[$key] = array(
      'title' => module_exists('pictogram') ? theme('pictogram', array('icon' => $icon)) : check_plain(t(ucfirst($action))),
      'html' => TRUE,
      'attributes' => array(
        'target' => '_blank',
        'class' => array($key),
        'title' => t(ucfirst($action)),
      ),
    );

    if ($entity) {
      if ($action == 'view') {
        if (($entity_type === 'node' && node_access('view', $entity)) || ($entity_type === 'user' && user_view_access($entity))) {
          $links[$key]['href'] = $entity_type . '/' . $entity_id;
        }
      }
      elseif ($action == 'edit') {
        if (($entity_type === 'node' && node_access('update', $entity)) || ($entity_type === 'user' && user_edit_access($entity))) {
          $links[$key]['href'] = $entity_type . '/' . $entity_id . '/edit';
        }
      }
    }

    if (!isset($links[$key]['href'])) {
      $links[$key]['attributes']['fragment'] = ' ';
      $links[$key]['attributes']['class'][] = 'element-invisible';
    }
  }

  $theme_links = theme('links', array('links' => $links, 'attributes' => array('class' => 'inline', 'style' => (module_exists('pictogram') ? 'white-space:nowrap;' : '')), 'heading' => array()));

  $element['#field_suffix'] = $theme_links . $element['#field_suffix'];

  $element['#attributes']['data-img'] = !empty($img) ? $img : $default_img;
  $element['#attributes']['data-default-img'] = $default_img;
  $module_path = drupal_get_path('module', 'addons');
  $element['#attached']['js'][$module_path . '/js_behaviors/entity_selector.js']['weight'] = 99999999;
  $element['#attached']['css'][] = $module_path . '/css/entity_selector.css';


  $element['#post_render'][] = '_addons_entity_selector_post_render';

  return $element;
}

function _addons_entity_selector_post_render($html, $element) {
  $link_attributes = array(
    'class' => array('entity-selector-preview', strpos(request_uri(), 'confirm_dialog_ajax') !== FALSE ? 'popup' : 'ajax-popup'), // Use simple popup instead of ajax-popup where we already in ajax-popup context
    'target' => '_blank',
    'href' => $element['#attributes']['data-url'] ? url($element['#attributes']['data-url']) : '#',
    'title' => t('View'),
  );
  $wrapper_classes = array(
    'entity-selector-text-input-wrapper',
  );
  if (!$element['#attributes']['data-img']) {
    $link_attributes['class'][] = 'element-invisible';
  }
  else {
    $wrapper_classes[] = 'has-image';
    $link_attributes['style'] = 'background-image: url(\'' . $element['#attributes']['data-img'] . '\');';
  }
  if ($element['#value']) {
    $wrapper_classes[] = 'has-del-button';
  }

  $link = '<a ' . drupal_attributes($link_attributes) . '></a>';

  $clear_button =
    '<a class="' . ($element['#value'] && empty($element['#attributes']['readonly']) && empty($element['#attributes']['disabled']) ? '' : 'element-invisible ') . 'entity-selector-clear-link pictogram" href="#" title="' . check_plain(t('Clear')) . '">' .
    (module_exists('pictogram') ? pictogram('cancel-1') : '&#x2718;') .
    '</a>';

  if (!empty($element['#selector_entity_id']) && $element['#selector_entity_type'] == 'node') {
    $link = '';
  }

  $out = preg_replace('/<input(?:\s+.*?)?(?:"|\s+)type="text".*?>/sui', '<span class="' . implode(' ', $wrapper_classes) . '">' . $link . '$0' . $clear_button . '</span>', $html);

  if (!empty($element['#selector_entity_id']) && !empty($element['#view_mode'])) {
    if ($element['#selector_entity_type'] == 'node') {
      $entity = node_load($element['#selector_entity_id']);
      $entity_view = node_view($entity, $element['#view_mode']);
    }
    elseif ($element['#selector_entity_type'] == 'user') {
      $entity = user_load($element['#selector_entity_id']);
      $entity_view = user_view($entity, $element['#view_mode']);

    }
    if(isset($entity) && isset($entity_view)) {
      $out = preg_replace('/<\/div>/sui', '<div class="entity-selector-entity-view">' . render($entity_view) . '</div>' . '$0', $out);
    }
  }

  return $out;
}


/**
 * Helper func. Get first occurrence of image in rendered entity or default image.
 */
function _addons_entity_selector_get_entity_default_image($entity_type, $entity = NULL) {
  $img = NULL;
  if ($entity_type === 'user') {
    $img = $entity ? theme('user_picture', array('account' => $entity)) : theme('user_picture');
  }
  else {
    $img = NULL;
  }

  if ($img && preg_match('/<img\s/sui', $img)) {
    $style = '56x42';
    $img = trim(preg_replace('/^.*?<img(?:\s+.*?)?(?:"|\s+)src="(.*?)".*?$/sui', '$1', $img));
    $img = preg_replace('/[\?\#].*$/', '', $img);
    $img = preg_replace('/^' . preg_quote($GLOBALS['base_url'], '/') . '\//', '', $img);
    $img = preg_replace('/^' . preg_quote(variable_get('file_public_path', conf_path() . '/files'), '/') . '\//', '', $img);
    $img = preg_replace('/^styles\/.*?\/public\//', '', $img);
    if (!is_file('public://' . $img) && is_file(DRUPAL_ROOT . '/' . $img)) {
      if (!is_file($preview_file = image_style_path($style, $img))) {
        image_style_create_derivative(image_style_load($style), $img, $preview_file);
      }
    }
    return image_style_url($style, $img);
  }
}


function addons_validate_entity_selector(&$element, &$form_state, $complete_form) {
  if (empty($element['#value'])) {
    return;
  }
  $label = isset($element['#title']) ? $element['#title'] : NULL;
  if (module_exists('field') && !empty($element['#entity_type']) && !empty($element['#field_name']) && !empty($element['#bundle']) && ($inst = field_info_instance($element['#entity_type'], $element['#field_name'], $element['#bundle']))) {
    $label = $inst['label'];
    $field = field_info_field($element['#field_name']);
    if ($field['cardinality'] != 1) {
      $label .= ' (' . t('#@number', array('@number' => $element['#delta'] + 1)) . ')';
    }
  }
  $entity_info = entity_get_info($element['#selector_entity_type']);
  if (!($is_num = ctype_digit('' . trim($element['#value']))) && !preg_match('/\[\:' . $entity_info['entity keys']['id'] . '\:(\d+):\]/i', $element['#value'], $matches)) {
    form_error($element, t('Invalid value in field !field. Type number of existing object.', array('!field' => '<strong>"' . check_plain($label) . '"</strong>')));
  }
  else {
    $id = $is_num ? $element['#value'] : $matches[1];
    if (!($entities = entity_load($element['#selector_entity_type'], array($id)))) {
      form_error($element, t('Non existing object in !field field.', array('!field' => '<strong>"' . check_plain($label) . '"</strong>')));
    }
    else {
      $has_errors = FALSE;
      $entity = reset($entities);
      if ($element['#selector_entity_type'] === 'user' && !empty($element['#selector_roles'])) {
        if (in_array(DRUPAL_AUTHENTICATED_RID, $element['#selector_roles'])) {
          if (!$entity->uid) {
            form_error($element, t('You can\'t select this user.'));
            $has_errors = TRUE;
          }
        }
        else {
          if (!array_intersect(array_keys($entity->roles), array_values($element['#selector_roles']))) {
            form_error($element, t('You can\'t select this user.'));
            $has_errors = TRUE;
          }
        }
      }
      list($id, $vid, $bundle) = entity_extract_ids($element['#selector_entity_type'], $entity);
      if ($element['#selector_bundles'] && !in_array($bundle, $element['#selector_bundles'], TRUE)) {
        $allowed = array();
        foreach ($entity_info['bundles'] as $b => $item) {
          if (in_array($b, $element['#selector_bundles'], TRUE)) {
            $allowed[] = $item['label'];
          }
        }
        form_error($element, t('Incorrect type of object in !field field.', array('!field' => '<strong>"' . check_plain($label) . '"</strong>',)) . ' ' . t('Allowed object types is: %types', array('%types' => $allowed ? drupal_strtolower(implode(', ', $allowed)) : $element['#selector_bundles'])));
        $has_errors = TRUE;
      }


      // Process #selector_conditions property
      $selector_conditions = isset($element['#selector_conditions']) ? $element['#selector_conditions'] : NULL;
      // #selector_conditions may be not actual where new entity was added/deleted in other tab or by other user.
      // Re-generate temporary form copy with same params into memory and get actual #selector_conditions parameter from it.
      static $recalled_forms = array();
      $args = $form_state['build_info']['args'];
      array_unshift($args, $form_state['values']['form_id']);
      $cid = md5(serialize($args));
      if (!isset($recalled_forms[$cid])) {
        // Temporary clear POST data for avoid validation temporary created form
        $post = $_POST;
        $_POST = array();
        $recalled_forms[$cid] = call_user_func_array('drupal_get_form', $args);
        $_POST = $post; // restore POST
      }
      if ($recalled_forms[$cid]) {
        // retrieve element and update selector conditions paramers for current element
        if ($actual_element = drupal_array_get_nested_value($recalled_forms[$cid], $element['#array_parents'])) {
          $selector_conditions = isset($actual_element['#selector_conditions']) ? $actual_element['#selector_conditions'] : NULL;
        }
      }

      if ($selector_conditions) {
        $table_alias = 'base';
        $id_key = $entity_info['entity keys']['id'];
        $query = db_select($entity_info['base table'], $table_alias)
          ->fields($table_alias, array($id_key,))
          ->condition($table_alias . '.' . $id_key, $id);

        $query->addTag('entity_selector');
        $query->addTag($element['#selector_entity_type'] . '_selector');
        // $query->addTag($element['#selector_entity_type'] . '_selector_autocomplete');
        $query->addMetaData('selector_params', $element['#selector_params']);

        foreach ((array)$selector_conditions as $field_name => $data) {
          $query->condition(preg_match('/.\../', $field_name) ? $field_name : $table_alias . '.' . $field_name, isset($data[0]) ? $data[0] : NULL, isset($data[1]) ? $data[1] : NULL);
        }
        if (!$query->execute()->fetchCol()) {
          form_error($element, t('@entity_type "%name" does not match the specified conditions.', array('@entity_type' => $entity_info['label'], '%name' => entity_label($element['#selector_entity_type'], $entity))));
          $has_errors = TRUE;
        }
      }


      if (!$has_errors) {
        if (!empty($element['#allow_edit_title'])) {
          if (function_exists($entity_save_callback = $element['#selector_entity_type'] . '_save') && !empty($entity_info['entity keys']['label']) && isset($entity->{$entity_info['entity keys']['label']})) {
            $old_label = $entity->{$entity_info['entity keys']['label']};
            $new_label = preg_replace('/\s*\[:' . $entity_info['entity keys']['id'] . ':' . $id . ':\]\s*$/u', '', $element['#value']);
            $new_label = preg_replace('/\s+/u', ' ', $new_label); // double spaces
            if ($new_label = preg_replace('/(^\s*)|(\s*$)/u', '', $new_label)) { // utf8 trim
              if ($old_label !== $new_label) {
                $entity->{$entity_info['entity keys']['label']} = $new_label;
                $entity_save_callback($entity);
                drupal_set_message(t('@entity_name %label1 has been renamed to %label2.', array('@entity_name' => $entity_info['label'], '%label1' => $old_label, '%label2' => $new_label,)));
              }
            }
          }
        }
        $element['#return_field'] = !empty($element['#return_field']) ? $element['#return_field'] : $entity_info['entity keys']['id'];
        if ($element['#return_field'] === 'url') {
          $entity_uri = entity_uri($element['#selector_entity_type'], $entity);
          form_set_value($element, $entity_uri['path'], $form_state);
        }
        else {
          form_set_value($element, isset($entity->{$element['#return_field']}) ? $entity->{$element['#return_field']} : NULL, $form_state);
        }
        if (in_array('recents', $element['#selector_order_by'], TRUE)) {
          $entity_selector_recents = !empty($_COOKIE['entity_selector_recents']) ? @unserialize($_COOKIE['entity_selector_recents']) : array();
          $entity_selector_recents = is_array($entity_selector_recents) ? $entity_selector_recents : array();
          if (!isset($entity_selector_recents[$element['#selector_entity_type']]) || !is_array($entity_selector_recents[$element['#selector_entity_type']])) {
            $entity_selector_recents[$element['#selector_entity_type']] = array();
          }
          $entity_selector_recents[$element['#selector_entity_type']] = drupal_map_assoc($entity_selector_recents[$element['#selector_entity_type']]);
          $entity_selector_recents[$element['#selector_entity_type']] = array($id => $id) + $entity_selector_recents[$element['#selector_entity_type']];
          array_splice($entity_selector_recents[$element['#selector_entity_type']], ADDONS_ENTITY_SELECTOR_COOKIE_MAX_RECENTS);
          $_COOKIE['entity_selector_recents'] = serialize($entity_selector_recents);
          setcookie('entity_selector_recents', $_COOKIE['entity_selector_recents'], REQUEST_TIME + (24 * 3600 * 7), '/');
        }
      }
    }
  }
}

function _addons_entity_selector_autocomplete_item($entity_type, $entity, $string = NULL) {
  $entity_info = entity_get_info($entity_type);
  list($id, , $bundle) = entity_extract_ids($entity_type, $entity);
  $html = '';
  if ($img = _addons_entity_selector_get_entity_default_image($entity_type, $entity)) {//_addons_entity_selector_get_entity_image_preview($entity_type, $entity)) {
    $html .= '<img src="' . check_plain($img) . '"/> ';
  }

  $classes = array('autocomplete-item');
  if ($unpublished = isset($entity->status) && !$entity->status) {
    $classes[] = 'unpublished';
  }

  $label = entity_label($entity_type, $entity);
  $note = $more_info = array();

  if ($entity_type === 'node' && !empty($entity_info['bundles'][$bundle]['label'])) {
    $note[] = drupal_strtolower($entity_info['bundles'][$bundle]['label']);
  }
  if ($entity_type === 'user') {
    $more_info[] = $entity->name;
  }
  if ($unpublished) {
    $note[] = drupal_strtolower($entity_type === 'user' ? t('Blocked') : t('Not published'));
  }

  $edit_url = NULL;

  if (($entity_type === 'node' && node_access('update', $entity)) || ($entity_type === 'user' && user_edit_access($entity))) {
    $edit_url = $entity_type . '/' . $id . '/edit';
  }

  $html .=
    '<div
    data-url="' . check_plain(url(entity_uri($entity_type, $entity)['path'])) . '"
    data-edit-url="' . check_plain(url($edit_url)) . '"
    data-selector-entity-id="' . $id . '"
    class="' . implode(' ', $classes) . '">' .
    (isset($string) ? preg_replace('/(' . preg_quote($string, '/') . ')/ui', '<strong class="highlighted">$1</strong>', check_plain($label)) : check_plain($label)) .
    ($note = $note ? ' <em>(' . implode(', ', $note) . ')</em>' : '') .
    ($more_info ? '<br /><small>' . implode(', ', $more_info) . '</small>' : '') .
    '</div>';

  $html = '<div class="entity-selector-autocomplete-item">' . $html . '</div>';
  return array($label . ' [:' . $entity_info['entity keys']['id'] . ':' . $id . ':]' => $html);
}


function addons_entity_selector_delivery_callback($page_callback_result) {
  drupal_add_http_header('Content-Type', 'application/json; charset=utf-8');
  drupal_json_output($page_callback_result);
}


function addons_entity_selector_autocomplete($selector_params_encoded, $token, $string = '') {
  $matches = array();
  $selector_params = unserialize(base64_decode(str_replace('------', '/', $selector_params_encoded)));
  $entity_type = $selector_params['entity_type'];
  if (

    ($code = !drupal_valid_token($token, 'addons/entity_selector/autocomplete/' . $selector_params_encoded) ? 'invalid token' : NULL) ||
    ($code = $entity_type === 'user' && !user_access('access user profiles') ? 'user profiles' : NULL)

  ) {
    $matches[''] = '<div class="messages error">' . t('Access denied') . ($code ? ' - ' . $code : '') . '</div>';
  }
  else {
    if ($string = trim($string)) {
      $entity_selector_recents = !empty($_COOKIE['entity_selector_recents']) ? @unserialize($_COOKIE['entity_selector_recents']) : array();
      $recents = is_array($entity_selector_recents) && !empty($entity_selector_recents[$entity_type]) && is_array($entity_selector_recents[$entity_type]) ? $entity_selector_recents[$entity_type] : array();
      $recents = array_filter($recents, 'is_numeric');


      if (!empty($selector_params['bundles'])) {
        $bundles = array_filter($selector_params['bundles'], 'trim');
        sort($bundles);
      }
      else {
        $bundles = array();
      }
      $offset = 0;
      if (preg_match('/:p=\d+$/', $string)) {
        $offset = preg_replace('/^.*?:p=(\d+)$/', '$1', $string);
        $string = preg_replace('/:p=\d+$/', '', $string);
      }
      $original_string = $string;
      $string = in_array($string, array('_', '%',), TRUE) ? '*' : $string;

      $entity_info = entity_get_info($entity_type);
      if (preg_match('/\[\:' . $entity_info['entity keys']['id'] . '\:(\d+):\]/', $string, $m)) {
        $id = $m[1];

        if ($entities = entity_load($entity_type, array($id))) {
          $entity = reset($entities);
          $matches += _addons_entity_selector_autocomplete_item($entity_type, $entity);
        }
      }
      if (!$matches) {
        $table_alias = 'base';
        $id_key = $entity_info['entity keys']['id'];
        $query = db_select($entity_info['base table'], $table_alias)
          ->fields($table_alias, array($entity_info['entity keys']['id'],));
        if ($entity_type === 'node') {
          $query->addTag('node_access');
        }
        $query->addTag('entity_selector');
        $query->addTag($entity_type . '_selector');
        $query->addTag($entity_type . '_selector_autocomplete');
        $query->addMetaData('selector_params', $selector_params);
        if ($entity_type === 'user') {
          $query->condition($table_alias . '.' . $id_key, 0, '>');
          if ($roles = !empty($selector_params['roles']) ? $selector_params['roles'] : array()) {
            if (count($roles) > 1 || !in_array(DRUPAL_AUTHENTICATED_RID, $roles)) {
              $query->join('users_roles', 'ur', 'ur.uid = ' . $table_alias . '.' . $id_key);
              $query->condition('ur.rid', $roles);
            }
          }
        }
        if ($bundles && !empty($entity_info['entity keys']['bundle'])) {
          $query->condition($table_alias . '.' . $entity_info['entity keys']['bundle'], $bundles);
        }
        $db_or = FALSE;
        if ($string !== '*') {
          if (!empty($entity_info['entity keys']['label'])) {
            $db_or = db_or()->condition($table_alias . '.' . $entity_info['entity keys']['label'], '%' . $string . '%', 'LIKE');
            $query->addExpression('LOCATE (:s, ' . $table_alias . '.' . $entity_info['entity keys']['label'] . ')', '__substr_position__', array(':s' => $string));
            $query->orderBy('__substr_position__', 'ASC');
          }
          else {
            if ($entity_type === 'user') {
              $db_or = db_or()->condition($table_alias . '.name', '%' . db_like($string) . '%', 'LIKE');
              if (module_exists('social_profile')) {
                $db_or->condition($table_alias . '.lfm', '%' . db_like($string) . '%', 'LIKE');
              }
            }
          }
        }
        if (!empty($selector_params['conditions']) && is_array($selector_params['conditions'])) {
          foreach ($selector_params['conditions'] as $field_name => $data) {
            $query->condition(preg_match('/.\../', $field_name) ? $field_name : $table_alias . '.' . $field_name, isset($data[0]) ? $data[0] : NULL, isset($data[1]) ? $data[1] : NULL);
          }
        }
        if (ctype_digit($string)) {
          $db_or = $db_or ? $db_or : db_or();
          $db_or->condition($table_alias . '.' . $id_key, $string . '%', 'LIKE');
        }
        if ($db_or) {
          $query->condition($db_or);
        }

        foreach ($selector_params['order_by'] as $method) {
          if (preg_match('/^(' . $table_alias . '\.[a-z_]+)(\s+([a-zA-Z_]+))?$/', $method, $m)) {
            $query->orderBy($m[1], !empty($m[3]) ? $m[3] : 'DESC');
          }
          elseif ($method === 'recents' && $recents) {
            $query->addExpression($table_alias . '.' . $id_key . ' IN (' . implode(', ', $recents) . ')', 'in_recents');
            $query->orderBy('in_recents', 'DESC');
          }
        }
        $limit = 30;
        $ids = $query->range($offset * $limit, $limit + 1)->execute()->fetchCol();
        foreach ($ids ? entity_load($entity_type, $ids) : array() as $entity) {
          $matches += _addons_entity_selector_autocomplete_item($entity_type, $entity, $string);
        }
        if ($matches) {
          $next = FALSE;
          if (count($matches) > $limit) {
            array_pop($matches);
            $next = TRUE;
          }
          if ($offset) {
            $matches = array(
                $original_string . ':p=' . ($offset - 1) => '<span data-original-string="' . check_plain($original_string) . '" class="prev-next"><a href="#">&#8593; ' . t('prev @count', array('@count' => $limit)) . '</a></span>',
              ) + $matches;
          }
          if ($next) {
            $matches[$original_string . ':p=' . ($offset + 1)] = '<span data-original-string="' . check_plain($original_string) . '" class="prev-next"><a href="#">&#8595; ' . t('next @count', array('@count' => $limit)) . '</a></span>';
          }
        }
      }
    }
    if (!empty($selector_params['allow_add'])) {

      $add_link = NULL;

      if ($selector_params['entity_type'] === 'node') {
        $node_types = $selector_params['bundles'] ? $selector_params['bundles'] : array_keys(node_type_get_types());
        if (count($node_types) == 1) {
          $node_type = reset($node_types);
          if (node_access('create', $node_type)) {
            $add_link = array(
              'title' => t('Add new @node_type', array('@node_type' => node_type_get_name($node_type))),
              'href' => 'node/add/' . str_replace('_', '-', $node_type),
            );
          }
        }
      }

      $query = array(
        'entity-selector' => $selector_params['element_name'],
        'entity-label' => $string,
        'entity-type' => $selector_params['entity_type'],
        'token' => drupal_get_token($selector_params['element_name'] . $selector_params['entity_type']),
      );

      if ($selector_params['entity_type'] === 'user' && user_access('administer users')) {
        $add_link = array(
          'title' => t('New user'),
          'href' => 'admin/people/create',
        );
        if (!empty($selector_params['roles'])) {
          if (count($selector_params['roles']) == 1 && ($r = user_role_load(reset($selector_params['roles'])))) {
            $add_link['title'] = t('New @role_name', array('@role_name' => t($r->name)));
          }
          $query['roles'] = implode(',', $selector_params['roles']);
        }
      }

      if ($add_link) {
        $matches = array(
            '+' => l($add_link['title'], $add_link['href'], array(
              'query' => $query,
              'attributes' => array(
                'target' => '_add_object_page_',
                'class' => array('entity-selector-add-link', 'form-submit',),
              ),
            ))
          ) + $matches;
      }
    }
  }
  return $matches ? $matches : array('' => t('Content not found'));
}

/**
* Callback for /addons/entity_selector/node/%node
*/
function addons_entity_selector_entity_view_callback($node, $view_mode = 'full') {
  $view = node_view($node, $view_mode);
  unset($view['#contextual_links']);
  print render($view);
  drupal_exit();
}


























