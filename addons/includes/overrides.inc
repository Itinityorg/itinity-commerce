<?php


/**
* Implements hook_preprocess_user_picture()
*/
function addons_preprocess_user_picture(&$variables) {
	if (!empty($variables['user_picture'])) {
		// Get classes from themed username (skip username* class) and apply its onto own wrapper span
		$classes = '';
		if (!empty($variables['account'])) {
			$username = theme('username', array('account' => $variables['account']));
			if (preg_match('/<[a-z]+(?:\s+|(?:\s(?:[^>]*?)(?:\s|\"|\')))class=(\"|\')([a-z0-9\-_\s]*username[a-z0-9\-_\s]*?)\1/i', $username, $m)) {
				$classes .= ' ' . $m[2];
			}
			$classes = preg_replace('/(^|\s)username[\-_a-z]*/', '', $classes);
		}
		$variables['user_picture'] = '<span class="user-picture-wrapper ' . $classes . '">' . $variables['user_picture'] . '</span>';
	}
}


/**
* container type preprocess @see addons_theme_registry_alter()
*/
function _addons_preprocess_container(&$variables, $hook) {
  // Implementation of #inline property
  if (!empty($variables['element']['#inline'])) {
    if (isset($variables['element']['#attributes']['class']) && is_scalar($variables['element']['#attributes']['class'])) {
      $variables['element']['#attributes']['class'] = array($variables['element']['#attributes']['class']);
    }
    $variables['element']['#attributes']['class'][] = 'container-inline';
  }
}


/**
* Override for theme_form_element() @see addons_theme_registry_alter()
*/
function _addons_theme_form_element($variables) {
// Overrided code from form.inc -->> theme_form_element():
/* 4205*/  $element = &$variables['element'];
/* 4206*/  
/* 4207*/  // This function is invoked as theme wrapper, but the rendered form element
/* 4208*/  // may not necessarily have been processed by form_builder().
/* 4209*/  $element += array(
/* 4210*/    '#title_display' => 'before',
/* 4211*/  );
/* 4212*/  
/* 4213*/  // Add element #id for #type 'item'.
/* 4214*/  if (isset($element['#markup']) && !empty($element['#id'])) {
/* 4215*/    $attributes['id'] = $element['#id'];
/* 4216*/  }
/* 4217*/  // Add element's #type and #name as class to aid with JS/CSS selectors.
/* 4218*/  $attributes['class'] = array('form-item');
/* 4219*/  if (!empty($element['#type'])) {
/* 4220*/    $attributes['class'][] = 'form-type-' . strtr($element['#type'], '_', '-');
/* 4221*/  }
/* 4222*/  if (!empty($element['#name'])) {
/* 4223*/    $attributes['class'][] = 'form-item-' . strtr($element['#name'], array(' ' => '-', '_' => '-', '[' => '-', ']' => ''));
/* 4224*/  }
/* 4225*/  // Add a class for disabled elements to facilitate cross-browser styling.
/* 4225*/  if (!empty($element['#attributes']['disabled'])) {
/* 4227*/    $attributes['class'][] = 'form-disabled';
/* 4228*/  }

    if (!empty($element['#type'])) {
      // Add full attributes support for 'item' type
      if ($element['#type'] === 'item') {
        if (!empty($element['#attributes'])) {
          foreach ($element['#attributes'] as $attr => $val) {
            if ($attr === 'class') {
              if (!is_array($val)) {
                $val = preg_split('/\s+/u', $val);
              }
              $attributes['class'] = array_merge($attributes['class'], $val);
            }
            else {
              $attributes[$attr] = $val;
            }
          }
        }
      }
    }
    // Add support #inline property
    if (!empty($element['#inline'])) {
      $attributes['class'][] = 'container-inline';
    }
    else {
      $element['#inline'] = !isset($element['#inline']) && in_array('container-inline', $attributes['class'], TRUE);
    }
    // Add colon after title label in inline elements
    if ($element['#inline'] && isset($element['#title']) && $element['#title'] !== '' && (!isset($element['#title_display']) || in_array($element['#title_display'], array('before', 'invisible'), TRUE)) && !preg_match('/:\s*$/u', $element['#title'])) {
      $element['#title'] .= ':';
    }
    // fix duplicated and empty classes
    $attributes['class'] = array_values(drupal_map_assoc(array_filter(preg_split('/\s+/u', implode(' ', $attributes['class'])), 'trim')));

    
    if ($element['#title_display'] !== 'none' && isset($element['#title']) && $element['#title'] !== '' && $element['#title_display'] !== 'invisible' && $element['#title_display'] !== 'attribute') {
      $attributes['class'][] = 'has-label';
    }
    
    if ($element['#title_display'] === 'before' && isset($element['#title'])) {
      $attributes['class'][] = 'form-item-label-above';
    }

    // Add 'checked' class to checked radio button div wrappers
    if (!empty($element['#type']) && ($element['#type'] == 'radio')) {
      if (!empty($element['#default_value']) && !empty($element['#return_value'])) {
        if ($element['#default_value'] == $element['#return_value']) {
          $attributes['class'][] = 'checked';
        }
      }
    }
    
/*4229*/  $output = '<div' . drupal_attributes($attributes) . '>' . "\n";
/*4230*/  
/*4231*/  // If #title is not set, we don't display any label or required marker.
/*4232*/  if (!isset($element['#title'])) {
/*4233*/    $element['#title_display'] = 'none';
/*4234*/  }
/*4235*/  $prefix = isset($element['#field_prefix']) ? '<span class="field-prefix">' . $element['#field_prefix'] . '</span> ' : '';
/*4236*/  $suffix = isset($element['#field_suffix']) ? ' <span class="field-suffix">' . $element['#field_suffix'] . '</span>' : '';
/*4237*/  
/*4238*/  switch ($element['#title_display']) {
/*4239*/    case 'before':
/*4240*/    case 'invisible':
/*4241*/      $output .= ' ' . theme('form_element_label', $variables);
/*4242*/      $output .= ' ' . $prefix . $element['#children'] . $suffix . "\n";
/*4243*/      break;
/*4244*/  
/*4245*/    case 'after':
/*4246*/      $output .= ' ' . $prefix . $element['#children'] . $suffix;
/*4247*/      $output .= ' ' . theme('form_element_label', $variables) . "\n";
/*4248*/      break;
/*4249*/  
/*4250*/    case 'none':
/*4251*/    case 'attribute':
/*4252*/      // Output no label and no required marker, only the children.
/*4253*/      $output .= ' ' . $prefix . $element['#children'] . $suffix . "\n";
/*4254*/      break;
/*4255*/  }
/*4256*/  
/*4257*/  if (!empty($element['#description'])) {
/*4258*/    $output .= '<div class="description">' . $element['#description'] . "</div>\n";
/*4259*/  }
/*4260*/  
/*4261*/  //$output .= "</div>\n";
            
            $output .= "</div>"; // No \n, fix space after div
            
/* 4144*/ 
/* 4145*/   return $output;
}


