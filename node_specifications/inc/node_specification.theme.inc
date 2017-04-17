<?php

function theme_node_specifications_block_configure_form($variables) {
	$form = $variables['form'];
	$rows = array();

	foreach (element_children($form) as $key) {
		if (!isset($form[$key]['#type'])) {
			$field = &$form[$key];
			$row = array(
				'data' => array(
					array(
						'data' => drupal_render($field['name']),
					),
					array(
						'data'  => drupal_render($field['show']),
						'align' => 'center'
					),
					array(
						'data'  => drupal_render($field['expanded']),
						'align' => 'center'
					),
				),
			);
			if (strstr($key, '/')) {
				$row['data'][0]['style'] = 'padding-left:1em;';
			}

			$rows[$key] = $row;
		}
	}

	$header = array(t('Element name'), t('Show'), t('Expand'));
	$output = theme(
		'table', array(
		'header' => $header,
		'rows'   => $rows,
	)
	);
	$output .= drupal_render_children($form);

	return $output;
}

function theme_node_specifications_fields_overview($vars) {
	$output = '';

	$form = $vars['form'];
	drupal_add_tabledrag('ns-field-list', 'order', 'sibling', 'field-weight');

	$rows = array();
	if (!empty($form['specification_fields'])) {
		foreach (element_children($form['specification_fields']) as $delta) {
			$field = &$form['specification_fields'][$delta];
			$field['weight']['#attributes']['class'] = array('field-weight');

			$rows['_' . $delta]['data'] = array();
			$rows['_' . $delta]['class'][] = 'draggable';
			foreach (element_children($field) as $element) {
				$rows['_' . $delta]['data'][$element] = render($field[$element]);
			}
		}
	}

	$output .= drupal_render($form['specification_info']);
	$output .= theme(
		'table', array(
		'header' => $form['#header'],
		'rows' => $rows,
		'attributes' => array('id' => 'ns-field-list'),
		'empty' => t('specification fields is empty'),
	)
	);
	$output .= drupal_render_children($form);

	return $output;
}

function theme_node_specifications_filter_presets_form($variables) {
	$form = $variables['form'];
	$rows = array();
	foreach (element_children($form['filter_presets']) as $id) {
		$form['filter_presets'][$id]['weight']['#attributes']['class'] = array('example-item-weight');
		$rows[] = array(
			'data'  => array(
				drupal_render($form['filter_presets'][$id]['title']),
				drupal_render($form['filter_presets'][$id]['actions']),
				drupal_render($form['filter_presets'][$id]['weight']),
			),
			'class' => array('draggable'),
		);
	}

	$table_id = 'node-specifications-filter-presets-table';
	$output = theme(
		'table', array(
		'header'     => array(t('Title'), t('Actions'), t('Weight')),
		'rows'       => $rows,
		'attributes' => array('id' => $table_id),
	)
	);

	$output .= drupal_render_children($form);
	drupal_add_tabledrag($table_id, 'order', 'sibling', 'example-item-weight');

	return $output;
}


