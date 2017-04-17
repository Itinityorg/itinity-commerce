<?php

function theme_contact_persons($variables) {
	$users = array(
		'#type'		=> 'fieldset',
		'#title'	=> count($variables['users']) > 1 ? t('Contact persons') : t('Contact person'),
	);
	foreach ($variables['users'] as $key => $item) {
		$account = user_load($item['uid']);

		$users[$account->uid] = array(
			'#type'			=> 'item',
			'#attributes'	=> array(
				'class'	=>	array('contact-person', ),
			),
		);

		if (!empty($item['sphere'])) {
			$users[$account->uid]['sphere'] = array(
				'#markup'	=> '<h4>' . check_plain($item['sphere']) . '</h4>',
			);
		}

		$users[$account->uid]['lfm'] = array(
			'#type'		=> 'item',
			'#markup'	=> theme('username', array('account' => $account)),
		);

		$fields = array(
			//'lfm'	=> t('Given name'),
			'job'	=> t('Job'),
			'phone'	=> t('Phone'),
		);

		foreach ($fields as $key => $title) {
			if (!empty($account->$key)) {
				$users[$account->uid][$key] = array(
					'#type'		=> 'item',
					'#markup'	=> theme('phone', array('phone' => $account->$key)),
				);
			}
		}

		$users[$account->uid]['message'] = array(
			'#type'		=> 'item',
			'#markup'	=> l(drupal_strtolower(t('Write message')), 'user/' . $account->uid . '/contact', array(
				'attributes' => array('class' => array('ajax-popup')),
				'query' => drupal_get_destination(),
			)),
			'#access' => contacts_access($account),
		);
	}
	return drupal_render($users);
}


function theme_contacts_general($variables) {
	$fields = array(
		'phone'	=> array(
			'#title'	=> 'Phone',
		),
		'url'	=> array(
			'#title'	=> 'Site',
		),
		'email'	=> array(
			'#title'	=> 'Email',
		),
		'icq'	=> array(
			'#title'	=> 'ICQ',
		),
		'skype'	=> array(
			'#title'	=> 'Skype',
		),
		'textfield'	=> array(),
	);
	foreach ($variables['general'] as $contact) {
		switch ($contact['element']['type']) {
			case 'textfield' :
				if (!empty($contact['element']['data'])) {
					$fields['other'][] = $contact['element']['data'];
				}
			break;
			default :
				if (!empty($contact['element']['data'])) {
					$fields[$contact['element']['type']][] = theme($contact['element']['type'], array($contact['element']['type'] => $contact['element']['data']));
				}
			break;
		}
		if ($variables['type'] == 'short') {
			break;
		}
	}
	$output = '';
	foreach ($fields as $key => $elements) {
		$singular = isset($fields[$key]['#title']) ? $fields[$key]['#title'] : '';
		$plural = !empty($singular) ? $singular.'s' : '';

		unset($fields[$key]['#title']);
		unset($elements['#title']);
		if (!empty($elements)) {
			if (!in_array($key, array('phone', 'url', 'email'))) {
				$plural = $singular;
			}
			$string = format_plural(count($elements), $singular, $plural);
			$string = (!empty($string)) ? $string.':&nbsp;'.implode($fields[$key], ", ").'<br/>' : implode($fields[$key], ", ");
			$output .= $string;
		}
	}
	return $output;
}


function theme_map_link($variables) {
	$address_elements = $variables['address_elements'];
	if (isset($variables['street_link']) && isset($address_elements['street']) && !empty($address_elements['street'])) {
		if (isset($address_elements['building']) && !empty($address_elements['building'])) {
			$title = $address_elements['street'].', '.$address_elements['building'];
			unset($address_elements['building']);
		}
		else {
			$title = $address_elements['street'];
		}
		return l($title, $variables['street_link']['path'], $variables['street_link']['options']);
	}
}

