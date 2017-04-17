<?php
/**
 * Created by PhpStorm.
 * User: mpatrin
 * Date: 27.09.16
 * Time: 9:42
 */


/**
 * @param $tracking_record
 *
 * @see anonymous_tracker_tracking_record_save()
 */
function hook_anonymous_tracking_record_insert($tracking_record) {

}

/**
 * @param $tracking_record
 *
 * @see anonymous_tracker_tracking_record_save()
 */
function hook_anonymous_tracking_record_update($tracking_record) {

}

/**
 * @param $tracking_record
 *
 * @see anonymous_tracker_tracking_record_delete()
 */
function hook_anonymous_tracking_record_delete($tracking_record) {

}


/**
 * @param $new_tracking_id
 * @param null $old_tracking_id
 *
 * @see _anonymous_tracker_id_to_cookie()
 */
function hook_anonymous_tracking_cookie_save($new_tracking_id, $old_tracking_id = NULL) {
  if (!$old_tracking_id && $old_tracking_id != $new_tracking_id) {
    // do something dirty
  }
}