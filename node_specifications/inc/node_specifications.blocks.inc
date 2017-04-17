<?php
/**
 * Filter block
 */
function node_specifications_filters_block($form, &$form_state, $nids = array(), $nsids = array(), $mode = 'block') {
  $arg = arg();

  // page mode $nsids and $nids extraction
  if (isset($_GET['nsids']) && $arg[0] == 'specification_filters') {
    $mode = 'page';
    $nsids = explode('_', $_GET['nsids']);
    // get queried catalog nids
    $g = $_GET;
    // clear $_GET from filter query
    foreach (_node_specifications_get_filter_query_symbols() as $symbol) {
      unset($_GET[$symbol]);
    }
    $nids = _node_specifications_get_catalog_queried_nids();
    $_GET = $g;
  }

  $form_state['#nsids'] = $nsids;
  $d_options = variable_get('specifications_filter_block_display_options', array()); // block display options
  $module_path = drupal_get_path('module', 'node_specifications');
  $form['#attached'] = array(
    'library' => array(array('system', 'jquery.cookie')),
    'js' => array($module_path . '/js/node_specifications.js',),
    'css' => array($module_path . '/css/node_specifications.css',),
  );
  $form_classes = array('specification-filters');
  $form_classes[] = "mode-$mode";
  if ($mode == 'block') {
    $form_classes[] = isset($d_options['view_layout']) ? $d_options['view_layout'] : 'vertical';
  }
  $form['#attributes']['class'] = $form_classes;

  // filters container
  $form['filters'] = array(
    '#type' => 'container',
    '#tree' => TRUE,
    '#attributes' => array('class' => array('filter-elements-wrapper', (!empty($d_options['count_col']) && $mode === 'block') ? 'count-col-' . $d_options['count_col'] : '')),
  );

  // shop filters
  if (module_exists('shop')) {
    // price
    if (!empty($_GET['p']) || $mode === 'page' || !$d_options || !empty($d_options['price']['show'])) {
      $price = cache_get('product_price_' . sha1(implode('.', $nids)));
      if (empty($price) && $nids) {
        // query max/min values for price
        $q = db_query('SELECT price FROM {shop_index} WHERE entity_type = \'node\' AND entity_id IN(:nids)', array(':nids' => $nids))->fetchCol();
        if (!empty($q)) {
          $price = array('max' => max($q), 'min' => min($q),);
          cache_set('product_price_' . sha1(implode('.', $nids)), $price);
        }
      }
      elseif (isset($price->data)) {
        $price = $price->data;
      }

      if (!empty($price)) {
        $form['filters']['p'] = array(
          '#type' => 'fieldset',
          '#title' => t('Price'),
          '#collapsible' => empty($_GET['p']) && empty($d_options['price']['expanded']),
          '#collapsed' => $mode !== 'page',
          'slider' => array(
            '#type' => 'slider',
            '#left_value' => !empty($_GET['p']['min']) ? $_GET['p']['min'] : NULL,
            '#right_value' => !empty($_GET['p']['max']) ? $_GET['p']['max'] : NULL,
            '#left' => t('from'),
            '#right' => t('to'),
            '#min' => $price['min'],
            '#max' => $price['max'],
            '#field_suffix' => theme('price', array('currency' => shop_get_info('currency', 'default'))),
          ),
        );
      }
    }

    // in stock
    if (!empty($nids) && ($mode == 'page' || empty($d_options) || !empty($d_options['in_stock']['show']))) {
      // show this filter only for product node types bundled in $catalog_object
      if ($arg[0] == 'catalog' && is_numeric($arg[1])) {
        $catalog_object = catalog_object_load($arg[1]);
        if ($catalog_object && $catalog_object->entity_type == 'node' && shop_get_info('entity_types', 'node', $catalog_object->bundle)) {
          $form['filters']['in_stock'] = array(
            '#type' => 'checkbox',
            '#title' => t('In stock'),
            '#default_value' => !empty($_GET['in_stock']),
            '#weight' => 99,
          );
        }
      }
    }
  }

  // specification filters
  $form['filters']['f']['#type'] = 'markup';
  // get max/min number_integer field numbers
  $max_min_cache_cid = 'node_specifications_min_max_field_data_' . sha1(implode('.', $nids)) . '_' . sha1(implode('.', $nsids));
  $max_min = cache_get($max_min_cache_cid);
  if ($max_min) {
    $max_min = $max_min->data;
  }
  else {
    $max_min = array();
    if (!empty($nids)) {
      $q = db_select('node_specifications_fields_data', 'nsfd');
      $q->fields('nsfd', array('nsid', 'delta'));
//			$q->innerJoin('node_specifications_fields', 'nsf', "nsfd.nsid = nsf.nsid AND nsfd.delta = nsf.delta AND nsf.field_type='number_integer'");
      $q->addExpression('MAX(var_id)', '_max');
      $q->addExpression('MIN(var_id)', '_min');
      if ($nsids) {
        $q->condition('nsfd.nsid', $nsids);
      }
      $q->condition('nsfd.nid', $nids);
      $q->groupBy('nsfd.nsid');
      $q->groupBy('nsfd.delta');
      foreach ($q->execute() as $result) {
        $max_min[$result->nsid][$result->delta] = array(
          'max' => $result->_max,
          'min' => $result->_min,
        );
      }
      cache_set($max_min_cache_cid, $max_min);
    }
  }

  foreach (node_specifications_load_multiple($nsids) as $nsid => $specification) {
    if (!empty($specification->fields)) {
      if ($mode == 'page') {
        $form['filters']['f'][$nsid] = array(
          '#type' => 'fieldset',
          '#title' => $specification->name,
          '#collapsible' => TRUE,
          '#collapsed' => FALSE,
        );
      }
      else {
        $form['filters']['f'][$nsid]['#type'] = 'markup';
      }

      // specification fields
      foreach ($specification->fields as $field) {

        $delta = $field['delta'];
        $val = !empty($_GET['f'][$nsid][$delta]) ? $_GET['f'][$nsid][$delta] : NULL;
        $w = $mode === 'page' || empty($d_options) || empty($d_options[$nsid . '/' . $delta]);
        if ($val || $w || !empty($d_options[$nsid . '/' . $delta]['show'])) {

          $unit_title = _node_specifications_get_field_unit_symbol($field);

          $elements = array();
          switch ($field['field_type']) {
            case 'color_select' :
              $vars = array();
              foreach (isset($field['vars']) ? $field['vars'] : array() as $var) {
                if (!empty($var['var_value']) || !empty($var['var_data'])) {
                  $vars[$var['var_id']] =
                    (!empty($var['var_value']) ? '<span class="color_me" data="' . $var['var_value'] . '"></span>' : '') .
                    (!empty($var['var_data']) ? $var['var_data'] : '');
                }
              }
              $elements['vars'] = array(
                '#type' => 'checkboxes',
                '#options' => $vars,
                '#default_value' => !empty($val) ? $val : array(),
                '#prefix' => $unit_title,
              );
              break;

            case 'list_text' :
              $vars = array();
              foreach (isset($field['vars']) ? $field['vars'] : array() as $var) {
                $vars[$var['var_id']] = isset($var['var_value']) ? ($var['var_value'] . ' ' . $unit_title) : NULL;
              }
              $elements['vars'] = array(
                '#type' => 'checkboxes',
                '#options' => $vars,
                '#default_value' => !empty($val) ? $val : array(),
              );
              break;

            case 'number_integer':
              $min = !empty($max_min[$nsid][$delta]['min']) ? $max_min[$nsid][$delta]['min'] : 0;
              $max = !empty($max_min[$nsid][$delta]['max']) ? $max_min[$nsid][$delta]['max'] : 0;
              if ($min != $max) {
                $elements['slider'] = array(
                  '#type' => 'slider',
                  '#left_value' => !empty($val['min']) ? $val['min'] : NULL,
                  '#right_value' => !empty($val['max']) ? $val['max'] : NULL,
                  '#left' => t('from'),
                  '#right' => t('to'),
                  '#min' => !empty($max_min[$nsid][$delta]['min']) ? $max_min[$nsid][$delta]['min'] : 0,
                  '#max' => !empty($max_min[$nsid][$delta]['max']) ? $max_min[$nsid][$delta]['max'] : 0,
                  '#field_suffix' => $unit_title,
                );
              }
              break;

            case 'logic':
              $elements['logic'] = array(
                '#type' => 'radios',
                '#options' => array(
                  '' => t('Not important'),
                  2 => t('Yes'),
                  1 => t('No'),
                ),
                '#default_value' => !empty($val['l']) ? $val['l'] : array(),
              );
              break;

            case 'text':
              unset($form['filters']['f'][$nsid][$delta]);
              break;
          }

          if ($elements) {
            $form['filters']['f'][$nsid][$delta] = array(
                '#type' => 'fieldset',
                '#title' => $field['title'],
                '#collapsible' => empty($val) && empty($d_options[$nsid . '/' . $delta]['expanded']),
                '#collapsed' => $mode !== 'page',
                '#description' => !empty($field['hint']) ? filter_xss_admin($field['hint']) : '',
              ) + $elements;
          }
        }

      }
    }

  }
  // taxonomy filters
  $vids = array();
  foreach ($d_options as $idx => $value) {
    $v = strstr($idx, 't/');
    if ($v && $value['show']) {
      $v = explode('/', $idx);
      $vids[] = $v[1];
    }
  }
  $vocabs = taxonomy_vocabulary_load_multiple($vids);
  $current_tid = NULL;

  if (count($arg) > 2 && is_numeric(end($arg))) {
    $current_term = taxonomy_term_load(end($arg));
    if (is_object($current_term) && !empty($vocabs) && !empty($vocabs[$current_term->vid])) {
      $form['filters']['t'] = array(
        '#type' => 'fieldset',
        '#title' => $vocabs[$current_term->vid]->name . ' : ' . $current_term->name,
      );
    }
  }
  //elseif  (!empty($vids))  -  заменил элсеиф на иф чтоб показать доп настройки таксономий для greenwest Ruban
  if (!empty($vids)) {
    foreach ($vids as $vid) {
      $queried_tids = isset($_GET['t']) ? (array) $_GET['t'] : array();

      $vocabulary = array();
      if (!empty($vocabs[$vid])) {
        $vocabulary = $vocabs[$vid];
      }
      if (is_object($vocabulary)) {
        $options = array();
        $founded_in_queried_tids = FALSE;
        $tree = taxonomy_get_tree($vocabulary->vid);
        if ($tree) {
          foreach ($tree as $term) {
            $options[$term->tid] = str_repeat('-', $term->depth) . $term->name;
            if (in_array($term->tid, $queried_tids)) {
              $founded_in_queried_tids = TRUE;
            }
          }
        }
        $form['filters']['t'][$vid] = array(
          '#type' => 'fieldset',
          '#title' => $vocabulary->name,
          '#collapsible' => empty($d_options["t/$vid"]['expanded']) && !$founded_in_queried_tids,
          '#collapsed' => TRUE,
        );
        $form['filters']['t'][$vid]['tid'] = array(
          '#type' => 'select',
          '#options' => $options,
          '#empty_value' => '',
          '#default_value' => $queried_tids,
        );
      }
    }
  }

  $form['actions'] = array(
    '#type' => 'actions',
    '#weight' => 99,
  );
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Pick up'),
    '#attributes' => array(
      'class' => array(
        'js-hide' => 'js-hide',
        'submit',
      ),
    ),
    '#weight' => 0,
  );
  $form['actions']['reset'] = array(
    '#type' => 'submit',
    '#value' => t('Reset'),
    '#attributes' => array('class' => array('js-hide' => 'js-hide'),),
    '#weight' => 1,
  );
  foreach (_node_specifications_get_filter_query_symbols() as $symbol) {
    if (isset($_GET[$symbol])) {
      unset($form['actions']['reset']['#attributes']['class']['js-hide']);
    }
  }

  // if any filter was hidden, provide link to all specification filters
  if ($mode != 'page' && !empty($nsids) && !empty($d_options)) {
    foreach ($d_options as $value) {
      if (isset($value['show']) && $value['show'] == 0) {
        $form['actions']['all_specifications'] = array(
          '#type' => 'submit',
          '#value' => t('All specifications'),
          '#weight' => 10,
        );
        break;
      }
    }
  }

  if ($mode === 'block' && isset($d_options['count_col']) && !empty($d_options['count_col'])) {
    foreach ($form['filters'] as $new => $key) {
      if (is_array($key) && (count($key) > 1)) {
        $form['filters'][$new]['#prefix'] = '<div class="inline-block-wrapper-' . $d_options['count_col'] . '">';
        $form['filters'][$new]['#suffix'] = '</div>';
      }
    }
  }

  return $form;
}

