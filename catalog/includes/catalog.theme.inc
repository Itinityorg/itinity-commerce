<?php

function theme_catalog_list_entities($variables) {
  if ($variables['catalog_object']->materials_settings['mode'] === 'table') {
    if (is_array($variables['entities'])) {
      $catalog_object = $variables['catalog_object'];
      $ufo_weights = array();
      foreach (element_children($variables['entities']) as $idx) {
        foreach (element_get_visible_children($variables['entities'][$idx]) as $key) {
          if (is_array($variables['entities'][$idx][$key])) {
            if (isset($variables['entities'][$idx][$key]['#title'])) {
              if (isset($variables['entities'][$idx][$key]['#weight'])) {
                $ufo_weights[$key] = $variables['entities'][$idx][$key]['#weight'];
              }
            }
          }
        }
      }
      //remove hidden extra fields and fields from header
      //get instances settings
      $instances = field_info_instances($catalog_object->entity_type, $catalog_object->bundle);
      //get extra fields visibility settings

      /**
       * Select view mode of entities, from catalog object
       */
      if (!empty($catalog_object->materials_settings['mode']) && !empty($catalog_object->materials_settings['modes'][$catalog_object->materials_settings['mode']]['settings']['view_mode'])) {
        $view_mode = $catalog_object->materials_settings['modes'][$catalog_object->materials_settings['mode']]['settings']['view_mode'];
      }
      else {
        $view_mode = 'teaser';
      }

      $field_extra_fields_get_display = field_extra_fields_get_display($catalog_object->entity_type, $catalog_object->bundle, $view_mode);

      //get extra fields labels, do not use labels for getting extra fields visibility - it's not altered
      $field_info_extra_fields = field_info_extra_fields($catalog_object->entity_type, $catalog_object->bundle, 'display');

      // temporary crutch forced enable non exposed by default sorting links in table header
      foreach (catalog_sql_info($catalog_object, 'orders') as $m => $d) {
        //if (!empty($field_info_extra_fields[$m])) {
          $catalog_object->orders[$m]['exposed'] = TRUE;
        //}
      }

      $header = $catalog_object->content['#header'];

      $sort_links = catalog_object_get_sorting_links($catalog_object);
      foreach ($sort_links as $order_method => $link) {
        if (isset($header[$order_method])) {
          $header[$order_method] = array('data' => theme('link', $link));
        }
      }


      $weights = array();

      foreach ($header as $key => $name) {
        //field is an instance of drupal field
        if (isset($instances[$key])) {
          $type = isset($instances[$key]['display'][$view_mode])
            ? $instances[$key]['display'][$view_mode]['type']
            : $instances[$key]['display']['default']['type'];

          if ($type == 'hidden') {
            unset($header[$key]);
          }
          else {
            /*$header[$key] = array(
              'data' => $instances[$key]['label'],
              'field'  => $key,
            );*/
            $weights[$key] = isset($instances[$key]['display'][$view_mode])
              ? $instances[$key]['display'][$view_mode]['weight']
              : $instances[$key]['display']['default']['weight'];
          }
        }
        //field is extra field
        elseif (isset($field_extra_fields_get_display[$key])) {
          if (!$field_extra_fields_get_display[$key]['visible']) {
            unset($header[$key]);
          }
          else {
            if (!is_array($header[$key])) {
              $header[$key] = array(
                'data' => isset($field_extra_fields_get_display[$key]['label']) ? $field_extra_fields_get_display[$key]['label'] : $field_info_extra_fields[$key]['label'],
              );
            }
            $weights[$key] = isset($field_info_extra_fields[$key]['display'][$view_mode])
              ? $field_info_extra_fields[$key]['display'][$view_mode]['weight']
              : $field_info_extra_fields[$key]['display']['default']['weight'];
          }
        }
        //field is UFO
        else {
            //$header[$key] = array('data' => $name);
            if (isset($ufo_weights[$key])) {
              $weights[$key] = $ufo_weights[$key];
            }
        }
      }

      //order fields and entities by weights
      asort($weights);
      foreach ($header as $key => $data) {
        $weights[$key] = is_scalar($data) ? array('data' => $data,) : $data;
        $weights[$key]['class'] =
        array('th-' . drupal_html_class($key));
      }
      $header = $weights;

      if (isset($header['title'])) {
        //$header['title']['field'] = 'title';
      }

      //fill table rows
      $rows = array();
      $contextual_access = module_exists('contextual') && user_access('access contextual links');
      foreach (element_children($variables['entities']) as $idx) {
        $row = array();
        $last_key = NULL;
        foreach ($header as $key => $data) {
          $row[$key]['data'] = array();
          if (!empty($variables['entities'][$idx][$key])) {
            $variables['entities'][$idx][$key]['#title_display'] = 'invisible';
            $row[$key]['data'][] = $variables['entities'][$idx][$key];
            $last_key = $key;
          }
          $row[$key]['class'][] = 'catalog-cell ' . drupal_html_class('td-' . $key);
        }

        //add row classes and row id
        $row_classes = array('catalog-row', 'content',);
        //add contextual links
        if ($last_key && $contextual_access && !empty($variables['entities'][$idx]['#contextual_links'])) {
          $row_classes[] = 'contextual-links-region';
          $row[$last_key]['data']['contextual_links_td'] = array(
            '#type' => 'container',
            '#weight' => 0,
            '#attributes' => array(
              'class' => array('contextual-links-td',),
            ),
          );
          $row[$last_key]['data']['contextual_links_td']['contextual_links'] = array(
            '#type' => 'contextual_links',
            '#contextual_links' => $variables['entities'][$idx]['#contextual_links'],
          );
        }

        $row_id = '';
        if (!empty($variables['entities'][$idx]['#node'])) {
          $row_id = 'nid-' . $variables['entities'][$idx]['#node']->nid;
          $row_classes[] = 'node';
          if (empty($variables['entities'][$idx]['#node']->status)) {
            $row_classes[] = 'node-unpublished';
          }
        }

        //add classes form other's module preprocess function
        if (!empty($variables['entities'][$idx]['#tabe_row_classes'])) {
          $row_classes = array_merge($row_classes, $variables['entities'][$idx]['#tabe_row_classes']);
        }

        $rows[] = array(
          'data' => $row,
          'class' => $row_classes,
          'id' => $row_id,
        );
      }

      //remove empty cols from header and rows (such as currency in old shop)
      $remove = $header;
      foreach ($header as $key => $data) {
        foreach ($rows as $row) {
          if (!empty($row['data'][$key]['data'])) {
            unset($remove[$key]);
          }
        }
      }
      if (!empty($remove)) {
        foreach ($remove as $key => $data) {
          unset($header[$key]);
          foreach ($rows as &$row) {
            unset($row['data'][$key]);
          }
        }
      }

      //add catalog classes and styles
      $classes = array('catalog-table', 'table-view', 'block-catalog', $catalog_object->entity_type . 's');

      $m_s = $catalog_object->materials_settings;
      if (!empty($m_s['modes'][$m_s['mode']]['settings']['center'])) {
        $classes[] = 'centered';
      }
      $style = '';
      $zoom = isset($m_s['modes'][$m_s['mode']]['settings']['zoom']) ? $m_s['modes'][$m_s['mode']]['settings']['zoom'] : '';
      if ($zoom) {
        $style .= 'font-size:' . ($zoom / 100) . 'em;';
      }

      return array(
        '#prefix'  => isset($variables['entities']['#prefix']) ? $variables['entities']['#prefix'] : NULL,
        '#markup'  => theme('table', array(
          'header' => $header,
          'rows' => $rows,
          'empty' => t('No available content.'),
          'attributes' => array(
            'class' => $classes,
            'style' => !empty($style) ? $style : NULL,
          ),
        )),
        '#suffix'  => isset($variables['entities']['#suffix']) ? $variables['entities']['#suffix'] : NULL,
      );
    }

    return $variables['entities'];
  }

  // Adaptive layout ($catalog_object->materials_settings['mode'] === 'list') :
  // @see output html example in ./catalog.html file
  else if ($variables['catalog_object']->materials_settings['mode'] === 'list') {
    $i = 0;
      //if (!$variables['catalog_object']->is_block) m(count($variables['entities']));
    $col_wrapper = !empty($variables['catalog_object']->materials_settings['modes']['list']['settings']['colwrap']);
        $count_entities = $variables['entities'];
        if ($col_wrapper) {
          $current_col = 0;
          $ColArray = array();
        }
    $cols = !empty($variables['catalog_object']->materials_settings['modes']['list']['settings']['cols']) ? $variables['catalog_object']->materials_settings['modes']['list']['settings']['cols'] : 1;

    foreach ($variables['entities'] as $idx => $entity) {
      $i++;
      $variables['entities'][$idx] += array(
        '#prefix' => '',
        '#suffix' => '',
      );
      $classes = array(
        'catalog-cell-wrapper',
      );
      if ($i == 1) {
        $classes[] = 'first';
      }
      if ($i == count($variables['entities'])) {
        $classes[] = 'last';
      }
      $variables['entities'][$idx]['#prefix'] = '<div class="' . implode(' ', $classes) . '"><div class="catalog-cell">' . $variables['entities'][$idx]['#prefix'];
      $variables['entities'][$idx]['#suffix'] .= '</div></div>';
      // partition by column
      if ($col_wrapper) {
        if ($current_col < $cols) {
          $current_col++;
        }
        else {
          $current_col = 1;
        }
        $ColArray[$current_col][$idx] = $variables['entities'][$idx];
        unset($variables['entities'][$idx]);
      }
    }
    // now wrapper partition by column
    if ($col_wrapper) foreach ($ColArray as $current_col => $entities) {
      $variables['entities']['col'][$current_col]['#prefix'] ='<div class="catalog-col-wrapper">';
      $variables['entities']['col'][$current_col]['#suffix'] = '</div>';
      foreach ($entities as $idx => $entity) {
        $variables['entities']['col'][$current_col]['entities'][$idx] = $entity;
      }
    }
    $classes = array(
      'catalog-table',  // old legacy
      'cols-' . min($cols, $count_entities),//$variables['catalog_object']->cols,
      $variables['catalog_object']->entity_type . 's',   // 'users', 'nodes' ...
      preg_replace('/([^s])$/', '$1s', $variables['catalog_object']->bundle) // 'page' -->> 'pages', 'news' -->> 'news'
    );
    $m_s = $variables['catalog_object']->materials_settings;
    if (!empty($m_s['modes'][$m_s['mode']]['settings']['center'])){
      $classes[] = 'centered';
    }
    $styles = array();
    $zoom = isset($m_s['modes'][$m_s['mode']]['settings']['zoom']) ? $m_s['modes'][$m_s['mode']]['settings']['zoom'] : '';
    if ($zoom) {
      $styles[] = 'font-size:' . ($zoom / 100) . 'em;';
    }
    $variables['entities'] += array(
      '#prefix' => '',
      '#suffix' => '',
    );
    $variables['entities']['#prefix'] = '<div class="' . implode(' ', $classes) . '">' . $variables['entities']['#prefix'];
    $variables['entities']['#suffix'] .= '</div>';
    return $variables['entities'];
  }
}

