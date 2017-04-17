<?php


/**
 * Implements hook_mail().
 * Send messages about order actions.
 */
function order_mail($key, &$message, $params) {
	if (in_array($key, array('order_inserted', 'order_inserted_copy', 'payment_accepted'))) {

		$shop_cart = $params['shop_cart'];
		$account = !empty($params['account']) ? $params['account'] : drupal_anonymous_user();
		$drupal_save_session = drupal_save_session();
		drupal_save_session(FALSE);
		global $user;
		$real_user = $user;
		$user = $account;

		$view = node_view($params['order']);

		// @TODO: refactor it:
		unset(
			$view['comments'],
			$view['status_field'],
			$view['created'],
			$view['changed'],
			$view['#contextual_links'],
			// DEPRECATED 16.08.2016: $view['prev_next_links'],
			$view['payment_widget']
		);


		$shop_cart_html = "\n" . theme('shop_cart', array(
				'mode' => 'mail',
				'shop_cart' => $shop_cart,
				'langcode' => $message['language']->language,
			)) . "\n";
		$shop_cart_html = str_replace("\r\n", "\n", $shop_cart_html);
		$shop_cart_html = preg_replace('/(\/t([hd])>)\s*(<t\\2[\s>])/u', "$1\t$3", $shop_cart_html);
		$shop_cart_html = preg_replace('/\s*<\/label>\s*/u', '</label> ', $shop_cart_html);
		$shop_cart_html = strip_tags($shop_cart_html, '<a> <strong> <label>');
		$shop_cart_html = preg_replace('/\n\s*\n/u', "\n", $shop_cart_html);
		$shop_cart_html = preg_replace('/\n[ 	]*/u', "\n", $shop_cart_html);
		$shop_cart_html = preg_replace('/ +/u', ' ', $shop_cart_html);
		$shop_cart_html = trim($shop_cart_html);
		$shop_cart_html = nl2br($shop_cart_html);

		$mail_content['order_content'] = array('#markup' => $shop_cart_html,);
		if (isset($view['recipient'])) {
			$mail_content['recipient'] = $view['recipient'];
		}
		if (isset($view['customer'])) {
			$mail_content['customer'] = $view['customer'];
		}
		if (isset($view['delivery'])) {
			$mail_content['delivery'] = $view['delivery'];
		}

		$user = $real_user;
		drupal_save_session($drupal_save_session);
	}

	if ($key === 'order_inserted' || $key === 'order_inserted_copy') {
		$order = $params['order'];
		$options = array(
			'langcode'	=> $message['language']->language,
			'context'	=> 'mail:order:new', // @see order.mail.texts.ru.po
		);
		$message['subject'] = t('New @order_title', array('@order_title' => drupal_strtolower($order->title)), $options);
		if ($order->uid && ($acc = user_load($order->uid))) {
			if ($acc->mail === $message['to']) {
				$message['subject'] = t('You have created an @order_title', array('@order_title' => drupal_strtolower($order->title)), $options);
				$message['body']['intro'] = t('You have created an !order-link', array(
					'!order-link' => l(drupal_strtolower($order->title), 'node/' . $order->nid,
						array(
							'absolute' => TRUE,
							'language' => $message['language'],
						)),
				), $options);
			}
			else {
				$message['subject'] = t('Created new @order-name on site @sitename by user @username', array(
						'@username' => format_username($acc),
						'@sitename' => variable_get('site_name', 'Drupal'),
						'@order-name' => drupal_strtolower($order->title),
				), $options);
				$message['body']['intro'] = t('Created new !order-link on site @sitename by user !username-link', array(
						'!username-link' => l(format_username($acc), 'user/' . $acc->uid,
								array(
										'absolute' => TRUE,
										'language' => $message['language'],
								)),
						'@sitename' => variable_get('site_name', 'Drupal'),
						'!order-link' => l(drupal_strtolower($order->title), 'node/' . $order->nid, array(
								'absolute' => TRUE,
								'language' => $message['language'],
						)),
				), $options);
			}
		}
		else {
			$message['body']['intro'] = t('Created new !order-link on site @sitename',	array(
				'@sitename' => variable_get('site_name', 'Drupal'),
				'!order-link' => l(drupal_strtolower($order->title), 'node/' . $order->nid, array(
					'absolute'	=> TRUE,
					'language'	=> $message['language'],
				)),
			), $options);
		}

	//$message['body']['content't'] = t('Order number is: #@num', array('@num' => $order->order_num), $options);

		$message['body']['content'] = drupal_render($mail_content);
	}



	if ($key === 'order_status_changed') {
		$options = array(
			'langcode'	=> $message['language']->language,
			'context'	=> 'mail:order:status', // @see order.mail.texts.ru.po
		);
		$subject = t('Status of your order [order:title] is changed to [order:status]', array(), $options);
		$body = t('Status of your order [order:title] is changed to [order:status]\n\nYou can view the order progress by reference [order:view]', array(), $options);

		$token_params = array(
			'language'	=> $message['language'],
			'callback'	=> 'order_mail_tokens',
			'sanitize'	=> FALSE,
		);

		$message['subject'] = token_replace($subject, array('order' => $params['order']), $token_params);

		$message['body'][] = token_replace($body, array('order' => $params['order']), $token_params);
	}



	if ($key === 'request_anonymous_link') {
		// Send tempoarary private access link to anonymous owner of order node.
		// @see order_request_token_form_submit()
		$message['subject'] = t('Temporary access to order #@num', array('@num' => $params['order']->order_num), array('langcode' => $message['language']->language));
		$message['body'][] = t('Hello! You or another user has requested access to view order #@num placed in our store.', array('@num' => $params['order']->order_num), array('langcode' => $message['language']->language));
		$message['body'][] = t('For access to the order #@num please follow to this link:', array('@num' => $params['order']->order_num), array('langcode' => $message['language']->language));
		$message['body'][] = $params['url'];
		$message['body'][] = t('Remember that this link will active within @timeout. After this period, you will have to repeat the request.', array('@timeout' => format_interval(ORDER_ANNONYMOUS_ACCESS_TIMEOUT),), array('langcode' => $message['language']->language));
		$message['body'][] = '';
		$message['body'][] = t('Permanent link to this order: @uri', array('@uri' => url('node/' . $params['order']->nid, array('absolute' => TRUE, 'alias' => TRUE,)),));
	}

	if ($key === 'payment_accepted') {
		$message['subject'] = t('Payment Accepted');
		$message['body']['intro'] = t('Paid !order-link on site @site_name', array(
			'@site_name' => variable_get('site_name', 'Drupal'),
			'!order-link' => l(drupal_strtolower($params['order']->title), 'node/' . $params['order']->nid, array(
				'absolute'	=> TRUE,
				'language'	=> $message['language'],
			)),
		));
		$message['body']['content'] = drupal_render($mail_content);
	}
}