function theme_node_specifications_field($variables) {
  $variables['field']['#children'] = drupal_render_children($variables['field']);
  $node = $variables['field']['#node'];
  $nsid = $variables['field']['#nsid'];
  $delta = $variables['field']['#delta'];
  // #label_display is deprecated, use #title_display
  $title_display = isset($variables['field']['#title_display']) ? $variables['field']['#title_display'] : (isset($variables['field']['#label_display']) ? $variables['field']['#label_display'] : NULL);
  $label = isset($variables['field']['#title']) ? $variables['field']['#title'] : (isset($variables['field']['#label']) ? $variables['field']['#label'] : NULL);
  if (empty($node->specifications[$nsid]->fields[$delta])) {
    return '';
  }
  $field = $node->specifications[$nsid]->fields[$delta];
  $field_vars = !empty($field['vars']) ? $field['vars'] : array();
  $markup = '';
  $values = '';
  if ($variables['field']['#children']) {
    $markup = $variables['field']['#children'];
  }
  else {
    // field data
    if (!empty($node->specifications_data[$nsid][$delta])) {
      $field_data = $node->specifications_data[$nsid][$delta];
      if (in_array($field['field_type'], array(
          'color_select',
          'list_text',
          'image'
        )) && !is_array($field_data)
      ) {
        $field_data = array($field_data => $field_data);
      }
      $field_markup = array();
      $field_values = array();
      $unit = isset($field['unit_id']) ? ' ' . t($field['unit_symbol']) : '';
      switch ($field['field_type']) {
        case 'color_select':
          foreach ($field_data as $var_id) {
            if (isset($field_vars[$var_id]['var_value'])) {
              
              $var_value = check_plain($field_vars[$var_id]['var_data'] ? $field_vars[$var_id]['var_data'] : $field_vars[$var_id]['var_value']);
              $field_values[] = $var_value;
              $field_markup[] = '<div style="border-left: #' . $var_value . ' solid 1em; padding-left: 0.4em;">' . (isset($field_vars[$var_id]['var_data']) ? check_plain($field_vars[$var_id]['var_data']) : '') . '</div>';
            }
          }
          break;

        case 'image':
          foreach ($field_data as $var_id) {
            if (!empty($field_vars[$var_id]['var_value']) && ($file = file_load($field_vars[$var_id]['var_value']))) {
              $var_value = check_plain($field_vars[$var_id]['var_data']);
              $field_values[] = $var_value;
              $field_markup[] = '<div><img class="node-specification-image-field" src="' . file_create_url($file->uri) . '"/>' . (isset($field_vars[$var_id]['var_data']) ? $var_value : '') . '</div>';
            }
          }
          break;

        case 'list_text':
          foreach ($field_data as $var_id) {
            if (isset($field_vars[$var_id]['var_value'])) {
              $var_value = check_plain($field_vars[$var_id]['var_value']);
              $field_values[] = $var_value;
              $field_markup[] = '<div>' . $var_value . $unit . '</div>';
            }
          }
          break;

        case 'logic':
          if ($title_display === 'hidden') {
            if ($field_data == 2) {
              $field_values[] = $label;
              $field_markup[] = $label;
            }
          }
          else {
            $var_value = $field_data == 1 ? t('No') : t('Yes');
            $field_values[] = $var_value;
            $field_markup[] = $var_value;
          }
          break;

        default:
          $var_value = check_plain($field_data) . $unit;
          $field_values[] = $var_value;
          $field_markup[] = $var_value;
          break;
      }
      $markup = implode(', ', $field_markup);
      $values = implode(', ', $field_values);
    }
  }
  if ($markup) {
    $markup = '<div class="data"' . ($title_display === 'hidden' && $label ? ' title="' . check_plain($label) . '"' : '') . '>' . $markup . '</div>';

    $module_path = drupal_get_path('module', 'node_specifications');
    drupal_add_css($module_path . '/css/node_specifications.field.css');
    drupal_add_js($module_path . '/js/node_specifications.hint.js');

    $element_title = $label && $title_display !== 'hidden' ? $label : '';

    $element = array(
      '#title' => $element_title,
      '#type' => 'item',
      '#inline' => $title_display === 'inline',
      '#markup' => $markup,
    );

    if (!$element_title) {
      $element['#title'] = $values;
      $element['#markup'] = '';
    }

    if (!empty($variables['field']['#attributes'])) {
      $element['#attributes'] = $variables['field']['#attributes'];
    }
    $element['#attributes']['class'][] = 'node-specification-field';
    $element['#attributes']['class'][] = drupal_html_class($field['field_type']);
    // field hint
    if (!empty($field['hint'])) {
      $element['#title'] = '<span class="hint hidden">' . filter_xss_admin($field['hint']) . '</span>' . $element['#title'];
    }

    if (!empty($field['hint'])) {
      $element['#attributes']['class'][] = 'hint';
    }

    return drupal_render($element);
  }
  return '';
}


/**
 * Returns HTML for the node_specifications overview form as a sortable list of vocabularies.
 *
 * @param $variables
 *   An associative array containing:
 *   - form: A render element representing the form.
 * @see node_specifications_overview_form()
 */
function theme_node_specifications_overview_form($variables) {

  $form = $variables['form'];
  $rows = array();

  foreach (element_children($form) as $key) {
    if (isset($form[$key]['name'])) {
      $specif = &$form[$key];
      $row = array();
      $row[] = drupal_render($specif['name']);
      $row[] = drupal_render($specif['uses']);
      $row[] = drupal_render($specif['description']);
      $row[] = drupal_render($specif['operations']);
      if (isset($specif['weight'])) {
        $specif['weight']['#attributes']['class'] = array('vocabulary-weight');
        $row[] = drupal_render($specif['weight']);
      }
      $rows[] = array('data' => $row, 'class' => array('draggable'));
    }
  }
  $header = array('name' => t('Name'), 'uses'=> t('Uses'), 'description' => t('Description'), 'data' => t('Operations'));
  if (isset($form['actions'])) {
    $header[] = t('Weight');
    drupal_add_tabledrag('taxonomy', 'order', 'sibling', 'vocabulary-weight');
  }
  if ($rows) {
    return theme('table', array(
      'header' => $header,
      'rows' => $rows,
      //'empty' => t('There are no specifications set. <a href="@link">Add specification set</a>.', array('@link' => url('admin/structure/node_specifications/add'))),
      'attributes' => array('id' => 'taxonomy'),
    )) . drupal_render_children($form);
  }
  else {
    return '';
  }
}


















