<?php //; $Id: contacts.tokens.inc 2012/08/16 10:34 Ivan Demenkov <ivan@demenkov.com> $

/**
 * Implements hook_token_info().
 */
function contacts_token_info() {

	$types = array();

	$types['contacts'] = array(
		'name'			=> t('Contacts'),
	);

	$contacts = array();

	$contacts['company-trade-name'] = array(
		'name'			=> t('Company trade name'),
	);

	return array(
		'types'		=> $types,
		'tokens'	=> array(
			'subscriber'	=> $contacts,
		),
	);
}



/**
 * Implements hook_tokens().
 */
function contacts_tokens($type, $tokens, array $data = array(), array $options = array()) {

	if (isset($options['language'])) {
		$url_options['language'] = $options['language'];
		$language_code = $options['language']->language;
	}
	else {
		$language_code = NULL;
	}

	$replacements = array();
	if ($type == 'contacts') {
		$contacts = utils_load_packed_var('site_contacts');
		foreach ($tokens as $name => $original) {
			switch ($name) {
				case 'company-trade-name':
					$replacements[$original] = !empty($contacts['names']['trade']) ? $contacts['names']['trade'] : variable_get('site_name', basename(conf_path()));
				break;
			}
		}
	}
	return $replacements;
}

