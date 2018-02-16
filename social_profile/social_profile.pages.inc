<?php


/**
 * Menu callback for /user @see also social_profile_menu_alter()
 */
function _social_profile_userpage_page_callback() {

  if (!user_is_logged_in()) {
    return drupal_get_form('user_login');
  }

  global $user;
  $account = user_load($user->uid);
  $build = array(
    '#type' => 'container',
    '#attributes' => array(
      'class' => array('user-cabinet'),
    ),
    '#attached' => array(
      'css' => array(
        drupal_get_path('module', 'social_profile') . '/css/social_profile.css',
      ),
    ),
    'user_panel' => array(
      '#type' => 'container',
      '#attributes' => array(
        'class' => array('user-panel', 'container-inline'),
      ),
      'picture' => array(
        '#theme' => 'user_picture',
        '#account' => $user,
      ),
      'links' => array(
        '#theme' => 'links',
        '#attributes' => array('class' => array('inline', 'links')),
        '#links' => array(
          'username' => array(
            'title' => '<h5>' . check_plain(format_username($user)) . '</h5> <small class="description">(' . check_plain($user->mail) . ')</small>',
            'html' => TRUE,
          ),
          'profile' => array(
            'title' => theme('pictogram', array('icon' => 'user')) . '<span class="user-link-inline">' . t('Profile') . '</span>',
            'href' => 'user/' . $user->uid,
            'html' => TRUE,
          ),
          'edit' => array(
            'title' => theme('pictogram', array('icon' => 'pencil')) . '<span class="user-link-inline">' . t('Edit') . '</span>',
            'href' => 'user/' . $user->uid . '/edit',
            'html' => TRUE,
          ),
          'settings' => array(
            'title' => theme('pictogram', array('icon' => 'cog')) . '<span class="user-link-inline">' . t('Settings') . '</span>',
            'href' => 'user/' . $user->uid . '/settings',
            'html' => TRUE,
          ),
        ),
      ),
      'links_logout' => array(
        '#theme' => 'links',
        '#attributes' => array('class' => array('link-logout', 'inline', 'links')),
        '#links' => array(
          'logout' => array(
            'title' => theme('pictogram', array('icon' => 'off')) . '<span class="user-link-inline">' . t('Logout') . '</span>',
            'href' => 'user/logout',
            'html' => TRUE,
          ),
        ),
      ),
    ),
    'blocks' => array(
      '#prefix' => '<div class="user-cabinet-links">',
      '#suffix' => '</div>',
    ),

  );
  //get available for user menu items
  foreach (
    db_query('SELECT * FROM {menu_router} mr WHERE path REGEXP \'^user\/%(\/[^%\/]+)+\' AND type AND (type&:t1 OR type&:t2 OR type&:t3)', array(
      ':t1' => MENU_LOCAL_TASK,
      ':t2' => MENU_NORMAL_ITEM,
      ':t3' => MENU_DEFAULT_LOCAL_TASK,
    )) as $r
  ) {
    $path = $r->path;
    if (
      !arg(4, $path)
      && !in_array(arg(2, $path), array('view', 'edit', 'settings', 'cancel',), TRUE)
      && ($item = menu_get_item(preg_replace('/^user\/%/', 'user/' . $user->uid, $path)))
      && $item['access']
    ) {
      // CRUTCH (@valeriesokolova wish): disabled menu items must be not visible it admin menu and user "cabinet" page
      // @see also social_profile_admin_menu_output_alter()
      $hidden = db_query('SELECT hidden FROM {menu_links} ml WHERE (ml.router_path = :path) AND module = \'system\'', array(':path' => preg_replace('/\/%($|\/)/', '$1', $item['path'])))->fetchField();
      if ($hidden === FALSE || !$hidden) {
        if (!arg(3, $path) && !in_array($path, array('user/%/edit', 'user/%/settings', 'user/%/cancel'))) {
          $build['blocks'][$path] = array(
            '#type' => 'container',
            '#weight' => $item['weight'],
            'title' => array(
              '#markup' => '<h2 class="item-title">' . l($item['title'], $path === 'user/%/view' ? 'user/' . $user->uid : $item['href']) . '</h2>',
            ),
            '#attributes' => array(
              'class' => array('block', 'has-title', 'support-colors', 'cabinet-item', 'user-' . drupal_html_class(substr($path, 7))),
            ),
            '#prefix' => '<div class="outer-cabinet-item inline-block-wrapper-3">',
            '#suffix' => '</div>',
          );
        }
        else {
          if ($item['title'] && !preg_match('/^user\/%\/edit\//', $path)) {
            if (in_array($path, array('user/%/edit', 'user/%/settings', 'user/%/cancel'))) {
              $parent_path = 'user/%/view';

            }
            else {
              $parent_path = preg_replace('/(^user\/%\/[^\/]+).*/', '$1', $path);
            }
            $build['blocks'] += array($parent_path => array());
            $build['blocks'][$parent_path] += array(
              'content' => array(
                '#type' => 'container',
                '#attributes' => array(
                  'class' => array('content'),
                ),
                'links' => array(
                  '#theme' => 'links',
                  '#links' => array(),
                  '#attributes' => array('class' => array('menu')),
                ),
              ),
            );
            $build['blocks'][$parent_path]['content']['links']['#links'][drupal_html_class($path)] = array(
              'title' => $item['title'],
              'href' => $item['href'],
              'weight' => $item['weight'],
            );
            uasort($build['blocks'][$parent_path]['content']['links']['#links'], 'drupal_sort_weight');
          }
        }
      }

    }
  }
  foreach (element_children($build['blocks']) as $key) {
    if (empty($build['blocks'][$key]['title'])) {
      unset($build['blocks'][$key]);
    }
    elseif (empty($build['blocks'][$key]['content'])) {
      $build['blocks'][$key]['#attributes']['class'][] = 'is-empty';
    }
  }
  return array('content' => $build);
}

