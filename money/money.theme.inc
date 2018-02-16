<?php

/**
* This file always included by money.module
*/


/**
* Imlements hook_theme()
*/
function money_theme($existing, $type, $theme, $path) {
  return array(
    'price' => array(
      'variables' => array(
        'qty' => NULL, 
        'price' => NULL, 
        'currency' => NULL, 
        'format' => NULL, 
        'attributes' => array(),
      ),
    ),
  );
}


/**
* Themed html output for price
*/
function theme_price($variables) {
	$vars = array();
	foreach (array('qty', 'price', 'currency') as $key) {
		if (isset($variables[$key])) {
			$vars[$key] = $variables[$key];
		}
	}
	
	$variables = $vars + $variables;
	if (isset($variables['qty']) && !empty($variables['price'])) {
		$variables['qty'] .= ' x ';
	}

	if (isset($variables['price'])) {
		$price = $variables['price'] * 1;
		$variables['price'] = format_price($price, $variables['currency']);
	}

  $currency = $variables['currency'] ?? variable_get('money_default_currency', 'RUB');
//	if (!empty($variables['currency'])) {
//		$currency = $variables['currency'];
    $variables['currency'] = format_currency($currency, $variables['format']);
//	}

	$spacer = '&thinsp;';
	$out = array();
	foreach ($variables as $key => $data) {
		if ($key !== 'theme_hook_original') {
			if (isset($data)) {
				if ($key === 'price') {
          $out[$key] = '<span class="shop-item-' . $key . '" ' . ($currency ? 'data-currency="' . $currency . '"' : '') . ' data-price="' . (isset($price) ? $price : '') . '" style="white-space:nowrap;">' . preg_replace('/\s+/', $spacer, trim(check_plain($data))) . '</span>';
//					$out[$key] = '<span class="shop-item-' . $key . '" ' . ($currency ? 'data-currency="' . $currency . '"' : '') . ' data-price="' . (isset($price) ? $price  : '') . '" style="white-space:nowrap;">' . ($price ? preg_replace('/\s+/', $spacer, trim(check_plain($data))) : '-') . '</span>';
				}
				elseif ($key !== 'attributes' && $key !== 'format') {
					$out[$key] = '<span class="shop-item-' . $key . '">' . ($key == 'currency' ? '&nbsp;' : '') . check_plain($data)  . '</span>';
				}
			}
		}
	}
	$variables['attributes']['class'][] = 'shop-price';
	return $out ? '<span' . drupal_attributes($variables['attributes']) . '>' . implode('', array_filter($out)) . '</span>' : '';
}


/**
* Formatted price plain text
*/
function format_price($price, $currency = NULL) {
  static $results = array();
  $key = $price . ':::' . $currency;
  if (!array_key_exists($key, $results)) {
    $default_currency = variable_get('money_default_currency', 'RUB'); // price is always in rub
    $currency = !is_null($currency) ? $currency : $default_currency;
   // $rate = (drupal_strtolower($default_currency) != drupal_strtolower($currency)) ? money_currency_info($currency, 'rate') : 1;
   // $price *= (float) $rate;

    $currency = $default_currency;

    if ($currency) {
      $currency = format_currency($currency, 'id');
      if ($currency && ($info = money_currency_info($currency))) {
        if ($price) {
          $price = number_format($price, preg_match('/\./u', $price) ? $info['decimals'] : 0, '.', ' ');
        }
      }
      else {
        $price = number_format($price, preg_match('/\./u', $price) ? 2 : 0, '.', ' ');
      }
    }
    // IKW: Нолики после запятой не нужны $price = preg_replace('/^([^\.]+)$/', '$1.00', $price);
    $price = preg_replace('/^([^\.]+)$/', '$1', $price);
    $results[$key] = $price;
  }
  return $results[$key];
}


/**
* Formatted currency plain text
*/
function format_currency($currency, $format = 'reduction') { // full|symbol|id
  static $results = array();
  $key = $currency . ':::' . $format;
  if (!array_key_exists($key, $results)) {
    $results[$key] = NULL;
    if (preg_match('/[^A-Z]/u', $currency)) {
      if (module_exists('transliteration')) {
        $currency = transliteration_get($currency);
      }
      $currency = preg_replace('/[^a-z]/i', '', $currency);
    }
    
    if ($currency) {
      if ($info = money_currency_info($currency)) {
        switch ($format) {
          case 'full':
            $results[$key] = $info['full'];
          break;
          case 'symbol':
            $results[$key] = $info['symbol'];
          break;
          case 'id':
            $results[$key] = $info['id'];
          break;
          default:
            $results[$key] = $info['reduction'];
        }      
      }
      else {
        $results[$key] = $format === 'id' ? drupal_strtoupper(t($currency)) : t(drupal_strtolower($currency));
      }
    }    
  }
  return $results[$key];
}









