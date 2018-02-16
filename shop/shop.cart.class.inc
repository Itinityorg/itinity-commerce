<?php
/**
 * Implementation of shopping cart
 * Defined in shop.info
 */

namespace Shop;

class Cart {
  protected $data = array();
  public $no_commit = FALSE;


  /**
   * Magic method
   */
  function __construct($shop_id, $account, $order_node) {
    $this->load($shop_id, $account, $order_node);
  }


  /**
   * Magic method
   */
  function __get($name) {
    if (in_array($name, array('uid', 'visitor_id', 'items', 'total_sum', 'shop_id', 'account', 'order_node'), TRUE)) {
      return $this->{$name} ?? NULL;
    }
  }


  /**
   * Internal storage for custom values
   * For saving cart state commit() call is required.
   */
  public function data($data_or_key = NULL, $value = NULL) {
    if (isset($data_or_key)) {
      if (is_scalar($data_or_key)) { // string, integer, float, bool, etc
        if (isset($value)) {
          if (!isset($this->data[$data_or_key]) || serialize($this->data[$data_or_key]) !== serialize($value)) {
            $this->data[$data_or_key] = $value;
            $this->commit();
          }
          return $this;
        }
        else {
          return isset($this->data[$data_or_key]) ? $this->data[$data_or_key] : NULL;
        }
      }
      else { // $data_or_key is array
        if (serialize($this->data) !== serialize($data_or_key)) {
          $this->data = $data_or_key;
          $this->commit();
        }
        return $this;
      }
    }
    else {
      return $this->data;
    }
  }


