(function ($) {
  'use strict';

  Drupal.behaviors.selectToUISlider = {
    attach: function (context, settings) {
      $('select.mode-slider', context).once('selectToUISlider', function () {
        var $e = $(this);
        var $current_label = $('<label class="current-state-label" style="display:inline;padding-left:1em">' + $('option:selected', $e).text() + '</label>');
        var opts_count = $('option', $e).length;
        var $parent = $e.closest('.form-type-select');
        $e.selectToUISlider({
          labels: opts_count > 10 ? 10 : opts_count,
          labelSrc: 'value',
          tooltip: false,
          sliderOptions: {
            slide: function (e, ui) {
              $(ui.handle)
                .attr('aria-valuetext', ui.value)
                .attr('aria-valuenow', ui.value);
              $e.val(ui.value).find('option').eq(ui.value).attr('selected', 'selected');
              $current_label.text($('option:selected', $e).text());
            },
            stop: function (e, ui) {
              $e.change();
            }
          }
        });

        $e.hide();
        $current_label.insertAfter($('.ui-slider', $parent).css({
          width: Math.min(100, $('option', $e).length * 3.5) + '%',
          display: 'inline-block'
        }));
        $e.bind('change.selectToUISlider', function () {
          $current_label.text($('option:selected', $e).text());
        });

        if ($e.attr('hide-scale')) {
          $('span.ui-slider-label', $parent).hide();
        }

      });
    }
  };
})(jQuery);