/**
* Pre renderer for form elemens @see addons_element_info_alter()
*/
function _addons_pre_render_form_element($element) {
	if (!empty($element['#type'])) {
		if ($element['#type'] === 'managed_file') {
			if (!empty($element['#upload_validators']['file_validate_extensions'])) {
				// Add 'accept' attribute to input type="file"
				if (!empty($element['upload']['#type']) && $element['upload']['#type'] === 'file' && empty($element['upload']['#attributes']['accept'])) {
					$exts = array();
					foreach ($element['#upload_validators']['file_validate_extensions'] as $item) {
						$exts += drupal_map_assoc(array_filter(preg_split('/\s+/', $item)));
					}
					$mime = array();
					foreach ($exts as $ext) {
						$mime[] = file_get_mimetype('.' . $ext);
					}
					if ($mime) {
						$element['upload']['#attributes']['accept'] = implode(',', drupal_map_assoc($mime));
					}
				}
			}
		}
		if ($element['#type'] === 'item') {
			if (!empty($element['#prefix'])) {
				if (preg_match('/^\s*<div\s+class\s*=\s*(\"|\')([a-z0-9_\-\s]+)\1\s*>\s*$/iu', $element['#prefix'])) {
          // Try to optimize output html - compact <div class="abc"><div class="def">...</div></div> to <div class="abc def">...</div>
          if ($classes = preg_split('/\s+/u', preg_replace('/^\s*<div\s+class\s*=\s*(\"|\')([a-z0-9_\-\s]+)\1\s*>\s*$/iu', '$2',  $element['#prefix']))) {
            if (!empty($element['#suffix']) && preg_match('/^\s*<\/div>\s*$/u', $element['#suffix'])) {
              $existing_classes = !empty($element['#attributes']['class']) ? $element['#attributes']['class'] : array();
              $existing_classes = is_scalar($existing_classes) ? preg_split('/\s+/u', $existing_classes) : $existing_classes;
              $element['#attributes']['class'] =  array_unique(array_merge($existing_classes, $classes));//drupal_map_assoc($existing_classes) + drupal_map_assoc($classes);
              unset($element['#prefix'], $element['#suffix']);
              utils_html_debug_element($element, 'Two nested div\'s was collapsed to one.');
            }            
          }
          // Add colon after title label in inline elements
          if (empty($element['#inline']) && isset($element['#title']) && $element['#title'] !== '' && !preg_match('/:\s*$/u', $element['#title']) && in_array('container-inline', $classes, TRUE) && !preg_match('/:\s*$/u', $element['#title'])) {
            $element['#title'] .= ':';
          }
				}
			}
		}
	}
	return $element;
}



