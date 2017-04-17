(function ($) {
	Drupal.behaviors.node_specifications_compare_checkboxes = {
		attach: function (context, settings) {
			"use strict";

			//compare chekcbox processing
			var compare_list = $.cookie('compare_list'),
				compare_array = compare_list ? compare_list.split(',') : [],
				$compare_chkboxes = $('input.ns-compare', context),
				nid, nid_position, $labels, $label;
			var make_link_from_label = function ($label) {
				var link = $label.text().link('/node/compare' + '?destination=' + Drupal.encodePath(location.href.slice(location.origin.length + 1)));
				$(link).insertAfter($label.siblings('input'));
				$label.addClass('js-hide');
			};

			//-	initial replacment of <label>'s
			if (compare_list) {
				$labels = $compare_chkboxes.filter(':checked').siblings('label');
				if (compare_list.split(',').length > 1) {
					$labels.each(function () {make_link_from_label($(this));});
				}
			}
			$compare_chkboxes.click(function (e) {
				nid = $(this).val();
				nid_position = compare_array.indexOf(nid);

				//-	add/remove nid to 'compare_list' cookie
				if (this.checked && nid_position === -1) {
					compare_array.push(nid);
				}
				if (!this.checked && nid_position !== -1) {
					compare_array.splice(nid_position, 1);
				}
				compare_list = compare_array.join(',');
				$.cookie('compare_list', compare_list, {path: '/'});

				//-	<lable>'s replacment with link to comparing
				$compare_chkboxes.siblings('a').remove();
				$compare_chkboxes.siblings('label').removeClass('js-hide');
				if (compare_array.length > 1) {
					$compare_chkboxes.each(function () {
						$label = $(this).siblings('label');
						if (this.checked) {
							make_link_from_label($label);
						}
						else {
							$label.removeClass('js-hide');
							$label.siblings('a').remove();
						}
					});
				}
			});
		}
	};
})(jQuery);
