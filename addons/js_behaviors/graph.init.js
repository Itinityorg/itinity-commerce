(function ($) {
	Drupal.behaviors.addons_graph_init = {
		attach: function (context, settings) {
			"use strict";

			// translations
			if (Highcharts) {
				Highcharts.setOptions({
					lang: {
						contextButtonTitle: Drupal.t('Chart context menu'),
						downloadJPEG      : Drupal.t('Download JPEG image'),
						downloadPDF       : Drupal.t('Download PDF document'),
						downloadPNG       : Drupal.t('Download PNG image'),
						downloadSVG       : Drupal.t('Download SVG vector image'),
						loading           : Drupal.t('Loading...'),
						printChart        : Drupal.t('Print chart'),
						resetZoom         : Drupal.t('Reset zoom'),
						resetZoomTitle    : Drupal.t('Reset zoom level 1:1'),
						shortMonths       : [Drupal.t('Jan'), Drupal.t('Feb'), Drupal.t('Mar'), Drupal.t('Apr'), Drupal.t('May'), Drupal.t('Jun'), Drupal.t('Jul'), Drupal.t('Aug'), Drupal.t('Sep'), Drupal.t('Oct'), Drupal.t('Nov'), Drupal.t('Dec')],
						months            : [Drupal.t('January'), Drupal.t('February') , Drupal.t('March') , Drupal.t('April') , Drupal.t('May') , Drupal.t('June') , Drupal.t('July') , Drupal.t('August') , Drupal.t('September') , Drupal.t('October') , Drupal.t('November') , Drupal.t('December')],
						weekdays          : [Drupal.t('Sunday'), Drupal.t('Monday'), Drupal.t('Tuesday'), Drupal.t('Wednesday'), Drupal.t('Thursday'), Drupal.t('Friday'), Drupal.t('Saturday')]
					}
				});

				if (settings.addons_graph_data) {
					$.each(settings.addons_graph_data, function (id, graphs_settings) {
						var $graph = $('#' + id);
						if ($graph.length && !$graph.hasClass('highcharts-processed')) {
							$graph.highcharts(graphs_settings).addClass('highcharts-processed');
						}
					})
				}
			}
		}};
})(jQuery);