  /**
   * Load cart from database
   */
  private function load($shop_id, $account, $order_node) {

    if (!empty($order_node->nid)) {
      $visitor_id = NULL;
    }
    else {
      $visitor_id = anonymous_tracker_track_user($account, 'id');
    }
    $cart = array();
    
    $shop_cart_load = &drupal_static('shop_cart_load', array());
    $cid = implode(':', array($order_node->nid ?? NULL, $visitor_id, $shop_id, $account->uid));
    
    if (!isset($shop_cart_load[$cid])) {
      if (!empty($order_node->nid)) {
        // load items from ordered items table
        $cart += array(
          'uid' => $account->uid,
          'items_source' => 'order',
        );
        $query = db_select('shop_cart_items_ordered', 'scio')->fields('scio', array('entity_type', 'entity_id', 'qty', 'price', 'currency', 'title', 'frozen', 'snapshot_id'));

        $query->condition('scio.oid', $order_node->nid);
        $query->leftJoin('shop_index', 'si', 'si.entity_type = scio.entity_type AND si.entity_id = scio.entity_id');
        $query->fields('si', array('price', 'currency', 'qty_type', 'qty_fraction', 'title', 'entity_uri'));

        foreach ($query->execute() as $item) {
          
          
          if (!isset($item->si_price)) {
            $item->si_price = NULL;
          }
          if (!isset($item->si_currency)) {
            $item->si_currency = NULL;
          }

          if (is_null($item->qty_type)) {
            /**
             * NULL is pcs
             * @see product_form_alter()
             * 'qty_type' => array('#type' => 'select', '#options' => ...
             */
            $item->qty_type = t('pcs');
          }
          elseif (is_string($item->qty_type)) {
            $item->qty_type = t($item->qty_type);
          }
          
          $key = self::key($item->entity_type, $item->entity_id, $item->snapshot_id);
          if (!$item->frozen && empty($order_node->frozen)) {
            if ($e = entity_load($item->entity_type, array($item->entity_id))) {
              $entity = clone $e[$item->entity_id];
              $entity->snapshot_id = $item->snapshot_id;
              $uri = entity_uri($item->entity_type, $entity);
              $item->entity_uri = $uri['path'];

              $price_array = shop_build_price('product', $entity, $account);
              $item->price = array_sum($price_array);
              $item->price_array = $price_array;

              $item->title = entity_label($item->entity_type, $entity);
              $item->currency = shop_get_info('currency', 'default');//isset($entity->currency) ? $entity->currency : NULL;
            }
            else {
              $item->title = t('< --- Deleted object --- >');
            }
          }
          else {
            $price_array = shop_build_price('shop_cart_item', $item, $account);
            $item->price = array_sum($price_array);
            $item->price_array = $price_array;
            $item->currency = shop_get_info('currency', 'default');
          }
          $cart['items'][$key] = $item;
        }
      }
      else { // shopping cart is not ordered
          // load by by anonymous key + shop_id pair
        if ($visitor_id && !is_null($shop_id)) {
          if ($cart = db_query('SELECT * FROM {shop_cart} WHERE visitor_id = :visitor_id AND shop_id = :shop_id', array(':visitor_id' => $visitor_id, ':shop_id' => $shop_id, ))->fetchAssoc()) {
            $cart['items_source'] = 'cart';
            $cart['data'] = $cart['data'] ? unserialize($cart['data']) : array();
            $cart['data'] = $cart['data'] ? $cart['data'] : array();
            $query = db_select('shop_cart_items', 'sci')->fields('sci');
            $query->condition('sci.visitor_id', $cart['visitor_id']);
            $query->join('shop_index', 'si', 'si.entity_type = sci.entity_type AND si.entity_id = sci.entity_id');
            $query->condition('si.shop_id', $shop_id);
            foreach ($query->execute() as $item) {
              if ($e = entity_load($item->entity_type, array($item->entity_id))) {
                $entity = clone $e[$item->entity_id];
                $entity->snapshot_id = $item->snapshot_id;

                $key = self::key($item->entity_type, $item->entity_id, $item->snapshot_id);
                $uri = entity_uri($item->entity_type, $entity);
                $price_array = shop_build_price('product', $entity, $account);

                $item = (object) array(
                  'entity_type' => $item->entity_type,
                  'entity_id' => $item->entity_id,
                  'qty' => $item->qty,
                  'price' => array_sum($price_array),
                  'price_array' => $price_array,
                  'currency' => shop_get_info('currency', 'default'),
                  'si_price' => NULL,
                  'si_currency' => NULL,
                  'qty_fraction' => $entity->qty_fraction,
                  'title' => entity_label($item->entity_type, $entity),
                  'entity_uri' => $uri['path'],
                  'snapshot_id' => $item->snapshot_id,
                );

                if (is_null($entity->qty_type)) {
                  /**
                   * NULL is pcs
                   * @see product_form_alter()
                   * 'qty_type' => array('#type' => 'select', '#options' => ...
                   */
                  $item->qty_type = t('pcs');
                }
                elseif (is_string($entity->qty_type)) {
                  $item->qty_type = t($entity->qty_type);
                }

                $cart['items'][$key] = $item;
              }
            }
          }
          else {
            $cart = array('items_source' => NULL,);
          }
        }


      }

      // Fill incomplete properties
      
      $cart += array(
        'items_source' => NULL, // 'order'|'cart'|NULL
        'shop_id' => $shop_id,
        'uid' => $account->uid,
        'account' => $account,
        'items' => array(),
        'changed' => NULL,
        'data' => array(),
        'visitor_id' => $visitor_id,
        'order_node' => $order_node,
      );
      $shop_cart_load[$cid] = $cart;
    }
    else {
      $cart = $shop_cart_load[$cid];
    }
    

    foreach ($cart as $key => $item) {
      $this->$key = $item;
    }
    $this->calc_total(); 
    
    // DEPRECATED
    // @TODO: next hook used only in progressive_discounts.module, migrate this module to shop_build_price() conception.
   
    /*
    foreach (module_implements('shop_cart_load') as $m) {
      if (function_exists($fn = $m . '_shop_cart_load')) {
        $fn($this);
      }
    }
    */

    $this->frozen = !empty($order_node->frozen);

    return $this->calc_total();
  }


  /**
  * Unique identifier of cart object generated by situation (shop_id, order or visitor_id can be NULL)
  * Used for form wrappers, url tokens, etc
  */
  public function currentKey() {
    return $this->shop_id . ':' . $this->visitor_id . ':' . ($this->order_node->nid ?? NULL);
  }
  
  
  /**
   * Calculates total sum
   */
  public function calc_total() {
    $this->total_sum = 0;
    foreach ($this->items as $item) {
      $this->total_sum += $item->price * $item->qty;
    }
    return $this;
  }

