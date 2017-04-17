(function($) {
  'use strict';
  Drupal.behaviors.catalogMaterialsSettings = {
    attach: function(context, settings) {
      $('.catalog-modes>.fieldset-wrapper', context).once('catalog-modes', function() {
        $(this).sortable({
          handle: '.sort-handle',
          items: '>.form-wrapper'
        });
        $('.sort-handle', this).addClass('pictogram-resize-vertical').css('cursor','move');
        $('>.form-wrapper>.form-wrapper', this).css('padding-left','2em');
      });
    
    }
  };
})(jQuery);
















