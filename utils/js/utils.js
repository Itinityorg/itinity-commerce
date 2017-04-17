// $Id: utils.js, v 1.0 2011/03/23 16:53:37 ITKLD Exp $

/**
* JavaScript representation of some utils.module functionality
*/


var Utils = Utils || {};

(function ($) {
	/**
	* Colors operations
	*/

	Utils.validateHex = function(hex) {
		var regexp = /^(#)?([0-9a-fA-F]{3})([0-9a-fA-F]{3})?$/;
		return regexp.test(hex);
	};
	Utils.inverseHex = function(hex) {
		if (Utils.validateHex(hex)) {
			var use_hash = hex.substr(0, 1) == '#';
			if (use_hash) {
				hex	= hex.substr(1);
			}
			hex = Utils.RGBtoHex(Utils.inverseRGB(Utils.HexToRGB(hex)));
			if (use_hash) {
				hex = '#' + hex;
			}
		} else {
			hex = '';
		}
		return hex;
	};

	Utils.inverseRGB = function(rgb) {
		var rgb_i = new Array();
		for (var i = 0; i < 3; i++) {
			rgb_i[i] = 255 - rgb[i];
		}
		return rgb_i;
	};


	Utils.DesaturateHex = function(hex) {
		if (Utils.validateHex(hex)) {
			var use_hash = hex.substr(0, 1) == '#';
			if (use_hash) {
				hex	= hex.substr(1);
			}
			return (use_hash ? '#' : '') + Utils.RGBtoHex(Utils.desaturateRGB(Utils.HexToRGB(hex)));
		}
	};

	Utils.desaturateRGB = function(rgb) {
		var middle = Math.round((parseInt(rgb[0]) + parseInt(rgb[1]) + parseInt(rgb[2])) / 3);
		return [middle, middle, middle];
	};

	Utils.RGBtoHex = function(rgb) {
		var x = '0123456789ABCDEF', hex = '', n = '';
		for (var i = 0; i < 3; i++) {
			n  = parseInt(rgb[i]);
			hex += x.charAt(n>>4) + x.charAt(n&15);
		}
		return hex;
	};

	Utils.HexToRGB = function(hex) {
		var rgb = '0:0:0';
		if (Utils.validateHex(hex)) {
			if (hex.substr(0, 1) == '#') {
				hex	= hex.substr(1);
			}
			var x = '0123456789ABCDEF', rgb = '';
			if (!hex) {
				hex = '000000';
			}
			var l = hex.length;
			if (l == 3 || l == 6) {
				hex = hex.toUpperCase();
				for (var i = 0; i < l; i += (l == 3 ? 1 : 2)) {
					rgb += 16 * x.indexOf(hex.charAt(i)) + x.indexOf(hex.charAt(i + (l == 6 ? 1 : 0))) + ':';
				}
			}
		}
		return rgb.split(':');
	};

	Utils.numberFormat = function (number, decimals, dec_point, thousands_sep) {
		var exponent = "";
		var numberstr = number.toString ();
		var eindex = numberstr.indexOf ("e");
		var i, z;
		if(eindex > -1){
			exponent = numberstr.substring (eindex);
			number = parseFloat (numberstr.substring (0, eindex));
		}

		if(decimals != null){
			var temp = Math.pow (10, decimals);
			number = Math.round (number * temp) / temp;
		}
		var sign = number < 0 ? "-" : "";
		var integer = (number > 0 ?
		Math.floor (number) : Math.abs (Math.ceil (number))).toString ();

		var fractional = number.toString ().substring (integer.length + sign.length);
		dec_point = dec_point != null ? dec_point : ".";
		fractional = decimals != null && decimals > 0 || fractional.length > 1 ? (dec_point + fractional.substring (1)) : "";
		if(decimals != null && decimals > 0){
			for(i = fractional.length - 1, z = decimals; i < z; ++i)
				fractional += "0";
		}

		thousands_sep = (thousands_sep != dec_point || fractional.length == 0) ? thousands_sep : null;
		if(thousands_sep != null && thousands_sep != ","){
			for (i = integer.length - 3; i > 0; i -= 3)
				integer = integer.substring (0 , i) + thousands_sep + integer.substring (i);
		}
		return sign + integer + fractional + exponent;
	};

	Utils.imagecacheUrl = function (url, preset) {
		var presets = [];
		for (var i in Drupal.settings.imagecachePresets) {
			presets.push(i);
		}
		presets.push('original');
		if ($.inArray(preset, presets) != -1) {
			var expr = new RegExp('' + presets.join('|') + '', 'gi');
			var searchString = new RegExp("^(?:https?\://.+)?(?:\/)(?:sites/.+?\/files\/)(?:(?:styles\/)(?:[^\/]+)(?:\/public\/))?((?:.+/)?.+?\..*)", 'gi');
			var filePath = url.replace(searchString, "$1");
			if (!filePath.search('/') == 0) {
				filePath = '/' + filePath;
			}
			url = Drupal.settings.mediaPath + (preset == 'original' ? '' : '/styles/' + Drupal.settings.imagecachePresets[preset] + '/public') + filePath;
		}
		return url;
	};

})(jQuery);
