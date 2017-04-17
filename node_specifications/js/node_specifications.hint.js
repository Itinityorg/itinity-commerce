(function ($) {
  "use strict";
  /**
   * Implementation of popup hints inside labels of node specifications fields
   * @see theme_node_specifications_field()
   */
  Drupal.behaviors.node_specifications_hint = {
    attach: function (context, settings) {
      function hint() {
        $('div.node-specification-field.hint > label', context).once('node-specification-field-hint', function () {

          var $hint = $('span.hint', this);
          $(this).hover(
            function (e) {
              $hint.css({
                bottom: $(this).innerHeight()
              }).removeClass('hidden');
            },
            function (e) {
              $hint.addClass('hidden');
            }
          );
        });
      }

      hint();
    }
  };
})(jQuery);
