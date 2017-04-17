(function ($) {
	"use strict";
	
	// Remind user about unsaved work.
	
	
	
	
	
	
	// DEPRECATED
	
	
	
	
	
	
	
	
	
	
	Drupal.behaviors.addons_form_changes_control = {
		attach: function (context, settings) {
			$('form.changes-control', context).each(function() { // @see addons_form_alter() and $form['#control_changes'] property.
				var $form = $(this);
				$form.find('select, input[type="text"], input[type="checkbox"], input[type="radio"], textarea:not(.ckeditor-mod)').once('changes-control', function() {
					$(this).change(function() {
						$(this).addClass('changed');
					});
				});
				$form.bind('submit', function() {
					// supress alert when form is submitted
					$form.removeClass('changes-control');
				});
				
				var message = Drupal.t('Warning! Changes will be lost!');
				
				
				window.onbeforeunload = function(e) { // Show reminder when user leave page when controlled forms has changed elements
					
					// Warning! Onbeforeunload event is non cross-browser feature: do not work in oOpera.
					// Warning! Onbeforeunload event is non cross-browser feature: message text will replaced to native text in Mozilla Firefox.
					
					if ($('form.changes-control .changed').length) {
						return message;
					}
					// CkEditor do not transfer 'change' event to processed textareas. CkEditor use own muddy checkDirty() method ...
					if (typeof(CKEDITOR) != 'undefined' && CKEDITOR.env.isCompatible && typeof(CKEDITOR.instances) != 'undefined' && CKEDITOR.instances) {
						for (var i in CKEDITOR.instances) {
							if (CKEDITOR.instances[i].checkDirty() && $('form.changes-control #' + i).length) {
								return message;
							}
						}
					}

				};
			});
			
		}
	}
})(jQuery);


