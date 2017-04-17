<?php

/**
 * @see http://api.highcharts.com/highcharts for API reference
 *
 * @param array $variables
 *
 * @return string
 */
function theme_graph($variables) {
	$options = $variables['options'];
	$graph_id = drupal_html_id('graph-container');
	$module_path = drupal_get_path('module', 'addons');
	drupal_add_js($module_path . '/lib/highcharts/highcharts.js', array('scope' => 'footer'));
	drupal_add_js($module_path . '/js_behaviors/graph.init.js', array('scope' => 'footer'));

  if ($variables['modules']) {
    foreach ($variables['modules'] as $module_name) {
      drupal_add_js($module_path . '/lib/highcharts/modules/' . $module_name, array('scope' => 'footer'));
    }
  }

  if ($variables['plugins']) {
    foreach ($variables['plugins'] as $plugin_name) {
      drupal_add_js($module_path . '/lib/highcharts/plugins/' . $plugin_name, array('scope' => 'footer'));
    }
  }

	$container = array(
		'#type' => 'container',
		'#attributes' => array(
			'id' => $graph_id,
			'style' => array('margin: 0 auto;'),
			'class' => array('graph-container'),
		),
	);

	drupal_add_js(array('addons_graph_data' => array($graph_id => $options)), 'setting');

	return drupal_render($container);
}