/**
* Process callback for 'text_format' element
*/
function _addons_process_text_format($element, &$form_state, &$complete_form) {
	// Implementation of own '#summary' property.
	// Convert 'value' textarea from one to two 'summary' and 'value' fields, like as node body standard field.
	if (!empty($element['#summary'])) {
		// Get summary and full texts by special HTML splitter
		$full = preg_split('/<!--break-->/i', $element['#default_value']);
		if ($summary = count($full) > 1 ? array_shift($full) : '') {
			$element['value']['#default_value'] = implode('', $full);
		}
		// C/P form text_field_widget_form() -> case 'text_textarea_with_summary':
		$element['summary'] = array(
			'#type' => 'textarea',
			'#default_value' => $summary,
			'#title' => t('Summary'),
			'#rows' => isset($element['#rows']) && $element['#rows'] > 5 ? round($element['#rows'] / 2) : 3,
			'#description' => t('Leave blank to use trimmed value of full text as the summary.'),
			'#attached' => array(
				'js' => array(drupal_get_path('module', 'text') . '/text.js'),
			),
			'#attributes' => array('class' => array('text-summary')),
			'#prefix' => '<div class="text-summary-wrapper">',
			'#suffix' => '</div>',
			'#weight' => -10,
		);
		$element['value']['#attributes']['class'][] = 'text-full';

		$element += array(
			'#prefix' => '',
			'#suffix' => '',
			'#element_validate' => array()
		);
		$element['#prefix'] .= '<div class="field-type-text-with-summary">'; // See var $widget = $(this).closest('div.field-type-text-with-summary'); in modules/field/text/text.js
		$element['#suffix'] = '</div>' . module_invoke('utils', 'html_debug') . $element['#suffix'];
		array_unshift($element['#element_validate'], '_addons_text_format_summary_value_validate');
	}
	return $element;
}



/**
* Additional validation for form_text element with enabled #summary property
*/
function _addons_text_format_summary_value_validate(&$element, &$form_state) {
	// Before saving we combine element text from 'summary' and 'value' fields into one 'value' field using <!--break--> splitter
	form_set_value(
		$element['value'],
		implode('<!--break-->', array_filter(array(
			trim(str_ireplace('<!--break-->', "\r\n", $element['summary']['#value'])),
			trim(str_ireplace('<!--break-->', "\r\n", $element['value']['#value'])),
		))),
		$form_state
	);
}




/**
* Process callback for 'fieldset' element
*/
function _addons_process_fieldset($element, &$form_state, &$complete_form) {

	// Implementation of own #use_tabs property.
	// Make fieldsets (and nested fieldsets) like standard drupal tabs. @see js_behaviors/fieldset_tabs.js
	if (!empty($element['#use_tabs']) && empty($element['#_tabs_processed'])) {
		$element['#_tabs_processed'] = TRUE;
		$element['#attached']['library'][] = array('system', 'jquery.cookie');
		$element['#attached']['js'][] = drupal_get_path('module', 'addons') . '/js_behaviors/fieldset_tabs.js';
		$parents = isset($element['#tab_parents']) ? $element['#tab_parents'] : $element['#array_parents'];
		array_pop($parents);
		$parent_element = &drupal_array_get_nested_value($complete_form, $parents);
		$classes = array('tab-wrapper');
		$id = $complete_form['form_id']['#value'] . '-' . str_replace('_', '-', implode('-', $element['#array_parents']));
		$complete_form += array(
			'__active_fieldset_tab_detector__' => array(
				'#type' => 'hidden',
				'#name' => '__active_fieldset_tab_detector__',
				'#attributes' => array(
					'class' => array('__active_fieldset_tab_detector__'),
				),
			),
		);
		if (!empty($form_state['input']['__active_fieldset_tab_detector__']) && $form_state['input']['__active_fieldset_tab_detector__'] === $id) {
			$active = TRUE;
		} else {
			$active = !empty($element['#active']);
		}

		if ($active) {
			$classes[] = 'active';
		}

		if (!isset($parent_element['_fieldset_tabs'])) {
			$parent_element = array(
				'_fieldset_tabs' => array(
					'#theme' => 'links',
					'#links' => array(),
					'#attributes' => array(
						'class' => array('js-visible', 'element-tabs', 'tabs', 'primary',),
					),
					'#weight' => $element['#weight'] - 0.00001,
				),
			) + $parent_element;
			unset($parent_element['#sorted']);

		} else {
			$classes[] = 'js-invisible';
		}

		$parent_element['_fieldset_tabs']['#links']['tab-link-' . count($parent_element['_fieldset_tabs']['#links'])] = array(
			'title' => '<a id="' . $id . '-link" href="#' . $id . '-tab">' . check_plain(strip_tags($element['#title'])) . '</a>',
			'html' => TRUE,
		);
		$element += array(
			'#prefix' => '',
			'#suffix' => '',
		);

		$element['#prefix'] = '<div class="' . implode(' ', $classes) . '" id="' . $id . '-tab-wrapper">' . $element['#prefix'];
		$element['#suffix'] .= '</div>';
		$element['#attributes']['class'][] = 'tabbed';
		$element['#collapsible'] = FALSE;
	}
	return $element;
}



