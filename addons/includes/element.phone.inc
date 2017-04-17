<?php


/**
* Format phone number to more readable representation
*/
function addons_format_phone($phone, $country_code = NULL, $region_code = NULL) {
  $country_code = preg_replace('/[^\d]/u', '', $country_code);
  $region_code = preg_replace('/[^\d]/u', '', $region_code);

  if ($country_code === '' && $region_code === '' && strlen(preg_replace('/[^\d]/u', '', $phone)) >= 10) {
    list($country_code, $region_code, $phone) = array_values(addons_parse_phone($phone));
    
    if (!$country_code) { // Try to detect russian cellular phone number without +7|8 by first 3 digits (phones like '9114567891')
      $p = preg_replace('/[^\d]/u', '', $phone);
      if (strlen($p) == 10 && $p[0] === '9') {
        list($country_code, $region_code, $phone) = array_values(addons_parse_phone('+7' . $phone));
      }
    }
  }
  
  if ($country_code !== '') {
    $country_code = '+' . $country_code;
  }
  if ($region_code !== '') {
    $region_code = '(' . $region_code . ')';
  }
  if ($phone !== NULL && $phone !== '') {
    $phone = preg_replace('/[^\d]/u', '', $phone);
    $phone = rtrim(preg_replace('/\d{3}/', '$0-', $phone), '-'); // 1234567 -->> 123-456-7
    $phone = preg_replace('/(\d)-(\d)$/', '-$1$2', $phone); // 123-456-7 -->> 123-45-67
  }
  if ($country_code !== '' && $phone !== '' && $region_code === '') {
    $region_code = '-';
  }
  return $country_code . $region_code . $phone;
}


function addons_parse_phone($phone) {
  module_load_include('inc', 'addons', 'includes/phone.parser');
  return PhonesParser::getInstance()->parsePhoneNumber($phone);
}

function _addons_theme_phone($variables) {
	//$variables['phone'] = preg_replace('/(^([^0-9(]+))|([^0-9-()])|([^0-9)]+$)/', '', $variables['phone']);
	//разбивка на префикс(код)номер - deprecated так как мы живём в дс
	//$variables['phone'] = preg_replace('/(^([^0-9]+))|([^0-9-])|([^0-9]+$)/', '', $variables['phone']);
	//$variables['phone'] = preg_replace('/^(\+?){1}([0-9]){1}([0-9]{3})([0-9]+)$/', "\$1\$2(\$3)\$4", $variables['phone']);

  // Build phone numbers from parts
  $phone_number = addons_format_phone($variables['phone'], $variables['country_code'], $variables['region_code']);
 // m($variables, $phone_number);
  if ($phone_number) {
    $variables += array(
      'attributes' => array(),
    );
    $variables['attributes'] += array(
      'title' => t('Phone number'),
    );
    $variables['attributes']['class'][] = 'phone';
    $variables['attributes']['href'] = 'tel:' . $phone_number;

    return '<a' . drupal_attributes($variables['attributes']) . '>' . $phone_number . '</a>';
  }
}


function addons_validate_phone(&$element, &$form_state) {
  if (!empty($element['#skip_validation'])) {
    return; // @see field_phone_form_field_ui_field_edit_form_alter()
  }

	if ($element['#value']) {

    // specific check for regional restrictions
    if (!empty($element['#fixed_country'])) {
      $regional_restrictions_regexp = NULL;
      if (!empty($element['#mobile_only'])) { // check that this number is mobile
        if ($country_mobile_mask = _addons_phone_get_mobile_mask_for_country($element['#fixed_country'])) {
          if (
              empty($element['#required'])
              && _addons_phone_number_matches_the_mask($element['#value'], $country_mobile_mask)
             ) { // field had a prefilled mask but was not filled in (user left default value as is)
            form_set_value($element, NULL, $form_state); // clear prefilled value from submit
            return; // stop any further validation
          }

          if (empty($element['#allow_change_country'])) { // if the user is not allowed to change default country
            $regional_restrictions_regexp = _addons_phone_mask_to_regexp($country_mobile_mask);
            if (!preg_match('/' . $regional_restrictions_regexp . '/', $element['#value'])) {
              form_error($element, t('%phone is not a mobile phone', array('%phone' => $element['#value'])));
            }
          }
        }
      }
      else { // check only that number has a valid country code (only russian and USA are possible by the moment)
        if ($country_phone_code = addons_phone_code_info($element['#fixed_country'], 'code')) {
          if (
              empty($element['#required'])
              && _addons_phone_number_matches_the_mask($element['#value'], $country_phone_code)
             ) { // field had a prefilled mask but was not filled in (user left default value as is)
            form_set_value($element, NULL, $form_state); // clear prefilled value from submit
            return; // stop any further validation
          }

          if (empty($element['#allow_change_country'])) { // if the user is not allowed to change default country
            $regional_restrictions_regexp = '^\+?' . $country_phone_code;
            if (!preg_match('/' . $regional_restrictions_regexp . '/', $element['#value'])) {
              form_error($element, t('%phone - invalid country code', array('%phone' => $element['#value'])));
            }
          }
        }
      }
    }

    // generic check
    //$regexp = "/^\(?\+?(?:[0-9().\s-]){10,21}[0-9]$/i";/*we living in dc*/
		//$regexp = "/^\(?\+?(?:[0-9().\s-]){4,21}[0-9]$/i"; /*we need in additional number*/
		$regexp = '/^\(?\+?(?:[0-9().\s-]){4,21}[0-9](?:[ #0-9]{6})?$/i';
		if (!preg_match($regexp, $element['#value'])) {
			form_error($element, t('%phone is not a valid phone', array('%phone' => $element['#value'])));
		}
    else {
      form_set_value($element, $element['#value'] = str_replace('-', '', addons_format_phone($element['#value'])), $form_state);
    }
	}
}

