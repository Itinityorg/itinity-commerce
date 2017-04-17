(function ($) {
	Drupal.behaviors.product_editable_list = {
		attach: function (context) {
			"use strict";

			var $table = $('table#product-editable-list, table#product-list', context);
			if ($table.length) {
				$table.find('tr.filter-row').bind('change propertychange', function (e) {
					$(this).find('input.submit').click();
				});
			}
		}
	};
})(jQuery);

