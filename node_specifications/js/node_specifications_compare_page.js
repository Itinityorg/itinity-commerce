(function ($) {
    "use strict";
	Drupal.behaviors.node_specifications_compare = {
		attach: function (context, settings) {
            $('#compare-table-wrapper', context).once('compare-page', function () {
                var $all_btn = $('<a href="#" class="active all-btn">' + Drupal.t('All specifications') + '</a>'),
                    $min_btn = $('<a href="#" class="min-btn">' + Drupal.t('Only differs') + '</a>'),
                    $compare_table = $('.compare-table', this),
                    $trs = $compare_table.find('tbody tr'),
                    differs_trs = [];

                //find <tr> with diffrent <td> by comparing html of 1st <td> with html of other <td>s in <tr>
                $trs.each(function () {
                    var
                        $tr = $(this),
                        $tds = $tr.find('td'),
                        $first_td = $tds.first();

                    $tds.not(':first').each(function () {
                        if ($first_td.html() != $(this).html()) {
                            differs_trs.push($tr[0]);
                            return false;
                        }
                    });
                });

                $all_btn.insertBefore($compare_table).click(function (e) {
                    $min_btn.removeClass('active');
                    $(this).addClass('active');
                    $trs.show();
                    e.preventDefault();
                });
                $min_btn.insertBefore($compare_table).click(function (e) {
                    $all_btn.removeClass('active');
                    $(this).addClass('active');
                    $trs.hide();
                    $(differs_trs).show();
                    e.preventDefault();
                });
            });

		}
	};
})(jQuery);

