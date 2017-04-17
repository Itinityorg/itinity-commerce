<?php

/**
 * @file
 * Builds placeholder replacement tokens for user-related data.
 */

/**
 * Implements hook_token_info().
 */
function social_profile_token_info() {
  $types['user'] = array(
    'name' => t('Users'),
    'description' => t('Tokens related to individual user accounts.'),
    'needs-data' => 'user',
  );

  $user['lfm'] = array(
    'name' => t('Last First Middle'),
    'description' => t("Last First Middle"),
  );
  $user['phone'] = array(
    'name' => t("Phone"),
    'description' => t("Phone"),
  );

  return array(
    'types' => $types,
    'tokens' => array('user' => $user),
  );
}


/**
 * Implements hook_tokens().
 */
function social_profile_tokens($type, $tokens, array $data = array(), array $options = array()) {
  $sanitize = !empty($options['sanitize']);

  $replacements = array();

  if ($type == 'user' && !empty($data['user'])) {
    $account = $data['user'];
    foreach ($tokens as $name => $original) {
      switch ($name) {
        case 'lfm':
          $replacements[$original] = $sanitize ? check_plain($account->lfm) : $account->lfm;
          break;
        case 'phone':
          $replacements[$original] = $sanitize ? check_plain($account->phone) : theme('phone', array('phone' => $account->phone));
          break;
      }
    }
  }

  return $replacements;
}