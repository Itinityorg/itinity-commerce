<?php


/**
 * Callback for /admin/reports/contacts
 */
function contacts_report_page() {
  $header = array(
    '_timestamp' => array(
      'data' => t('Date'),
      'field' => 'cs.timestamp',
      'sort' => 'desc',
    ),
    '_url' => array(
      'data' => t('Page from message sent'),
      'field' => 'cs.url',
    ),
    '_name' => array(
      'data' => t('L.F.M.'),
      'field' => 'cs.lfm',
    ),
    '_ip' => array(
      'data' => t('IP'),
      'field' => 'cs.ip',
    ),
    '_phone' => array(
      'data' => t('Phone'),
      'field' => 'cs.phone',
    ),
    '_mail' => array(
      'data' => t('E-mail'),
      'field' => 'cs.mail',
    ),
    '_message' => array(
      'data' => t('Message'),
      'field' => 'cs.message',
    ),
  );

  $query = db_select('contacts_submissions', 'cs')
    ->fields('cs')
    ->extend('PagerDefault')
    ->extend('TableSort');
  $query->limit(50)->orderByHeader($header);
  $rows = array();
  foreach ($query->execute() as $r) {
    $row = array(
      '_timestamp' => array(
        'data' => format_date($r->timestamp),
      ),
      '_url' => array(
        'data' => $r->url ? l($r->url, $r->url, array(
          'attributes' => array(
            'target' => '_blank',
            'class' => array('popup',),
          ),
        )) : '&nbsp;',
      ),
      '_name' => array(
        'data' => $r->uid ? l($r->lfm, 'user/' . $r->uid, array(
          'attributes' => array(
            'target' => '_blank',
            'class' => array('popup',),
          ),
        )) : check_plain($r->lfm),
      ),
      '_ip' => array(
        'data' => check_plain($r->ip),
      ),
      '_phone' => array(
        'data' => theme('phone', array('phone' => ($r->phone))),
      ),
      '_mail' => array(
        'data' => theme('email', array('email' => $r->mail)),
      ),
      '_message' => array(
        'data' => l(truncate_utf8($r->message, 100, TRUE, TRUE), 'admin/reports/contacts/' . $r->id, array(
          'html' => TRUE,
          'attributes' => array(
            'title' => t('Details'),
            'target' => '_balnk',
            'class' => array('popup',)
          )
        )),
      ),
    );
    $rows[] = $row;
  }

  $build['table'] = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
    '#empty' => t('Messages not found'),
  );
  $build['pager']['#theme'] = 'pager';

  return $build;
}


/**
 * Callback for /admin/reports/contacts/%
 */
function contacts_form_view_message($msgid) {
  if ($msg = db_query('SELECT * FROM {contacts_submissions} WHERE id=:id', array(':id' => $msgid))->fetch()) {
    $build = array(
      'message' => array(
        '#type' => 'item',
        '#title' => t('Message text'),
        '#markup' => check_markup($msg->message),
      ),
      'lfm' => array(
        '#type' => 'item',
        '#title' => t('Sender'),
        '#inline' => TRUE,
        '#markup' => $msg->uid ? l($msg->lfm, 'user/' . $msg->uid, array(
          'attributes' => array(
            'target' => '_blank',
            'class' => array('popup',)
          )
        )) : check_plain($msg->lfm),
      ),
      'phone' => array(
        '#type' => 'item',
        '#access' => !empty($msg->phone),
        '#title' => t('Phone'),
        '#inline' => TRUE,
        '#markup' => theme('phone', array('phone' => $msg->phone)),
      ),
      'mail' => array(
        '#type' => 'item',
        '#access' => !empty($msg->mail),
        '#title' => t('E-mail'),
        '#inline' => TRUE,
        '#markup' => theme('email', array('email' => $msg->mail)),
      ),
      'timestamp' => array(
        '#type' => 'item',
        '#title' => t('Submission date'),
        '#inline' => TRUE,
        '#markup' => format_date($msg->timestamp),
      ),
      'ip' => array(
        '#type' => 'item',
        '#title' => t('IP'),
        '#inline' => TRUE,
        '#markup' => check_plain($msg->ip),
      ),
      'url' => array(
        '#type' => 'item',
        '#access' => !empty($msg->url),
        '#title' => t('URL of the page from which the message was sent'),
        '#inline' => TRUE,
        '#markup' => $msg->url ? l($msg->url, $msg->url) : NULL,
      ),
    );
    return $build;
  }
  drupal_not_found();
  drupal_exit();
}


