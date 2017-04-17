(function($) {
  
  Drupal.behaviors.money_admin_form = {
    attach: function(context, settings) {
      $('form#money-payment-config-form', context).once('money-admin-sortable', function() {
        $(this).sortable({
          items: 'fieldset.money-payment-sortable',
          handle: '>legend',
          clone: 'clone',
          placeholder: 'ui-state-highlight',
          cursor: 'move',
          forcePlaceholderSize: false,
          axis: 'y',
          revert: 100
        });
        $('fieldset.money-payment-sortable', this).once('money-admin-form', function() {
          var $fieldset = $(this);
          var $checkbox = $('>legend input.form-checkbox', $fieldset);
          if (!$checkbox.attr('checked')) {
            $('>.fieldset-wrapper', $fieldset).hide();
          }
          $checkbox.click(function() {
            if ($(this).attr('checked')) {
              $('>.fieldset-wrapper', $fieldset).show();
            }
            else {
              $('>.fieldset-wrapper', $fieldset).hide();
            }
          });
        });
      });
    }
  };
  
})(jQuery);