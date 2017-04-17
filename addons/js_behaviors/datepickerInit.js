/**
 * Attach the datepicker element form element behavior.
 */
(function ($) {
	"use strict";
	Drupal.behaviors.datepicker = {
		attach : function (context, settings) {	
					var datepickerDateFormat = function(y, m, d) {
						if (y <= 0 || m <= 0  || d <= 0) {
							return ' . . ';
						}
						y = y < 10 ? '0' + y : y;
						m = m < 10 ? '0' + m : m;
						return d + '.' + m + '.' + y;
					}				
					$('.form-type-date>.datepicker', context).once('datepicker', function() {
						var initial = datepickerDateFormat($('select.year', this).val(), ($('select.month', this).val()), $('select.day', this).val());
						var clearBtn = $('<button class="ui-datepicker-clear ' + (initial == ' . . ' ? 'js-hide' : '') + '">X</button>').attr('title', Drupal.t('Clear')).click(function() {
							$('select', $(this).parent()).val('');
							$(this).prev('button').text(datepickerDateFormat(0, 0, 0));
							$(this).hide();
							return false;
						});
						
						var input = $('<input type="text" style="visibility:hidden;border:none;margin:0;padding:0;width:0"/>')// stub for normal displaying of calendar.
						.appendTo(this)
						.datepicker({
						
							showOn: "button",
							buttonText: initial,							
							minDate : new Date(settings.datePicker.minDate),
							maxDate : new Date(settings.datePicker.maxDate),
							changeYear: true,
							beforeShow: function(el, inst) {
								var year = $('select.year', $(this).parent()).val(),
									month = $('select.month', $(this).parent()).val() - 1,
									day = $('select.day', $(this).parent()).val();
								$(this).datepicker("option", {
									buttonText: datepickerDateFormat(year, month + 1, day),
									defaultDate : year == '' && month == -1 && day == '' ? new Date() : ((year !== undefined && month !== undefined && day !== undefined) ? new Date(year, month, day) : null)
								});
								$('.ui-datepicker-trigger', $(this).parent()).addClass('form-select');
								
							},
							
							onSelect: function (dateText, inst) {
								$('select.year', $(this).parent()).val(inst.selectedYear);
								$('select.month', $(this).parent()).val(inst.selectedMonth + 1);
								$('select.day', $(this).parent()).val(inst.selectedDay);
								var text = datepickerDateFormat(inst.selectedYear, inst.selectedMonth + 1, inst.selectedDay);
								$('.ui-datepicker-trigger',  $(this).parent()).text(text);
								if (text != ' . . ') {
									clearBtn.show();
								}
							},
							firstDay : settings.datePicker.firstDay,
							dayNamesMin : settings.datePicker.dayNamesMin,
							monthNames : settings.datePicker.monthNames
						});
						clearBtn.appendTo(this);
						$('.ui-datepicker-trigger, .ui-datepicker-clear', this).addClass('form-select');
						//console.log(input);
						//$('.ui-datepicker-trigger', this).addClass('form-select');
						$('<!-- see datepickerInit.js -->').appendTo(this);
					});		
			if (settings.datepickerElements) {
				$.each(settings.datepickerElements, function (source_id, options) {

					
				});
			}
		}
	};
})(jQuery);