/**
* Process callback for 'textarea' element
*/
function _addons_process_textarea($element, &$form_state, &$complete_form) {
	// Implementation of own #editarea property.
	// Attach visual code editor plugin to textarea
	if (!empty($element['#editarea']) && (!isset($element['#access']) || $element['#access'])) {
		$element += array(
			'#editarea' => array(),
		);
		$element['#editarea'] += array(
			'syntax' => 'php',
			'linenum' => 0,
		);
		$langs_scripts = array(
			'php' => array(
					'xml', 'clike', 'javascript', 'php',
					),
		);
		$langs_scripts += array($element['#editarea']['syntax'] => array($element['#editarea']['syntax']));
		static $lib_added = FALSE;
		if (!$lib_added) {
			$lib_added = TRUE;
			$codemirror = 'codemirror-2.34';
			drupal_add_css(drupal_get_path('module', 'addons').'/lib/' . $codemirror . '/lib/codemirror.css');
			drupal_add_js(drupal_get_path('module', 'addons').'/lib/' . $codemirror . '/lib/codemirror-compressed.js', array('cache' => FALSE,)); // suppress js-caching, since drupal aggregation broke editarea script
			drupal_add_js(drupal_get_path('module', 'addons').'/js_behaviors/CodeMirror.js', array('scope' => 'footer', 'cache' => FALSE,));

			foreach ($langs_scripts[$element['#editarea']['syntax']] as $scr) {
				drupal_add_js(drupal_get_path('module', 'addons').'/lib/' . $codemirror . '/mode/' . $scr. '/' . $scr.'.js', array('cache' => FALSE,));
			}

		}



		$setting = array(
			'lineNumbers' => 1,
			'matchBrackets' => TRUE,
			'syntax' => $element['#editarea']['syntax'],
			'indentWithTabs' => TRUE,
			'smartIndent' => FALSE,
			'readOnly' => !empty($element['#disabled']) || !empty($element['#attributes']['readonly']),
			'linenum' => $element['#editarea']['linenum'],
			);

		$element['#resizable'] = FALSE;
		$element['#attached']['js'][] = array(
			'type' => 'setting',
			'data' => array(
				'sCodeMirror' => array(
					$element['#id'] => $setting,
				),
			),
		);
	}
	if (!empty($element['#maxlength'])) {
		$element['#attributes']['class'][] = 'maxlength';
		$element['#attributes']['data-maxlength'] = $element['#maxlength'];
		$element['#attached']['js'][] = drupal_get_path('module', 'addons') . '/js_behaviors/addons.textarea.js';
		$remains = 0;
		if ($element['#value']) {
			$remains = $element['#maxlength'] - drupal_strlen($element['#value']);
			if ($remains >= 0) {
				$t = format_plural($remains, '1 symbol remains', '@count symbols remains');
			}
			else {
				$t = format_plural(-$remains, 'Please remove 1 extra character', 'Please remove @count extra characters');
			}
		}
		else {
			$t = format_plural($element['#maxlength'], '1 symbol remains', '@count symbols remains');
		}
		$element += array('#field_suffix' => '',);
		$element['#field_suffix'] = '<div class="maxlength-warning' . ($remains < 0 ? ' error' : '') . '"><small>' . $t . '</small></div>' . $element['#field_suffix'];
	}
	return $element;
}

