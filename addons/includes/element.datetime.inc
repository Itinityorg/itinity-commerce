<?php

function addons_process_datetime($element, &$form_state) {

  $element['#attached']['css'][] = drupal_get_path('module', 'addons') . '/css/addons.datetime.css';
  $element['#attached']['js'][] = drupal_get_path('module', 'addons') . '/js_behaviors/addons.datetime.js';
  $element['#attached']['library'][] = array('system', 'ui.datepicker');
  $element = array('date' => array(), 'time' => array()) + $element;
  
  $element += array(
    '#date' => TRUE, // on/off date field
    '#time' => FALSE, // on/off time field
    '#min' => NULL, // minimum datetime. timestamp; '+10 days'; '-1 week'; ...
    '#max' => NULL, // maximum datetime
    '#group' => NULL, // use two datetime elements as period picker
    '#inline' => FALSE,
    '#title' => '',
  );
  $element['#tree'] = TRUE;
  $element = array('label' => array(
    '#markup' => theme('form_element_label', array('element' => $element,)),
  )) + $element;
  if ($element['#inline']) {
    $element['#attributes']['class'][] = 'container-inline';
  }
  $element['#attributes']['class'][] = 'form-item';
  $element['#attributes']['class'][] = 'form-type-datetime';
  
  if ($element['#group']) {
    $element['#attributes']['class'][] = 'grouped';
    $element['#attributes']['class'][] = 'group-' . drupal_html_class($element['#group']);
    $element['#attributes']['data-group'][] = 'group-' . drupal_html_class($element['#group']);
  }
  if (!$element['#date'] && !$element['#time']) {
    $element['#date'] = TRUE;
  }
  
  if ($element['#required']) {
    $element['#attributes']['class'][] = 'required';
  }
  $min = $element['#min'] ? (preg_match('/^-?\d+$/', $element['#min']) ? $element['#min'] : strtotime($element['#min'])) : NULL;
  $max = $element['#max'] ? (preg_match('/^-?\d+$/', $element['#max']) ? $element['#max'] : strtotime($element['#max'])) : NULL;

  if ($element['#value'] && is_scalar($element['#value']) && !preg_match('/^-?\d+$/', $element['#value'])) {
    $element['#value'] = strtotime($element['#value']);
  }
  
  if (is_array($element['#value']) && (isset($element['#value']['date']) || isset($element['#value']['time']))) {
    $element['#value'] = (!empty($element['#value']['date']) ? strtotime($element['#value']['date']) : 0) + (!empty($element['#value']['time']) ? (preg_replace('/^\s*(\d\d?):/',  '$1', $element['#value']['time']) * 3600) + (preg_replace('/:(\d\d?)\s*$/', '', $element['#value']['time']) * 60) : 0);
  }
  $element['date'] += array(
    '#size' => 10,
    '#default_value' => isset($element['#value']) && $element['#value'] !== '' && is_scalar($element['#value']) ? date('d.m.Y', $element['#value']) : NULL,
    '#access' => $element['#date'],
  );
  $element['date']['#type'] = 'textfield';
  $element['date']['#attributes']['class'][] = 'datetime-date';
  $element['date']['#attributes'] += array(
    'placeholder' => t('DD.MM.YYYY'),
  );


  $element['date']['#attributes']['data-min'] = $min ? date('d.m.Y', $max ? min($min, $max) : $min) : '';
  $element['date']['#attributes']['data-max'] = $max ? date('d.m.Y', $min ? max($min, $max) : $max) : '';
  

  $year_range = array('c-10', 'c+10');
  // @see http://api.jqueryui.com/datepicker/#option-yearRange
  if ($min) {
    $year_range[0] = date('Y', $max ? min($min, $max) : $min);
  }
  if ($max) {
    $year_range[1] = date('Y', $min ? max($min, $max) : $max);
  }
  
  $element['date']['#attributes']['data-year-range'] = implode(':', $year_range);
  $element['time'] += array(
    '#size' => 5,
    '#maxlength' => 5,
    '#default_value' => NULL,//isset($element['#value']) && $element['#value'] !== '' ? date('H', $element['#value']) . ':' . date('i', $element['#value']) : NULL,
    '#access' => $element['#time'],
  );
  if (isset($element['#value']) && is_scalar($element['#value']) && $element['#value'] !== '' && $element['#value'] !== FALSE) {
    $t = ($element['#value'] > 24 * 3600 ? 0 : strtotime('today')) + $element['#value'];
    $element['time']['#default_value'] = date('H', $t) . ':' . date('i', $t);
  }
  $element['time']['#type'] = 'textfield';
  $element['time']['#attributes']['class'][] = 'datetime-time';
  $element['time']['#attributes'] += array(
    'placeholder' => t('HH:MM'),
  );
  
  if (!empty($element['#ajax'])) {
    if ($element['#date']) {
      $element['date'] += array(
        '#ajax' => $element['#ajax'],
      );
    }
    $element['time'] += array(
      '#ajax' => $element['#ajax'],
    );
  }
  
  // Wrap two date+time sub-elements into inline container 
  if ($element['date']['#access'] && $element['time']['#access']) {
    $element['date'] += array('#prefix' => '',);
    $element['date']['#prefix'] = '<div class="container-inline">' . $element['date']['#prefix'];
    
    $element['time'] += array('#suffix' => '',);
    $element['time']['#suffix'] .= '</div>';
  }
  
  return $element;
}


