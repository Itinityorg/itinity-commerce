/**
* Fieldset-to-tabs plugin.
* @see _addons_process_fieldset()
*/
(function ($) {
	"use strict";
	Drupal.behaviors.fieldset_tabs = {
		attach: function (context, settings) {
			$('.element-tabs', context).once('fieldset-tabs', function() {
				$('a', this).once('tabs-link', function() {
					$(this).bind('click.tabbed', function() {
            if (!$(this).hasClass('active')) {
              var $a = $(this);
              var $form = $a.closest('form');
              // Store active tab in form action for better usability
              if ($form.length) {
                
                $form.attr('action', $form.attr('action').replace(/#.*$/, '') + $a.attr('href'));
                $('.__active_fieldset_tab_detector__', $form).val($a.attr('id').replace(/-link$/, ''));
              }
              // Toggle tab visibility
              var tab_id = $a.attr('href') + '-wrapper';
              var $tab = $(tab_id);
              
              $tab.show().removeClass('js-invisible').addClass('active').parent().children('.tab-wrapper').not(tab_id).addClass('js-invisible').removeClass('active');
              
              // Toggle .active class for a and li tags
              
             
              $('li.active, a.active', $a.closest('ul')).not('#' + $a.attr('id')).removeClass('active');
              $a.addClass('active').closest('li').addClass('active');
              // Get UP by tree and show all parent tabs.
              var $parent = $a.closest('.tab-wrapper');
              if ($parent.length) {
                $('#' + $parent.attr('id').replace(/-tab-wrapper$/, '-link'), $form).click(); // recursive call
              }
              
            }
          });
				});
			});

      $('.tab-wrapper input, .tab-wrapper select, .tab-wrapper textarea', context).bind('select.tabbed focus.tabbed', function() {
        var $parent = $(this).closest('.tab-wrapper');
        if ($parent.length) {
          $('#' + $parent.attr('id').replace(/-tab-wrapper$/, '-link')).click();
        }
      });
			
      
      // Global initialization. Find tabs contains error highlighted elements or has special class "active" or tab id is equal to location #hash
			var active_id = ''; // id of tab for activate
			var $erronymous = $('.tab-wrapper .error', context).first();
			if ($erronymous.length) {
				active_id = $erronymous.closest('.tab-wrapper').attr('id').replace(/-tab-wrapper$/, '');
			} else {
				active_id = document.location.hash.match(/-tab$/) ? document.location.hash.replace(/-tab$/, '').replace(/^#/, '') : ($('.tab-wrapper.active', context).attr('id') + '').replace(/-tab-wrapper$/, '');
			}
			if (active_id && active_id != 'undefined') { // Emulate click -->> activate default tab.
				$('.element-tabs a#' + active_id + '-link', context).click();
			}
      else {
				$('.element-tabs.tabs.primary a:first', context).click();
			}
		}
	}
})(jQuery);


















