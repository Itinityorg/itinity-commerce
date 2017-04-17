// the custom function for selecting the files for upload with just the 'nicer' interface
(function ($) {
	"use strict";
	Drupal.behaviors.fileupload = {
		attach: function (context, settings) {
		  $(".form-type-file input.form-file, .form-managed-file.non-imce input.form-file", context).once('fileupload', function() {
				if (!$.browser.msie || $.browser.version > 9) { // do not support ie 9 and below
          var $file = $(this);
          var $choose_btn = $('<a href="#" class="form-submit">' + Drupal.t('Choose file') + '</a>');
          
          if ($file.hasClass('error')) {
            $choose_btn.addClass('error');
          }
          
          $choose_btn.insertAfter($file).click(function() {
            // choose button click
            $file.click().change();
            return false;
          });
          
          var $clear_btn = $('<a style="display:none;" href="#" title="' + Drupal.t('Clear') + '" class="form-submit"><span class="pictogram">X</span></a>');
          
          $clear_btn.insertAfter($choose_btn).click(function() {
             // clear button click
            $file.val('').change();
            return false;
          });
          
          
          $file.change(function() { // crutches for IE
            $file.focus().blur();
          });
          $file.bind('change blur', function() {
            var val = $file.val();
            if (val.length) {
              var sz = $file[0].files[0].size;
              var sz_limit_error = false;
              if (sz > settings.uploadMaxFilesize) {
                sz_limit_error = true;
                if (!$('small.error', $choose_btn).length) {
                  alert(Drupal.t('Maximum file size must be less than @size', {'@size' : settings.uploadMaxFilesizeFormatted}));
                }
              }
              var i = Math.floor(Math.log(sz) / Math.log(1024));
              sz = (sz / Math.pow(1024, i)).toFixed(2) * 1 + ' ' + ['B', 'kB', 'MB', 'GB', 'TB'][i];
              $choose_btn.html(val + ' <small style=' + (sz_limit_error ? '"color:#F00;font-weight:700;" class="error"' : '"color:#070;"') + '>' +  sz + '</small>');
              $clear_btn.show();
            }
            else {
              $choose_btn.text(Drupal.t('Choose file'));
              $clear_btn.hide();
            }
          });
				}

				
			});
		}
	};
})(jQuery);








