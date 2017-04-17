(function ($) {

  Drupal.behaviors.admin_money_transactions_overview_form = {
    attach: function (context, settings) {
      $('#transactions-overview-ajax-wrapper', context).once('admin_money_transactions_overview_form', function () {

        var $wrapper = $(this);
        //var $filter = $('.filter', $wrapper);
        var $filter = $('.money-transactions-filter', $wrapper);
        var $current_query = $('input[name="current_query"]', $wrapper);
        var $reset = $('input.form-submit.reset', $filter).click(function () {
          $('input[name="current_query"]', $filter.closest('form')).val('').change();
          return false;
        });
        //alert($('input:visible:not(.form-submit)[name="filter[user]"]', $filter).val());
        $('input.form-text, select', $filter).each(function () {
          if (($(this).val()).length) {
            $reset.removeClass('js-hide');
            // $filter.removeClass('collapsed');
            return false;
          }
        });
        $(window)
          .unbind('popstate.admin_money_transactions_overview_form')
          .bind('popstate.admin_money_transactions_overview_form', function (e) {
            if ($current_query.val() !== document.location.href) {
              $current_query.val(document.location.href).change();
            }
          });
        if ($current_query.val() !== document.location.href) {
          history.pushState({}, null, $current_query.val());
        }
        $('table.money-transaction-list>thead>tr>th>a, .pager a', $wrapper).bind('click.admin_money_transactions_overview_form', function (e) {
          e.preventDefault();
          $current_query.val($(this).attr('href')).change();
        });

      });

    }
  };
})(jQuery);