function contacts_requisites_settings_form($form, &$form_state) {
  $form['view'] = array(
    '#type' => 'item',
    '#markup' => '<div style="text-align:right">' . l(t('User agreement'), 'agreement') . '</div><div class = "clearfix"></div>',
  );

  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Juridical name'),
    '#size' => 70,
  );

  $form['full_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Full name', array(), array('context' => 'contacts')),
    '#size' => 70,
  );

  $form['inn'] = array(
    '#type' => 'textfield',
    '#title' => t('TIN'),
    '#description' => t('Taxpayer Identification Number'),
    '#maxlength' => 12,
  );

  $form['ogrn'] = array(
    '#type' => 'textfield',
    '#title' => t('MSRN'),
    '#description' => t('The main state registration number'),
    '#maxlength' => 20,
  );

  $form['kpp'] = array(
    '#type' => 'textfield',
    '#title' => t('TRVC'),
    '#description' => t('Taxpayer record validity code'),
    '#maxlength' => 20,
  );

  $form['account'] = array(
    '#type' => 'textfield',
    '#title' => t('Settlement account'),
    '#maxlength' => 20,
  );

  $form['bic'] = array(
    '#type' => 'textfield',
    '#title' => t('BIC'),
    '#description' => t('Bank identification code'),
    '#maxlength' => 9,
  );

  $form['bank'] = array(
    '#type' => 'textfield',
    '#title' => t('Bank name'),
  );

  $form['corr'] = array(
    '#type' => 'textfield',
    '#title' => t('Correspondent Account'),
    '#maxlength' => 20,
  );

  $form['representative'] = array(
    '#type' => 'fieldset',
    '#title' => t('The representative of the company'),
    '#description' => t('The company concludes contracts on behalf of the representative'),
    '#tree' => TRUE,
    '#access' => FALSE,
  );

  $form['representative']['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Last First Middle'),
  );

  $form['representative']['job'] = array(
    '#type' => 'textfield',
    '#title' => t('Job'),
  );

  $form['representative']['base'] = array(
    '#type' => 'textfield',
    '#title' => t('Base'),
  );

  $form['address'] = array(
    '#type' => 'textfield',
    '#title' => t('Juridical address'),
    '#size' => 100,
  );

  $form['mail_address'] = array(
    '#type' => 'textfield',
    '#title' => t('Mail address'),
    '#size' => 100,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  utils_fill_form_defaults($form, contacts_load_settings('company_requisites'));

  return $form;
}

function contacts_requisites_settings_form_submit($form, $form_state) {
  form_state_values_clean($form_state);
  contacts_save_settings('company_requisites', $form_state['values']);
  drupal_set_message(t('The changes have been saved.'));
}


/**
 * Form callback for /admin/content/contact
 * Setting up geo position, contacts and working time of company.
 */
function contacts_about_settings_form($form, &$form_state) {
//IKW: Redundant functional
//  $form['view'] = array(
//    '#type' => 'item',
//    '#markup' => '<div style="text-align:right">' . l(t('About company'), 'about') . '</div><div class = "clearfix"></div>',
//  );
  $form['names'] = array(
    '#type' => 'item',
    '#tree' => TRUE,
  );

  $form['names']['trade'] = array(
    '#type' => 'textfield',
    '#title' => t('Trade name'),
    '#size' => 128, //IKW
    '#default_value' => variable_get('site_name', basename(conf_path())),
  );
  $form['logotype'] = array(
    '#type' => 'managed_file',
    '#title' => t('Logotype'),
    '#upload_location' => 'public://',
    '#upload_validators' => array(
      'file_validate_extensions' => array('png jpg jpeg gif'),
    ),
    '#button_name' => t('Insert a picture'),
  );
  //IKW: Redundant functional
//  $form['about_company'] = array(
//    '#type' => 'text_format',
//    '#title' => t('About company'),
//  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  $site_contacts = contacts_load_settings('about_company');
  if (module_exists('notify')) {
    unset($site_contacts['notify']);
  }

  $site_contacts['about_company'] = !empty($site_contacts['about_company']['value']) ? $site_contacts['about_company']['value'] : '';
  utils_fill_form_defaults($form, $site_contacts);

  if (($logo = theme_get_setting('logo')) && ($f = file_load_multiple(NULL, array('uri' => preg_replace('/^' . preg_quote(file_create_url('public://'), '/') . '/i', 'public://', $logo))))) {
    $form['logotype']['#default_value'] = key($f);
  }

  return $form;
}