/**
 * Theme output for terms
 */
function theme_catalog_list_terms($variables) {
  $build = array();
  if ($variables['table_view']) {
    $header = $rows = array();
    $terms = taxonomy_term_view_multiple($variables['tree'], 'teaser');
    $terms = $terms['taxonomy_terms'];
    unset($terms['#sorted']);
    foreach ($terms as $tid => $term) {
      element_children($term, TRUE); // Force pre-sort elements by weight
      foreach (element_get_visible_children($term) as $key) {
        static $field_info_extra_fields = array();
        if (!isset($field_info_extra_fields[$term['#term']->vocabulary_machine_name])) {
          $field_info_extra_fields[$term['#term']->vocabulary_machine_name] = field_info_extra_fields('taxonomy_term', $term['#term']->vocabulary_machine_name, 'display');
        }
        if (!empty($field_info_extra_fields[$term['#term']->vocabulary_machine_name][$key])) {
          if (!empty($field_info_extra_fields[$term['#term']->vocabulary_machine_name][$key]['label'])) {
            static $field_extra_fields_get_display;
            if (!isset($field_extra_fields_get_display)) {
              $field_extra_fields_get_display = field_extra_fields_get_display('taxonomy_term', $term['#term']->vocabulary_machine_name, 'teaser');
            }
            if ($field_extra_fields_get_display[$key]['visible']) {
              if ($title = !empty($term[$key]['#title']) ? $term[$key]['#title'] : (!empty($field_info_extra_fields[$term['#term']->vocabulary_machine_name][$key]['label']) ? $field_info_extra_fields[$term['#term']->vocabulary_machine_name][$key]['label'] : NULL)) {
                $header[$key]['data'] = $title;
                $header[$key]['weight'] = isset($term[$key]['#weight']) ? $term[$key]['#weight'] : $field_extra_fields_get_display[$key]['weight'];
              }
            }
          }
        }
        else {
          if ($field_info_instance = field_info_instance('taxonomy_term', $key, $term['#term']->vocabulary_machine_name)) {
            $field_get_display = field_get_display($field_info_instance, 'teaser', $term['#term']);
            if ($title = empty($term[$key]['#title']) ? $term[$key]['#title'] : (!empty($field_info_instance['label']) ? $field_info_instance['label'] : NULL)) {
              $header[$key]['data'] = $title;
              $header[$key]['weight'] =  isset($term[$key]['#weight']) ? $term[$key]['#weight']  : $field_info_instance['weight'];
            }
          }
        }
      }
    }
    uasort($header, 'drupal_sort_weight');
    foreach ($terms as $tid => $term) {
      foreach ($header as $key => $title) {
        $rows[$tid]['class']['catalog-row'] = 'catalog-row';
        $rows[$tid]['data'][$key]['data'] = !empty($term[$key]) ? array('#title_display' => 'invisible', '#label_display' => 'hidden') + $term[$key] : array();
      }
    }
    $build = array(
      '#theme' => 'table',
      '#header' => $header,
      '#rows' => $rows,
      '#attributes' => array(
        'class' => array(
          'catalog-table terms mode-table',
          ($variables['center'] ? '' : 'not-') . 'centered',
        ),
      ),
    );
  }
  else {
    // Adaptive layout
    $i = 0;
    $terms_view_multiple = taxonomy_term_view_multiple($variables['tree']);
    if ($variables['colwrap']) {
          $current_col = 0;
          $ColArray = array();
        }
    foreach ($variables['tree'] as $tid => $term) {
      $view = $terms_view_multiple['taxonomy_terms'][$tid];//taxonomy_term_view($term, 'teaser');
      $i++;
      $classes = array(
        'catalog-cell-wrapper',
      );
      if ($i == 1) {
        $classes[] = 'first';
      }
      else {
        if ($i == count($variables['tree'])) {
          $classes[] = 'last';
        }
      }
      $view = array(
        '#prefix' => '<div class="' . implode(' ', $classes) . '"><div class="catalog-cell parent-term">',
        '#suffix' => '</div></div>',
        'term' => $view,
      );
      // partition by column
      if ($variables['colwrap']) {
        if ($current_col < $variables['cols'])  { $current_col++; } else { $current_col = 1; }
        $ColArray[$current_col][$tid] = $view;
      } else {
        $build['terms'][] = $view;
      }

    }

    // now wrapper partition by column
    if ($variables['colwrap']) foreach ($ColArray as $current_col => $col_views) {
      $build['terms'][$current_col]['#prefix'] ='<div class="catalog-col-wrapper">';
      $build['terms'][$current_col]['#suffix'] = '</div>';
      foreach ($col_views as $tid => $view) {
        $build['terms'][$current_col]['it'][$tid] = $view;
      }
    }

    if (!empty($build['terms'])) {
      $classes = array(
        'catalog-table',
        'terms',
        'cols-' . min($variables['cols'] ? $variables['cols'] : 1, count($variables['tree'])),
        ($variables['center'] ? '' : 'not-') . 'centered',
      );
      $build['#prefix'] = '<div class="' . implode(' ', $classes) . '">';
      $build['#suffix'] = '</div>';
      $build['#attached']['css'][] = drupal_get_path('module', 'catalog') . '/catalog.css';
    }



    // OLD CODE :::
    /*
    $build = array(
      '#theme' => 'table',
      '#rows' => array(),
      '#attributes' => array(
        'class' => array(
          'catalog-table table-invisible terms',
          'cols-' . ($variables['cols'] ? $variables['cols'] : 1),
          ($variables['center'] ? '' : 'not-') . 'centered',
        ),
      ),
    );

    if (!empty($variables['tree'])) {
      $col_num = $row_num = $max_cols = 0;

      foreach ($variables['tree'] as $term) {
        if ($col_num >= $variables['cols']) {
          $row_num++;
          $col_num = 0;
        }
        $col_num++;

        $build['#rows'][$row_num]['data'][$term->tid] = array(
          'class' => array('parent-term'),
          'data' => taxonomy_term_view($term, 'teaser'),
        );
        $max_cols = max(count($build['#rows'][$row_num]['data']), $max_cols);

        $build['#rows'][$row_num]['class'] = array('catalog-row');
      }
      $build['#rows'][$row_num]['data'] = array_pad($build['#rows'][$row_num]['data'], $max_cols, array());

      foreach ($build['#rows'] as $row_num => $data) {
        foreach ($data['data'] as $colnum => $coldata) {
          $build['#rows'][$row_num]['data'][$colnum]['style'] = 'width:' . (100 / $max_cols) . '%';
        }
      }
    }*/
    // ::: OLD CODE










  }
  return $build;
}

