(function ($) {
	Drupal.behaviors.addons_checkboxes_element = {
		attach: function (context, settings) {
			$('.form-item.form-type-checkboxes .form-checkboxes.sortable', context).once('sortable-checkboxes', function() {
				$(this).sortable({
					placeholder: 'sortable-checkboxes-placeholder ui-corner-all',
					cursor: 'move',
					distance: 15
				});
			});
		}
	}
})(jQuery);