/**
 * Menu callback for /user/%user/content/*
 */
function social_profile_user_content_list($account, $node_type = NULL) {

  $header = array(
    'title' => array('data' => t('Title'), 'field' => 'n.title'),
  );
  if (!$node_type) {
    $header += array(
      'type' => array('data' => t('Type'), 'field' => 'n.type'),
    );
  }

  $header += array(
    'status' => array('data' => t('Status'), 'field' => 'n.status'),
    'changed' => array('data' => t('Updated'), 'field' => 'n.changed', 'sort' => 'desc'),
    'operations' => array('data' => t('Operations')),
  );

  $node_types = array();
  if ($node_type) {
    $node_types[] = $node_type;
  }
  else {
    foreach (node_type_get_types() as $nt) {
      if (($item = menu_get_item('user/' . $account->uid . '/content/' . $nt->type)) && arg(3, $item['path']) === str_replace('_', '-', $nt->type) && $item['access']) {
        $node_types[] = $nt->type;
      }
    }
  }
  $rows = array();
  $pager_element = PagerDefault::$maxElement;
  if ($node_types) {
    $query = db_select('node', 'n')
      ->fields('n', array('nid', 'uid', 'changed', 'title', 'status', 'type',))
      ->extend('PagerDefault')->element($pager_element)
      ->extend('TableSort');
    $query->condition('n.uid', $account->uid)->condition('n.type', $node_types);
    $query->limit(50)->orderByHeader($header);
    foreach ($query->execute() as $r) {
      $rows[$r->nid] = array(
        'class' => $r->status ? array() : array('unpublished'),
        'data' => array(
          '_title' => array(
            'data' => array(
              'link' => array(
                '#theme' => 'entity_link',
                '#entity_type' => 'node',
                '#entity' => $r->nid,
              ),
            ),
          ),
        ),
      );
      if (!$node_type) {
        $rows[$r->nid]['data']['_type'] = array(
          'data' => check_plain(node_type_get_name($r->type)),
        );
      }
      $rows[$r->nid]['data']['_status'] = array(
        'data' => $r->status ? t('Published') : t('Unpublished'),
      );
      $rows[$r->nid]['data']['_changed'] = array(
        'data' => format_date($r->changed, 'medium'),
      );

      $node = node_load($r->nid); // node_access() expects node object (line 3046)
      // IKW: add edit (and delete) links to publications list
      $rows[$r->nid]['data']['_operations'] = array(
        'data' => array(
          'edit' => array(
            '#theme' => 'link',
            '#text' => t('edit'),
            '#path' => "node/{$r->nid}/edit",
            '#access' => node_access('update', $node),
            '#options' => array(
              'html' => FALSE,
              'attributes' => array(),
              'query' => drupal_get_destination(),
            ),
            '#suffix' => ' &nbsp; ',
          ),
          'delete' => array(
            '#theme' => 'link',
            '#text' => t('delete'),
            '#path' => "node/{$r->nid}/delete",
            '#access' => node_access('delete', $node),
            '#options' => array(
              'html' => FALSE,
              'attributes' => array(),
              'query' => drupal_get_destination(),
            ),
          ),
        ),
      );
    }
  }


  $form['nodes'] = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
    '#empty' => t('List is empty'),
  );
  if ($rows) {
    $form['pager'] = array(
      '#theme' => 'pager',
      '#element' => $pager_element,
    );
  }
  else {
    PagerDefault::$maxElement--;
    global $pager_page_array, $pager_total, $pager_total_items, $pager_limits;
    unset($pager_total_items[$pager_element]);
    unset($pager_total[$pager_element]);
    unset($pager_page_array[$pager_element]);
    unset($pager_limits[$pager_element]);
  }
  return $form;
}

