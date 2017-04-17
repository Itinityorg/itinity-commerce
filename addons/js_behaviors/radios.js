(function ($) {
	"use strict";
	/**
	* @see _addons_radios_process()
	*/
	Drupal.behaviors.radios_element = {
		attach: function (context, settings) {
			$('.form-radios.items-as-links .form-item.form-type-radio', context).once('items-as-links', function(idx, item) {
				$('label.option>a', item).bind('click.items-as-links', function() {
					$('>input.form-radio', item).click().change();
					return false;
				});
				$('>input.form-radio', item).bind('click.items-as-links', function() {
					$('label.option>a', $(item).siblings()).removeClass('active').find('.ok-mark').addClass('element-invisible');
					$('label.option>a', item).addClass('active').find('.ok-mark').removeClass('element-invisible');
				});
				
			});
		}
	}
})(jQuery);