/**
 * Prepare form element values for building query filter
 */
function node_specifications_filters_block_validate($form, &$form_state) {
  if (!empty($form_state['values']['op']) && !empty($form_state['values']['filters'])) {
    $filters = &$form_state['values']['filters'];
    $op = $form_state['values']['op'];

    if (
      (isset($form_state['values']['submit']) && $op == $form_state['values']['submit']) ||
      (isset($form_state['values']['all_specifications']) && $op == $form_state['values']['all_specifications'])
    ) {
      $query = array();
      // price filter
      if (!empty($filters['p'])) {
        $slider_values = _node_specifications_extract_slider_values($filters['p']);
        if (!empty($slider_values)) {
          $query['p'] = $slider_values;
        }
      }

      // in stock
      if (!empty($filters['in_stock'])) {
        $query['in_stock'] = 1;
      }

      // specifications filter
      if (!empty($filters['f'])) {
        foreach ($filters['f'] as $nsid => $fields) {
          foreach ($fields as $delta => $data) {
            if (!empty($data['vars'])) {
              $v = array_filter($data['vars']);
              if (!empty($v)) {
                $query['f'][$nsid][$delta] = $v;
              }
            }
            elseif (!empty($data['logic'])) {
              $query['f'][$nsid][$delta]['l'] = $data['logic'];
            }
            elseif (!empty($data['slider'])) {
              $v = _node_specifications_extract_slider_values($filters['f'][$nsid][$delta]);
              if (!empty($v)) {
                $query['f'][$nsid][$delta] = $v;
              }
            }
          }
        }
      }

      // taxonomy filter
      if (!empty($filters['t'])) {
        foreach ($filters['t'] as $vid => $term_filter) {
          if (!empty($term_filter['tid'])) {
            $query['t'][$vid] = $term_filter['tid'];
          }
        }
      }

      $form_state['#query'] = $query;
    }
  }
}

