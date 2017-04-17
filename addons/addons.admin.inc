<?php


/**
* Callback for /admin/structure/field/custom-view-mode/%/%
* Custom view mode edit/delete form
* @see addons_form_alter() -->> 'field_ui_display_overview_form'
* @see addons_entity_info_alter()
*/
function addons_custom_display_admin_form($form, &$form_state, $entity_type, $custom_view_mode_id = NULL, $op = NULL) {
  $entity_info = entity_get_info($entity_type);
  if (!$entity_info['fieldable'] || (!$custom_view_mode_id && $op === 'delete')) {
    drupal_not_found();
    drupal_exit();    
  }
  $def_name = NULL;
  $form['#custom_view_mode_id'] = NULL;
  $addons_entity_custom_view_modes = variable_get('addons_entity_custom_view_modes', array());
  if (!$custom_view_mode_id) {
    $op = 'new';
  }
  elseif (!preg_match('/^addons_\d+/', $custom_view_mode_id) || !isset($addons_entity_custom_view_modes[$entity_type][$custom_view_mode_id])) {
    drupal_not_found();
    drupal_exit();
  }
  else {
    $def_name = $addons_entity_custom_view_modes[$entity_type][$custom_view_mode_id];
    $form['#custom_view_mode_id'] = $custom_view_mode_id;
    $op = $op === 'delete' ? $op : 'edit';
    drupal_set_title(t('Edit view mode "%name" in %entity_type', array('%name' => $def_name, '%entity_type' => $entity_info['label'])), PASS_THROUGH);
  }
  if ($op === 'new') {
    drupal_set_title(t('Add new view mode for %entity_type', array('%entity_type' => $entity_info['label'])), PASS_THROUGH);
  }
  $form['#entity_type'] = $entity_type;
  $form['#op'] = $op;
  if ($op === 'delete') {
    return confirm_form($form, t('Do you really want delete view mode %name?', array('%name' => $def_name)), $_GET['q']);
  }
  else {
    $form['name'] = array(
      '#title' => t('View mode name'),
      '#type' => 'textfield',
      '#default_value' => $def_name,
      '#required' => TRUE,
    );
    $form['actions'] = array(
      '#type' => 'actions',
      'submit' => array(
        '#type' => 'submit',
        '#value' => t('Save'),
      ),
    );    
  }

  return $form;
}


/**
* Validate callback for addons_custom_display_admin_form
*/
function addons_custom_display_admin_form_validate($form, &$form_state) {
  // Deny duplicaded names
  if ($form['#op'] === 'new' || $form['#op'] === 'edit') {
    $addons_entity_custom_view_modes = variable_get('addons_entity_custom_view_modes', array());
    $name = preg_replace('/(^\s+)|(\s+$)/u', '', $form_state['values']['name']);
    $name = preg_replace('/\s+/u', ' ', $name);
    foreach ($addons_entity_custom_view_modes as $entity_type => $modes) {
      foreach ($modes as $id => $n) {
        if ($id !== $form['#custom_view_mode_id']) {
          $n = preg_replace('/(^\s+)|(\s+$)/u', '', $n);
          $n = preg_replace('/\s+/u', ' ', $n);
          if ($n === $name) {
            form_error($form['name'], t('View mode with this name already exists'));
          }
        }
      }
    }
  }
}


/**
* Submit callback for addons_custom_display_admin_form
*/
function addons_custom_display_admin_form_submit($form, &$form_state) {
  $addons_entity_custom_view_modes = variable_get('addons_entity_custom_view_modes', array());
  if ($form['#op'] === 'delete') {
    drupal_set_message(t('View mode %name heas been deleted.', array('%name' => $addons_entity_custom_view_modes[$form['#entity_type']][$form['#custom_view_mode_id']])), 'warning');
    unset($addons_entity_custom_view_modes[$form['#entity_type']][$form['#custom_view_mode_id']]);
  }
  else {
    if ($form['#op'] === 'new') {
      $id = 'addons_' . db_next_id();
    }
    else { // op == edit
      $id = $form['#custom_view_mode_id'];
    }
    $addons_entity_custom_view_modes[$form['#entity_type']][$id] = $form_state['values']['name'];
    
    drupal_set_message(t('The changes have been saved.'));
  }
  variable_set('addons_entity_custom_view_modes', $addons_entity_custom_view_modes);
  field_info_cache_clear();
  variable_set('menu_rebuild_needed', TRUE);
}
