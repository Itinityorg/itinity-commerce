(function ($) {
	Drupal.behaviors.node_specifications_form = {
		attach: function (context, settings) {

			$('table[id*="edit-fields"]', context).find('tbody>tr').each(function () {
				var $field_container = $(this)
					, $select_input = $field_container.find('.node-spec-field-type-selector');

				toggle_visibility($select_input.val(), $field_container);
				$select_input.change(function (e) {
					toggle_visibility($(this).val(), $field_container);
				});
			});

			function toggle_visibility(field_type, $field_container) {
				console.log(field_type);

				$field_container.find('.sub-fields').addClass('js-hide');

				// toggle "multiple" checkbox
				$.inArray(field_type, ['color_select', 'list_text', 'image']) !== -1
					? $field_container.find('.multiple-checkbox, .' + field_type).removeClass('js-hide')
					: $field_container.find('.multiple-checkbox').addClass('js-hide');

				// toggle "units" input
				$.inArray(field_type, ['number_integer', 'list_text']) !== -1
					? $field_container.find('.unit-id-select').removeClass('js-hide')
					: $field_container.find('.unit-id-select').addClass('js-hide');
			}
		}
	};
})(jQuery);