  /**
   * Get items found (with qty optional) in cart
   */
  public function is_empty($check_qty = TRUE) {
    if ($check_qty) {
      foreach ($this->items as $item) {
        if ($item->qty > 0) {
          return FALSE;
        }
      }
    }
    else {
      return empty($this->items);
    }
    return TRUE;
  }


  /**
   * @param $entity
   */
  public static function key($entity_type, $entity_id, $snapshot_id = NULL) {
    return implode(':', array($entity_type, $entity_id, $snapshot_id));
  }


  /**
   * Add item in cart.
   * $entity can be object or its id only.
   * For saving cart state commit() call is required.
   * @see commit()
   */
  public function add($entity_type, $entity, $qty = 1) {
    $entity_id = self::entity2id($entity_type, $entity);

    $snapshot = snapshot_create('cart_item_snapshot', $entity_type, $entity_id);
    snapshot_save($snapshot);

    $key = self::key($entity_type, $entity_id, $snapshot->sid);
    $new_qty = isset($this->items[$key]->qty) ? $this->items[$key]->qty + $qty : $qty;
    return $this->set($entity_type, $entity_id, $snapshot->sid, $new_qty);
  }

  
  public static function shop_id_format($shop_id) {
    return in_array($shop_id, array(0, '0',), TRUE) ? '0' : ($shop_id ? $shop_id . '' : NULL);
  }
  
  
  /**
   * Set existing item qty or add new in cart
   * $entity can be object or its id only.
   * For save cart state commit() call is required.
   * @see commit()
   */
  public function set($entity_type, $entity, $snapshot_id = NULL, $qty) {
    $entity = self::entity2entity($entity_type, $entity);

    if ($this->shop_id !== self::shop_id_format($entity->shop_id)) {
      throw new \Exception('shop_id\'s must be equal.');
    }
    if ($qty < 0 || !$entity) {
      return $this;
    }

    // Allow insert/update entities without status support
    // Disallow insert/update unpublished entities with status support
    if (!isset($entity->status) || $entity->status) {
      list($entity_id, , $bundle) = entity_extract_ids($entity_type, $entity);
      if (shop_get_info('entity_types', $entity_type, $bundle)) { // if entity is sellable

        if ($qty && shop_get_info('balances', 'settings', 'track') && isset($entity->qty) && ($qty > $entity->qty) && empty($entity->goods_on_request)) {
          $qty = $entity->qty;
        }

        if (!$snapshot_id) {
          $snapshot = snapshot_create('cart_item_snapshot', $entity_type, $entity_id);
          $snapshot_id = snapshot_save($snapshot);
        }

        $key = self::key($entity_type, $entity_id, $snapshot_id);

        if (!$this->item($entity_type, $entity_id, $snapshot_id)) {
          $entity_uri = entity_uri($entity_type, $entity);
          $price_array = shop_build_price('product', $entity, $this->account);

          $this->items[$key] = (object) array(
            'entity_type' => $entity_type,
            'entity_id' => $entity_id,
            'qty' => 0,
            'price' => array_sum($price_array),
            'price_array' => $price_array,
            'currency' => shop_get_info('currency', 'default') ?? variable_get('money_default_currency', 'RUB'),
//            'currency' => $entity->currency,
            'title' => entity_label($entity_type, $entity),
            'qty_type' => $entity->qty_type,
            'qty_fraction' => $entity->qty_fraction,
            'entity_uri' => $entity_uri ? $entity_uri['path'] : '',
            'snapshot_id' => $snapshot_id,
          );
        }
        $this->items[$key]->qty = $qty;

        $this->commit();
      }
    }

    return $this;
  }


  /**
   * Delete item from cart.
   * $entity may be full object or his id only.
   * For save cart state commit() call is required.
   */
  public function delete($entity_type, $entity, $snapshot_id) {
    $id = self::entity2id($entity_type, $entity);

    if ($this->item($entity_type, $id, $snapshot_id)) {
      $key = self::key($entity_type, $id, $snapshot_id);
      unset($this->items[$key]);
      $this->commit();

      snapshot_delete($snapshot_id);
    }

    return $this;
  }


