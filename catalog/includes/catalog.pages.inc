<?php

/**
* Callback for /catalog/%catalog_object
*/
function catalog_page($catalog_object, $term = NULL) {

  if (empty($catalog_object->cid)) {
    // We have virtual catalog object (created at runtime). This object not guaranteed contains all fields like dtatabse columns
    // Fill missing
    
    $missing = array(
      'status' => 1,
      'orders' => array(),
      'cid' => 0,
      'tid' => NULL,
      'vid' => NULL,
      'is_block' => 0,
      'title' => NULL,
      'menu_name' => NULL,
      'materials_settings' => array(),
      'description' => NULL,
      'format' => NULL,
      'filters' => array(),
      //'table_view' => FALSE,
      //'cols' => 1,
      //'rows' => 10,
    //  'zoom' => NULL,
      //'center' => NULL,    
    );
    foreach ($missing as $n => $v) {
      if (!property_exists($catalog_object, $n)) {
        $catalog_object->{$n} = $v;
      }
    }

  }
  if (isset($_GET['feed']) && $catalog_object->entity_type === 'node') {
    $catalog_object->depth = 0;
    $catalog_object->orders = array('created' => array('enabled' => TRUE, 'descending' => FALSE,));
    // $catalog_object->cols = 1;
    // $catalog_object->rows = 50;
    //var_export($catalog_object);die;
    $ids = _catalog_select_entities($catalog_object, TRUE);
    $site_name = variable_get('site_name', '');
    node_feed($ids, array(
      'title'  => !empty($site_name) ?
        $site_name . ' | ' . $catalog_object->title :
        $catalog_object->title,
      'description' => $catalog_object->description ? check_markup($catalog_object->description, $catalog_object->format, '', TRUE) : variable_get('feed_description', ''),
      'link'  => url('catalog/' . $catalog_object->cid, array(
        //'query'    => array('feed' => NULL),
        'absolute'  => TRUE,
      )),
    ));
    //stop processing render
    drupal_exit();
  }
  $level = count(arg()) - 2;
  $view_mode = 'level_' . min($level, 9);

    // settings for display materials
  if ($catalog_object->cid) {
    $catalog_object->materials_settings = addons_extra_field_get_settings('catalog_object', $catalog_object->cid, 'display', $view_mode, 'materials');
    if (isset($catalog_object->materials_settings)) {
      $mode = isset($_COOKIE['catalog_mode_' . $catalog_object->cid]) ? $_COOKIE['catalog_mode_' . $catalog_object->cid] : $catalog_object->materials_settings['mode'];
      if (!empty($catalog_object->materials_settings['modes'][$mode]['enabled'])) {
        $catalog_object->materials_settings['mode'] = $mode;
      }
      else {
        unset ($_COOKIE['catalog_mode_' . $catalog_object->cid]);
      }
    }
    else {
      $catalog_object->materials_settings = array (
          'mode' => 'list',  // current mode type
          'modes' => array(
            'list' => array(
              'enabled' => 1,
                'settings' => array( 'caption' => t('List'), 'cols' => '1', 'rows' => '10', 'center' => 0,  'zoom' => '', ),
                ),
        ),
      );
    }
  }


  if (!$term && $catalog_object->tid) { // we in root url "/catalog/1"  and catalog is set to sub-term by default -->> therefore we use default term id.
    // Problems: duplicate urls ( /catalog/2 will equal to /catalog/2/10 )
    $term = taxonomy_term_load($catalog_object->tid);
  }
  if ($term && function_exists('i18n_taxonomy_localize_terms')) {
    $term = i18n_taxonomy_localize_terms($term);
  }

  // Case of /catalog/4/50 and catalog settings not attached to term or vid
  if ($term && !$catalog_object->vid && !$catalog_object->tid && arg(0) === 'catalog' && arg(1) == $catalog_object->cid && arg(2)) {
    menu_set_active_item('catalog/' . $catalog_object->cid);
  }

  $catalog_object->current_params = array(
    'term' => $term,
    'vocabulary' => isset($catalog_object->vid) ? taxonomy_vocabulary_load($catalog_object->vid) : NULL,
    'entity_type' => $catalog_object->entity_type,
    'bundle'  => isset($catalog_object->bundle) && $catalog_object->entity_type === 'node' ? node_type_load($catalog_object->bundle) : NULL,
  );

  if ($catalog_object->is_block) {
    $data = &drupal_static('menu_local_tasks');
    $data = array();
    drupal_not_found();
    drupal_exit();
  }

  if ($catalog_object->cid && !$catalog_object->menu_name && $term && $catalog_object->tid != $term->tid) {
    $breadcrumb  = drupal_get_breadcrumb();

    $parents = taxonomy_get_parents_all($term->tid);
    array_shift($parents);
    $catalog_uri = catalog_uri($catalog_object);

    $breadcrumb[] = l($catalog_object->title, $catalog_uri['path']);

    while ($parent = array_pop($parents)) {
      if ($parent->tid != $catalog_object->tid) {
        $breadcrumb[] = l($parent->name, $catalog_uri['path'] . "/" . _catalog_build_term_url($parent->tid));
      }
    }
    $breadcrumb = array_unique($breadcrumb);
    drupal_set_breadcrumb($breadcrumb);
  }

  $build = array(
    '#attached' => array(
      'css' => array(
        drupal_get_path('module', 'catalog') . '/catalog.css',
      ),
    ),
    '#prefix' => '<div class="catalog-object' . ($catalog_object->status ? ''
        : ' inactive') . ' ' . $catalog_object->materials_settings['mode'] . '-view">', //highlight with #fff4f4 bg, see catalog.css (as unpublished node)
    '#suffix' => '</div>',
  );

  //term and material add links
  //($term ? $term : ($vocabulary ? $vocabulary : ($node_type ? $node_type : NULL)));

  _catalog_set_shared_object_state($catalog_object->cid, $term ? $term->tid : NULL);



  /*// "order by ..." link was clicked:
  if (!empty($_GET['order'])) { // Do not allow the user to sort entities by urls not presented in sorting header or not exposed.
    if (!empty($catalog_object->orders[$_GET['order']]['exposed'])) {
      // Deactivate enabled by defaults order methods
      foreach ($catalog_object->orders as $method => $data) {
        unset($catalog_object->orders[$method]['enabled']);
      }
      // Activate order method selected by user
      $catalog_object->orders[$_GET['order']]['enabled'] = TRUE;
      // Apply asc|desc sort rule
      if (!empty($_GET['sort'])) {
        $catalog_object->orders[$_GET['order']]['descending'] =  strtolower($_GET['sort']) == 'desc';
      }
  }
  }*/

  // Change current state of catalog object orders/filters by input user query
  catalog_object_apply_query_url($catalog_object);

  catalog_object_build_content($catalog_object, $view_mode, $term);


  // Store last visited catalog in cookies
  if ($catalog_object->cid) {
    user_cookie_save(array('catalog_id' => $catalog_object->cid));
    $_COOKIE['Drupal_visitor_catalog_id'] = $catalog_object->cid;
  }

  $title = $catalog_object->title;
  if (module_exists('site_translate') && $catalog_object->cid) {
    global $language;
    $translations = site_translate_translations_load('catalog', $catalog_object->cid, NULL, $language->language);
    if (!empty($translations['title'])) {
      $title = $translations['title'];
    }
  }
  global $language;
  if (language_default('language') != $language->language && module_exists('translation') && drupal_multilingual()) {
    if (!empty($catalog_object->translations[$language->language]['title'])) {
      $title = $catalog_object->translations[$language->language]['title'];
    }
  }
  drupal_set_title($term && (!$catalog_object->tid || $catalog_object->tid != $term->tid) ? $term->name : $title);

  $build['#contextual_links'] = array();
  if ($term) {
    $build['#contextual_links']['taxonomy'] = array(
      'taxonomy/term',
      array($term->tid),
    );
  }
  $build += $catalog_object->content;
  if ($catalog_object->cid) {
    $sub_url = $term ? _catalog_build_term_url($term->tid, FALSE) : NULL;
    $uri = 'catalog/' . $catalog_object->cid . ($sub_url ? '/' . $sub_url : '');
    $q = $_GET;
    unset($q['q']);
    ksort($q);
    drupal_add_html_head_link(array('rel' => 'shortlink', 'href' => url($uri, array('query' => $q, 'alias' => TRUE,))), TRUE);
    unset($q['page']); // A.Lazarev wish 18 dec. 2017: Ќа всех страницах пагинации указать атрибут rel="canonical" на первую (основную) страницу.
    drupal_add_html_head_link(array('rel' => 'canonical', 'href' => url($uri, array('query' => $q))), TRUE);
  }


  return $build;
}




