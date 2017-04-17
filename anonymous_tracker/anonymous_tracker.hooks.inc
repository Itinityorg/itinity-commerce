<?php

/**
* Temporary include (@see module)
* Hooks that should be in other modules
*/


/**
* Implements hook_anonymous_tracking_record_delete(); @see anonymous_tracker_tracking_record_delete()
*/
//TODO: MOVE TO shop.module
function shop_anonymous_tracking_record_delete($tracking_record) {
  if ($cart_id = db_query('SELECT cart_id FROM {shop_cart} WHERE anonymous_key = :key', array(':key' => $tracking_record->id,))->fetchField()) {
    db_query('DELETE FROM {shop_cart} WHERE cart_id = :id', array(':id' => $cart_id));
    db_query('DELETE FROM {shop_cart_items} WHERE cart_id = :id', array(':id' => $cart_id));
    drupal_static_reset('shop_cart');
  }  
}


/**
* Implements hook_anonymous_tracking_record_update(); @see anonymous_tracker_tracking_record_save()
*/
//TODO: MOVE TO shop.module
function shop_anonymous_tracking_record_update($tracking_record) {
  if ($tracking_record->original->uid != $tracking_record->uid) {
    if (!$tracking_record->original->uid) {
      // anonymous assigned to logged-in:
      db_query('UPDATE {shop_cart} SET uid = :uid WHERE anonymous_key = :id', array(':uid' => $tracking_record->uid, ':id' => $tracking_record->id));
      drupal_static_reset('shop_cart');
    }
    else {
      if (!empty($tracking_record->original->id) && $tracking_record->original->id != $tracking_record->id) {
        if ($cart = db_query('SELECT * FROM {shop_cart} WHERE anonymous_key = :id', array(':id' => $tracking_record->original->id))->fetch()) {
          if (!db_query('SELECT * FROM {shop_cart} WHERE anonymous_key = :id', array(':id' => $tracking_record->id))->fetch()) {
            $old_cart_id = $cart->cart_id;
            $cart->anonymous_key = $tracking_record->id;
            unset($cart->cart_id);
            $cart->uid = $tracking_record->uid;
            if (drupal_write_record('shop_cart', $cart)) {
              foreach (db_query('SELECT * FROM {shop_cart_items} WHERE cart_id = :id', array(':id' => $old_cart_id)) as $r) {
                $r->cart_id = $cart->cart_id;
                db_merge('shop_cart_items')->key(array(
                  'cart_id' => $cart->cart_id,
                  'entity_type' => $r->entity_type,
                  'entity_id' => $r->entity_id,                  
                  ))
                ->fields(array(
                  'qty' => $r->qty,
                  'snapshot_id' => $r->snapshot_id,
                ))->execute();
              }
            }
          }
        }
      }
    }
  }
}