/**
 * Pre-render for ['users'] in contacts_about_settings_form
 */
function _contacts_fill_users($element) {
  if (isset($element['#default_value']) && !empty($element['#default_value'])) {
    foreach ($element['#default_value'] as $key => &$acc) {
      $user = user_load($acc['uid']);
      $element[$key]['job']['#markup'] = !empty($user->job) ? check_plain($user->job) : '';
      $element[$key]['lfm']['#markup'] = check_plain($user->lfm);
      $element[$key]['phone']['#markup'] = check_plain($user->phone);
      $element[$key]['mail']['#markup'] = check_plain($user->mail);
    }
  }
  return $element;
}


/**
 * Submit contacts_about_settings_form from
 * Save settings into variables.
 * Save images permanent.
 * Set laim theme logo.
 */
function contacts_about_settings_form_submit($form, &$form_state) {

  //return;
  if (isset($form_state['values']['logotype'])) {
    if (!empty($form_state['values']['logotype']) && ($file = file_load($form_state['values']['logotype']))) {
      //$values = variable_get('theme_laim_settings', NULL);
      global $theme;
      $values = variable_get('theme_' . $theme . '_settings', NULL);

      if (!isset($values)) {
        $values = variable_get('theme_settings', array());
      }
      $values['default_logo'] = 0;

      // Use system "public://" file stream wrapper.
      $values['logo_path'] = $file->uri;
      // DO NOT use absolute paths (with HTTP://site.domain/....)
      // $values['logo_path'] = file_create_url($file->uri);
      if ($file->status != FILE_STATUS_PERMANENT) {
        $file->status = FILE_STATUS_PERMANENT;
        file_save($file);
      }
    }
    else {
      $values['default_logo'] = 0;
      $values['logo_path'] = '';
    }

    //	variable_set('theme_' . $theme . '_settings', $values);
    variable_set('theme_settings', $values);

  }

  if (!empty($form_state['values']['facade']) && ($file = file_load($form_state['values']['facade']))) {
    if ($file->status != FILE_STATUS_PERMANENT) {
      $file->status = FILE_STATUS_PERMANENT;
      file_save($file);
    }
  }

  form_state_values_clean($form_state);

  contacts_save_settings('about_company', $form_state['values']);

  drupal_set_message(t('The changes have been saved.'));
}


/**
 * Form builder for watermark settings.
 *
 * @ingroup forms
 * @see contacts_about_contacts_form_submit()
 */
