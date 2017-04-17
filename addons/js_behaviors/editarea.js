(function ($) {
	Drupal.behaviors.editarea_element = {
		attach: function (context, settings) {
			if (settings.editareas) {
				$.each(settings.editareas, function(sourceId, options) {
					$("#"+sourceId).once('editarea', function() {
						options.plugins = '';
						options.min_width = parseInt($(this).css('width'));
						options.min_height = parseInt($(this).css('height')) + 100;
						options.id = sourceId;
						editAreaLoader.init(options);						
					});
				});
			}
		}
	}
})(jQuery);
