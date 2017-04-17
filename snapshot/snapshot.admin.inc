<?php
/**
 * Created by PhpStorm.
 * User: mpatrin
 * Date: 26.05.16
 * Time: 10:42
 */


function snapshot_admin() {
  $build = array();
  foreach (module_invoke_all('snapshot_info') as $bundle => $bundle_info) {
    $build[$bundle] = array(
      '#type' => 'fieldset',
      '#title' => $bundle_info['label'],
    );

    $build[$bundle]['actions'] = array(
      '#theme' => 'links',
      '#attributes' => array('class' => array('links', 'inline')),
      '#links' => array(
        array(
          'title' => t('manage fields'),
          'href' => 'admin/config/system/snapshot/manage/' . $bundle . '/fields',
        ),
        array(
          'title' => t('manage display'),
          'href' => 'admin/config/system/snapshot/manage/' . $bundle . '/display',
        ),
      ),
    );
  }

  return $build;
}