/*******   SRuban mode-switch   ********/
function theme_catalog_mode_switches($variables) {

// m(file_create_url(drupal_get_path('module', 'addons') . '/lib/datepicker/ico_cal.png'));
  $catalog_object = $variables['catalog_object'];
//  if ($variables['catalog_object']->materials_settings['mode'] === 'table') {}
  $content_switches = array();
  $mode_switches = array();
  $active_switches = _catalog_materials_settings_mode_switches($catalog_object->materials_settings);
  //m($active_switches);
  if (count($active_switches)>1) {
    foreach ($active_switches as $mode_switch => $switch_caption) {
      if ($catalog_object->materials_settings['mode'] === $mode_switch) {
        $mode_switches[$mode_switch]['attributes']['class'][] = 'active';     
      }
      $mode_switches[$mode_switch]['title'] = $switch_caption;
      $mode_switches[$mode_switch]['href'] = 'catalog/' . $catalog_object->cid . '/switch/' . $mode_switch;
      $mode_switches[$mode_switch]['query'] = drupal_get_destination();
    }
  }

  if ($mode_switches) {
    $content_switches = array(
      '#theme' => 'links',
      '#links' => $mode_switches,
      '#attributes' => array(
        'class' => array(
          'tabs',
          'secondary',
        ),
      ),
      '#prefix'  => '<div class="catalog-switch-mode">',
      '#suffix'  => '</div>',
    );
  }
  return $content_switches;
}

