<?php


function addons_validate_icq(&$element, &$form_state) {
	if($element['#value']) {
		if (!filter_var($element['#value'], FILTER_VALIDATE_INT)) {
			form_error($element, t('"%icq" is not a valid icq', array('%icq' => $element['#value'])));
		}
	}
}
