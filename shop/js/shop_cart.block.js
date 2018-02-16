(function($) {
  'use strict';
  /**
  * Process shop cart block
  * @see shop_carts_block_form()
  */
  Drupal.behaviors.shopCartsBlock = {
    attach: function(context, settings) {

      /**
      * Helper func for valid html ids generation on backend @see drupal_html_id()
      */
      function getAllHtmlIds() {
        var ids = [];
        $('[id]').each(function () {
          ids.push(this.id);
        });
        return '&ajax_html_ids[]=' + encodeURIComponent(ids.join(','));
      }

      /**
      * Handle shop cart changes, submit it to server and reload html parts such as shop cart block and product-buy-buttons forms on current page
      */
      function shopCartBlockFormAjax($shopCartForm, $clickedButton) {
        var data = $shopCartForm.serialize();
        data += getAllHtmlIds(); // Post HTML ids to server @see drupal_html_id()
        
        if ($clickedButton) {
          data += '&' + encodeURIComponent($clickedButton.attr('name')) + '=' + encodeURIComponent($clickedButton.val());
        }

        var buyButtonFormClicked = $('form.buy-button-form.purchase-action-stay.clicked-form').removeClass('clicked-form').length;

        $.ajax({
          async: false,
          cache: false,
          url: $shopCartForm.data('reload_block_url'),
          data: data,
          dataType: 'json',
          type: 'POST',
          success: function(response) {
            var $newBlock = $(response.block);
            var $messages = response.messages.length ? $(response.messages) : null;
            if (!buyButtonFormClicked && $messages) {
              $messages.css('width', $shopCartForm.closest('.content').css('width')).prependTo($('.content', $newBlock));
              setTimeout(function() {

                  $messages.slideUp(function() {
                    $messages.remove();
                  });
              }, 2000);
            }
            $shopCartForm.closest('.block').replaceWith($newBlock);
            Drupal.attachBehaviors($newBlock.parent(), response[0].settings);
            // overwrite variable because replaceWith() returns OLD DOM object, NOT new:
            $shopCartForm = $('form.shop-cart-wrapper', $newBlock);
            $.each(response.touched, function (idx, item) {
              // Highlight added/modified items
              $('tr#cart-item-' + item[0] + '-' + item[1], $shopCartForm).addClass('js-recent');
              $('form.buy-button-form[data-product-entity-type="' + item[0] + '"][data-product-entity-id="' + item[1] +'"]').each(function() {
                var $buyForm = $(this);
                var data = 'entity_type=' + $buyForm.data('product-entity-type')
                    + '&entity_id=' + $buyForm.data('product-entity-id')
                    + '&bundle=' + $buyForm.data('product-bundle')
                    + '&view_mode=' + $buyForm.data('product-view-mode')
                    + '&is_secondary=' + $buyForm.data('product-is-secondary')
                    + getAllHtmlIds();
                $.ajax({
                  async: true,
                  cache: false,
                  url: $shopCartForm.data('buy_button_form_url'),
                  data: data,
                  dataType: 'json',
                  type: 'POST',
                  success: function(response) {
                    var $newForm = $(response.form);
                    if (buyButtonFormClicked && $messages) {
                      $messages.prependTo($newForm);
                      setTimeout(function() {
                          $messages.slideUp(function() {
                            $messages.remove();
                          });
                      }, 2000);
                    }
                    $buyForm.replaceWith($newForm);
                    Drupal.attachBehaviors($newForm.parent(), response[0].settings);
                  },
                  error: function (jqXHR, textStatus, errorThrown) {}
                });
              });
            });
          },
          error: function (jqXHR, textStatus, errorThrown) {}
        });
      }

      $('.block.block-shop-cart', context).once('shop-cart-block', function() {
        var $shopCartForm = $('form.shop-cart-wrapper', this);
        var $submitButton = $('.form-actions input[name="op"].default-submit:last', $shopCartForm);
        $('td.col-qty input.form-digit', $shopCartForm).change(function(e) {
          $submitButton.click();
        });
        $('.col-delete input.form-submit', $shopCartForm).click(function(e) {
          e.preventDefault();
          shopCartBlockFormAjax($shopCartForm, $(this));
        }); 
        $submitButton.click(function(e) {
          e.preventDefault();
          shopCartBlockFormAjax($shopCartForm, $(this));
        });
      });


      $('form.buy-button-form.purchase-action-stay', context).once('shop-cart-buy-button-form', function() {
        var $buyForm = $(this);
        $('input.form-submit.default-submit:not(.ajax-processed)', $buyForm).click(function(e) {
          e.preventDefault();
          $buyForm.addClass('clicked-form');
          var $shopCartForm = $('.block.block-shop-cart form.shop-cart-wrapper');
          if ($shopCartForm.length) {
            // Fill hidden textfield for submission to server @see shop_carts_block_form_submit()
            var entity_type = $buyForm.data('product-entity-type');
            var entity_id = $buyForm.data('product-entity-id');
            $('input[name="add"]', $shopCartForm).val([entity_type, entity_id, $('input[name="qty"]', $buyForm).val() || 1].join(':'));
            
            // Animated flying good into basket
            var $movableObject = {};
            var $receiver = {};
            // Find preferred movable element
            $.each(['.field-type-image .field-item img', '.image-block-gallery img', '.node-title', '.form-item-qty', '.product-price', 'form.buy-button-form'], function (idx, selector) {
              if (($movableObject = $(selector + ':visible:first', $buyForm.closest('.content'))).length) {
                return false; // break each()
              }
            });
            if ($movableObject.length) {
               // find preferred receiver element
              $receiver = $('table.shop-cart-items-list:visible tr#cart-item-' + entity_type + '-' + entity_id + ' td:first:visible', $shopCartForm);
              if (!$receiver.length) {
                $receiver = $('.shop-cart-block-caption:visible', $shopCartForm).first();
                if (!$receiver.length) {
                  $receiver = $shopCartForm.is(':visible') ? $shopCartForm : $shopCartForm.closest('.block');
                }
              }
            }
            if ($movableObject.length && $receiver.length) {
              // clone item and place it to <body>
              $movableObject
                .clone()
                .addClass('shop-item-movable') // see shop.css
                .appendTo('body')
                .css({
                  left: $movableObject.offset().left + 'px',
                  top: $movableObject.offset().top + 'px',
                  width:  $movableObject.width(),
                  height:  $movableObject.height(),
                  'font-size': $movableObject.css('font-size') // copy font size from original because element placed into <body>
                })
                .animate({ // move item into cart block
                  opacity: 0.7,
                  left: $receiver.offset().left,
                  top: $receiver.offset().top,
                  width: $receiver.width(),
                  height: $receiver.height()
                }, 500, function () { // animation complete
                  $(this).remove();
                  // Call main ajax routines in block form:
                  $('.form-actions input[name="op"].default-submit:last', $shopCartForm).click(); 
                });        
            }
            else {
              $('.form-actions input[name="op"].default-submit:last', $shopCartForm).click(); 
            }

          }
        });
      });
    }
  };
})(jQuery);



