<?php


/**
 * Callback for /contacts
 */
function contacts_page() {
  $page['#attached']['css'][] = drupal_get_path('module', 'contacts') . '/css/contacts.css';

  drupal_add_library('system', 'drupal.collapse');
  // WTF module_load_include('inc', 'contact', 'contact.pages');
  $contacts = contacts_load_settings('site_contacts');
  $list = '';
  if (!empty($contacts['general'])) {
    $list = theme('contacts_general', array(
      'general' => $contacts['general']
    ));
  }

  $page['edit'] = array(
    '#type' => 'item',
    '#markup' => '<div style="text-align:right">' . l(t('Edit contacts'), 'admin/content/about/contacts') . '</div><div class = "clearfix"></div>',
    '#access' => user_access('bypass node access'),
  );

  $requisites = contacts_load_settings('company_requisites');
  if (isset($requisites)) {
    $juridical_name = (!empty($requisites['name'])) ? array(
      '#type' => 'item',
      '#markup' => t('Juridical name') . ':&nbsp' . $requisites['name'],
    ) : '';
  }

  $page['col-1']['names'] = array(
    '#children' => (!empty($contacts['names']['trade']) ? '<h2>' . $contacts['names']['trade'] . '</h2>' : '') . $list,
    '#collapsible' => FALSE,
    '#attributes' => array(
      'id' => 'names',
    ),
    '#type' => 'fieldset',

  );

  //address
  if (isset($contacts)) {
    $facade = '';
    if (!empty($contacts['facade']) && ($file = file_load($contacts['facade']))) {
      if (in_array(substr(strrchr($file->filename, '.'), 1), array(
        'png',
        'gif',
        'jpg',
        'jpeg'
      ))) {
        $facade = theme('image_style', array(
          'style_name' => module_exists('features') /* @see features_image_default_styles() */ ? '240x180' : 'large',
          'path' => $file->uri
        ));
      }
    }
    $address = theme('contacts_address', array(
      'address_elements' => !empty($contacts['address']) ? $contacts['address'] : array(),
    ));
    if ($facade || $address) {
      $page['col-1']['address'] = array(
        '#title' => t('Address'),
        '#children' => '<div class="site-contacts-address-wrapper">' . $facade . $address . '</div>',
        '#collapsible' => FALSE,
        '#attributes' => array(
          'id' => 'address',
        ),
        '#type' => 'fieldset',
      );
    }
  }
  if (!empty($contacts['additional']['working_time'])) {
    $page['col-1']['working_time'] = array(
      '#markup' => theme('working_time', array(
        'working_time' => $contacts['additional']['working_time'],
        'detailed' => !empty($contacts['additional']['detailed']) ? $contacts['additional']['detailed'] : array(),
      )),
    );
  }


  //message
  $contact_form = drupal_get_form('contacts_form');
  $contact_form['subject']['#value'] = '';
  $contact_form['subject']['#type'] = 'value';


  $page['col-1']['message'] = ((module_exists('notify') && notify_is_assigned('contacts', 'send_contact_form')) || !module_exists('notify')) ? array(
    '#title' => t('Write message'),
    $contact_form,
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
    '#attributes' => array(
      'id' => 'message',
    ),
    '#type' => 'fieldset',
  ) : NULL;

  //users
  $users = array();
  if (!empty($contacts['users'])) {
    $page['col-1']['contact_persons']['#markup'] = theme('contact_persons', array('users' => $contacts['users']));
  }


  //map
  if (!empty($contacts['address']['gmap']['lat']) && !empty($contacts['address']['gmap']['lng']) && !empty($address)) {
    $id = _contacts_add_map($contacts['address']['gmap']['lat'], $contacts['address']['gmap']['lng']);

    if (!empty($contacts['address']['state']) || !empty($contacts['address']['city'])) { // IKW : do not display map where address not specified
      $page['col-2']['map'] = array(
        //'#title'		=> t('Location on the map'),
        '#children' => '<div id="' . $id . '"></div>',
        '#collapsible' => FALSE,
        '#collapsed' => FALSE,
        '#attributes' => array(
          'id' => 'map',
        ),
        '#type' => 'fieldset',
        //'#prefix'		=> '<div class="column-with-map">',
        //'#suffix'		=> '</div>',
      );
    }

  }

  if (!empty($page['col-1']) && !empty($page['col-2'])) {
    $page['col-1'] += array(
      '#prefix' => '<div class="contacts-page-col-1">',
      '#suffix' => '</div>',
    );
    $page['col-2'] += array(
      '#prefix' => '<div class="contacts-page-col-2">',
      '#suffix' => '</div>',
    );
  }
  return $page;
}


function contacts_form($form, &$form_state, $recipient = NULL) {
  form_load_include($form_state, 'inc', 'contacts', 'contacts.pages');
  global $user;
  $form = array();
  $form['lfm'] = array(
    '#type' => 'textfield',
    '#title' => t('Your name'),
    '#required' => TRUE,
  );
  $form['phone'] = array(
    '#type' => 'phone',
    '#title' => t('Phone'),
    '#required' => TRUE,
    '#masked' => FALSE,
  );
  $form['mail'] = array(
    '#type' => 'email',
    '#title' => t('Email address'),
    '#required' => TRUE,
  );
  foreach (array('lfm', 'phone', 'mail') as $key) {
    if (!empty($user->{$key})) {
      $form[$key]['#default_value'] = $user->{$key};
    }
  }
  $form['message'] = array(
    '#type' => 'textarea',
    '#title' => t('Message'),
    '#required' => TRUE,
  );
  $form['account'] = array(
    '#type' => 'value',
    '#value' => $user,
  );
  if (!empty($recipient)) {
    $form['recipient'] = array(
      '#type' => 'value',
      '#value' => $recipient,
    );
  }
  $form['actions'] = array(
    '#type' => 'actions',
    'submit' => array(
      '#type' => 'submit',
      '#value' => t('Submit'),
    ),
  );
  $form['#personal_data_agreement'] = array('group_name' => 'actions', 'button_name' => 'submit');

  return $form;
}