function _addons_phone_number_matches_the_mask($phone_number, $phone_mask) {
  $phone_number = preg_replace('/\D/', '', $phone_number); // remove all except digits
  $phone_mask = preg_replace('/\D/', '', $phone_mask); // remove all except digits
  return $phone_number === $phone_mask;
}

function _addons_pre_render_phone($element) {
  $element['#value'] = addons_format_phone($element['#value']);
  drupal_add_js(array('addonsFormatPhoneUrl' => url('addons/format-phone')), 'setting');
  if (!empty($element['#masked'])) {

      //drupal_add_library('addons', 'inputmask-multi');
      // однозначно без накладок форматим вывод
      $element['#value'] = addons_format_phone('+' . str_replace(array(' ', '+', '-', '/', '(', ')'), '', $element['#value']));
      /*
      drupal_add_library('addons', 'inputmask-multi');
      // setting to use in js when loading JSON map with country/area codes
      drupal_add_js(array(
        'addons' => array(
          'inputmaskDirectoryPath' => file_create_url(drupal_get_path('module', 'addons') . '/lib/inputmask_multi'),
        ),
      ), 'setting');
      */
      // js to attach the library
      drupal_add_js(drupal_get_path('module', 'addons') . '/js_behaviors/phone_field_masked.js');
       // add a class to attach the library against
      $element['#attributes']['class'][] = 'form-phone-masked-field';

  }

  // setup additional attributes on element to use later
  if (!empty($element['#fixed_country'])) { // country code (e.g. "ru", "us") to restrict numbers input

    $element['#attributes']['data-country-code'] = $element['#fixed_country'];

    $element_has_no_value = (!isset($element['#default_value']) || !strlen($element['#default_value'])) && (!isset($element['#value']) || !strlen($element['#value']));

    if (!empty($element['#allow_change_country'])) { // country can be changed by the user when inputting a number
      $element['#attributes']['data-can-change-country'] = 'can-change-country'; // зачем тут это длинное 'can-change-country', если потом в js это используется всего лишь в контектсе if() ?
    }

    if (!empty($element['#mobile_only'])) { // can be used only with #fixed_country together (by the moment - for "ru" only)
      $element['#attributes']['mobile_only'] = 'mobile_only';

      // preset mobile mask for a country
      if ($country_mobile_mask = _addons_phone_get_mobile_mask_for_country($element['#fixed_country'])) {
        $element['#attributes']['data-origin-regexp'] = _addons_phone_mask_to_regexp($country_mobile_mask);
        if ($element_has_no_value) {
          if (!empty($element['#masked'])) { // for masked field - set mask as is (with '#' placeholders)
            $element['#value'] = $country_mobile_mask;
          }
          else { // for simple field - set clean value without any '#', '-', ' ' etc.
            $element['#value'] = '+' . preg_replace('/\D/', '', $country_mobile_mask);
          }
        }
      }
    }
    else { // only country restriction is set
      // preset phone code for a country
      if ($country_phone_code = addons_phone_code_info($element['#fixed_country'], 'code')) {//_addons_phone_get_country_phone_code($element['#fixed_country'])) {
        $element['#attributes']['data-origin-regexp'] = '^\+?' . $country_phone_code;
        if ($element_has_no_value) {
          $element['#value'] = '+' . $country_phone_code;
        }
      }
    }

  }

  $element['#attributes']['size'] = 17;

  return $element;
}


