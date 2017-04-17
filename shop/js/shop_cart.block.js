(function ($) {
  'use strict';

  Drupal.behaviors.cart_block = {
    attach: function (context, settings) {

      function shopCartKey(entity_type, entity_id, qty) {
        return entity_type + ':' + entity_id + ':' + qty;
      }

      function _shop_cart_ajax(url, async, $form, extra_data, success) {
        var data = $form.serialize() + (extra_data ? '&' + extra_data : '');
        var ids = [];
        $('[id]').each(function () {
          ids.push(this.id);
        });
        data += '&ajax_html_ids[]=' + encodeURIComponent(ids.join(','));
        $.ajax({
          async: async,
          cache: false,
          url: url,
          data: data,
          dataType: 'json',
          type: 'POST',
          success: success,
          error: function (jqXHR, textStatus) {
          }
        });
      }

      $('.block.block-shop form.shop-cart-wrapper', context).once('shop-cart-block', function () {
        var $shop_cart_form = $(this);
        var $block = $shop_cart_form.closest('.block-shop');
        var $button = $('.form-actions input[name="op"].form-submit.default-submit', $shop_cart_form);
        if ($button.length) {
          $button.addClass('ajax-processed').click(function (e) {
            e.preventDefault();
            _shop_cart_ajax(settings.shop_cart.block_uri, false, $shop_cart_form, encodeURIComponent($button.attr('name')) + '=' + encodeURIComponent($button.val()), function (response) {
              var $new_block = $(response.block);

              $.each(response.touched, function (idx, item) {
                // Highlight added/modified items
                $('tr#cart-item-' + item[0] + '-' + item[1], $new_block).addClass('js-recent');
              });

              // response.touched contains list of goods that was added/modified or deleted at last action @see _shop_cart_ajax()
              if (response.touched) {
                $('form.buy-button-form').each(function () {
                  var $form = $(this);
                  var entity_type = $form.attr('data-product-entity-type');
                  var entity_id = $form.attr('data-product-entity-id');
                  var key = shopCartKey(entity_type, entity_id, '');
                  var touched_idx = null;

                  $.each(response.touched, function (idx, item) {
                    if (idx.indexOf(key) >= 0) {
                      touched_idx = idx;
                      return false;
                    }
                  });

                  if (response.touched[touched_idx]) {
                    var extra_data = 'entity_id=' + entity_id
                      + '&entity_type=' + entity_type
                        //+ '&snapshot_id=' + snapshot_id
                      + '&bundle=' + $form.attr('data-product-bundle')
                      + '&view_mode=' + $form.attr('data-product-view-mode')
                      + '&is_secondary=' + $form.attr('data-product-is-secondary');
                    _shop_cart_ajax(settings.shop_cart.payment_button_form_uri, true, $form, extra_data, function (response2) {
                      var $new_form = $(response2.form);
                      if (response.messages && $form.hasClass('messages-target')) {
                        $(response.messages).prependTo($new_form);
                        response.messages = '';
                      }
                      $form.replaceWith($new_form);
                      Drupal.attachBehaviors(null, response2[0].settings);
                    });
                  }
                });
              }
              if ($(response.messages).length && !$('form.messages-target').length) {
                $(response.messages).addClass('shop-cart-container').prependTo($new_block.find('form.shop-cart-wrapper>div'));
              }

              $block.replaceWith($new_block);
              Drupal.attachBehaviors(null, response[0].settings);
            });
          });

          $('.col-delete input.form-submit', $shop_cart_form).click(function (e) {
            e.preventDefault();
            $button.val($(this).val()).attr('name', $(this).attr('name')).click();
          });
          $('.col-qty input.form-text', $shop_cart_form).change(function (e) {
            if (!$('.col-delete input:focus', $(this).closest('tr')).length) {
              $button.click();
            }
          }).keypress(function (e) {
            if (e.which == 13) { // Avoid delete button click on enter key pressed
              e.preventDefault();
              $button.click();
            }
          });

          $('.col-qty input.form-text', $shop_cart_form).focus(function (e) {
            $(this).select();
          });

        }
      });

      $('form.buy-button-form.purchase-action-stay input[type="submit"].form-submit.default-submit', context).once('shop-cart-block', function () {
        $(this).click(function (e) {
          var $shop_cart_form = $('.block.block-shop form.shop-cart-wrapper', context);
          var $block = $shop_cart_form.closest('.block-shop');
          if ($shop_cart_form.length) {
            e.preventDefault();
            var $form = $(this).closest('form').removeClass('messages-target');
            var $qty = $('input[name="qty"]', $form);
            var qty = $qty.length ? $qty.val() : 1;
            if (qty) {
              $(this).attr('disabled', true);
              var entity_type = $('input[name="entity_type"]', $form).val();
              var entity_id = $('input[name="entity_id"]', $form).val();
              //var snapshot_id = $('input[name="snapshot_id"]', $form).val();
              // fill hidden textfield
              //$('input[name="add"]', $shop_cart_form).val(entity_type + ':' + entity_id + ':' + snapshot_id + ':' + qty);
              $('input[name="add"]', $shop_cart_form).val(shopCartKey(entity_type, entity_id, qty));

              // Move good into cart animated effects
              // find main wrapper for current good
              var $product_wrapper = $form.closest('.content');
              if ($product_wrapper.parent().hasClass('block')) { // case of ".block>div.content" (legacy support ?)
                $product_wrapper = $form.parent();
              }
              var movable_source = {};
              // Find preferred movable element
              $.each(['.field-type-image .field-item img', '.image-block-gallery img', '.node-title', '.form-item-qty', '.product-price', 'form.buy-button-form'], function (idx, selector) {
                if ((movable_source = $(selector + ':visible:first', $product_wrapper)).length) {
                  return false;
                }
              });
              if (movable_source.length) {
                // find receiver element
                var $receiver = $('table.shop-cart-items-list:visible tr#cart-item-' + entity_type + '-' + entity_id, $shop_cart_form);
                if (!$receiver.length) {
                  $receiver = $('.shop-cart-block-caption:visible', $shop_cart_form).first();
                  if (!$receiver.length) {
                    $receiver = $shop_cart_form.is(':visible') ? $shop_cart_form : $block;
                  }
                }

                movable_source // clone item and place before original
                  .clone()
                  .addClass('shop-item-movable') // see shop.css
                  .appendTo('body')
                  .css({
                    width: Math.min(movable_source.width(), $receiver.width()),
                    height: Math.min(movable_source.height(), $receiver.height()),
                    left: movable_source.offset().left + 'px',
                    top: movable_source.offset().top + 'px',
                    'font-size': movable_source.css('font-size')
                  })
                  .animate({ // move item into cart block
                    opacity: 0.3,
                    left: $receiver.offset().left,
                    top: $receiver.offset().top
                  }, 500, function () { // moving completed.
                    $(this).remove();
                    // Call main ajax routines in block form:
                    $form.addClass('messages-target');
                    $('input[name="op"].form-submit.ajax-processed', $shop_cart_form).click();

                  });
              }
              else {
                $('input[name="op"].form-submit.ajax-processed', $shop_cart_form).click();
              }

            }
          }
        });
      });
    }
  };
})(jQuery);







