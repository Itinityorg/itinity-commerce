(function($) {
	'use strict';
	
	
	/**
	* Interactive node specifications with taxonomy term fields states. Hide/show corresponding specifications by selected terms
	* @see node_specifications_form_node_form_alter()
	*/
	
	function specificationsShowByTermsState(context) {
		var tids = [];
    $('.field-type-taxonomy-term-reference', context).each(function() {
      $($('select', this).get().reverse()).each(function() { // reverse for hierarhical selects: last select is actual
        //if (!tids.length) {
          $('option', this).each(function() {
            // skip empty or '_none' options
            if ($(this).attr('selected') && $(this).attr('value') * 1) {
              if (($(this).attr('value') + '').match(/^\d+$/)) {
                tids.push($(this).attr('value'));
              }
            }
          });        
        //}
      });      
    });

    
		if (tids.length) {
			$('#edit-specifications-data fieldset:not([data-tids=""])', context).each(function() {
        var tids2 = ($(this).data('tids') + '').split(/,/);
				if (tids2) {
					var found = false;
					$.each(tids, function(idx, val) {
						$.each(tids2, function(idx1, val1) {
							if (val1 == val) {
								found = true;
								return;
							}
						});
					});
					$(this)[found ? 'removeClass' : 'addClass']('js-hide');
				}
			});
		}
		if ($('#edit-specifications-data fieldset:not(.js-hide)', context).length) {
			$('#edit-specifications-data', context).closest('.node-specifications-field').removeClass('js-hide');
		}
		else {
			$('#edit-specifications-data', context).closest('.node-specifications-field').addClass('js-hide');
		}
	}

	Drupal.behaviors.nodeSpecificationsNodeForm = {
		attach: function(context, settings) {
      $('#edit-specifications-data fieldset', context).each(function() {
        $(this).find('.node-specifications-edit-label').prependTo($(this));
      });
			$('#edit-specifications-data input, #edit-specifications-data select, #edit-specifications-data textarea', context).removeAttr('required');
			specificationsShowByTermsState(context);
			$('.field-type-taxonomy-term-reference select', context).once('specificationsShowByTermsState', function() {
				$(this).bind('change.specificationsShowByTermsState', function() {
					specificationsShowByTermsState(context);
				});
			});
		}
	};
	
})(jQuery);









