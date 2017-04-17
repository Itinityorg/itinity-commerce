(function ($) {
	Drupal.behaviors.order_orders = {
		attach: function (context, settings) {
			"use strict";

			var $filter_row = $('.filter-row', context);
			$filter_row.find('input, select').bind('change', function (e) {
				$filter_row.find('div.form-actions input.submit').click();
			});

		}
	};
})(jQuery);