function contacts_form_submit($form, &$form_state) {

  // Flood protection (C/P from contact_site_form())
  $limit = variable_get('contact_threshold_limit', 5);
  $window = variable_get('contact_threshold_window', 3600);
  if (!flood_is_allowed('contact', $limit, $window) && !user_access('administer contact forms')) {
    drupal_set_message(t("You cannot send more than %limit messages in @interval. Try again later.", array(
      '%limit' => $limit,
      '@interval' => format_interval($window)
    )), 'error');
    drupal_access_denied();
    drupal_exit();
  }
  flood_register_event('contact', variable_get('contact_threshold_window', 3600));


  $site_mail = variable_get('site_mail', ini_get('sendmail_from'));
  $params = array(
    'lfm' => $form_state['values']['lfm'],
    'phone' => $form_state['values']['phone'],
    'mail' => $form_state['values']['mail'],
    'message' => $form_state['values']['message'],
    'from' => $form_state['values']['account'],
  );


  if (!empty($form_state['values']['recipient'])) {
    $recipient = $form_state['values']['recipient'];
    drupal_mail('contacts', 'message_to', $recipient->mail, user_preferred_language($recipient), $params);
  }
  else {
    // Log message
    $record = array(
      'ip' => ip_address(),
      'uid' => $GLOBALS['user']->uid,
      'lfm' => truncate_utf8($form_state['values']['lfm'], 128),
      'phone' => truncate_utf8($form_state['values']['phone'], 64),
      'mail' => truncate_utf8($form_state['values']['mail'], 255),
      'message' => $form_state['values']['message'],
      'timestamp' => REQUEST_TIME,
      'url' => $_GET['q'],
    );
    drupal_write_record('contacts_submissions', $record);

    //send form to admin
    drupal_mail('contacts', 'send_contact_form', $site_mail, language_default(), $params);
  }
  //send copy
  $language = user_preferred_language($form_state['values']['account']);
  if (!empty($form_state['values']['mail'])) {
    drupal_mail('contacts', 'copy', $form_state['values']['mail'], $language, $params);
  }
  drupal_set_message(t('Your message has been sent.'));
}


/**
 * Callback for /agreement
 */
function contacts_agreement_page() {
  $requisites = contacts_load_settings('company_requisites');
  $page = array();

  if (module_exists('social_profile')) {
    $page['edit'] = array(
      '#type' => 'item',
      '#markup' => '<div style="float:right">' . l(t('Edit agreement'), 'admin/content/about/agreement') . '</div><div class = "clearfix"></div>',
      '#access' => user_access('bypass node access'),
    );
    if ($text = social_profile_get_user_agreement_text()) {
      $page['user_agreement'] = array(
        '#type' => 'container',
        '#children' => $text,
      );
    }
  }

  $keys = array(
    'full_name' => t('Full name'),
    'inn' => t('Taxpayer Identification Number'),
    'ogrn' => t('The main state registration number'),
    'kpp' => t('Taxpayer record validity code'),
    'account' => t('Settlement account'),
    'bic' => t('Bank identification code'),
    'bank' => t('Bank name'),
    'corr' => t('Correspondent Account'),
    'address' => t('Juridical address'),
    'mail_address' => t('Mail address'),
  );

  foreach ($keys as $key => $title) {
    if (!empty($requisites[$key])) {
      $page['requisites'][$key] = array(
        '#type' => 'item',
        '#inline' => TRUE,
        '#title' => $title,
        '#markup' => check_plain($requisites[$key]),
      );
    }
  }

  if (!empty($page['requisites'])) {
    $page['requisites'] += array(
      '#type' => 'fieldset',
      '#title' => '<h2>' . t('Requisites') . '</h2>',
    );
  }
  $site_contacts = contacts_load_settings('site_contacts');

  if (!empty($site_contacts['address'])) {
    $address = '';
    foreach ($site_contacts['address'] as $key => $title) {
      if (!empty($title) && is_string($title)) {
        $address .= check_plain($title) . ', ';
      }
    }
    if ($address) {
      $page['contacts']['address'] = array(
        '#type' => 'item',
        '#inline' => TRUE,
        '#title' => t('Address'),
        '#markup' => substr($address, 0, count($address) - 3),
      );
    }
  }
  if (isset($site_contacts['general'])) {
    foreach ($site_contacts['general'] as $item) {
      if (isset($item['element']['data'])) {
        $page['contacts'][$item['element']['type']] = array(
          '#type' => 'item',
          '#inline' => TRUE,
          '#title' => $item['element']['type'],
          '#markup' => check_plain($item['element']['data']),
        );
      }
    }
  }
  if (!empty($page['contacts'])) {
    $page['contacts'] += array(
      '#type' => 'fieldset',
      '#title' => '<h2>' . t('Contacts') . '</h2>',
    );
  }

  return $page;
}


/**
 * Callback for /about
 */
function contacts_about_page() {
  $edit = '<div style="float:right">' . l(t('Edit info about company'), 'admin/content/about/about') . '</div><div class = "clearfix"></div>';
  $about = NULL;
  if ($about_company = contacts_load_settings('about_company')) {
    $about = $about_company['about_company']['value'];
  }
  return $edit . $about;
}