/**
 * Implements hook_mail_alter().
 */
function order_mail_alter(&$message) {
	if (strpos($message['id'], 'status_order_') === 0) {
		$message['params']['order'] = $message['params']['node'];
	}
}

/**
* Callback used by token_replace() in order_mail()
*/
function order_mail_tokens(&$replacements, $data, $options) {
	if (isset($data['order'])) {
		$node = $data['order'];
		$statuses = array();
		if (array_sum($node->order_status) != array_sum($node->original->order_status)) {
			foreach (_order_statuses() as $status => $title) {
				if (isset($node->order_status[$status])) {
					$statuses[$status] = $title;
				}
			}
		}

		if ($node->status != $node->original->status) {
			$statuses[] = $node->status ? t('Accepted') : t('Canceled');
		}
    	$replacements['[order:status]'] = drupal_convert_to_UTF8('«', 'cp1251'). implode(drupal_convert_to_UTF8('»', 'cp1251') . ' ' . drupal_convert_to_UTF8('«', 'cp1251'), $statuses) . drupal_convert_to_UTF8('»', 'cp1251');
	}
}


/**
 *	Implements hook_notify_info() defined in notify.module
 *	This informational helper hook need for our notify system (@see notify.module) and should reflect mail notifications implemented in order_mail() hook.
 */
function order_notify_info() {
	$info =  array (
    'order_inserted' => array(
      'title'	=> t('New order'),
      'description'	=> t('Notification when order created'),
      'recipients' => array(
        'roles' => TRUE,
        'users' => TRUE,
				'order customer' => TRUE,
				'order author' => TRUE,
				'order recipient' => TRUE,
      ),
      'methods' => array(
        'email' => array(
					'default' => TRUE,
          'locked' => FALSE,
        ),
				'sms' => array(
					'default' => FALSE,
					'locked' => TRUE,
				),
      ),
    ),
		'payment_accepted' => array(
			'title' => t('Payment accepted'),
			'description' => t('Notification generated when payment accepted'),
			'recipients' => array(
				'users' => TRUE,
				'roles' => TRUE,
				'order customer' => TRUE,
				'order recipient' => TRUE,
			),
			'methods' => array(
				'email' => array(
					'default' => TRUE,
					'locked' => FALSE,
				),
			),
		),
  );

	return $info;
}


/**
 * Implements hook_notify_info_alter().
 */
function order_notify_info_alter(&$info) {
  if (module_exists('status') && !empty($info['status'])) {
    foreach ($info['status'] as $notify_key => &$notify) {
      if (strpos($notify_key, 'order_') === 0) {
        $notify['recipients'] += array(
          'order customer' => TRUE,
          'order recipient' => TRUE,
        );
      }
    }
  }
}

/**
 * Implements hook_notify_recipients_info().
 */
function order_notify_recipients_info() {
	return array(
		'order customer' => array(
			'title' => t('Order Customer'),
			'callback' => '_notify_order_customer',
		),
		'order author' => array(
			'title' => t('Order Author'),
			'callback' => '_notify_order_author',
		),
		'order recipient' => array(
			'title' => t('Order Recipient'),
			'callback' => '_notify_order_recipient',
		),
	);
}



function _notify_order_customer($message) {
	$result = array();
	if (!empty($message['params']['order']->customer)) {
		$result =  array($message['params']['order']->customer);
	}
	return $result;
}

function _notify_order_author($message) {
	$result = array();
	if (!empty($message['params']['node']->uid)) {
		$result =  array($message['params']['node']->uid);
	}
	return $result;
}

function _notify_order_recipient($message) {
  return isset($message['params']['order']->recipient_fields['mail']) ? array($message['params']['order']->recipient_fields['mail']) : array();
}


/**
* Callback for /admin/config/shop/messages
*/
function order_setting_recipients_form($form, &$form_state) {
	$form = array();
  if (module_exists('notify')) {
    notify_admin_module($form, $form_state, array('order'), FALSE);
    $form['submit'] = array(
      '#type'		=> 'submit',
      '#value'	=> t('Save'),
    );
  }

	return $form;
}


function order_setting_recipients_form_submit($form, &$form_state) {
	notify_admin_module_save('order', $form_state['values']['order']);
}




