/**
* Returns a list of available countries (for those we have JSON map with codes).
* Use optional parameters $country and $property for more flexibility
e.g. addons_phone_code_info(NULL, 'title') will return array('ru' => 'Russia', ..., ...);
addons_phone_code_info('ru', 'title') will return 'Russia'
addons_phone_code_info('ru', 'code') will return 7
*/
function addons_phone_code_info($country = NULL, $property = NULL) {
  $list = array(
    'ru' => array(
      'title' => t('Russia'),
      'code' => 7,
    ),
    'us' => array(
      'title' => t('USA'),
      'code' => 1,
    ),
    'ua' => array(
      'title' => t('Ukraine'),
      'code' => 380,
    ),
  );
  foreach ($list as $code => $data) {
    $list[$code]['title'] .= ' (+' . $data['code'] . ')';
    if ($property) {
      $list[$code] = isset($list[$code][$property]) ? $list[$code][$property] : NULL;
    }
  }
  return $country ? (isset($list[$country]) ? $list[$country] : NULL) : $list;
}


/**
 * Returs a list of available countries (for those we have JSON map with codes).
 * @return array
 *//*
function _addons_phone_get_available_countries_list() {
  return array(
    'ru' => t('Russia (+7)'),
    'us' => t('U.S.A. (+1)'),
  );
}*/

/**
 * Returns common country phone code for a country
 * @param string $country_code Country code, e.g. "ru"
 * @return string|null
 *//*
function _addons_phone_get_country_phone_code($country_code) {
  switch ($country_code) {
    case 'ru':
      return '7';
    case 'us':
      return '1';
    default:
      return NULL;
  }
}
*/
/**
 * Loads and decodes a map containing regional codes data for a country.
 * @param string $country_code Country code, e.g. "ru"
 * @return array|null
 */
function _addons_phone_get_regional_codes_map($country_code) { // @TODO: merge body of this function with _addons_phone_get_mobile_mask_for_country() body
  static $cache = array();
  if (!isset($cache[$country_code])) {
    $cache[$country_code] = array();
    //$map_filename = sprintf('%s/lib/inputmask_multi/phones-%s.json', drupal_get_path('module', 'addons'), $country_code);
    $map_filename = drupal_get_path('module', 'addons') . '/lib/inputmask_multi/phones-' . $country_code . '.json';
    if (is_file($map_filename) && ($json = file_get_contents($map_filename))) {
      $cache[$country_code] = json_decode($json);
    }
  }
  return $cache[$country_code];
}


/**
 * Searches for a mask that describes mobile numbers for a country.
 * @param string $country_code Country code, e.g. "ru"
 * @return array|null
 */
function _addons_phone_get_mobile_mask_for_country($country_code) {
  static $cache = array();
  if (!isset($cache[$country_code])) {
    $cache[$country_code] = array();
    foreach (_addons_phone_get_regional_codes_map($country_code) as $mask_data) {
      if (isset($mask_data->type) && $mask_data->type === 'mobile') {
        $cache[$country_code] = $mask_data->mask;
      }
    }
  }
  return $cache[$country_code];
}


/**
 * Converts phone mask (as returned from _addons_phone_get_mobile_mask_for_country()) to a regexp
 * @param string $phone_mask
 * @return string
 */
function _addons_phone_mask_to_regexp($phone_mask) {
  $regexp = '^' . str_replace('#', '.?', $phone_mask) . '$'; // mask includes "#" instead of every missing digit
  $regexp = str_replace(array('(', ')', '-', '+'), array('\(?', '\)?', '\-?', '\+?'), $regexp); // escape special characters
  return $regexp;
}


/**
* Delivery callback for /ajax/element-phone-mask/%
*/
function addons_phone_field_ajax_delivery($page_callback_result) {
	drupal_add_http_header('Content-Type', 'text/html; charset=utf-8');
	print $page_callback_result;
}


/**
 *  Callback for /ajax/element-phone-mask/%
 */
function addons_phone_field_ajax($phone) {
  $result = addons_format_phone(str_ireplace('plus', '+', $phone)); // 'plus' - @see phone_field_masked.js
  if (strpos($phone, 'plus') === 0 && strpos($result, '+') === FALSE) {
    $result = '+' . $result;
  }
  return $result;
}