  /**
   * Get item in cart.
   * $entity may be full object or his id only.
   */
  public function item($entity_type, $entity, $snapshot_id = NULL, $property = NULL) {
    $entity_id = self::entity2id($entity_type, $entity);
    $key = self::key($entity_type, $entity_id, $snapshot_id);

    if (!isset($property)) {
      return isset($this->items[$key]) ? $this->items[$key] : NULL;
    }
    else {
      return isset($this->items[$key]->{$property}) ? $this->items[$key]->{$property} : NULL;
    }
  }


  /**
   * @param $entity_type
   * @param $entity
   * @param null $snapshot_id
   */
  public function countItem($entity_type, $entity, $snapshot_id = NULL) {
    $entity_id = self::entity2id($entity_type, $entity);
    $key = self::key($entity_type, $entity_id, $snapshot_id);

    $result = 0;
    foreach ($this->items as $item_key => $item) {
      if (strpos($item_key, $key) === 0) {
        $result += $item->qty;
      }
    }

    return $result;
  }


  /**
   * Attach cart to order node.
   * For save cart state commit() call is required.
   */
  public function order($order_node) {
    //if (!$this->order_node->nid && $order_node->type === 'order') {
      //if (!db_query('SELECT oid FROM {shop_cart_items_ordered} WHERE oid = :id', array(':id' => $order_node->nid))->fetch()) {
        $this->order_node = $order_node;
        $this->commit();
      //}
    //}
    return $this;
  }


  /**
   * Freeze ordered cart items. Price, title and currency will unbounded from drupal entities hooks
   */
  public function freeze($entity_type = NULL, $entity_id = NULL) {
    if (!empty($this->order_node->nid) && $this->items) {
      $this->commit();
      foreach ($this->items as $item) {
        if (!$entity_type || !$entity_id || ($break = ($entity_type && $entity_id && $item->entity_type == $entity_type && $item->entity_id == $entity_id))) {
          if ($e = entity_load($item->entity_type, array($item->entity_id))) {
            $entity = reset($e);
            $entity->snapshot_id = $item->snapshot_id;
            $data = array('object' => $entity, 'html' => '');
            if ($entity_uri = entity_uri($item->entity_type, $entity)) {
              // render entity into memory
              if ($render_array = menu_execute_active_handler($entity_uri['path'], FALSE)) {
                $data['html'] = drupal_render($render_array);
              }
            }
            db_query('UPDATE {shop_cart_items_ordered} SET frozen = 1, price = :price, title = :title, data = :data, currency = :currency WHERE oid = :oid AND entity_type = :entity_type AND entity_id = :entity_id',
              array(
                ':price' => $entity->price,
                ':title' => entity_label($item->entity_type, $entity),
                ':data' => serialize($data),
                ':currency' => money_fix_currency($entity->currency),
                ':oid' => $this->order_node->nid,
                ':entity_type' => $item->entity_type,
                ':entity_id' => $item->entity_id,
              ));
            $item->frozen = 1;
          }
          else {
            db_query('UPDATE {shop_cart_items_ordered} SET frozen = 1, price = :price, title = :title, currency = :currency WHERE oid = :oid AND entity_type = :entity_type AND entity_id = :entity_id',
              array(
                ':price' => $item->price,
                ':title' => $item->item,
                ':currency' => money_fix_currency($item->currency),
                ':oid' => $this->order_node->nid,
                ':entity_type' => $item->entity_type,
                ':entity_id' => $item->entity_id,
              ));
            $item->frozen = 1;
          }
          if (!empty($break)) {
            break;
          }
        }

      }
    }
    return $this;
  }


  /**
   * Unfreeze order items
   */
  public function unfreeze($entity_type = NULL, $entity_id = NULL) {
    if (!empty($this->order_node->nid) && $this->items) {
      $this->commit();

      foreach ($this->items as $item) {
        if ($item->frozen) {
          if (!$entity_type || !$entity_id || ($break = ($entity_type && $entity_id && $item->entity_type == $entity_type && $item->entity_id == $entity_id))) {
            db_query('UPDATE {shop_cart_items_ordered} SET frozen = 0 WHERE oid = :oid AND entity_type = :entity_type AND entity_id = :entity_id',
              array(
                ':oid' => $this->order_node->nid,
                ':entity_type' => $item->entity_type,
                ':entity_id' => $item->entity_id,
              ));
            $item->frozen = 0;

            if (!empty($break)) {
              break;
            }
          }
        }

      }
    }
    return $this;
  }