/**
* orders in edit page
*/
function theme_catalog_admin_orders_table($variables) {
  $element = $variables['element'];
  $header = array(
    array('data' => t('Field name'), 'colspan' => 2,),
    array('data' => t('Sorting order'),),
  );
  if (!$element['#is_block']) {
    $header[]['data'] = ''; //t('Sorting allowed to user');
  }
  $header[]['class'][] = 'element-invisible';
  $rows  = array();
  foreach (element_children($element) as $key) {
    $element[$key]['default']['#title_display'] = $element[$key]['exposed']['#title_display'] = 'invisible';
    $row = array(
      '',
      $element[$key]['#title'],
      array('data' => $element[$key]['default']),
    );
    if (!$element['#is_block']) {
      $row[]['data'] = $element[$key]['exposed']; // hidden
    }
    $row[] = array(
      'data' => '<input type="hidden" class="drag-weight" />',
      'class' => array('element-invisible',),
    );
    $rows[] = array(
      'class' => array('draggable',),
      'data' => $row,
    );
  }
  drupal_add_tabledrag($element['#id'], 'order', 'sibling', 'drag-weight');
  setrawcookie('Drupal.tableDrag.showWeight', '', REQUEST_TIME - 3600, '/');
  drupal_add_css('.tabledrag-toggle-weight {display: none;}', 'inline');
  return theme('table', array(
    'attributes' => array('id' => $element['#id']),
    'rows' => $rows,
    'header' => $header,
  ));
}
/**
* orders with allowuser sets in new sort&filter page
*/
function theme_catalog_admin_filtersorting_table($variables) {
   //$element
  $element = $variables['element'];
  $parent = current($element['#parents']); // filters or orders etc

  $header = array(
    array('data' => t('Field name'), 'colspan' => 2,),
  );
  if ($parent === 'filters') {
    $header[]['data'] = t('Collapsed');
  }
  if (empty($element['#is_block'])) {
    $header[]['data'] = t('Show');
  }
  $header[]['class'][] = 'element-invisible';
  $rows  = array();
  foreach (element_children($element) as $key) {
    $element[$key]['default']['#title_display'] = $element[$key]['exposed']['#title_display'] = 'invisible';
    $row = array(
      '',
      $element[$key]['#title'],
      /*array('data' => $element[$key]['default']),*/
    );
    if ($parent === 'filters') {
      $row[]['data'] = $element[$key]['collapsed'];
    }
    if (empty($element['#is_block'])) {
      $row[]['data'] = $element[$key]['exposed'];
    }
    $row[] = array(
      'data' => '<input type="hidden" class="drag-weight" />',
      'class' => array('element-invisible',),
    );
    $rows[] = array(
      'class' => array('draggable',),
      'data' => $row,
    );
  }
  drupal_add_tabledrag($element['#id'], 'order', 'sibling', 'drag-weight');
  setrawcookie('Drupal.tableDrag.showWeight', '', REQUEST_TIME - 3600, '/');
  drupal_add_css('.tabledrag-toggle-weight {display: none;}', 'inline');
  return theme('table', array(
    'attributes' => array('id' => $element['#id']),
    'rows' => $rows,
    'header' => $header,
  ));
}

/*******   filter form of exposed filter elements  ********/
function theme_catalog_filter_exposed($variables) {
/*   THEME TEMPORALY NOT USED! see drupal_get_form ('catalog_filter_exposed_form',$catalog_object)
    $exposed = FALSE;
  $catalog_object = $variables['catalog_object'];

  foreach ($catalog_object->filters as $method => $data)  {
    if (!empty($data['exposed'])) { $exposed = TRUE ; break;}
  }
  if ($exposed) {
   //   $content_filters = array('#markup' => 'filters exposed : not implemented yet');
    $content_filters = drupal_get_form ('catalog_filter_exposed_form',$catalog_object);
  } else {
    $content_filters = array();
//      $content_filters = array(  '#markup' => 'theme_catalog_filter_exposed -> none filters exposed'   );
  }
  return $content_filters;
*/
}
