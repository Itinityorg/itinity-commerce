<?php // $Id: shop.theme.inc, v 1.0 2011/06/23 15:57:26 ITKLD Exp $


function theme_shop_cart($variables) {
  if (!isset($variables['has_links'])) {
    $variables['has_links'] = $variables['mode'] != 'mail';
  }

  $items = $variables['shop_cart']->items;
  $total = shop_build_price('shop_cart', $variables['shop_cart'], $variables['shop_cart']->uid);

  unset($total['delivery']);

  $total = $total ? theme('price', array('price' => array_sum($total))) : '';
  $curr = shop_get_info('currency', 'default');

  $qtys = shop_units_options();

  switch ($variables['mode']) {
    case 'max':
      $header = array(
        'image' => array(
          'data' => t('Image'),
          'class' => array('col-image',),
        ),
        'name' => array(
          'data' => t('Name', array(), array('context' => 'product')),
          'class' => array('col-title',),
        ),
        'qty' => array(
          'data' => t('Qty'),
          'class' => array('col-qty',),
        ),
        'price' => array(
          'data' => t('Price') . ', ' . theme('price', array('currency' => $curr,)),
          'class' => array('col-price',),
        ),
        'discount' => array(
          'data' => t('Discount') . ', %',
          'class' => array('col-price'),
        ),
        'sum' => array(
          'data' => t('Sum') . ', ' . theme('price', array('currency' => $curr,)),
          'class' => array('col-sum',),
        )
      );

      $rows = array();
      $has_images = FALSE;
      $has_discounts = FALSE; // IKW: Show 'discount' column if discount were applied for at least one shop cart item

      foreach ($items as $item) {
        $img = NULL;
        $key = Shop\Cart::key($item->entity_type, $item->entity_id, $item->snapshot_id);

        $entities = entity_load($item->entity_type, array($item->entity_id));
        $entity = isset($entities[$item->entity_id]) ? clone $entities[$item->entity_id] : NULL;

        if ($entity) {
          $entity->snapshot_id = $item->snapshot_id;
          $img = product_get_image($entity, $item->entity_type);
          $has_images = $has_images || !empty($img);
        }

        if ($variables['shop_cart']->frozen) {
          $item_snapshot = snapshot_load($item->snapshot_id);
          $original_price = $item_snapshot->data['price']['price'] ?? $item->si_price;
        }
        else {
          $original_price = $entity ? shop_build_price('shop_cart_item', $entity, $variables['shop_cart']->uid, 'price') : $item->si_price;
        }


        $has_discounts = $has_discounts || ($item->price != $original_price);

        $rows[$key] = array(
          'data' => array(
            'image' => array(
              'data' => $img ? $img : '&nbsp;',
              'class' => array('col-image'),
            ),
            'name' => array(
              'data' => !$variables['has_links'] ? check_plain($item->title) : l($item->title, $item->entity_uri),
              'class' => array('col-title'),
            ),
            'qty' => array(
              'data' => $item->qty * 1 . ' ' . (isset($qtys[$item->qty_type]) ? $qtys[$item->qty_type] : $item->qty_type),
              'class' => array('col-qty'),
            ),
            'price' => array(
              'data' => format_price($original_price, $item->currency), // IKW не показывать валюту в колонке "Цена" (показывать только в "Итого")
              'class' => array('col-price'),
            ),
            'discount' => array(
              'data' => round(!$original_price ? '0' : $item->price * 100 / $original_price - 100) . '%',
              'class' => array('col-price'),
            ),
            'sum' => array(
              'data' => theme('price', array('price' => $item->price * $item->qty,)),
              'class' => array('col-sum'),
            ),
          ),
        );
      }

      foreach (array('image' => $has_images, 'discount' => $has_discounts) as $key => $flag) {
        if (!$flag) {
          unset($header[$key]);
          array_walk($rows, function (&$row) use ($key) {
            unset($row['data'][$key]);
          });
        }
      }

      // Total sum
      if ($total) {
        $total_sum = shop_build_price('shop_cart', $variables['shop_cart'], $variables['shop_cart']->uid);

        $rows[] = array(
          'class' => array('shop-cart-footer'),
          'data' => array(
            array(
              'colspan' => 3 + $has_images + $has_discounts,
              'data' => t('Total') . ':',
            ),
            array(
              'data' => theme('price', array('price' => $total_sum['total_sum'])),
            ),
          ),
        );

        // Delivery
        if (isset($total_sum['delivery'])) {
          $rows[] = array(
            'class' => array('shop-cart-footer'),
            'data' => array(
              array(
                'colspan' => 3 + $has_images + $has_discounts,
                'data' => t('Delivery') . ':',
              ),
              array(
                'data' => in_array($total_sum['delivery'], array(), TRUE) ? t('Unknown') : ($total_sum['delivery'] ? theme('price', array('price' => $total_sum['delivery'])) : t('for free')),
              ),
            ),
          );
        }

        // Discount
        $discount = array_filter($total_sum, function ($value) {
          return $value < 0;
        });

        if ($discount) {
          $rows[] = array(
            'class' => array('shop-cart-footer'),
            'data' => array(
              array(
                'colspan' => 3 + $has_images + $has_discounts,
                'data' => t('Discount') . ':',
              ),
              array(
                'data' => theme('price', array('price' => array_sum($discount))),
              ),
            ),
          );
        }

        // To pay
        $rows[] = array(
          'class' => array('shop-cart-footer', 'strong'),
          'data' => array(
            array(
              'colspan' => 3 + $has_images + $has_discounts,
              'data' => t('Total to pay') . ':',
            ),
            array(
              'data' => theme('price', array('price' => array_sum($total_sum))),
            ),
          ),
        );
      }

      return array(
        '#theme' => 'table',
        '#empty' => t('Empty'),
        '#header' => $header,
        '#rows' => $rows,
        '#attributes' => array('class' => array('shop-cart'))
      );
      break;

    case 'mail':
      $hr = "\r\n"; //str_repeat(html_entity_decode('&mdash;', ENT_COMPAT, 'UTF-8'), 52);
      $rows[] = $hr;

      foreach ($items as $item) {
        // @see case 'max'
        $entities = entity_load($item->entity_type, array($item->entity_id));
        $entity = isset($entities[$item->entity_id]) ? clone $entities[$item->entity_id] : NULL;

        if ($entity) {
          $entity->snapshot_id = $item->snapshot_id;
        }

        if ($variables['shop_cart']->frozen) {
          $item_snapshot = snapshot_load($item->snapshot_id);
          $original_price = $item_snapshot->data['price']['price'] ?? $item->si_price;
        }
        else {
          $original_price = $entity ? shop_build_price('shop_cart_item', $entity, $variables['shop_cart']->uid, 'price') : $item->si_price;
        }

        $has_discounts = $item->price != $original_price;

        $price_string = $item->qty * 1 . ' ' . (isset($qtys[$item->qty_type]) ? $qtys[$item->qty_type] : $item->qty_type);
        $price_string .= ' x ';
        $price_string .= format_price($original_price, $item->currency);
        $price_string .= $has_discounts ? ' ' . round(!$original_price ? '0' : $item->price * 100 / $original_price - 100) . '%' : '';
        $price_string .= ' = ';
        $price_string .= theme('price', array('price' => $item->price * $item->qty,));

        $dots_count = max(3, _shop_cart_mail_dots(check_plain($item->title), $price_string));
        $rows[] = l($item->title, $item->entity_uri) . ' ' . str_repeat('.', $dots_count) . ' ' . $price_string;
      }

      $total_sum = shop_build_price('shop_cart', $variables['shop_cart'], $variables['shop_cart']->uid);

      if (!empty($total_sum['delivery'])) {
        $price_string = theme('price', array('price' => $total_sum['delivery']));
        $dots_count = max(3, _shop_cart_mail_dots(t('Delivery'), $price_string));
        $rows[] = t('Delivery') . ' ' . str_repeat('.', $dots_count) . ' ' . $price_string;
      }

      $discount = array_filter($total_sum, function ($value) {
        return $value < 0;
      });

      if ($discount) {
        $price_string = theme('price', array('price' => array_sum($discount)));
        $dots_count = max(3, _shop_cart_mail_dots(t('Discount'), $price_string));
        $rows[] = t('Discount') . ' ' . str_repeat('.', $dots_count) . ' ' . $price_string;
      }

      $rows[] = $hr;
      $t = t('Total', array(), !$variables['langcode'] ? array() : array('langcode' => $variables['langcode'])) . ':';
      $price_string = theme('price', array('price' => array_sum($total_sum)));
      $dots_count = max(3, _shop_cart_mail_dots($t, $price_string));
      $rows[] = $t . ' ' . str_repeat('.', $dots_count) . ' ' . $price_string;
      $rows[] = $hr;

      return implode("\r\n", $rows);
      break;
  }
}


function _shop_cart_mail_dots($title, $price_string) {
  return 50 - drupal_strlen($title . preg_replace('/\s+/su', ' ', html_entity_decode(strip_tags($price_string), ENT_COMPAT, 'UTF-8')));
}


/**
 * @see shop_admin_panel_page()
 */
function theme_shop_admin_panel_page($variables) {
  drupal_add_css(drupal_get_path('module', 'shop') . '/css/shop.admin.panel.css');

  $panel = $variables['panel'];
  $output = '';

  // sales funnel / purchases dynamics / revenue dynamics
  $output .= '<div class="panel-left" style="width: 70%; display: inline-block; vertical-align: top;">';
  $output .= drupal_render($panel['sales_funnel']) . drupal_render($panel['purchases_dynamics']) . drupal_render($panel['revenue_dynamics']);
  $output .= '</div>';

  // orders progress / orders journal / product journal
  $output .= '<div class="panel-right" style="width: 30%; display: inline-block; vertical-align: top;">';
  $output .= drupal_render($panel['orders_progress']) . drupal_render($panel['orders_journal']) . drupal_render($panel['product_journal']);
  $output .= '</div>';

  return $output;
}



