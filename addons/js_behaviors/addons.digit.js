(function($) {
  'use strict';
  /**
  * Behaviors for 'digit' form element 
  * @see addons module -->> element.digit.inc
  */
  Drupal.behaviors.addons_element_digit = {
    attach: function(context, settings) {
      $('.form-item.form-type-digit input.form-text.form-digit', context).once('addons_element_digit', function() {
        var $el = $(this);
        var $btnMinus = $el.closest('.digit-element-buttons-wrapper').find('.digit-minus');
        var $btnPlus = $el.closest('.digit-element-buttons-wrapper').find('.digit-plus');
        var hasButtons = $btnMinus.length && $btnMinus.length;
        var step = $el.attr('data-step');
        var validateStep = $el.attr('data-validate-step');
        var isFloat = $el.attr('data-float');
        var min = $el.attr('data-min');
        var max = $el.attr('data-max');
        
        /**
        * Ajax crutches, needed for multiple cicks per second
        */
        var timeout_id = null;
        function delayedChange($el) {
          
          if ($el.hasClass('ajax-processed')) {
            if (!$el.hasClass('change-wait')) {
              $el.addClass('change-wait');
              timeout_id = setTimeout(function() {
                $el.removeClass('change-wait').change();
              }, 1000);
            }
            else {
              $el.removeClass('change-wait');
              clearTimeout(timeout_id);
              delayedChange($el);
            }              
          }
          else {
            $el.change();
          }
        }
        
        /**
        * Inc/dec value of 'digit' element
        */
        function sum($el, val) {
          if (!$el.attr('readonly')) {
            if ($el.val() != '' || val) {
              var result = parseFloat($el.val().replace(/,/g, '.').replace(/[^\d\.\-]/g, '')) + (val * 1);
              if (isNaN(result) || result < min) {
                result = min;
              }
              else if (result > max) {
                result = max;
              }
              if (validateStep) {
                result = Math.round(result / step) * step;
                if (result > max) {
                  result = result - step;
                }
                else if (result < min) {
                  result = result + step;
                }
              }
              result = (+result).toFixed(step.replace(/^-?\d*?\.?/, '').length) + '';
              result = result.replace(/(\.[^0]*)?0+$/, '$1'); // remove .*000
              result = result.replace(/\.$/, '');
              if (result !== $el.val()) {
                $el.val(result);
                delayedChange($el);
              }
              if (hasButtons) {
                $btnPlus.attr('disabled', result * 1 >= max);
                $btnMinus.attr('disabled', result * 1 <= min);
              }               
            }
          
          }

        }
        
        var events = 'change click blur';
        
        $el.bind(events, function() {
           sum($el, 0);
        });
        
        // Set events first in queue
        $.each(events.split(' '), function(i, v) {
          // take out the handler we just inserted from the end and move it at the beginning
         var handlers = $el.data('events')[v];
         handlers.unshift(handlers.pop());
        });
  
        if (hasButtons) {
          if ($el.val() * 1 >= max) {
            $btnPlus.attr('disabled', true);
          }
          if ($el.val() * 1 <= min) {
            $btnMinus.attr('disabled', true);
          }
          $btnPlus.click(function() {
            var t = $(this).val();
            $(this).val('').val(t); // trick for clear selection in input buttons
            if (!$btnPlus.attr('disabled') && !$el.attr('disabled')) {
              sum($el.focus(), step);
            }
          });
          $btnMinus.click(function() {
            var t = $(this).val();
            $(this).val('').val(t); // trick for clear selection in input buttons
            if (!$btnMinus.attr('disabled') && !$el.attr('disabled')) {
              sum($el.focus(), -step);
            }
          });
        }

      });
    }
  }
})(jQuery);