/**
* Override for theme('username'). See addons_theme_registry_alter()
* Add and implements support for extra 'picture' parameter.
* Add and implements support for extra 'words_count' parameter.
* Usage: theme('username', array('account' => $account, 'picture' => TRUE))
*/
function _addons_theme_username_override($variables) {
  if (!$variables['profile_access'] && empty($variables['link_path']) && !user_access('access user profiles') && ($GLOBALS['user']->uid && $GLOBALS['user']->uid == $variables['account']->uid)) {
    $variables['profile_access'] = TRUE;
    $variables['link_path'] = 'user/' . $variables['uid'];
    $variables += array('link_options' => array());
    $variables['link_options'] += array('attributes' => array());
    $variables['link_options'] += array('title' => t('View user profile.'));
    $variables['link_options']['attributes']['class'][] = 'username';
  }
	$img = '';
	if (!empty($variables['picture'])) {
		if (variable_get('user_pictures', 0)) {
			// Get user picture and render small icon.
			$account = $variables['account'];
			if (!empty($account->picture)) {
				if (is_numeric($account->picture)) {
					$files = &drupal_static(__FUNCTION__ . ':files');
					if (!isset($files[$account->picture])) {
						$account->picture = $files[$account->picture] = file_load($account->picture);
					} else {
						$account->picture = $files[$account->picture];
					}

				}
				if (!empty($account->picture->uri)) {
					$filepath = $account->picture->uri;
				}
			}
			else {
				$filepath = variable_get('user_picture_default', '');
			}
			if (!empty($filepath)) {
				$alt = t("@user's picture", array('@user' => format_username($account)));
        $style = !empty($variables['picture_style']) ? $variables['picture_style'] : NULL;
        
        $style = $style ? $style : (module_exists('features') ? '56x42' : 'thumbnail');
        
        $img = theme('user_picture', array('account' => $account, 'style' => $style, 'height' => $style ? NULL : 32));
        $img = preg_replace('/\sclass="(.*?)"/u', ' class="$1 username-image"', $img);
				/*$img = theme('image', array(
					'path'   => $filepath,
					'alt'    => $alt,
					'title'  => $alt,
					'height' => '32',
					'attributes' => array(
						'class' => array('username-image'),
					),
				));*/
				if ($variables['profile_access']) {
					$opts = array('html' => TRUE) + $variables['link_options'];
					unset($opts['attributes']['class']);
					if (isset($variables['link_path'])) {
						$img = l($img, $variables['link_path'], $opts);
					}
				}
			}
		}
	}
	// Call default theme function, overrided and stored by addons_theme_registry_alter()
	$fn = !empty($variables['_original_func']) ? $variables['_original_func'] : 'theme_username';

	if (!empty($variables['words_count'])) {
		$variables['name'] = trim(preg_replace('/^\s*(([^\s]*\s+){' . $variables['words_count'] . '})(.*?)$/u', '$1', $variables['name']));
	}

	// Place user icon before standard themed username and wrap all in own span.
	$username = $fn($variables);
	$classes = 'username-wrapper' . ($img ? ' with-picture' : '');
	if (preg_match('/<' . ($variables['profile_access'] ? 'a' : 'span') . '+(?:\s+|(?:\s(?:[^>]*?)(?:\s|\"|\')))class=(\"|\')([a-z0-9\-_\s]*username[a-z0-9\-_\s]*?)\1/i', $username, $m)) {
		$classes .= ' ' . $m[2];
	}
	$classes = drupal_map_assoc(preg_split('/\s+/', $classes));
	unset($classes['username']);
	$classes = implode(' ', $classes); // See also addons.css and who module
	// Todo : merge this who + addons logic
	$html = '<span class="' . $classes . '">' . $img . ' ' . $username . '</span>';
	return $html . module_invoke('utils', 'html_debug');
}



function _addons_insert_element_to_parent(&$parent_element, $new_element_name, $new_element, &$form_state, &$complete_form) {
	$parent_element_clone = $parent_element;
	foreach (element_children($parent_element_clone) as $key) {
		if ($parent_element['#array_parents'] || !in_array($key, array('form_build_id', 'form_token', 'form_id',))) {
			unset($parent_element_clone[$key]);
		}
	}
	$parent_element_clone[$new_element_name] = $new_element;
	$submitted = $form_state['submitted'];
	form_builder($complete_form['#form_id'], $parent_element_clone, $form_state);
	$form_state['submitted'] = $submitted;
	$parent_element[$new_element_name] = $parent_element_clone[$new_element_name];
}

/** Selects processing **/

