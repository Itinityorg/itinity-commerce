 (function($) {
	"use strict";
	Drupal.behaviors.addons_textarea_maxlength = {
		attach : function(context, settings) {
			$('textarea.maxlength', context).once('maxlength', function() { // @see _addons_process_textarea()
        var already_error = $(this).hasClass('error'); // an error class was added before in another script
        $(this).bind('change.maxlength keyup.maxlength', function() {
          var max = parseInt($(this).attr('data-maxlength'));
          var remains = max - $(this).val().replace(/[\r\n]/g, '-').length;
          if (remains >= 0) {
            if (!already_error) {
              $(this).removeClass('error');
            }
            $(this).closest('.form-item').find('.maxlength-warning>small').removeClass('error').text(Drupal.formatPlural(remains, '1 symbol remains', '@count symbols remains'));
          }
          else {
            if (!already_error) {
              $(this).addClass('error');
            }
            $(this).closest('.form-item').find('.maxlength-warning>small').addClass('error').text(Drupal.formatPlural(-remains, 'Please remove 1 extra character', 'Please remove @count extra characters'));
          }
        });	
			});
		}
	} 
})(jQuery);











