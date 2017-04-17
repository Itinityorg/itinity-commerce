/**
 * Created by mpatrin on 29.11.16.
 */

(function ($) {
  Drupal.behaviors.orders_statistics = {
    attach: function (context, settings) {
      "use strict";

      $('form#order-statistics-filter-form a').removeClass('active');

      $('form#order-statistics-filter-form', context).once('orders-statistics', function () {
        $(this).find('a').each(function () {
          if ($(this).hasClass('set-active')) {
            $(this).addClass('active');
          }
        });
      });


      $('#orders-statistics-strict').click(function (e) {
        e.preventDefault();
        $('#edit-strict').toggle();
      });

    }
  };
})(jQuery);

