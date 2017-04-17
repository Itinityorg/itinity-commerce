(function ($) {
	Drupal.behaviors.timepicker_element = {
		attach: function (context, settings) {
			$.each(settings.timepickers, function(source_id, options) {
				$.each(settings.timepickers_defaults, function(key, val){options[key] = val;});
				$('#'+source_id).datetimeEntry(options);
			});
		}
	}
})(jQuery);
