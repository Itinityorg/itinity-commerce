(function ($) {
	Drupal.behaviors.node_specifications = {
		attach: function (context, settings) {
			"use strict";

			//filters block
			//if something change inputs reveal submit button
			var $block = $('form.specification-filters', context);
			if ($block.length) {
				$block.find('input:not(input:submit), select').bind('change', function (e) {
					$block.find('input:submit.submit').removeClass('js-hide');
				});
			}

			//colorizing color_field checkboxes
			$('span.color_me', context).once('color_me', function () {
				$(this).css({
					'border-left-color': '#' + $(this).attr('data')
				});
			});
		}
	};
})(jQuery);
