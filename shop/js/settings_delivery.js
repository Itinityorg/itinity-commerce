(function ($) {
Drupal.behaviors.shop_admin_delivery = {
	attach: function (context, settings) {
    $('form#shop-settings-delivery-form', context).once('shop-delivery-sortable', function() {
      $(this).sortable({
        items: 'fieldset.shop-delivery-sortable',
        handle: '>legend',
        clone: 'clone',
          placeholder: 'ui-state-highlight',
          cursor: 'move',
          forcePlaceholderSize: false,
          axis: 'y',
          revert: 100
      });
    });
    
	}
};
})(jQuery);