function catalog_object_build_content($catalog_object, $view_mode = NULL, $term = NULL) {
  $catalog_object->cid = $catalog_object->cid ?? NULL;
  $catalog_object->content = array(
    '#entity_type' => 'catalog_object',
    '#bundle' => $catalog_object->cid,
  );
  $field_bundle_settings = field_bundle_settings('catalog_object', $catalog_object->cid);
  //$level = count(arg()) - 2;
  //$view_mode = 'level_' . min($level, 9);
  $view_mode = key(entity_view_mode_prepare('catalog_object', array($catalog_object->cid => $catalog_object), $view_mode));
  $catalog_object->content['#view_mode'] = $view_mode;

  $extra_fields_display_settings = field_extra_fields_get_display('catalog_object', $catalog_object->cid, $view_mode);


  if (($catalog_object->vid || $catalog_object->tid || $term) && !empty($extra_fields_display_settings['terms']['visible'])) {
    $tree = array();
    foreach (taxonomy_get_tree($catalog_object->vid, ($term ? $term->tid : ($catalog_object->tid ? $catalog_object->tid  : 0)), 1, TRUE) as $t) {
      if (taxonomy_term_view_access($t)) {
        $tree[$t->tid] = $t;
      }
    }

    if ($tree) {
    // extra field:
      $catalog_object->content['terms'] = array(
        '#type' => 'item',
        '#title' => t('Terms'),
        'table' => theme('catalog_list_terms', array(
          'tree' => $tree,
          'catalog_object' => $catalog_object,
          'cols' => addons_extra_field_get_settings('catalog_object', $catalog_object->cid, 'display', $view_mode, 'terms', 'cols'),
          'colwrap' => addons_extra_field_get_settings('catalog_object', $catalog_object->cid, 'display', $view_mode, 'terms', 'colwrap'),
          'center' => addons_extra_field_get_settings('catalog_object', $catalog_object->cid, 'display', $view_mode, 'terms', 'center'),
          'table_view' => addons_extra_field_get_settings('catalog_object', $catalog_object->cid, 'display', $view_mode, 'terms', 'table_view'),
        )));
    }
  }

  if ($catalog_object->cid) {
     /******* catalog filter ********/
    // MOVED TO block :: $exposed = FALSE;// MOVED TO block ::
    // MOVED TO block :: foreach ($catalog_object->filters as $method => $data)  {
    // MOVED TO block ::   if (!empty($data['exposed'])) {
    // MOVED TO block ::     $exposed = TRUE;
    // MOVED TO block ::     break;
    // MOVED TO block ::   }
    // MOVED TO block :: }
    // MOVED TO block :: if ($exposed) {
    // MOVED TO block ::   $catalog_object->content['filter_exposed']['form'] =  drupal_get_form('catalog_filter_exposed_form', $catalog_object);
    // MOVED TO block :: }

     /******* catalog mode switcher   ********/
    $catalog_object->content['mode_switches'] = theme('catalog_mode_switches', array(
      'catalog_object' => $catalog_object, 'center' => TRUE,
    ));

    if (!empty($extra_fields_display_settings['admin_links']['visible'])) {
      //create node link
      if ($catalog_object->entity_type == 'node' && !empty($catalog_object->bundle) && node_access('create', $catalog_object->bundle)) {
        $node_type_name = node_type_get_name($catalog_object->bundle);
        // extra field:
        $catalog_object->content['admin_links'][] = array(
          '#markup' => l($term ? t('+ @node_type in %term', array('@node_type' => $node_type_name, '%term' => $term->name)) : t('+ @node_type', array('@node_type' => $node_type_name)), 'node/add/' . str_replace('_', '-', $catalog_object->bundle), array(
              'query' => ($term ? array('tid' => $term->tid) : array()) + array('destination' => $_GET['q']),
              'html' => TRUE,
              'attributes' => array(
                'class' => array('form-submit'),
                'title' => $term ? t('Add %name into "%term" term', array('%name' => $node_type_name, '%term' => $term->name,)) : t('Add %name', array('%name' => $node_type_name,)),
              )
            )
          )
        );
      }
      //create user link
      elseif ($catalog_object->entity_type == 'user' && !empty($catalog_object->bundle) && $catalog_object->bundle == 'user' && user_access('administer users')) {
        // extra field:
        $catalog_object->content['admin_links'][] = array(
          '#markup' => l(t('user')/*t('Add user')*/, 'admin/people/create', array(
              'query' => ($term ? array('tid' => $term->tid) : array()) + drupal_get_destination(),
              'attributes' => array(
                'class' => array('ajax-popup', 'form-submit'),
              )
            )
          )
        );
      }


      //create term link
      if (!empty($catalog_object->vid) && ($v = taxonomy_vocabulary_load($catalog_object->vid))) {
        $access = user_access('administer taxonomy') || user_access("edit terms in $v->vid");
        if (!$access) {
          $item = menu_get_item('admin/structure/taxonomy/' . $v->machine_name . '/add');
          if (isset($item['access'])) {
            $access = $item['access'];
          }
        }
        if ($access) {
        // extra field:
          $catalog_object->content['admin_links'][] = array(
            '#markup' => l(t('Category')/*t('Add term')*/, 'admin/structure/taxonomy/' . $v->machine_name . '/add', array(
              'query' => ($term ? array('tid' => $term->tid) : array()) + drupal_get_destination(),
              'attributes' => array(
                'title' => t('Add term into "%name" vocabulary', array('%name' => $v->name)),
                'class' => array('form-submit'),
              ),
            )),
          );
        }
      }

      if (!empty($catalog_object->content['admin_links'])) {
        $catalog_object->content['admin_links'] += array(
          '#type' => 'container',
          '#attributes' => array(
            'class' => array('catalog-admin-links', ),
          ),
        );
      }
    }

  }

  // extra field:
  // Display "description" extra field only where not is block, and we in top level or view mode is customized
  if (empty($catalog_object->is_block) && (!empty($field_bundle_settings['view_modes'][$view_mode]['custom_settings']) || $view_mode === 'level_0')) {
    if (!empty($extra_fields_display_settings['description']['visible'])) {
      $description = $catalog_object->description;
      $format = $catalog_object->format;
      global $language;
      if (module_exists('site_translate')) {
        $translations = site_translate_translations_load('catalog', $catalog_object->cid, NULL, $language->language);
        if (!empty($translations['description'])) {
          $description = $translations['description'];
        }
      }
      if (language_default('language') != $language->language && module_exists('translation') && drupal_multilingual()) {
        if (!empty($catalog_object->translations[$language->language]['description']['value'])) {
          $description = $catalog_object->translations[$language->language]['description']['value'];
          $format = $catalog_object->translations[$language->language]['description']['format'];
        }
      }
      if (!filter_format_load($format)) {
        $format = filter_fallback_format();
      }
      if ($description) {
        $catalog_object->content['description'] = array(
          '#type' => 'container',
          '#attributes' => array('class' => array('field', 'catalog-description')),
          array('#markup' => check_markup($description, $format),),
        );
      }
    }

  }
     /*
    if ($catalog_object->materials_settings['mode']=='map') {
         print_r($catalog_object->content['pager1']);
         die();
      }
       */
    // extra field:
  if (!empty($extra_fields_display_settings['object']['visible'])) {
    $catalog_object->content['object'] = NULL;
    if ($term) {
      $m = addons_extra_field_get_settings('catalog_object', $catalog_object->cid, 'display', $view_mode, 'object', 'view_mode');
      $catalog_object->content['object'] = taxonomy_term_view($term, $m ? $m : 'full');      
    }
  }
  if (!empty($extra_fields_display_settings['parent']['visible'])) {
    $catalog_object->content['parent'] = NULL; 
    if ($term && ($parent_terms = taxonomy_get_parents($term->tid))) {
      $parent_term = reset($parent_terms);
      $m = addons_extra_field_get_settings('catalog_object', $catalog_object->cid, 'display', $view_mode, 'parent', 'view_mode');
      $catalog_object->content['parent'] = taxonomy_term_view($parent_term, $m ? $m : 'full');
    }
  }

  //if (in_array( $catalog_object->materials_settings['mode'], array('list','table'))) // simplest way!
  if (isset($catalog_object->materials_settings['modes'][$catalog_object->materials_settings['mode']]['settings']['rows'])) { // adaptive variant
    if ((!$catalog_object->cid || !empty($extra_fields_display_settings['materials']['visible'])) && ($ids = _catalog_select_entities($catalog_object))) {
      $pager = array();
      if ($catalog_object->_enable_pager && isset($catalog_object->_pager_element)) { // Internal temporary property added by _catalog_select_entities() function
      
        if (!empty($catalog_object->content['description'])) {
           // A. Lazarev wish 18 dec 2017: —крыть поле "ќписание" на страницах пагинации в списках материалов https://itinity.ru/node/3088
           
          if ($pager_dependency = addons_extra_field_get_settings('catalog_object', $catalog_object->cid, 'display', $view_mode, 'description', 'pager_dependency')) { // NULL|first|last @see catalog_extra_field_formatter_settings_form()
            if (($p = pager_find_page($catalog_object->_pager_element))) {
              if ($pager_dependency === 'first' || $p != $GLOBALS['pager_total'][$catalog_object->_pager_element] - 1) {
                $catalog_object->content['description']['#access'] = FALSE;
              }        
            }
            elseif ($pager_dependency === 'last') {
              $catalog_object->content['description']['#access'] = FALSE;
            }
          }
        }



        $pager = array(
          '#theme' => 'pager',
          '#element' => $catalog_object->_pager_element,//_catalog_pager_element($catalog_object->cid),
        );
        // extra field:
        $catalog_object->content['pager1'] = $pager;
      }

      if ($catalog_object->materials_settings['mode'] === 'list') {
        if ($sort_links = catalog_object_get_sorting_links($catalog_object)) {
          // extra field:
          $catalog_object->content['sort_links'] = array(
            '#type' => 'item',
            '#title' => t('Order by'),
            '#attributes' => array(
              'class' => array('catalog-sort'),
            ),
            'links' => array(
              '#theme' => 'links',
              '#attributes' => array(
                'class' => array('links inline'),
              ),
            ),
          );
          foreach ($sort_links as $order_method => $link) {
            $catalog_object->content['sort_links']['links']['#links']['order-by-' . drupal_html_class($order_method) . ' '  . (in_array('active', $link['options']['attributes']['class']) ? 'active' : '')] = array(
              'html' => TRUE,
              'title' => theme('link', $link),
              '#order_method' => $order_method,
            );
          }
        }
      }

      $load_fn = $catalog_object->entity_type . '_load_multiple';
      $loaded_entities = $load_fn($ids);

      $m_s = $catalog_object->materials_settings;
      $c_rows = $m_s['modes'][$m_s['mode']]['settings']['rows'];
      $c_cols = isset($m_s['modes'][$m_s['mode']]['settings']['cols']) ? $m_s['modes'][$m_s['mode']]['settings']['cols'] : 1;
      if ($c_rows * $c_cols == 1) {
        $materials_view_mode = 'full';
      }
      else {
        $addons_entity_view_modes = addons_entity_view_modes($catalog_object->entity_type, $catalog_object->bundle, FALSE, 'label', array('rss', 'full', 'search_index',));
        $materials_view_mode = !empty($m_s['modes'][$m_s['mode']]['settings']['view_mode']) && !empty($addons_entity_view_modes[$m_s['modes'][$m_s['mode']]['settings']['view_mode']]) ? $m_s['modes'][$m_s['mode']]['settings']['view_mode'] : 'teaser';
      }

      //field_attach_prepare_view($catalog_object->entity_type, $loaded_entities, $c_cols * $c_rows == 1 ? 'full' : $materials_view_mode);
      //entity_prepare_view($catalog_object->entity_type, $loaded_entities);
      $entities = array();
      $view_fn = $catalog_object->entity_type . '_view';
      foreach ($loaded_entities as $entity) {
        _catalog_set_shared_object_state($catalog_object->cid, $term ? $term->tid : NULL);

        $entity->_current_catalog = $catalog_object;
        $entities[] = $view_fn($entity, $materials_view_mode);
      }
      $catalog_object->content += array(
        'materials' => theme('catalog_list_entities', array('catalog_object' => $catalog_object, 'entities' => $entities,)),
      );

      $catalog_object->content['pager2'] = $pager;
    }
    elseif (empty($catalog_object->content['terms']['#rows']) && user_is_logged_in()) {
      $catalog_object->content['materials'] = array(
        '#markup' => check_markup($catalog_object->entity_type == 'user' && $catalog_object->bundle == 'user' ? t('There is currently no users in this list.') : t('There is currently no content in this list.')),
      );
    }
  }

  if ($catalog_object->entity_type === 'node') {
    $state = _catalog_get_shared_object_state();
    $feed_uri = 'catalog/' . $catalog_object->cid;
    $feed_title = 'RSS' . (!empty($catalog_object->title) ? ' - ' . $catalog_object->title : '');
    if (!empty($state['cid']) == $catalog_object->cid && !empty($state['tid'])) {
      $feed_uri .= '/' . _catalog_build_term_url($state['tid']);
      $feed_title .= ' -> ' . taxonomy_term_load($state['tid'])->name;
    }
    drupal_add_html_head_link(array(
      'rel' => 'alternate',
      'type' => 'application/rss+xml',
      'title' => $feed_title,
      'href' =>url($feed_uri, array(
        'query' => array('feed' => NULL),
        'absolute' => TRUE,
      )),
    ));
    if (!empty($extra_fields_display_settings['feed']['visible'])) {
      $catalog_object->content['feed'] = array(
        '#type' => 'markup',
        '#markup' => theme('feed_icon', array(
        'url'  => url($feed_uri, array(
          'query' => array('feed' => NULL),
          'absolute' => TRUE,
        )),
        'title'  => $feed_title,
        )),
      );
    }

  }

  if (!empty($catalog_object->cid)) {
    field_attach_prepare_view('catalog_object', array($catalog_object->cid => $catalog_object), $view_mode);
    entity_prepare_view('catalog_object', array($catalog_object->cid => $catalog_object));

    // Display user added fields only where we in top level or view mode is customized
    if (!$catalog_object->is_block && (!empty($field_bundle_settings['view_modes'][$view_mode]['custom_settings']) || $view_mode === 'level_0')) {
      $catalog_object->content += field_attach_view('catalog_object', $catalog_object, $view_mode, $GLOBALS['language']->language);
    }
    elseif ($catalog_object->cid && !$catalog_object->is_block) {
      $catalog_object->content['#pre_render'][] = '_field_extra_fields_pre_render';
    }

    module_invoke_all('entity_view', $catalog_object, 'catalog_object', $view_mode, $GLOBALS['language']->language);
    $type = 'catalog_object';
    $catalog_object->content['#catalog_object'] = $catalog_object;
    drupal_alter(array('entity_view'), $catalog_object->content, $type);
    //$catalog_object->content = _field_extra_fields_pre_render($catalog_object->content);
  }

  return $catalog_object->content;


}
