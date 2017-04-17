(function($) {
  /** 
  * Implements refresh online user status by js events (mouse, keyboard, etc) 
  * @see social_profile_online_status_refresh()
  */
  Drupal.behaviors.online_status_refresh = {
    attach: function(context, settings) {
      var ajax_stop = false;
      $('html:not(.online-status-refresh-processed)')
        .addClass('online-status-refresh-processed')
        .bind('mousemove.online_status_refresh click.online_status_refresh keypress.online_status_refresh scroll.online_status_refresh', function() {
          if (!ajax_stop && !$.cookie('user_is_online')) {
            ajax_stop = true; // Stop multiple ajax calls
            $.ajax({
              url: settings.online_status_refresh_url, //  ?q=user/online-status-refresh
              async: false,
              timeout: 10,
              cache: false,
              dataType: 'json',
              complete: function(jqXHR, textStatus) {
                if ($.cookie('user_is_online')) {
                  ajax_stop = false; // Unlock ajax_stop flag if server return user is online
                }
                else { // Unlock ajax_stop flag for recheck after 10 minutes if no result (user logged out in other tab or server error case)
                  setTimeout(function() {
                    ajax_stop = false;
                  }, 600000);
                }
              }
            });
          }
        });
    }
  }  
})(jQuery);





