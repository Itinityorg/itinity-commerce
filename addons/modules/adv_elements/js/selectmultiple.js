//select form for multiple elements embedding checkboxes into the scroll list
(function ($) {
  'use strict';
	Drupal.behaviors.select = {
		attach: function (context, settings) {
			$('.form-type-select select.multiple', context).once('selectmultiple', function() {
				var $select = $(this);
				var h = parseInt($select.css('height'));
				if (!h) {
					var sz = $select.attr('size');
					if (!sz) {
						if ((sz = $('option', $select).length) > 9) { // IKW http://itinity.ru/node/424; @see also _adv_elements_process_select()
							sz = 9;
						}
					}
					if (sz) {
						$select.css('height', sz + 'em');
						h = parseInt($select.css('height'));
					}
					else {
						h = 150;
					}
				}
				var w = parseInt($select.css('width'));
        if (!w) {
          w = 150;
        }
				var $opts_wrapper = $('<div class="js-opts-wrapper ' + $select.attr('class') + '" style="color:' + $select.css('color') + '"/>')
					.bind('selectstart', function(){
            return false;
          })
					.css({
						height: (h + parseInt($select.css('padding-top')) + parseInt($select.css('padding-bottom'))) + 'px',
            width: w >= parseInt($select.parent().css('width')) ? '100%' : (25 + w + parseInt($select.css('padding-left')) + parseInt($select.css('padding-right')) + (parseInt($select.css('border-radius') * 1) * 2)) + 'px'
						})
					.insertAfter($select.addClass('element-invisible'));
				$('option, optgroup', this).each(function() {
					if ($(this).is('option')) {
						$('<span> ' + $(this).text() + '</span>').insertAfter(
              $('<input class="form-checkbox" type="checkbox" value="' + $(this).attr('value') + '"/>')
							.change(function() {
								if ($(this).is(':checked')) {
									if ($('option[value="' + $(this).val() + '"]:not(:selected)', $select).attr('selected', true).length) {
										$select.change();
									}
								}
								else {
									if ($('option[value="' + $(this).val() + '"]:selected', $select).attr('selected', false).length) {
										$select.change();
									}
								}
							})
              .click(function() {
                $('option[value="' + $(this).val() + '"]', $select).click();
              })
							.appendTo($('<label style="white-space: nowrap;padding-left: 0;' + parseInt($select.css('padding-left')) + 'px;padding-right:' + $select.css('padding-right') + ';margin:0;font-size:' + $select.css('font-size') + ';"></label>')
							.appendTo($opts_wrapper)));
					}
					else {
						$('<strong>&nbsp;' + $(this).attr('label') + '</strong>').appendTo($('<label style="white-space: nowrap;font-style:italic; padding-left:' + $select.css('padding-left') + ';padding-right:' + $select.css('padding-right') + ';margin:0;font-size:' + $select.css('font-size') + ';"></label>')
							.appendTo($opts_wrapper));
					}
				});

				$opts_wrapper[0].selfCheck = function() {
					$('option:selected', $select).each(function() {
						$('input.form-checkbox[value="' + $(this).attr('value') + '"]', $opts_wrapper).attr('checked', true).closest('label').addClass('selected');
					});
					$('option:not(:selected)', $select).each(function() {
						$('input.form-checkbox[value="' + $(this).attr('value') + '"]', $opts_wrapper).attr('checked', false).closest('label').removeClass('selected');
					});
					if ($select.attr('disabled')) {
						$('input.form-checkbox:not(:disabled)', $opts_wrapper).attr('disabled', true);
					}
					else {
						$('input.form-checkbox:disabled', $opts_wrapper).removeAttr('disabled');
					}
				};

				$select.change(function() {
					$opts_wrapper[0].selfCheck();
				});
				var hint = $select.attr('title');
        if (!!hint) { // !! from http://habrahabr.ru/sandbox/44911/
					hint += (hint.match(/[\.;!?]\s*$/) ? '' : ';') + "\r\n";
				}
				$opts_wrapper.attr('title', hint + Drupal.t('Select all: Ctrl+A; unselect all: Esc'));

        $opts_wrapper[0].selfCheck();
        
        // For better usability, initially scroll element to first checked item if exists
        var $selected = $('input.form-checkbox:checked:first', $opts_wrapper);
        if ($selected.length) {
          $opts_wrapper.scrollTop($opts_wrapper.scrollTop() + ($selected.position().top - $opts_wrapper.position().top) - ($opts_wrapper.height() / 2) + ($selected.height() / 2));
        }
        
				$opts_wrapper.keypress(function(e) {
					var keyCode = e.which || e.keyCode;
					// CTRL + A pressed:
					if (e.ctrlKey && keyCode === 1) {
						if ($('option:not(:selected)', $select).attr('selected', true).length) {
              $select.change();
            }
            /*$('input.form-checkbox:not(:checked)', this).each(function() {
							$(this).attr('checked', true).change();
						});*/
						return false;
					}

					// Escape
					if (keyCode === 27) {
            if ($('option:selected', $select).attr('selected', false).length) {
              $select.change();
            }
						/*$('input.form-checkbox:checked', this).each(function() {
							$(this).attr('checked', false).change();
						});*/
						return false;
					}

				}).keydown(function(e) {
					if (!e.ctrlKey) {
						var $first_opt = $('>label', $(this)).first();
						if ($first_opt.length) {
							var h = $first_opt.height() / 2;
							var keyCode = e.which || e.keyCode;
							if (keyCode === 36) { // Home 
								$opts_wrapper.scrollTop(0);
								$opts_wrapper.find('>label input').first().focus();
								return false;
							}
							else if (keyCode === 35) { // End 
								$opts_wrapper.scrollTop($opts_wrapper.find('>label').height() * $opts_wrapper.find('>label').length * 2);
								$opts_wrapper.find('>label input').last().focus();
								return false;
							}
							else if (keyCode === 38) { // up arrow
								$opts_wrapper.scrollTop($opts_wrapper.scrollTop() - h);
								return false;
							}
							else if (keyCode === 40) { // down arrow
								$opts_wrapper.scrollTop($opts_wrapper.scrollTop() + h);
								return false;
							}
						}
					}
				});
			});			
		}
	};
})(jQuery);
