/**
* @see addons_element_info()
*/
function _addons_process_select($element, &$form_state, &$complete_form) {
	
	if (!empty($element['#mode']) && $element['#mode'] === 'slider' && empty($element['#multiple'])) {
		$element['#attached']['library'][] = array('system', 'ui.slider');
		$element['#attached']['js'][] = drupal_get_path('module', 'addons') . '/lib/selectToUISlider/selectToUISlider.jQuery.js';
		$element['#attached']['css'][] = drupal_get_path('module', 'addons') . '/lib/selectToUISlider/ui.slider.extras.css';
		$element['#attached']['js'][] = drupal_get_path('module', 'addons') . '/js_behaviors/selectToUISlider.js';
		$element['#attributes']['class'][] = 'mode-slider';
		$element['#attributes']['class'][] = 'element-hidden';
		if(!empty($element['#hide-scale'])){
			$element['#attributes']['hide-scale'] = 1;
		}
		utils_html_debug_element($element);
	}
	else {
		/*
			Handle #hierarchy_prefix element property. Makes multiple dependent ajax selects from one select that contains tree-like #options array.
			Required #hierarchy_prefix property must contains part of string in option title that conditional indicates  depth of item in tree.
			Example : $element['#options'] = array(0 => '<None>', 1 => '1', 2 => '-- 2', 3 => '---- 3'). This is reflection of next hierarchy: array(1 => array(2 => array(3 => array())))
			So, for using hierarchical selection in this case, you must define '#hierarchy_prefix' property as '--'.
			NOTE: 	You can define '#empty_option_key' to specify what key in options must be skipped from hierarchy processing and used as empty key in all generated sub-selects. 
								This can be required where 'none' option have non empty value
								e.g. term referrence field contains option '_none' => ' - None - '
		*/
		
		$original_element = $element;
		$array_parents = $element['#array_parents'];
		array_pop($array_parents);
		$parent_element = &drupal_array_get_nested_value($complete_form, $array_parents);
		$trail = array();
		if ((!isset($element['#access']) || $element['#access']) && empty($element['#multiple']) && !empty($element['#hierarchy_prefix']) && empty($parent_element['#_hierarchical_select_processed']) && empty($element['#_hierarchical_select_processed'])) {
			$parent_element['#_hierarchical_select_processed'] = TRUE;
			$element_value = is_array($element['#value']) ? reset($element['#value']) : $element['#value'];
			// Build hierarchy from flat options array 
			$hierarchy = array();
			// Array of all parents in hierarchy for element default value
			

			$_parents = array();
			$_prev_depth = 0;
			foreach ($element['#options'] as $key => $title) {
				if ($key === NULL || $key === 0 || $key === '0' || $key === '' || (isset($element['#empty_option_key']) && $key == $element['#empty_option_key'])) {
					continue;
				}
				if (!preg_match('/^(' . preg_quote($element['#hierarchy_prefix'], '/') . ')+/u', $title, $m)) {
					$_depth = 0;
					$_parents = array($key);
					$hierarchy[$key] = array();
				}
				else {
					$_depth = strlen(trim($m[0])) / strlen($element['#hierarchy_prefix']);
					if ($_depth == $_prev_depth) {
						array_pop($_parents);
					}
					if ($_depth < $_prev_depth) {
						$_parents = array_slice($_parents, 0, $_depth , TRUE);
					}
					$_parents[] = $key;
					drupal_array_set_nested_value($hierarchy, $_parents, array());
				}
				if ($element_value !== FALSE && $key == $element_value) {
					$trail = drupal_map_assoc($_parents);
				}
				
				$_prev_depth = $_depth;
			}
			if ($hierarchy && count($hierarchy) <  count($element['#options'])) {
				
				// Create selects by hierarchy
				$wrapper_id = preg_replace('/[\[\]]/', '-', $element['#name']) . '-hierarchical-select-ajax-wrapper';
				$added_elements = array();
				$_trail_clone = $trail;
				while ($hierarchy) {
					$opts = array();
					if (isset($element['#empty_option_key'])) {
						if (isset($element['#options'][$element['#empty_option_key']])) {
							$opts[$element['#empty_option_key']] = $element['#options'][$element['#empty_option_key']];
						}
						elseif ($element['#required'] && $added_elements) {
							$opts[$element['#empty_option_key']] = t('- None -');
						}
					}
					else {
						if (!$added_elements) {
							if (isset($element['#options'][''])) {
								$opts[''] = $element['#options'][''];
							}
							else {
								if (isset($element['#options'][0])) {
									$opts[0] = $element['#options'][0];
								}
							}
						}
						else {
							$opts[''] = isset($element['#options']['']) ? $element['#options'][''] : t('- None -');
						}

						
					}
					foreach ($hierarchy as $key => $_data) {
						$opts[$key] = ltrim($element['#options'][$key], $element['#hierarchy_prefix'] . ' ') . ($_data ?  html_entity_decode('&nbsp; &#8227;', ENT_COMPAT, 'UTF-8') : '');
					}
					$v = array_shift($_trail_clone);
					$classes = array();
					if ($v && $element_value == $v) {
						$classes[] = 'active';
					}
					if (!$v) {
						$classes[] = 'value-none';
					}
					$added_elements[] = array(
						'#type' => 'select',
						'#_hierarchical_select_processed' => TRUE,
						'#options' => $opts,
						'#default_value' => $v,
						'#field_prefix' => '<div style="white-space:nowrap;"><span>' . ($added_elements ? (!empty($element['#inline']) ? ' ' : str_repeat('&nbsp; ', count($added_elements) * 2) . theme('pictogram', array('icon' => 'level-down',))) : '') . '</span>',
						'#field_suffix' => ($v && $element_value == $v ? theme('pictogram', array('icon' => 'ok',)) . '&nbsp;': '') . '</div>',
						'#suffix' => module_invoke('utils', 'html_debug'),
						'#attributes' => array(
							'class' => $classes,
						),
						'#disabled' => !empty($element['#disabled']),
						'#ajax' =>  array(
							'wrapper' => $wrapper_id,
							'callback' => '_addons_hierarchical_select_ajax',
							'prevent' => 'click',
							'progress' => array(
								'type' => 'throbber',
								'message' => '',
							),
						),
					);
					// Advance into deep of array
					$hierarchy = isset($hierarchy[$v]) ? $hierarchy[$v] : FALSE;
				}

				if ($added_elements) {
					$added_elements[count($added_elements) - 1]['#attributes']['class'][] = 'last';
					$parent_element += array(
						'#prefix' => '',
						'#suffix' => '',
					);
					$parent_element['#prefix'] = '<div id="' . $wrapper_id . '">' . $parent_element['#prefix'];
					$parent_element['#suffix'] .= '</div>' . module_invoke('utils', 'html_debug');
					$hierarchical_name = 'hierarchical_selects_' . implode('_', $element['#parents']);
					$p = $element['#parents'];
					array_pop($p);
					$p[] = $hierarchical_name;
					
					$new_element = array(
						'#_original_element' => $element,
						'#weight' => $element['#weight'],
						'#trail' => $trail,
						'#type' => 'item',
						'#title' => !empty($element['#required']) ?  t('!title !required', array('!title' => $element['#title'], '!required' =>  theme('form_required_marker'))) : $element['#title'],
						'#description' => !empty($element['#description']) ? $element['#description'] : NULL,
						'#inline' => isset($element['#inline']) ? $element['#inline'] : FALSE,
						'#tree' => TRUE,
						'#input' => TRUE,
						'#attributes' => array('class' => array('selects-hierarchy',)),
						'#element_validate' => array('_addons_hierarchical_select_validate'),
						) 
						+ $added_elements
						+ array(
							'_refresh_button_' => array(
								'#executes_submit_callback' => FALSE,
								'#access' => empty($element['#disabled']),
								'#type' => 'button',
								'#value' => 'ok',
								'#limit_validation_errors' => array($p),//array($element['#parents']),
								'#name' => preg_replace('/[\[\]]/', '_', $element['#name']) . '_refresh_button',
								'#attributes' => array('class' => array('js-hide',)),
								),		
						);
					// Hide initial selectbox. Do not use '#access' = FALSE or '#type' = 'value' or unset(), this may cause problems in validation.
					
					$element['#type'] = $element['#theme'] = 'hidden';
					unset($element['#theme_wrappers'], $element['#options']);
					
					if (module_exists('utils')) {
						utils_html_debug_element($element);
					}
		
					// Rebuild parent element for correct applying name, id and other properties to new added selectboxes.
					_addons_insert_element_to_parent($parent_element, $hierarchical_name, $new_element, $form_state, $complete_form);
				}			
			}

		}

		// IN CONSTRUCTION
		if (!empty($element['#new_value_key']) && empty($parent_element['#_new_value_key_select_processed'])) {
			$parent_element['#_new_value_key_select_processed'] = TRUE;
			$new_element_name = implode('_', $element['#parents']) . '_' . $element['#new_value_key'];
			
			$new_element = array(
				'#tree' => TRUE,
				'#type' => 'textfield',
				'#attributes' => array(
					'class' => array('new-value'),
				),
				'#size' => 35,
				'#title' => t('add new item into !title', array(
					'!title' => $trail && isset($original_element['#options'][$element_value]) 
					? 
					preg_replace('/^(' . preg_quote($element['#hierarchy_prefix'], '/') . ')+/', '', $original_element['#options'][$element_value])
					:
					html_entity_decode(strip_tags($original_element['#title']), ENT_COMPAT, 'UTF-8'),)),
				'#title_display' => 'invisible',
				'#field_prefix' => $trail ? '<span class="js-hide">'. str_repeat('&nbsp;', (count($trail) * 4)) . '&#8627; </span>' : '',
				'#inline' => TRUE,
				'#weight' => 10000,
			);
			
			if (!empty($original_element['#new_value_key']) && isset($hierarchical_name)) {
				$new_value_level_min = isset($original_element['#new_value_level_min']) ? $original_element['#new_value_level_min'] : 0;
				if ($new_value_level_min > count($trail)) {
					$new_element = array();
				}
				$new_value_level_max = isset($original_element['#new_value_level_max']) ? $original_element['#new_value_level_max'] : count($trail);
				if ($new_value_level_max > $new_value_level_min && $new_value_level_max < count($trail)) {
					$new_element = array();
				}
				// @TODO: add level validation
			}
			if (!empty($element['#hierarchy_prefix'])) {
				$parent_element['#attached']['js'][drupal_get_path('module' , 'addons') . '/js_behaviors/addons.selectbox.theme.js'] = array();
				
			}
			if ($new_element) {
				_addons_insert_element_to_parent($parent_element, $new_element_name, $new_element, $form_state, $complete_form);
			}
			if (empty($parent_element[$new_element_name]['#value'])) {
				$parent_element[$new_element_name]['#attributes']['class'][] = 'js-hide';
			}
			
		}		
	}

	return $element;
}

