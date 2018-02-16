(function ($) {
	"use strict";
	Drupal.behaviors.multifield = {
		attach: function (context, settings) {
			$('.form-type-multifield-wrapper.has-clear-btn .multifield-row.not-locked', context).once('clear-btn', function() {
				var $row = $(this);
				
				if (!$('input[id*="multifield-op-delete"]', $row).length) {
          
          var elements = 'input:not(:hidden):not(:submit), select, textarea';
          
          function canBeCleared($el) {
            return /*$el.is(':visible') && */!$el.attr('disabled') && !$el.attr('readonly') && ($el.val() || $el.is(':checked'));
          }
          
					var $a = $('<a class="form-submit multifield-op-delete element-invisible" style="float:right" href="#">' + Drupal.t('Clear') + '</a>');
					$(elements, $row).each(function() {
						if (canBeCleared($(this))) {
							$a.removeClass('element-invisible');
							return false; // break loop
						}						
					});
					
					$(elements, $row).bind('change click mouseout', function() {
						if (canBeCleared($(this))) {
							$a.removeClass('element-invisible');
						}
					});
					$a.click(function(e) {
            $(elements, $row).each(function() {
              if (canBeCleared($(this))) {
                $(this).val('').removeAttr('checked').change().blur();
              }
            });
						$(this).addClass('element-invisible');
            e.preventDefault();
					});
          $a.appendTo($('>td:visible:last', $row));				
				}

				
				
			});
		}
	}
})(jQuery);


