function addons_validate_datetime(&$element, &$form_state) {
  $value = NULL;

  if ($element['#date']) {
    if (($element['date']['#value'] = preg_replace('/\s/u', '', $element['date']['#value'])) !== '') {
      if (!preg_match('/(\d\d?)\.(\d\d?).(\d\d\d?\d?)/', $element['date']['#value'], $m) || !checkdate($m[2], $m[1], $m[3])) {
        form_error($element['date'], t('Invalid date format in !name field.', array('!name' => '<strong>"' . $element['#title'] . '"</strong>')));
      }
      else {
        $value += strtotime($m[1] . '.' . $m[2] . '.' . $m[3]); // 31.5.1940
      }
    }
  }

  if ($element['#time']) {
    if (($element['time']['#value'] = preg_replace('/\s/u', '', $element['time']['#value'])) !== '') {
      if (!preg_match('/^(\d\d?)\:(\d\d?)$/', $element['time']['#value'], $m) || $m[1] > 23 || $m[2] > 59) {
        form_error($element['time'], t('Invalid time in !name field.', array('!name' => '<strong>"' . $element['#title'] . '"</strong>')));
      }
      else {
        $value += ($m[1] * 3600) + ($m[2] * 60);
      }
    }
  }
  if (isset($value)) { // We use isset() instead of simple if() condition because #required and #min/max properties must be independent.
    if ($element['#min']) {
      $min = ctype_digit($element['#min'] . '') ? $element['#min'] : strtotime($element['#min']);
      if ($min && $value < $min) {
        form_error($element, t('Date/time in !name field must be later or equal to @val.', array('!name' => '<strong>"' . $element['#title'] . '"</strong>', '@val' => date(($element['#date'] ? 'd.m.Y' : '') . ' ' . ($element['#time'] ? 'H:i' : ''), $min))));
      }      
    }
    if ($element['#max']) {
      $max = ctype_digit($element['#max'] . '') ? $element['#max'] : strtotime($element['#max']);
      if ($max && $value > $max) {
        form_error($element, t('Date/time in !name field must be before or equal to @val.', array('!name' => '<strong>"' . $element['#title'] . '"</strong>', '@val' => date(($element['#date'] ? 'd.m.Y' : '') . ' ' . ($element['#time'] ? 'H:i' : ''), $max))));
      }      
    }

  }

  if (!empty($element['#return_format'])) {
    $value = date($element['#return_format'], $value);
  }
  form_set_value($element, $value, $form_state);
  $element['#value'] = $value;  
  
}

















