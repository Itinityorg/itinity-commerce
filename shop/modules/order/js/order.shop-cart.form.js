(function ($) {
	"use strict";
	Drupal.shop = Drupal.shop || {};
  
	Drupal.shop.formatPrice = function(price) {
		if (!price) {
			return '-';
		}
		var parts = price.toFixed(Drupal.settings.currencyFormat.decimals).replace(/[\.\,]/g, Drupal.settings.currencyFormat.dec).split(Drupal.settings.currencyFormat.dec);
		return parts[0].replace(/\B(?=(\d{3})+(?!\d))/g, "&thinsp;") + (parts[1] && !parts[1].match(/^0+$/) ? Drupal.settings.currencyFormat.dec + parts[1] : "");
		// todo settings.currencyFormat.thousands
	};


	Drupal.shop.cartRecalc = function($form) {
		var summ = {};
		var cart_empty = true;
		$('tr.cart-item', $form).each(function() {
			var $qty = $('.col-qty .form-type-digit input.form-digit', this);
			var curr = $('td.col-price .shop-item-price', this).attr('data-currency'); //$qty.attr('data-currency');
			if (!summ[curr]) {
				summ[curr] = 0;
			}
			var price = $qty.val() * $('td.col-price .shop-item-price', this).attr('data-price');
			if ($qty.val() * 1) {
				cart_empty = false;
			}
			summ[curr] += price;
			$('>td', this)[price ? 'removeClass' : 'addClass']('node-unpublished');
			$('td.col-sum .shop-item-price', this).html(Drupal.shop.formatPrice(price));
		});
		$.each(summ, function(currency, price) {
			$('td.col-total>h3 .shop-item-price[data-currency="' + currency + '"]', $form).html(Drupal.shop.formatPrice(price));
			$('td.col-total>h3 .shop-item-price[data-currency="' + currency + '"]', $form).attr('data-price', price);
		});
		if (!cart_empty) {
			$('.order-price-total', $form).removeClass('element-invisible');
			var $variant = $('input[name="shop_cart[delivery][courier_variant]"]:checked', $form);
			var threshold = $variant.attr('data-threshold');
			if ($variant.length && $variant.attr('data-price') && (!threshold || summ[Drupal.settings.currencyDefault] < threshold)) {
				$variant.parent().find('.courier-hint').removeClass('element-invisible').find('.shop-item-price').html(Drupal.shop.formatPrice(threshold - summ[Drupal.settings.currencyDefault]));
				summ[Drupal.settings.currencyDefault] += $variant.attr('data-price') * 1;
				$variant.parent().find('.courier-price').html($variant.attr('data-price-formatted'));
				$('.order-price-delivery .shop-item-price', $form).text($variant.attr('data-price'));
				$('.order-price-delivery', $form).removeClass('element-invisible');
				
			}
			else {
				$variant.parent().find('.courier-price').text(Drupal.t('for free'));
				$variant.parent().find('.courier-hint').addClass('element-invisible');
				
				$('.order-price-delivery', $form).addClass('element-invisible');
			}
			$('.order-price-total .shop-item-price', $form).html(Drupal.shop.formatPrice(summ[Drupal.settings.currencyDefault]));
      $('.order-price-total .shop-item-price', $form).attr('data-price', summ[Drupal.settings.currencyDefault]);
		}
		else {
			$('.order-price-delivery, .order-price-total, .courier-hint', $form).addClass('element-invisible');
			
		}
		
	};


	Drupal.behaviors.shop_order_form = {
		/**
		 * Attaches the behavior.
		 */
		attach: function (context, settings) {
			// Shop cart routines:
			// @see order_form()
			$('.shop-cart-sub-form', context).once('shop-cart', function() {
				$('input[name]', $(this).closest('form')).bind('change click', function() {
					Drupal.shop.cartRecalc($(this).closest('form'));
				});
			});
			
			if (settings.shop_cart_autosubmit === true) {
				$('<div class="ajax-progress"><div class="throbber"></div><p>' + Drupal.t('Order in progress ... ') + '</p></div>').insertBefore($('form#order-node-form', context));
				var $form = $('form#order-node-form', context);
				$form.attr('action', $form.attr('action').replace(/shop\/cart\/checkout\/.+/, 'shop/cart'));
				$('.form-actions input#edit-submit.form-submit', $form).click();
			}
			else {
				$('form.node-order-form', context).once('shop-cart', function() {
					var $form = $(this);
					if (!$('.save-continue-link', $form).length && $('.continue-button', $form).length) {
						$('input[name], select[name], textarea[name]', $form).live('change.shop-cart keyup.shop-cart', function() {
							$('.continue-button', $form).removeClass('js-hide');
						});
					}
				});
			}
		}
	};

})(jQuery);