/**
 * Menu callback for /user/%user/content/comments
 */
function social_profile_user_comments_list($account) {

  $header = array(
    'subject' => array('data' => t('Subject'), 'field' => 'c.subject'),
    'node' => array('data' => t('Material'), 'field' => 'n.title',),
  );

  $header += array(
    'status' => array('data' => t('Status'), 'field' => 'c.status'),
    'created' => array('data' => t('Created'), 'field' => 'c.created', 'sort' => 'desc'),
  );
  $pager_element = PagerDefault::$maxElement;
  $query = db_select('comment', 'c')
    ->fields('c', array('nid', 'cid', 'created', 'subject', 'status',))
    ->extend('PagerDefault')->element($pager_element)
    ->extend('TableSort');
  $query->condition('c.uid', $account->uid);

  $query->join('node', 'n', 'n.nid = c.nid');
  $query->fields('n', array('title', 'type',));

  $query->limit(50)->orderByHeader($header);
  $rows = array();
  foreach ($query->execute() as $r) {
    $rows[$r->cid] = array(
      'class' => $r->status ? array() : array('unpublished'),
      'data' => array(
        '_subject' => array(
          'data' => array(
            'link' => array(
              '#theme' => 'entity_link',
              '#entity_type' => 'comment',
              '#entity' => $r->cid,
            ),
          ),
        ),
        '_node' => array(
          'data' => array(
            'link' => array(
              '#theme' => 'entity_link',
              '#entity_type' => 'node',
              '#entity' => $r->nid,
              '#prefix' => node_type_get_name($r) . ' <em>',
              '#attributes' => array('class' => array('popup',)),
              '#suffix' => '</em>',
            ),
          ),
        ),
      ),
    );
    $rows[$r->cid]['data']['_status'] = array(
      'data' => $r->status ? t('Published') : t('Unpublished'),
    );
    $rows[$r->cid]['data']['_created'] = array(
      'data' => format_date($r->created, 'medium'),
    );
  }
  $form['comments'] = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
    '#empty' => t('List is empty'),
  );
  if ($rows) {
    $form['pager'] = array(
      '#theme' => 'pager',
      '#element' => $pager_element,
    );
  }
  else {
    PagerDefault::$maxElement--;
    global $pager_page_array, $pager_total, $pager_total_items, $pager_limits;
    unset($pager_total_items[$pager_element]);
    unset($pager_total[$pager_element]);
    unset($pager_page_array[$pager_element]);
    unset($pager_limits[$pager_element]);

  }

  return $form;
}