function theme_contacts_address($variables) {
	$separator = isset($variables['separator']) ? $variables['separator'] : ', <br />';
  
	$variables['keys'] = isset($variables['keys']) ? $variables['keys'] : 'all';
  $address_elements = array();
	if ($variables['keys'] !== 'all') {
		foreach ($variables['keys'] as $key) {
			if (isset($variables['address_elements'][$key])) {
				$address_elements[$key] = $variables['address_elements'][$key];
			}
		}
	}
	else {
		$address_elements = $variables['address_elements'];
	}
	if (isset($variables['street_link']) && isset($address_elements['street']) && !empty($address_elements['street'])) {
		if (isset($address_elements['building']) && !empty($address_elements['building'])) {
			$address_elements['street'] .= ', ' . $address_elements['building'];
			unset($address_elements['building']);
		}
		$address_elements['street'] = l($address_elements['street'], $variables['street_link']['path'], $variables['street_link']['options']);
	}

	$address = array();
  foreach (array_filter($address_elements) as $key => $value) {
    if (is_scalar($value) && ($value = trim($value)) !== '') {
      if (in_array($key, array('state', 'postcode', 'office'), TRUE) && count($address_elements) > 1) {
        $address[] =  check_plain($value) . $separator;
      }
      else {
        $address[] =  check_plain($value) . ', ';
      }
      unset($address_elements[$key]);
    }
  }
	$text = !empty($address) ? rtrim(implode($address), ', ') : (isset($variables['empty_text']) ? $variables['empty_text'] : '');
	if (!empty($variables['url'])) {
		$text = l($text, $variables['url'], array('html' => TRUE,));
	}
	return $text;
}


function theme_working_time(&$variables) {
		$content = $build = array();
		$day_names = array(
			t('Sun'),
			t('Mon'),
			t('Tue'),
			t('Wed'),
			t('Thu'),
			t('Fri'),
			t('Sat'),
		);
		$days = array(1, 2, 3, 4, 5, 6, 0);
    $build['working_time'] = array(
			'#type' => 'fieldset',
			'#title' => t('Business hours'),
			'#attributes' => array(
				'class' => array('site-contacts-working-time'),
			),
		);

		$build['working_time']['days']['#markup'] = '';
		foreach ($variables['working_time'] as $key => $time) {
      if (($time['working_hours']['start'] == '--:--') || ($time['working_hours']['end'] == '--:--')) {
        continue;
      } else {
        $build['working_time']['days']['#markup'] .= '<div class="site-contacts-working-week">';
        foreach ($days as $day) {
          if (isset($time['days'][$day])) { // $time['days'][$day] may be '0' (sunday)
            $build['working_time']['days']['#markup'] .= '<span class="site-contacts-working-day">' . $day_names[$day] . '</span>';
          }
          else {
            $build['working_time']['days']['#markup'] .= '<span class="site-contacts-working-day">&mdash;</span>';
          }
        }

        if ($time['working_hours']['start'] == $time['working_hours']['end']) {
          $build['working_time']['days']['#markup'] .= '<span class="site-contacts-working-hour">' . t('Round-the-clock') . '</span>';
        }
        else {
          $build['working_time']['days']['#markup'] .= '<span class="site-contacts-working-hour">'.
              t('on&nbsp;@start&nbsp;to&nbsp;@end', array(
                '@start'	=> $time['working_hours']['start'],
                '@end'		=> $time['working_hours']['end'],
              ))
            .'</span>';
        }

        if ($time['lunch_hours']['start'] == $time['lunch_hours']['end']) {
          $build['working_time']['days']['#markup'] .= '<span class="site-contacts-lunch-hour">&nbsp;' . t('without break') . '</span>';
        }
        else {
          $build['working_time']['days']['#markup'] .= '<span class="site-contacts-lunch-hour">&nbsp;'.
              t('break on&nbsp;@start&nbsp;to&nbsp;@end', array(
                '@start'	=> $time['lunch_hours']['start'],
                '@end'		=> $time['lunch_hours']['end'],
              ))
            .'</span>';
        }
        $build['working_time']['days']['#markup'] .= '</div>';
      }
    }

    $build['working_time']['days']['#prefix'] = '<div class="site-contacts-working-days">';
    $build['working_time']['days']['#suffix'] = '</div>';
    $build['working_time']['hours']['#prefix'] = '<div class="site-contacts-working-hours">';
    $build['working_time']['hours']['#suffix'] = '</div>';
    $build['working_time']['lunch']['#prefix'] = '<div class="site-contacts-lunch-hours">';
    $build['working_time']['lunch']['#suffix'] = '</div>';

    if (empty($build['working_time']['days']['#markup'])) {
      unset($build['working_time']);
    }

		if (!empty($variables['detailed'])) {
			$build['detailed'] = array(
				'#markup'	=> '<div class="additional-detailed">' .
				check_markup($variables['detailed']['value'], $variables['detailed']['format'] ? $variables['detailed']['format'] : NULL) .
				'</div>',
			);
		}
		return drupal_render($build);
}