  /**
   *  Synchronize cart items with passed array.
   *  Items non existing in input array will be deleted.
   *  Input items not existing in cart will be added
   *  Others items qty will be updated.
   */
  public function items(array $items) {
    if ($items) {
      if (is_array($items)) {
        $commit_needed = FALSE;
        foreach ($this->items as $key => $item) {
          if (empty($items[$key])) {
            $this->delete($item->entity_type, $item->entity_id);
            $commit_needed = TRUE;
          }
        }
        foreach ($items as $item) {
          $this->set($item['entity_type'], $item['entity_id'], $item['snapshot_id'], $item['qty']);
          $commit_needed = TRUE;
        }
        if ($commit_needed) {
          $this->commit();
        }
      }
    }

    return $this;
  }
  

  /**
   * Store cart state into database. @see also __destruct()
   */
  public function commit() {
    drupal_static_reset('shop_cart_load');
    if ($this->no_commit) {
      return $this->calc_total();
    }

    $transaction = db_transaction();
    try {
      if (empty($this->order_node->nid)) { // Cart not ordered
        if (!$this->items_source) { // Cart is new
          if ($this->visitor_id) {
            $db_merge = db_merge('shop_cart');
            $db_merge
              ->key(array('visitor_id' => $this->visitor_id, 'shop_id' => $this->shop_id,))
              ->fields(array(
                'uid' => $this->uid,
                'data' => serialize($this->data),
                'changed' => REQUEST_TIME,
                'created' => REQUEST_TIME,
              ))
              ->execute();
            foreach ($this->items as $item) {
              db_merge('shop_cart_items')
                ->key(array(
                  'visitor_id' => $this->visitor_id,
                  'entity_type' => $item->entity_type,
                  'entity_id' => $item->entity_id,
                  'snapshot_id' => $item->snapshot_id
                ))
                ->fields(array('qty' => $item->qty * 1))// Prevent db exception where $item->qty is string type ('123' instead of 123)
                ->execute();
            }
            $this->items_source = 'cart';
          }
        }
        else { // Cart already in database
          $db_merge = db_merge('shop_cart')
            ->key(array('visitor_id' => $this->visitor_id, 'shop_id' => $this->shop_id,))
            ->fields(array(
              'uid' => $this->uid, 
              'data' => serialize($this->data),
              'changed' => REQUEST_TIME, 
            ))
            ->execute();
          foreach (db_query(
          'SELECT si.entity_type, si.entity_id 
          FROM {shop_cart_items} sci 
          INNER JOIN {shop_index} si ON si.entity_type = sci.entity_type AND si.entity_id = sci.entity_id 
          WHERE sci.visitor_id = :vid 
          AND si.shop_id = :sid', array(':vid' => $this->visitor_id,':sid' => $this->shop_id)) as $r) {
            db_query('DELETE FROM {shop_cart_items} WHERE visitor_id = :vid AND entity_type = :t AND entity_id = :id', array(':vid' => $this->visitor_id, ':t' => $r->entity_type, ':id' => $r->entity_id,));
          }
          foreach ($this->items as $item) {
            db_merge('shop_cart_items')
              ->key(array(
                'visitor_id' => $this->visitor_id,
                'entity_type' => $item->entity_type,
                'entity_id' => $item->entity_id,
                'snapshot_id' => $item->snapshot_id
              ))
              ->fields(array('qty' => $item->qty * 1))// Prevent db exception where $item->qty is string type ('123' instead of 123)
              ->execute();
          }
        }
      }
      else { //  $this->order_node->nid  not empty - cart is ordered?
     
        if ($this->items_source !== 'order') {// Existing cart attach to order operation detected
        
          db_query('UPDATE {shop_cart} SET data = :data WHERE visitor_id = :vid AND shop_id = :sid', array(':vid' => $this->visitor_id, ':sid' => $this->shop_id, ':data' => serialize($this->data)));
          foreach (db_query(
          'SELECT si.entity_type, si.entity_id 
          FROM {shop_cart_items} sci 
          INNER JOIN {shop_index} si ON si.entity_type = sci.entity_type AND si.entity_id = sci.entity_id 
          WHERE sci.visitor_id = :vid 
          AND si.shop_id = :sid', array(':vid' => $this->visitor_id,':sid' => $this->shop_id)) as $r) {
            db_query('DELETE FROM {shop_cart_items} WHERE visitor_id = :vid AND entity_type = :t AND entity_id = :id', array(':vid' => $this->visitor_id, ':t' => $r->entity_type, ':id' => $r->entity_id,));
          }
        }
        // Ordered cart editing detected
        elseif (!order_is_complete($this->order_node)) {
          db_query('DELETE FROM {shop_cart_items_ordered} WHERE oid = :id', array(':id' => $this->order_node->nid));
        }


        // Update items only where order is new or order is not complete
        if ($this->items_source !== 'order' || !order_is_complete($this->order_node)) {
          $default_currency = shop_get_info('currency', 'default');
          foreach ($this->items as $item) {
            if (is_null($item->currency)) {
              $item->currency = $default_currency;
            }

            if ($old_snapshot = snapshot_load($item->snapshot_id)) {
              $context = array('old_snapshot' => $old_snapshot,);

              if ($old_snapshot->bundle == 'cart_item_snapshot') {
                // delete cart_item_snapshot and create cart_item_ordered_snapshot
                snapshot_delete($old_snapshot);
                $snapshot = snapshot_create('cart_item_ordered_snapshot', $item->entity_type, $item->entity_id, 0, 0, $context);
                $snapshot_id = snapshot_save($snapshot);
                $item->snapshot_id = $snapshot_id;
              }
              elseif (isset($this->order_node->frozen) && !$this->order_node->frozen) {
                // update existing cart_item_ordered_snapshot
                $snapshot = snapshot_create('cart_item_ordered_snapshot', $item->entity_type, $item->entity_id, 0, 0, $context);
                $snapshot->sid = $item->snapshot_id;
                snapshot_save($snapshot);
              }
            }

            $rate = (drupal_strtolower($default_currency) != drupal_strtolower($item->currency)) ? money_currency_info($item->currency, 'rate') : 1;

            if ($e = entity_load($item->entity_type, array($item->entity_id))) {
              $entity = clone $e[$item->entity_id];
              $entity->snapshot_id = $item->snapshot_id;
              $data = array('object' => $entity, 'html' => '');
            }

            $db_merge = db_merge('shop_cart_items_ordered')
              ->key(array(
                'oid' => $this->order_node->nid,
                'entity_type' => $item->entity_type,
                'entity_id' => $item->entity_id,
                'snapshot_id' => $item->snapshot_id,
              ))
              ->fields(array(
                'qty' => $item->qty * 1,
                'changed' => REQUEST_TIME,
                'price' => $item->price * $rate,
                'currency' => $default_currency,
                'title' => isset($item->title) ? $item->title : '',
                'data' => !empty($data) ? serialize($data) : NULL,
              ));
            $db_merge->execute();

          }
        }
      }
      db_ignore_slave();
    } catch (\Exception $e) {
      $transaction->rollback();
      watchdog_exception('shop', $e);
      throw $e;
    }
    // Reload cart from database after update
    $this->load($this->shop_id, $this->account, $this->order_node);
    if (!empty($this->order_node->nid)) {
      entity_get_controller('node')->resetCache(array($this->order_node->nid));
      cache_clear_all('field:node:' . $this->order_node->nid, 'cache_field');
    }
    return $this;
  }


  /** Private methods */

  /**
   * Detect and convert passed parameters
   * If $entity is object returns entity_id else returns $entity itself.
   *
   * @param $entity_type
   * @param $entity
   *
   * @return int entity_id
   */
  private static function entity2id($entity_type, $entity) {
    if (is_object($entity)) {
      list($id) = entity_extract_ids($entity_type, $entity);
      return $id;
    }
    else {
      return $entity;
    }
  }


  /**
   * Returns entity for given $entity_type and $entity.
   *
   * @param $entity_type
   * @param $entity
   *
   * @return object entity or NULL if entity wasn't loaded.
   */
  private static function entity2entity($entity_type, $entity) {
    if (!is_object($entity)) {
      $entity_id = self::entity2id($entity_type, $entity);
      $entity = entity_load($entity_type, array($entity_id));
      return isset($entity[$entity_id]) ? $entity[$entity_id] : NULL;
    }
    else {
      return $entity;
    }
  }
}