function node_specifications_filters_block_submit($form, &$form_state) {
  // clear $_GET from filter query
  foreach (_node_specifications_get_filter_query_symbols() as $symbol) {
    unset($_GET[$symbol]);
  }

  if (!empty($form_state['values']['op'])) {
    $op = $form_state['values']['op'];

    // submit button process````
    if (!empty($form_state['values']['submit']) && $op == $form_state['values']['submit']) {
      if (!empty($form_state['#query'])) {
        $redirect = $_GET['q'];
        if (isset($_GET['destination'])) {
          $redirect = $_GET['destination'];
          unset($_GET['destination']);
        }
        $form_state['redirect'] = array(
          $redirect,
          array('query' => $form_state['#query']),
        );
      }
    }

    // all specification button process
    elseif (!empty($form_state['values']['all_specifications']) && $op == $form_state['values']['all_specifications']) {
      if (!empty($form_state['#nsids'])) {
        $form_state['redirect'] = array(
          'specification_filters',
          array('query' => (!empty($form_state['#query']) ? $form_state['#query'] : array()) + drupal_get_destination() + array('nsids' => implode('_', $form_state['#nsids']))),
        );
      }
    }
  }
}

/**
 * Comparing block
 */
function node_specifications_compare_block($block, &$form_state) {
  $header = $rows = $nids = array();
  $destination = drupal_get_destination();

  $block['#attached'] = array(
    'library' => array(
      array(
        'system',
        'jquery.cookie'
      )
    ),
  );

  $compare_list = array_unique(array_filter(explode(',', $_COOKIE['compare_list'])));
  $nodes = node_load_multiple($compare_list);
  if ($nodes) {
    $nids = array_keys($nodes);
    foreach ($nodes as $node) {
      $rows[] = array(
        l($node->title, 'node/' . $node->nid),
        l('x', 'node/compare/remove/' . $node->nid, array(
          'query' => array($destination),
          'attributes' => array(
            'title' => t('Remove from comparing'),
            'class' => 'form-submit'
          ),
        ))
      );
    }
  }

  $block['tbl'] = array(
    '#markup' => theme('table', array(
        'header' => $header,
        'rows' => $rows,
        'attributes' => array('width' => '100%'),
      )
    )
  );

  $block['destination'] = array(
    '#type' => 'value',
    '#value' => $destination,
  );

  $block['nids'] = array(
    '#type' => 'value',
    '#value' => implode(',', $nids),
  );

  $block['actions'] = array('#type' => 'actions',);
  $nodes_in_compare = count($nodes);

  if ($nodes_in_compare >= 2) {
    $block['actions']['compare'] = array(
      '#type' => 'submit',
      '#value' => t('Compare'),
    );
  }

  if ($nodes_in_compare >= 1) {
    $block['actions']['clear'] = array(
      '#type' => 'submit',
      '#value' => t('Clear'),
    );
  }

  return $block;
}

