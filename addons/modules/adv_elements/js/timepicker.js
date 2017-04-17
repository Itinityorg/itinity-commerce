// the custom function of timepicker to make it with custom validation and 
// render it as UNIX time adjusted to the UTC
// with masked input (maskedinput.js plugin)
(function ($) {
	Drupal.behaviors.timepicker = {
		attach: function (context, settings) {
			$.mask.definitions['~']='[012]';
			$.mask.definitions['^']='[0-5]';
			$('.form-text.timepicker', context).once('timepicker', function() {
				var init_value = $(this).val();
				$(this).mask("~9:^9").val(init_value); // forced put default initial value (in some cases value from server may be incorrect and must be displayed as error message at validation step, for manual fixing this)
			});
		}
	};
})(jQuery);