<?php

/**
* Theme wrapper for order_shop_cart_form()
*/
function theme_shop_cart_form($variables) {
	$rows = array();
	$items = !empty($variables['shop_cart_form']['items']) ? $variables['shop_cart_form']['items'] : array();
	$items_qty = $has_images = $has_discounts = 0;
	$total_sum = $variables['shop_cart_form']['total_sum']['#value'];

	foreach (element_children($items) as $key) {
    $items_qty += ($qty = isset($items[$key]['qty']['#value']) ? $items[$key]['qty']['#value'] : 0);
		$row = array();

    $has_images = $has_images || !empty($items[$key]['image']['#markup']) ? 1 : 0;
    $has_discounts = $has_discounts || ($items[$key]['original_price']['#value'] != $items[$key]['price']['#value']) ? 1 : 0;

		$row['_image'] = array(
			'data' => $items[$key]['image'],
			'class' => array('col-image'),
		);
		$row['_title'] = array(
			'data' => $items[$key]['title'],
			'class' => array('col-title'),
		);
		$row['_qty'] = array(
			'data' => $items[$key]['qty'],
			'class' => array('col-qty'),
		);
		$row['_price'] = array(
			'data' => format_price($items[$key]['original_price']['#value'], $items[$key]['currency']['#value']),
			'class' => array('col-price'),
		);
    $row['_discount'] = array(
      'data' => $items[$key]['original_price']['#value'] > 0 ? round(($items[$key]['price']['#value'] * 100 / $items[$key]['original_price']['#value']) - 100) . '%' : '-',
      'class' => array('col-price'),
    );
		$price = ($items[$key]['qty']['#type'] === 'hidden' ? 0 : $items[$key]['price']['#value']) * $items[$key]['qty']['#value'];
		$row['_sum'] = array(
			'data' => theme('price', array('price' => $price, 'currency' => $items[$key]['currency']['#value'])),
			'class' => array('col-sum'),
		);

		if (!$variables['shop_cart_form']['#readonly']) {
			$row['_control'] =  array(
				'data' => array(
					$items[$key]['entity_type'],
					$items[$key]['entity_id'],
					$items[$key]['qty_type'],
					$items[$key]['currency'],
					$items[$key]['price'],
					$items[$key]['delete'],
					),
				'class' => array('col-control'),
			);
		}
    else {
			$row['_control'] =  array(
				'data' => array(),
				'class' => array('col-control'),
			);
		}
		if (!$items[$key]['qty']['#value']) {
			foreach ($row as $idx => $data) {
				$row[$idx]['class'][] = $items[$key]['entity_type']['#value'] . '-unpublished';
			}
		}
    $rows[$key] = array(
      'class' => array('cart-item'),
      'data' => $row,
    );
	}
	$shop_cart_form = NULL;
	if (!empty($variables['shop_cart_form']['add'])) {
		$shop_cart_form = &$variables['shop_cart_form']['add'];
		$rows['_add'] = array(
			'class' => array('cart-add'),
			'data' => array(
				array(
					'colspan' => 1 + $has_images + $has_discounts,
					'data' => '<div class="container-inline">' . drupal_render($shop_cart_form) . drupal_render($variables['shop_cart_form']['add_button']) . '</div>',
				),
				array(
					'colspan' => 4,
					'data' => '&nbsp;',
				),
			),
		);
	}


	$rows['_summary'] = array(
		'class' => array('cart-summary',),
		'data' => array(
			'_spacer' => array(
				'colspan' => 2 + $has_images + $has_discounts,
				'data' => '&nbsp;',
			),
			'_summ' => array(
				'colspan' => 3,
				'data' => '<h3>' . t('Total') . ': <span class="shop-elements-cart-cost-total">' . theme('price', array('price' => $total_sum)) . '</span></h3>', 
				'class' => array('col-total'),
				),
		),
		
	);

	$header = array(
		'_image' => array(
			'class' => array('col-image'),
		),
		'_title' => array(
			'data' => t('Name', array(), array('context' => 'product')),
			'class' => array('col-title'),
		),
		'_qty' => array(
			'data' => t('Qty'),
			'class' => array('col-qty'),
		),
		'_price' => array(
			'data' => t('Price') . ', ' . format_currency(shop_get_info('currency', 'default')),
			'class' => array('col-price'),
		),
    '_discount' => array(
      'data' => t('Discount') . ', %',
      'class' => array('col-price'),
    ),
		'_sum' => array(
			'data' => t('Sum') . ', ' . format_currency(shop_get_info('currency', 'default')),
			'class' => array('col-sum'),
		),
		'_control' => array(
			'class' => array('col-control'),
		),
	);
  
  foreach (array('_image' => $has_images, '_discount' => $has_discounts) as $key => $flag) {
    if (!$flag) {
      unset($header[$key]);
      array_walk($rows, function (&$row) use ($key) {
        unset($row['data'][$key]);
      });
    }
  }

	$variables['shop_cart_form']['items'] = array(
    '#_items' => $items,
    '#_total' => $total_sum,
		'#theme' => 'table',
		'#header' => $header,
		'#rows' => $rows,
		'#empty' => t('Cart is empty'),
		'#attributes'=> array(
			'class' => array(
				'shop-cart', 'use-tabledrag', // TODO 
			),
		),
	);
  drupal_alter('shop_cart_form', $variables['shop_cart_form']);
	return drupal_render_children($variables['shop_cart_form']);
}
















