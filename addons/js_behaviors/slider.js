(function ($, Drupal) {
	Drupal.behaviors.SliderAPI = {
		attach: function (context, settings) {
			"use strict";

			$('div.slider', context).once('slider', function () {
				var $slider = $(this);
				var $slider_container = $(this).find('.slider-container');
				var $text_inputs = $slider.find('input.slider-field');
				var min_val = parseFloat($slider.find('input.slider-min').val());
				var max_val = parseFloat($slider.find('input.slider-max').val());

				// Setup slider
				$slider_container.slider({
					min    : min_val,
					max    : max_val,
//					step   : (max_val - min_val) / 100,
					range  : true,
					values : [parseFloat($text_inputs[0].value) || min_val, parseFloat($text_inputs[1].value) || max_val],
					animate: 'fast',
					slide  : function (event, ui) {
						$($text_inputs[0]).val(ui.values[0]).change();
						$($text_inputs[1]).val(ui.values[1]).change();
					}
				});

				// Bind textfield changes
				$text_inputs.bind('input', function (e) {
					var value = parseFloat($(this).val());
					var position = $(this).hasClass('left') ? 'left' : 'right';
					var slider_handle_index = +(position === 'right');
					var slider_handle_values = $slider_container.slider('values');

					// validate text input value
					value = position === 'left' && isNaN(value) ? min_val : value;
					value = position === 'right' && isNaN(value) ? max_val : value;
					value = value > max_val ? max_val : value;
					value = value < min_val ? min_val : value;
					value = position === 'left' && value > slider_handle_values[1] ? slider_handle_values[1] - 1 : value;
					value = position === 'right' && value < slider_handle_values[0] ? slider_handle_values[0] + 1 : value;

					// change slider position according to value in text input
					$slider_container.slider('values', slider_handle_index, value)
				});
			});

		}
	};
})(jQuery, Drupal);