function node_specifications_compare_block_submit($form, &$form_state) {
  if (isset($form_state['values']['op'])) {
    $op = $form_state['values']['op'];
    $link_options = array('query' => array('destination' => $form_state['values']['destination']['destination']));
    if (isset($form_state['values']['compare']) && $op == $form_state['values']['compare']) {
      $form_state['redirect'] = array('node/compare', $link_options);
    }
    elseif (isset($form_state['values']['clear']) && $op == $form_state['values']['clear']) {
      $form_state['redirect'] = array('node/compare/clear', $link_options);
    }
  }
}

function _node_specifications_extract_slider_values($element) {
  $values = array();
  if (!empty($element['slider'])) {
    $e = $element['slider'];
    if (($e['min_val'] != $e['left']) || ($e['max_val'] != $e['right'])) {
      $values['min'] = $e['left'];
      $values['max'] = $e['right'];
    }
  }

  return $values;
}

function node_specifications_block_configure_form() {
  $form = array(
    '#tree' => TRUE,
    '#weight' => 0,
    '#type' => 'fieldset',
    '#title' => t('Specification settings'),
    '#theme' => 'node_specifications_block_configure_form',
  );

  $field_names = _node_specifications_get_field_names();
  $values = variable_get('specifications_filter_block_display_options', array());

  $form['view_layout'] = array(
    '#type' => 'select',
    '#title' => t('Specify display layout'),
    '#default_value' => isset($values['view_layout']) ? $values['view_layout'] : 'vertical',
    '#options' => array(
      'vertical' => t('Vertical'),
      'horizontal' => t('Horizontal'),
    ),
  );

  $form['count_col'] = array(
    '#type' => 'select',
    '#title' => t('Count column'),
    '#default_value' => isset($values['count_col']) ? $values['count_col'] : '',
    '#options' => array(
      '' => t('None'),
      '1' => 1,
      '2' => 2,
      '3' => 3,
      '4' => 4,
      '5' => 5,
      '6' => 6,
      '7' => 7,
      '8' => 8,
      '9' => 9,
      '10' => 10,
    ),
  );

  if (module_exists('shop')) {
    // price
    _node_specifications_block_configure_form_helper($form, 'price', t('Price'), (isset($values['price']) ? $values['price'] : array()));
    // in_stock
    _node_specifications_block_configure_form_helper($form, 'in_stock', t('In stock'), (isset($values['in_stock']) ? $values['in_stock'] : array()));
  }

  // specifications
  foreach (node_specifications_get_names() as $specification) {
    _node_specifications_block_configure_form_helper($form, $specification->nsid, $specification->name, NULL);
    foreach ($field_names[$specification->nsid] as $key => $field_name) {
      _node_specifications_block_configure_form_helper($form, $key, $field_name, isset($values[$key]) ? $values[$key] : array());
    }
  }

  // taxonomy
  _node_specifications_block_configure_form_helper($form, 't', t('Vocabularies'), NULL);
  foreach (taxonomy_vocabulary_load_multiple(FALSE) as $vocabulary) {
    $key = 't/' . $vocabulary->vid;
    _node_specifications_block_configure_form_helper($form, $key, $vocabulary->name, isset($values[$key]) ? $values[$key] : array('show' => FALSE));
  }

  return $form;
}

