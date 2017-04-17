(function ($) {

	"use strict";

	Drupal.behaviors.colorpicker_element = {
		/**
		 * Attaches the behavior.
		 */
		attach: function (context, settings) {
			
			function validColor(color) {
				return color !== '' && color !== null && color.match(/(^\s*#?[0-9a-f][0-9a-f][0-9a-f])|(^\s*rgba?\(\s*\d+\s*,\s*\d+\s*,\s*\d+\s*(,\s*(([01]?\.\d+)|[01]))?\s*)?\)/gi);
			};
			
			function attachSpectrum($el, showSpectrum) {
				if (!$el.hasClass('spectrum-attached')) {
					
					$el.next('.field-suffix').find('.sp-replacer').remove();
					
					var showPalette = $('.colorpicker', context).length > 1 && $el.attr('data-palette-group');
					$el.addClass('spectrum-attached').spectrum({
						allowEmpty: !$el.attr('required'),
						cancelText: Drupal.t('Cancel') + ' (esc)',
						chooseText: Drupal.t('Ok') + ' (enter)',
						clearText: Drupal.t('Clear'),
						disabled: $el.attr('readonly'),
						showAlpha: $el.attr('data-alpha'),
						preferredFormat: 'rgb',
						showInput: true,
						showInitial: true,
						showPalette: showPalette,
						palette: [],
						replacerClassName: 'form-submit',
						beforeShow: function() {
							if (showPalette) {
								var colors = [];
								$('.form-type-colorpicker input.colorpicker[data-palette-group="' + $el.attr('data-palette-group') + '"]', context).each(function() {
									var c = $(this).val();
									if (validColor(c)) {
										if ((c = window.tinycolor(c, {'format' : 'hex'})) + '') {
											if ($.inArray(c + '', colors) === -1) {
												if ($el.attr('data-alpha') || c._a === undefined || c._a >= 1) {
													colors.push(c + '');
												}
											}
										}
									}
								});
								$el.spectrum('option', 'palette', colors);
							}
							$('body').bind('keydown.colorpicker', function(e) {
								if (e.which == 27) {
									$el.spectrum('container').find('.sp-cancel').click();
									return false;
								}
								if (e.which == 13) {
									$el.spectrum('container').find('.sp-choose').click();
									return false;
								}
							});
							$el.data('initial-value', $el.val());
						},
						hide: function(color) {
							$('body').unbind('keydown.colorpicker');
						}
					});
					
					// Visual tweaks:
					$('.sp-input', $el.spectrum('container')).addClass('form-text');
					$('button.sp-choose', $el.spectrum('container')).addClass('form-submit');
					
					$('<label>' + Drupal.t('Palette') + '</label>').prependTo($el.spectrum('container').find('.sp-palette-container'));
					
					
					$el.spectrum('container').find('.sp-clear').bind('click.clear_btn', function() {
						$('button.sp-choose', $el.spectrum('container')).click();
					});
					$el.spectrum('container').css('border-radius', $el.next('.sp-replacer').css('border-radius'));
					
					// Show after visual prepared
					if (showSpectrum) {
						$el.spectrum('show');
					}

					if ($el.hasClass('error')) {
						$el.show().closest('.form-item').find('.sp-replacer').css({
							'border-width': $el.css('border-width'),
							'border-style': $el.css('border-style'),
							'border-color': $el.css('border-color')
						});
					}
					
					// Events:
					$el.spectrum('container').find('.sp-cancel').click(function() {
						$el.spectrum('set', $el.attr('data-initial-value'));
						$el.val($el.attr('data-initial-value')).change();
						return false;
					});
					
					if (showPalette) {
						// Double click
						var IE = !!/msie/i.exec(window.navigator.userAgent);
						$el.spectrum('container').find('.sp-palette').delegate('.sp-thumb-el', IE ? 'mousedown.spectrum' : 'click.spectrum touchstart.spectrum', function(e) {
							if ($el.hasClass('palette-clicked') && $(this).hasClass('sp-thumb-active')) {
								$el.removeClass('palette-clicked');
								$(this).trigger(IE ? 'mousedown.spectrum' : 'click.spectrum', e);
								$el.spectrum('set', $el.val());
								$el.change();
							}
							else {
								$el.addClass('palette-clicked');
								setTimeout(function() {$el.removeClass('palette-clicked');}, 500);
							}
						});
					}
				}
				return $el;
			};

			$('.form-type-colorpicker input.colorpicker', context).once('colorpicker', function() {
				var $el = $(this);
				$el.next('.field-suffix').find('.sp-replacer').click(function() {
					attachSpectrum($el, true);
					return false;
				});
				
				$el.change(function() {
				//	console.log($el.val());
					if (!$(this).hasClass('spectrum-attached')) {
						attachSpectrum($el, false);
					}					
					var c = $el.val();
					if (validColor(c)) {
						var n = window.tinycolor(c, {'format' : 'hex'}) + '';
						if (window.tinycolor($el.spectrum('get') + '', {'format' : 'hex'}) + '' !== n) {
							$el.spectrum('set', n);
						}
						$el.val(n);
					}
					return false;
				});
				
			});		
			$('.form-type-colorpicker input.colorpicker.error', context).once('colorpicker-error', function() {
				$(this).show().next('.field-suffix').find('.sp-replacer').css({
					'border-width': $(this).css('border-width'),
					'border-style': $(this).css('border-style'),
					'border-color': $(this).css('border-color')
				});
			});
			
		}
	}

})(jQuery);




