function contacts_about_contacts_form($form, &$form_state) {
  $form['view'] = array(
    '#type' => 'item',
    '#markup' => '<div style="text-align:right">' . l(t('View contacts'), 'contacts') . '</div><div class = "clearfix"></div>',
  );

  $form['general'] = array(
    '#type' => 'multifield_wrapper',
    '#collapsible' => TRUE,
    '#elements_to_cells' => TRUE,
    '#description' => t('General organization contacts'),
    '#title' => t('Contacts'),
    '#min' => 1,
    '#default_value' => array(
      0 => array(
        'element' => array(
          'type' => 'phone',
        ),
      ),
    ),
    'element' => array(
      '#type' => 'contacts',
      '#preview' => FALSE,
      '#required' => TRUE,
      '#default_value' => array(
        'type' => 'phone',
      ),
    ),
  );

  if (module_exists('notify')) {
    $form['notify'] = array(
      '#type' => 'item',
      '#tree' => TRUE,
    );
    notify_admin_module($form['notify'], $form_state, array(
      'contacts',
      'send_contact_form'
    ));
  }

  $form['contacts']['message']['#title'] = t('The form of sending messages');
  $form['contacts']['message']['uid']['#title'] = t('Recipients');

  $form['address'] = array(
    '#type' => 'address',
    '#title' => t('Address'),
    '#map' => TRUE,
  );

  $form['facade'] = array(
    '#type' => 'managed_file',
    '#title' => t('Appearance or location map'),
    '#upload_location' => 'public://',
    '#upload_validators' => array(
      'file_validate_extensions' => array('png jpg jpeg gif'),
    ),
    '#button_name' => t('Insert a picture'),
  );

  $form['additional'] = array(
    '#title' => t('Additional information'),
    '#tree' => TRUE,
  );

  $form['additional']['working_time'] = array(
    '#title' => t('Business time'),
    '#type' => 'multifield_wrapper',
    '#tree' => TRUE,
    '#prefix' => '<div class = "container-inline">',
    '#suffix' => '</div>',
  );
  $form['additional']['working_time']['days'] = array(
    '#type' => 'days',
  );

  $form['additional']['working_time']['working_hours'] = array(
    '#type' => 'item',
    '#title' => t('Business hours'),
  );

  $opts = array('--:--',);
  foreach (range(0, 24 * 3600, 1800) as $t) {
    $opts[] = gmdate('G:i', $t);
  }
  $opts = drupal_map_assoc($opts);

  $form['additional']['working_time']['working_hours']['start'] = array(
    '#type' => 'select',
    '#options' => $opts,
    '#field_prefix' => t('from'),
  );

  $form['additional']['working_time']['working_hours']['end'] = array(
    '#type' => 'select',
    '#options' => $opts,
    '#field_prefix' => t('to'),
    '#field_suffix' => ',',
  );

  $form['additional']['working_time']['lunch_hours'] = array(
    '#type' => 'item',
  );

  $form['additional']['working_time']['lunch_hours']['start'] = array(
    '#type' => 'select',
    '#options' => $opts,
    '#field_prefix' => t('break on'),
  );

  $form['additional']['working_time']['lunch_hours']['end'] = array(
    '#type' => 'select',
    '#options' => $opts,
    '#field_prefix' => t('to'),
  );
  $form['additional']['detailed'] = array(
    '#type' => 'text_format',
    '#title' => t('Other information'),
  );

  $form['users'] = array(
    '#title' => t('Contact persons'),
    '#type' => 'multifield_wrapper',
    '#elements_to_cells' => TRUE,
    '#tree' => TRUE,
    '#min' => 1,
    '#prefix' => '<div id = "select-users-div">',
    '#suffix' => '</div>',
    '#pre_render' => array('_contacts_fill_users'),
  );
  $form['users']['uid'] = array(
    '#type' => 'entity_selector',
    '#selector_entity_type' => 'user',
    '#title' => t('Username'),
  );

  $form['users']['lfm'] = array(
    '#type' => 'item',
    '#title' => t('Last First Middle'),
  );

  $form['users']['sphere'] = array(
    '#type' => 'textfield',
    '#title' => t('Sphere of responsibility'),
  );

  $form['users']['job'] = array(
    '#type' => 'item',
    '#title' => t('Job'),
  );

  $form['users']['mail'] = array(
    '#type' => 'item',
    '#title' => t('Email'),
  );

  $form['users']['phone'] = array(
    '#type' => 'item',
    '#title' => t('Phone'),
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  $site_contacts = contacts_load_settings('site_contacts');
  if (module_exists('notify')) {
    unset($site_contacts['notify']);
  }
  utils_fill_form_defaults($form, $site_contacts);

  $form['additional']['detailed']['#default_value'] = !empty($site_contacts['additional']['detailed']['value']) ? $site_contacts['additional']['detailed']['value'] : NULL;
  $form['additional']['detailed']['#format'] = !empty($site_contacts['additional']['detailed']['format']) ? $site_contacts['additional']['detailed']['format'] : 'advanced';

  return $form;
}


/**
 * Submit handler.
 *
 * @param array $form
 *   Form API form.
 * @param array $form_state
 *   Form API form.
 */
function contacts_about_contacts_form_submit($form, &$form_state) {
  form_state_values_clean($form_state);

  contacts_save_settings('site_contacts', $form_state['values']);
  if (module_exists('notify')) {
    notify_admin_module_save('contacts', $form_state['values']['notify']['contacts']);
  }

  drupal_set_message(t('The changes have been saved.'));
}









