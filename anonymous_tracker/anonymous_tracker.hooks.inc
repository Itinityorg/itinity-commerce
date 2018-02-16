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
  db_query('DELETE FROM {shop_cart} WHERE visitor_id = :id', array(':id' => $tracking_record->id));
  db_query('DELETE FROM {shop_cart_items} WHERE visitor_id = :id', array(':id' => $tracking_record->id));
  drupal_static_reset('shop_cart');  
}


/**
* Implements hook_anonymous_tracking_record_update(); @see anonymous_tracker_tracking_record_save()
*/
// @TODO: MOVE TO shop.module
function shop_anonymous_tracking_record_update($tracking_record) {
  if ($tracking_record->id != $tracking_record->original->id) {
    foreach (db_query('SELECT * FROM {shop_cart} WHERE visitor_id = :old_vid', array(':old_vid' => $tracking_record->original->id)) as $r) {
      $query = db_merge('shop_cart')
        ->key(array(
          'visitor_id' => $tracking_record->id,
          'shop_id' => $r->shop_id,
        ))
        ->fields(array(
          'data' => $r->data,
          'uid' => $tracking_record->uid ? $tracking_record->uid : $r->uid,
          'changed' => REQUEST_TIME,
        ));
      $query->execute();      
    }
    foreach (db_query('SELECT * FROM {shop_cart_items} WHERE visitor_id = :old_vid', array(':old_vid' => $tracking_record->original->id)) as $r) {
      db_merge('shop_cart_items')
        ->key(array(
          'visitor_id' => $tracking_record->id,
          'entity_type' => $r->entity_type,
          'entity_id' => $r->entity_id,
          'snapshot_id' => $r->snapshot_id,
        ))
        ->fields(array(
          'qty' => $r->qty,  
        ))
        ->execute();
    }
  }
  if ($tracking_record->uid && !$tracking_record->original->uid) { // Anonymous user logged in
    db_query('UPDATE {shop_cart} SET uid = :uid WHERE visitor_id = :id AND uid = 0', array(':uid' => $tracking_record->uid, ':id' => $tracking_record->id));
  }
  return;
  
  // ВСЕ ПОХЕРЕНО РЕФАКТОРИНГОМ (shop_id):
  
  
  $old_cart = !empty($tracking_record->original->id) ? db_query('SELECT * FROM {shop_cart} WHERE visitor_id = :id', array(':id' => $tracking_record->original->id))->fetch() : NULL;
    // IKW
    // Я как гость кладу товар в корзину, оформляю его и регистрируюсь. 
    // При этом корзина прописана за ранее зарегеным челом и я её теряю. 
    // Предлагаю передавать корзину последнему кто работает за этм браузером. 
    // Потому, что даже если мой предшественник лазил по сайту и кидал в корзину, то теперь всё-равно я всё увидел , удалил ненужное и осознано нажал кнопку оформления заказа.
  if ($old_cart && !($new_cart = db_query('SELECT * FROM {shop_cart} WHERE visitor_id = :id', array(':id' => $tracking_record->id))->fetch())) {
    if ($tracking_record->original->id != $tracking_record->id && $tracking_record->original->uid != $tracking_record->uid) {
      $old_cart = shop_cart(0, NULL, NULL, $old_cart->cart_id, TRUE);
      $new_cart = shop_cart(0, $tracking_record->uid);
      $new_cart->no_commit = $old_cart->no_commit = TRUE;
      foreach ($old_cart->items as $item) {
        $new_cart->add($item->entity_type, $item->entity_id, $item->qty);
      }
      $new_cart->visitor_id = $tracking_record->id;
      $new_cart->no_commit = FALSE;
      $new_cart->commit();
      drupal_static_reset('shop_cart');
    }
  }
  elseif ($old_cart && ($new_cart = db_query('SELECT * FROM {shop_cart} WHERE visitor_id = :id', array(':id' => $tracking_record->id))->fetch()) && $old_cart->cart_id != $new_cart->cart_id) {
    foreach (db_query('SELECT * FROM {shop_cart_items} WHERE cart_id = :id', array(':id' => $old_cart->cart_id)) as $r) {
      db_merge('shop_cart_items')->key(array(
        'cart_id' => $new_cart->cart_id,
        'entity_type' => $r->entity_type,
        'entity_id' => $r->entity_id,                  
        ))
      ->fields(array(
        'qty' => $r->qty,
        'snapshot_id' => $r->snapshot_id,
      ))->execute();
    }
    if (!$tracking_record->original->uid && $tracking_record->original->uid != $tracking_record->uid) {
      db_query('UPDATE {shop_cart} SET uid = :uid WHERE cart_id = :id', array(':uid' => $tracking_record->uid, ':id' => $new_cart->cart_id));
    }
    drupal_static_reset('shop_cart');
  }
}