/**
* Ajax callback for _addons_process_select()
*/
function _addons_hierarchical_select_ajax($form, &$form_state) {
	$parents = $form_state['triggering_element']['#array_parents'];
	array_pop($parents);
	array_pop($parents);
	return drupal_array_get_nested_value($form, $parents);
}


/**
* Validate hierarhical select element. @see _addons_process_select()
*/
function _addons_hierarchical_select_validate($element, &$form_state) {
	if ($has_error = form_get_error($element['#_original_element'])) {
		 form_set_error(implode('][', $element['#parents']), '');
	} 
	$val = '';
	$empty_option_key = isset($element['#_original_element']['#empty_option_key']) ? $element['#_original_element']['#empty_option_key'] : '';
	if ($element['#value']) {
		ksort($element['#value']);
		foreach ($element['#value'] as $idx => $v) {
			if ($v == $empty_option_key) {
				break;
			}
			$val = $v;
			if (!isset($element['#trail'][$v])) {
				break;
			}
		}
		
		if (!$val && !$has_error && $element['#_original_element']['#required']) {
			form_set_error(implode('][', $element['#parents']), t('!name field is required.', array('!name' => $element['#title'])));
		}
		drupal_array_set_nested_value($form_state['values'], $element['#_original_element']['#parents'], !empty($element['#_original_element']['#value_key']) ? array(array($element['#_original_element']['#value_key'] => $val)) : $val, TRUE);
		drupal_array_set_nested_value($form_state['input'], $element['#_original_element']['#parents'], $val, TRUE);
	}

}


