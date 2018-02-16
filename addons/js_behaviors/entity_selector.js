(function ($) {
  Drupal.behaviors.entity_selector = {
    attach: function (context, settings) {
      $('.form-item.form-type-entity-selector', context).once('entity-selector', function () {
        var $el = $(this);
        var $input = $('input.form-autocomplete', $el);
        var $wrapper = $('.entity-selector-text-input-wrapper', $el);
        var $placeholder = $('div.entity-selector-entity-view', $el);

        /* If the cursor is in the #autocomplete, not to scroll the window */
        /*$('#autocomplete').bind('mousewheel', function(e) {
         $(this).scrollTop($(this).scrollTop() - e.originalEvent.wheelDeltaY);
         return false;
         });
         */
        $input.bind('focus', function () {
          var wrapper_pos = $wrapper.offset().top;
          var wrapper_h = $wrapper.height();
          var window_h = $('body').height();
          //var autocomplete_h = $('#autocomplete', $el).height();
          if (Math.round(window_h - wrapper_pos) < 750) {
            setTimeout(function () {
              $('#autocomplete').css({
                'height': Math.round(window_h - (wrapper_pos + wrapper_h + 50)) + 'px',
              });
            }, 300);
          }
        });

        $input.bind('click.entity_selector', function () {
          $input.focus();
          if (!$(this).attr('disabled') && !$(this).attr('readonly')) {
            var val = $(this).val().replace(/\s/, '');
            if (val == '') {
              $(this).val('*').select().keyup();
            }
          }
        });

        $('a.entity-selector-preview', $el).bind('click.entity_selector_preview', function () {
          if (!$(this).attr('href').length || $(this).attr('href') === '#') {
            $input.click();
            return false;
          }
        });

        $input.bind('blur.entity_selector change.entity_selector', function (e) {
          var val = $(this).val().replace(/\s/, '');
          if (val == '*' || val == '') {
            $(this).val('');
            $('a.view-link', $el).attr('href', '#').addClass('element-invisible');
            $('a.edit-link', $el).attr('href', '#').addClass('element-invisible');
            $('a.entity-selector-clear-link', $el).addClass('element-invisible');
            $('div.entity-selector-entity-view', $el).addClass('element-invisible');

            if ($input.attr('data-default-img').length) {
              $('a.entity-selector-preview', $el).css('background-image', 'url(\'' + $input.attr('data-default-img') + '\')');
              $wrapper.addClass('has-image');
            }
            else {
              $('a.entity-selector-preview', $el).css('background-image', 'none').addClass('element-invisible');
              $wrapper.removeClass('has-image');
            }
            $('a.entity-selector-preview', $el).attr('href', '').removeClass('popup');
            $input.data('img', '');
            $wrapper.removeClass('has-del-button');
          }
          else {
            $('a.entity-selector-preview', $el).attr('href', $input.attr('data-url')).addClass('popup');
            if ($input.attr('data-default-img').length || $input.attr('data-img').length) {
              $('a.entity-selector-preview', $el)
                .removeClass('element-invisible')
                .css('background-image', 'url(\'' + ($input.attr('data-img').length ? $input.attr('data-img') : $input.attr('data-default-img')) + '\')');
              $wrapper.addClass('has-image');
            }
            $('a.entity-selector-clear-link', $el).removeClass('element-invisible');
            $wrapper.addClass('has-del-button');
            $placeholder.removeClass('element-invisible');
          }
        });

        $('a.entity-selector-clear-link', $el).bind('click.entity_selector', function (e) {
          if (!$input.attr('disabled') && !$input.attr('readonly')) {
            $input.focus().val('').keyup().blur().change();
          }
          e.preventDefault();
        });

      });
      
      function autotmpleteOnClick(e) {
        var popup = $('#autocomplete')[0];
        var $textfield = $(popup.owner.input);
        if ($('a.entity-selector-add-link', $(this)).length) {
          $textfield.val('').change();
          // @see also addons.module -->> addons_entity_insert() :
          var window_w = $(window).width() * 0.6;
          var window_h = $(window).height() * 0.7;
          var left = ($(window).width() - window_w) / 2;
          var top = ($(window).height() - window_h) / 2;
          var add_object_window = window.open($('a.entity-selector-add-link', $(this)).attr('href') + '#', 'add_object_window' + $textfield.attr('name'), 'width=' + window_w
            + ', height=' + window_h + ', left=' + left + ', top=' + top + ', resizable=yes, scrollbars=yes'); // hash prevents reload existing opened window
          add_object_window.blur();
          add_object_window.focus();
          return false;
        }
        else {
          if (!$('.prev-next', this).length) {
            if ($('img', this).length) {
              $textfield.data('img', $('img', this).attr('src'));
            }
            $textfield.data('url', $('.autocomplete-item', this).attr('data-url'));
            $textfield.closest('.form-item').find('a.view-link').attr('href', $('.autocomplete-item', this).attr('data-url')).removeClass('element-invisible');
            $textfield.closest('.form-item').find('a.edit-link').attr('href', $('.autocomplete-item', this).attr('data-edit-url')).removeClass('element-invisible');
            $textfield.change();

            var placeholder = $textfield.closest('.form-item').find('div.entity-selector-entity-view');
            var entity_type = $textfield.closest('input').attr('data-selector-entity-type');
            var entity_id = $('.autocomplete-item', this).attr('data-selector-entity-id');
            var view_mode = $textfield.closest('input').attr('data-selector-view-mode');

            if (entity_type == 'node') {
              $.ajax({
                url: Drupal.settings.basePath + '?q=addons/entity_selector/' + entity_type + '/' + entity_id + '/' + view_mode,
                type: 'POST',
                success: function (markup) {
                  placeholder.html(markup);
                },
                error: function () {
                  placeholder.html(Drupal.t('An error occurred'));
                }
              });
            }

          }
          else {
            $textfield.keyup().val($('.prev-next', this).attr('data-original-string'));
            return false;
          }
        }        
      }
      $('#autocomplete li').unbind('click.autocomplete_li');
      if ($.fn.live) { // Old & new jQuery support
        $('#autocomplete li').live('click.autocomplete_li', autotmpleteOnClick);
      }
      else {
        $(document).on('click.autocomplete_li', '#autocomplete li', autotmpleteOnClick);
      }
      
    }
  };
})(jQuery);






