<?php

/**
 * @see doc_import_preset_form()
 */
function _node_specifications_form_doc_import_preset_form_alter(&$form, &$form_state, $form_id) {
	$preset = $form['#preset'];

	$node_type = isset($form_state['values']['node_type'])
		? $form_state['values']['node_type']
		: ($preset ? $preset->node_type : NULL);
	if (!$node_type) {
		return;
	}

	$form['settings']['specifications'] = array(
		'#tree' => TRUE,
		'#type' => 'fieldset',
		'#title' => t('Data distribution for specifications fields'),
		'#weight' => 10,
	);

	$options = isset($form_state['#table_header'])
		? $form_state['#table_header']
		: (isset($preset->settings['table_header']) ? $preset->settings['table_header'] : array());

	if (!empty($preset->settings['first_line']) || !empty($form_state['values']['file_settings']['first_line'])) {
		foreach ($options as $col_num => $col_text) {
			$options[$col_num] = '[' . utils_num_to_alpha($col_num) . '] ' . $col_text;
		}
	}

	foreach (node_specifications_load_multiple(FALSE, array('node_type' => $node_type)) as $nsid => $specification) {
		if (!empty($specification->fields)) {
      $form['settings']['specifications'][$nsid] = array(
        '#type' => 'fieldset',
        '#title' => $specification->name,
      );

      foreach ($specification->fields as $delta => $field) {
       //if ($field['field_type'] == 'logic') {
       //  continue;
       //}

        if ($field['field_type'] === 'color_select') {
          // provide 2 selects for color field type
          $form['settings']['specifications'][$nsid][$delta] = array(
            '#type' => 'item',
            '#title' => $field['title'],
            '#attributes' => array('class' => array('container-inline')),
          );

          $form['settings']['specifications'][$nsid][$delta]['color'] = array(
            '#type' => 'select',
            '#title' => t('Color'),
            '#empty_value' => '',
            '#options' => $options,
            '#default_value' => isset($preset->settings['specifications'][$nsid][$delta]['color'])
                ? $preset->settings['specifications'][$nsid][$delta]['color']
                : '',
          );

          $form['settings']['specifications'][$nsid][$delta]['color_description'] = array(
            '#type' => 'select',
            '#title' => t('Color description'),
            '#empty_value' => '',
            '#options' => $options,
            '#default_value' => isset($preset->settings['specifications'][$nsid][$delta]['color_description'])
                ? $preset->settings['specifications'][$nsid][$delta]['color_description']
                : '',
          );
        }
        else {
          $form['settings']['specifications'][$nsid][$delta] = array(
            '#type' => 'select',
            '#title' => $field['title'],
            '#empty_value' => '',
            '#prefix' => '<div class="container-inline">',
            '#suffix' => '</div>',
            '#options' => $options,
            '#default_value' => isset($preset->settings['specifications'][$nsid][$delta])
                ? $preset->settings['specifications'][$nsid][$delta]
                : '',
          );
        }
      }
		}
	}
}

/**
 * @see _doc_import_parse_part()
 */
function _node_specifications_doc_import($node) {
	if (empty($node->doc_import_preset->settings['specifications'])) {
		return;
	}
	/** @var array $line */
	$line = $node->doc_import_csv;
	if (empty($line)) {
		return;
	}
	$data = array();
	foreach ($node->doc_import_preset->settings['specifications'] as $nsid => $field_list) {
		foreach ($field_list as $delta => $col_num) {
			if (!$col_num) {
				continue;
			}
			$spec_field = node_specifications_load_field($nsid, $delta);
			if (!$spec_field) {
				continue;
			}

			$var_id = NULL;
			if ($spec_field['field_type'] === 'color_select') {
				// color_select special case
				$color = !empty($col_num['color']) ? _node_specifications_doc_import_string_extract($col_num['color'], $line) : '';
				$color_description = !empty($col_num['color_description']) ? _node_specifications_doc_import_string_extract($col_num['color_description'], $line) : '';
				if (empty($color)) {
					continue;
				}

				$var_id = _node_specifications_doc_import_find_variant($spec_field['vars'], $color);
				// add new variant if it doesn't exist
				if (empty($var_id)) {
					$params = array(
						'nsid' => $nsid,
						'delta' => $delta,
						'var_value' => $color,
						'var_data' => $color_description,
					);
					$var_id = _node_specifications_doc_import_create_variant($params, $nsid, $delta);
				}
			}
			else {
				$str = _node_specifications_doc_import_string_extract($col_num, $line);
				
        if (in_array($spec_field['field_type'], array('number_decimal', 'number_float', 'number_integer'))) {
          $str = preg_replace('/[^\d,\.]/u', '', $str); // 'QWER TY123.45..6,789' --> '123.45..6,789'
          $str = preg_replace('/[,\.]+/u', '.', $str); // '123.45..6,789' -->> '123.45.6.789'

// INVALID:$str = preg_replace('/^(\d+(\.\d+)?)\..*$/u', '$1', $str); // '123.456.789' -->> '123.456'
           $str = preg_replace('/^(\d+)(\.\d+)?(.*?)$/u', '$1$2', $str); // '123.456.789' -->> '123.456'
        }
        elseif ($spec_field['field_type'] === 'logic') {
          $str = trim($str);
          $str = $str ? 2 : 1; // TODO: Re-factor wrong logic 1 == YES; 2 == NO.
        }
        if ($str) {
        // if specification field is variative
          if (!empty($spec_field['vars'])) {
            $var_id = _node_specifications_doc_import_find_variant($spec_field['vars'], $str);
            // add new variant if it doesn't exist
            if (empty($var_id)) {
              $params = array(
                'nsid' => $nsid,
                'delta' => $delta,
                'var_value' => $str,
              );
              $var_id = _node_specifications_doc_import_create_variant($params, $nsid, $delta);
            }
          }        
        }
        $var_id = $var_id ? $var_id : $str;  
			}

			$data[$nsid][$delta] = $var_id;
			unset($spec_field);
		}
	}

	if (!empty($data)) {
		$node->specifications_data = $data;
	}
}

function _node_specifications_doc_import_string_extract($position, array $strings) {
	return preg_replace('/\s+/u', ' ', trim(strtr($position, $strings)));
}

function _node_specifications_doc_import_find_variant($variants, $value) {
	$var_id = NULL;

	foreach ($variants as $variant) {
		if ($variant['var_value'] == $value) {
			$var_id = $variant['var_id'];
		}
	}

	return $var_id;
}

function _node_specifications_doc_import_create_variant(array $params, $nsid, $delta) {
  drupal_static_reset('node_specifications_save_field_variants');
	$spec_field = node_specifications_load_field($nsid, $delta);
	$spec_field['vars'][] = $params;
	$updated_variants = node_specifications_save_field_variants($spec_field['vars'], $nsid, $delta);

	return _node_specifications_doc_import_find_variant($updated_variants, $params['var_value']);
}
