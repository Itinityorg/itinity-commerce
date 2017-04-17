(function ($) {
	Drupal.behaviors.addons_date_time_pickers = {
		attach: function (context, settings) {
			"use strict";

			//process field_date elements			
			$('.date-element-wrapper', context).once('date', function () {
				var
					$field = $(this),
					$datepickers = $field.find('input.datepicker').datepicker({
						dateFormat : 'dd.mm.yy',
						changeMonth: true,
						changeYear : true
					}),
					$timepickers = $field.find('input.timepicker'),
					$second_date_wrapper = $field.find('.second-date-wrapper');

				//bind event handlers timepicker fields
				$timepickers
					//insert template text '00:00' in empty field
					.bind('focus', function (e) {
						if (!$(this).val()) {
							$(this).val('00:00');
						}
					})
					//clean field if time value is empty or '00:00'
					.bind('blur', function (e) {
						var numbers = $(this).val().split(':');
						var is_empty = true;

						if (numbers.length == 2) {
							$(numbers).each(function () {if (parseInt(this)) is_empty = false;});
						}
						if (is_empty) $(this).val('');
					})
					//pass only numbers into field
					.bind('keypress', function (e) {
						var
							key_is_number = e.charCode >= 48 && e.charCode <= 57,
							caret_position = getCaretPosition(this),
							chars = $(this).val().split(''),
							char_to_insert = String.fromCharCode(parseInt(e.charCode));

						if ((key_is_number) && caret_position != 5) {
							if (caret_position == 2) {
								caret_position++;
								if (chars.length == 2) chars.push(':');
							}
							chars.splice(caret_position, 1, char_to_insert);
							$(this).val(chars.join(''));
							setCaretPosition(this, ++caret_position);
						}
						e.preventDefault();
					});

				//set minDate option for second datepicker according selected date in first datepicker
				if ($datepickers.length == 2) {
					var fix_min_date = function () {
						var first_date = $datepickers.first().datepicker('getDate');
						var second_date = $datepickers.last().datepicker('getDate');
						if (first_date) {
							$datepickers.last().datepicker('option', 'minDate', first_date);
							if (!second_date || first_date > second_date) {
								$datepickers.last().datepicker('setDate', first_date);
							}
						}
					};

					fix_min_date();
					$datepickers.first().bind('change', function (e) {
						fix_min_date();
					});
				}

				//if second date is optional
				if ($field.hasClass('optional')) {
					var is_empty = true;

					//check for empty fields in second date
					$second_date_wrapper.find('input').each(function () {if ($(this).val()) is_empty = false;});
					if (is_empty) {
						//'add end date' link, on click reveals second date fields
						$('<a href="#" class="show-link">' + Drupal.t('add end date') + '</a>')
							.insertAfter($second_date_wrapper)
							.click(function (e) {
								e.preventDefault();
								$second_date_wrapper.toggleClass('js-hide');
								$(this).toggleClass('js-hide');

								var first_date = $datepickers.first().datepicker('getDate');
								if (first_date) {
									$datepickers.last().datepicker('option', 'minDate', first_date).datepicker('setDate', first_date);
								}
							});

						//link that hides second date
						$('<a href="#" class="hide-link">' + 'x' + '</a>')
							.appendTo($second_date_wrapper)
							.click(function (e) {
								e.preventDefault();
								$second_date_wrapper.toggleClass('js-hide');
								$field.find('a.show-link').toggleClass('js-hide');
								$second_date_wrapper.find('input').val('');
							});
					}
				}
			});

			/**
			 * Helper function for text input.
			 *
			 * @param input
			 *  Text input
			 * @param position
			 *  Position of caret that will be set
			 */
			function setCaretPosition(input, position) {
				if (input.setSelectionRange) {
					input.focus();
					input.setSelectionRange(position, position);
				}
				else if (input.createTextRange) {
					var range = input.createTextRange();
					range.collapse(true);
					range.moveEnd('character', position);
					range.moveStart('character', position);
					range.select();
				}
			}

			/**
			 * Helper function for text input.
			 *
			 * @param input
			 *  Text input
			 * @returns {number}
			 *  Current position of carret in input
			 */
			function getCaretPosition(input) {
				var position = 0;
				if (document.selection) {
					input.focus();
					var selection = document.selection.createRange();
					selection.moveStart('character', -input.value.length);
					position = selection.text.length;
				}
				else if (input.selectionStart || input.selectionStart == '0') {
					position = input.selectionStart;
				}
				return position;
			}
		}
	};
})(jQuery);
