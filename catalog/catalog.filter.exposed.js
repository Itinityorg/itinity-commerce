
(function ($) {
  Drupal.behaviors.catalogFilterLinks = {
    attach: function (context, settings) {
      "use strict";
      $('form#catalog-filter-exposed-form', context).once('filter-sets', function() {
        var $content = $(this).closest('.content');
        var $form = $(this);
        if ($form.find('input.error').length) {
          $form.removeClass('js-hide');
        }
        $('div.catalog-filter-links a.filter-link', $content).click(function (e) {
            e.preventDefault();
            var filters = $('fieldset#edit-filters');
            if (filters.is(':visible')) {
                filters.stop().slideUp(400);
            }
            else {
                filters.stop().slideDown(400);
            }
          }
        );
      }); 
    }
  };
})(jQuery);














