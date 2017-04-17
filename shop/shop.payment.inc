<?php

function DEPRECATED____shop_payment_widgets($order_node) {
	$options = array();
	$widgets = array();
	foreach (shop_get_info('payment') as $key => $data) {
		if ($data['enabled']) {
			$options[$key] = $data['title'];
		}
	}
	if ($options) {

		foreach ($options as $key => $title) {
			if ($key === 'shop' || ctype_digit($key . '')) {
				//m($key, shop_get_info('payment', $key));
				$widgets['payment_method_' . $key . '_shop_payment_button'] = shop_payment_button($order_node, $key) + array('#type' => 'item', '#inline' => TRUE, '#title' => $title,);
			}
			elseif (($callback = shop_get_info('payment', $key, 'widget callback')) && ($element = $callback($order_node))) {
				$widgets['payment_method_' . $key . '_' . $callback] = $element + array('#type' => 'item', '#inline' => TRUE, '#title' => $title,);
			}
		}
	}
	if ($widgets) {
		$widgets['#type'] = 'item';
		$widgets['#title'] = t('Payment methods');
		
		return $widgets;
	}
}