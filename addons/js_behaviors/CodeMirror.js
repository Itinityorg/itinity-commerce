(function ($) {
	Drupal.behaviors.bCodeMirror = {
		attach: function (context, settings) {
			if (settings.sCodeMirror) {
				$.each(settings.sCodeMirror, function(sourceId, options) {
					$('#' + sourceId, context).once('code-mirror', function() {
						var editor = CodeMirror.fromTextArea(document.getElementById(sourceId), options);
						$('.CodeMirror-scroll').css('height', Math.max(300, parseInt($(this).css('height'))) + 'px');
						if (options.linenum) {
							editor.setMarker(options.linenum - 1, "<strong style=\"color: #F00\">&rarr;</strong> %N%");
						}		
					});
					
				});
			}
		}
	}
})(jQuery);