(function ($, Drupal) {
  Drupal.behaviors.SliderAPI = {
    attach:function (context, settings) {
      var $reset_button = $('.block-node-specifications :submit#edit-reset');

      // Create sliders
      $('.slider-container:not(.Slider-processed)', context)
        .each(function () {
          $(this).addClass('Slider-processed');

          // Get values
          var $slider = $(this).parents('.slider', context);

          var min = $slider.find('.slider-min', context).val() - 0;
          var max = $slider.find('.slider-max', context).val() - 0;
          var left = $slider.find('.left', context);
          var right = $slider.find('.right', context);

          var left_val = left.val() - 0;
          var right_val = right.val() - 0;

          if (!left_val) {
            left.val(min);
          }

          if (!right_val) {
            right.val(max);
          }

          // Setup slider
          $(this).slider({
            min   :min,
            max   :max,
            range :true,
            values:[
              left_val ? left_val : min,
              right_val ? right_val : max
            ],
            slide :jsliderSlideProcess
          });

        })
        .bind("slide", function (event, ui) {
          $reset_button.show();
        })
        .bind("slidechange", function (event, ui) {
          $reset_button.show();
        });

      // Bind textfield changes
      $('.slider-field:not(.Slider-processed)', context)
        .addClass('Slider-processed')
        .change(
        function (e) {
          // Get position
          var pos;
          if ($(this).hasClass('left')) {
            pos = 'left';
          }
          else {
            if ($(this).hasClass('right')) {
              pos = 'right';
            }
          }

          // Get container
          var $slider = $(this).parents('.slider', context);

          // Get slider max/min values
          var $jSlider = $slider.find('.slider-container', context);
          var max = $jSlider.slider('option', 'max');
          var min = $jSlider.slider('option', 'min');

          // input value validate
          var value = $(this).val() - 0;
          if (isNaN(value)) {
            value = NaN;
            $(this).val(pos == 'left' ? min : max);
          }

          // Validate input
          if (value > max) {
            value = max;
            $(this).val(value);
          }
          if (value < min) {
            value = min;
            $(this).val(value);
          }

          var right = $slider.find('.right', context).val() - 0;
          var left = $slider.find('.left', context).val() - 0;

          if (pos == 'left') {
            if (right && value > right) {
              value = right;
              $(this).val(value);
            }
          }
          else {
            if (left && value < left) {
              value = left;
              $(this).val(value);
            }
          }

          right = $slider.find('.right', context).val() - 0;
          left = $slider.find('.left', context).val() - 0;

          // Move slider without toggling events
          $jSlider.slider({values:[
            left ? left : min,
            right ? right : max
          ]});
        }
      );
    }
  };

// Slider processor
  var jsliderSlideProcess = function (event, ui) {
    // Setup values
    var $slider = $(this).parents('.slider');
    var left = ui.values[0];
    var right = ui.values[1];

    $slider.find('.left').val(left);
    $slider.find('.right').val(right);
  }

})(jQuery, Drupal);