function _node_specifications_block_configure_form_helper(&$form, $id, $title, $values = array()) {
  $form[$id]['name'] = array(
    '#type' => 'item',
    '#title' => $title,
  );

  if ($values !== NULL) {
    $form[$id]['show'] = array(
      '#type' => 'checkbox',
      '#title' => $title,
      '#title_display' => 'invisible',
      '#default_value' => isset($values['show']) ? $values['show'] : TRUE,
    );

    $form[$id]['expanded'] = array(
      '#type' => 'checkbox',
      '#title' => $title,
      '#title_display' => 'invisible',
      '#default_value' => isset($values['expanded']) ? $values['expanded'] : FALSE,
    );
  }
}

function node_specifications_filter_presets_form() {
  if (!user_access('administer specifications')) {
    return array();
  }

  $form = array(
    '#tree' => TRUE,
    '#weight' => 0,
    '#type' => 'fieldset',
    '#title' => t('Filter presets'),
    '#theme' => 'node_specifications_filter_presets_form',
  );

  $form['filter_presets'] = array();
  foreach (node_specifications_filter_preset_load_multiple() as $filter_preset) {
    $id = $filter_preset['id'];

    $form['filter_presets'][$id] = array(
      'title' => node_specifications_filter_preset_view($filter_preset),
      'actions' => array(
        '#theme' => 'links',
        '#attributes' => array('class' => array('inline',)),
        '#links' => array(
          'edit' => array(
            'href' => "node_specifications_filter_presets/$id/edit",
            'title' => t('Edit'),
            'query' => array('destination' => current_path()),
          ),
          'delete' => array(
            'href' => "node_specifications_filter_presets/$id/delete",
            'title' => t('Delete'),
            'query' => array('destination' => current_path()),
          ),
        ),
      ),
      'weight' => array(
        '#type' => 'weight',
        '#title' => t('Weight'),
        '#default_value' => $filter_preset['weight'],
        '#delta' => 10,
        '#title_display' => 'invisible',
      ),
    );
  }

  return $form;
}