/** End of selects processing **/


function _addons_radios_process($element, &$form_state, &$complete_form) {
	$element += array(
		'#items_as_links' => FALSE, // make radios like links
	);
	if ($element['#items_as_links']) {
		$element['#attributes']['class'][] = 'items-as-links';
		utils_html_debug_element($element);
		$element['#attached']['js'][] = drupal_get_path('module', 'addons') . '/js_behaviors/radios.js';
		foreach ($element['#options'] as $key => $title) {
			$element[$key]['#title'] = '<a ' . ($element['#value'] === $key ? 'class="active" ' : '') . 'href="#"><span class="ok-mark' . ($element['#value'] === $key ? '' : ' element-invisible') . '">&#10004;</span>' . (isset($element[$key]['#title']) ? $element[$key]['#title'] : $title) . '</a>';
		}
	}
	return $element;
}



function _addons_checkboxes_process($element, &$form_state, &$complete_form) {
	//test: $element['#sortable'] = TRUE;
	if (!empty($element['#sortable'])) {
		if ($element['#options']) {
      // Order options by default element value
      $val = drupal_map_assoc($element['#value']);
      $opts = array();
      foreach ($val as $v) {
        if (isset($element['#options'][$v])) {
          $opts[$v] = $element['#options'][$v];
        }
      }   
      $opts += $element['#options'];
			foreach (array_keys($opts) as $key) {
				$opts[$key] = '<strong title="' . check_plain(t('Drag to re-order')) . '" class="js-visible updown">' . (!empty($element['#inline']) ? '&#8596;' : '&#8597;') .  '</strong> ' . $opts[$key];
			}
      $element['#options'] = $opts;
		}
    
		$element['#attributes']['class'][] = 'sortable';
		$element['#attached']['js'][] = drupal_get_path('module', 'addons') . '/js_behaviors/addons.checkboxes.js';
		$element['#attached']['library'][] = array('system', 'ui.sortable');
		$element += array('#title' => '');
	}
	return $element;
}





