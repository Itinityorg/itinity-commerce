(function($) {
  'use strict';
  /**
  * Custom fix for abandoned jQuery.browser in jquery migrate plugin. 
  * @see jquery_latest_library()
  */
  
  if (!$.uaMatch) { // Copypaste from JQ 1.8
    $.uaMatch = function( ua ) {
      ua = ua.toLowerCase();

      var match = /(chrome)[ \/]([\w.]+)/.exec( ua ) ||
        /(webkit)[ \/]([\w.]+)/.exec( ua ) ||
        /(opera)(?:.*version|)[ \/]([\w.]+)/.exec( ua ) ||
        /(msie) ([\w.]+)/.exec( ua ) ||
        ua.indexOf("compatible") < 0 && /(mozilla)(?:.*? rv:([\w.]+)|)/.exec( ua ) ||
        [];

      return {
        browser: match[ 1 ] || "",
        version: match[ 2 ] || "0"
      };
    };
  }
  if ( !$.browser ) { // Copypaste from jquery migrate plugin v.1.2.1
    var matched = $.uaMatch( navigator.userAgent );
    var browser = {};

    if ( matched.browser ) {
      browser[ matched.browser ] = true;
      browser.version = matched.version;
    }

    // Chrome is Webkit, but Webkit is also Safari.
    if ( browser.chrome ) {
      browser.webkit = true;
    } else if ( browser.webkit ) {
      browser.safari = true;
    }

    $.browser = browser;
  }          
  
})(jQuery);