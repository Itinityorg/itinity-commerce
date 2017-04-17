(function($) {
  "use strict";
  
  Drupal.behaviors.initPhoneMaskOnFields = {
    attach: function(context, settings) {
      $('.form-phone-masked-field', context).once(function() {
        $(this).bind('change', function(e) {
          var $el = $(this);
          var val = $el.val().replace(/\s/, '');
          if (val.length) {
            // fix for encoded '+' sign in url for different servers @see addons/includes/element.phone.inc -->> addons_phone_field_ajax()
            val = val.replace(/[^\d\+]/g, '').replace(/\+/g, 'plus');
            $.ajax({
              url: settings.addonsFormatPhoneUrl + '/' + (val.length ? val : '-'),
              dataType: 'text',
              complete: function(jqXHR, textStatus) {
                $el.val(jqXHR.responseText);
              }
            });
          }
          else {
            $el.val(val);
          }
        });
      });      
    }
  };
})(jQuery);
