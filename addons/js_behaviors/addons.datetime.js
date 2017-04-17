(function($){
	"use strict";
	
	// 'datetime' element type
	Drupal.behaviors.addons_datetime = {

		attach: function(context, settings) {
			$('.form-type-datetime', context).once('datetime', function() {
				var $el = $(this);
				$('input.datetime-date', $el).datepicker({ // jQuery ui.datepicker
					dateFormat : 'dd.mm.yy',
					changeMonth: true,
					changeYear : true,
					minDate: $('input.datetime-date', $el).attr('data-min'),
					maxDate: $('input.datetime-date', $el).attr('data-max'),
					yearRange: $('input.datetime-date', $el).attr('data-year-range'),
					beforeShow: function(input, inst) {
						if ($el.hasClass('grouped')) {
							var $siblings = $el.closest('form').find('.form-type-datetime.' + $el.attr('data-group'));
							if ($siblings.length > 1) {
								if ($el.attr('id') === $siblings.first().attr('id')) {
									inst.minDate = $('input.datetime-date', $el).attr('data-min');
									inst.maxDate = $siblings.last().find('input.datetime-date').val();
								}
								else {
									if ($el.attr('id') === $siblings.last().attr('id')) {
										inst.minDate = $siblings.first().find('input.datetime-date').val();
										inst.maxDate = $('input.datetime-date', $el).attr('data-max');
									}
								}
							}

						}
            setTimeout(function() {
                $('.ui-datepicker').css('z-index', 99999999999999);
            }, 10);
						return inst;
					}
				});

				$('input.datetime-time', $el).bind('change paste blur', function(e) {
					var val = $(this).val().replace(/(^|[^\d])([\d])([^\d]|$)/, ' 0$2 ').replace(/(^|[^\d])([\d])([^\d]|$)/, ' 0$2 ').replace(/[^\d]/g, '');
					if (val.length) {
						if (val.length == 1) {
							val = [val, 0];
						}
						else {
							if (val.length == 2) {
								val = val <= 23 ? [val, 0] : (val > 59 ? [val[0], val[1] > 5 ? val[1] : val[1] + '0'] : [0, Math.min(59, val)]);
							}
							else {
								if (val.length == 3) {
									val = val > 230 ? [val[0], Math.min(59, val[1] + val[2])] : (val[0] ? [val[0] + val[1], val[2] < 6 ? val[2] + '0' : val[2]] : [0, Math.min(59, val[1] + val[2])]);
								}
								else {
									val = [Math.min(23, val[0] + val[1]), Math.min(59, val[2] + val[3])];
								}
							}
						}

            

						/* TODO
						if ($el.hasClass('grouped')) {
							var $s = $('.form-type-datetime.' + $el.attr('data-group') + ':not(#' + $el.attr('id') +')', context).first();
							if ($s.length) {
								
							}
						}
						*/
						$(this).val(val.join(':').replace(/^(\d):/, '0$1:').replace(/:(\d)$/, ':0$1'));
					}
					else {
						$(this).val('');
					}
				});

				$('input.datetime-time', $el).focus(function() {
					$(this).select();
				});
			});
		}
	};
})(jQuery);















