 (function ($) {
	"use strict";
	Drupal.behaviors.addons_selectbox = {
		/**
		 * Attaches the behavior.
		*/
		attach: function (context, settings) {
			$('.selects-hierarchy', context).once(function() {
				var $new_val = $(this).parent().find('.form-text.new-value');
				if ($new_val.length) {
					$new_val.insertAfter($('select.active', this).length ? $('select.active', this).next() : $('select.value-none', this));
					var $button = $('<a href="#" class="form-submit" style="white-space:nowrap;" title="' + $new_val.attr('title') + '"> &gt; ' + ($new_val.hasClass('js-hide') ? Drupal.t('add') : Drupal.t('cancel')) + '</a> ').click(function() {
						if ($new_val.toggleClass('js-hide').hasClass('js-hide')) {
							$button.text(' > ' + Drupal.t('add'));
							$new_val.val('').change().keyup();
						}
						else {
							$new_val.focus();
							$button.text(Drupal.t('cancel'));
						}
						return false;
					}).insertAfter($new_val);
				}
			});
		}
	}
})(jQuery);
















