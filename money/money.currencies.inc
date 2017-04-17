<?php

/**
* This file included in money.module -->> money_currency_info() function
*/


/**
* Big internal function
* @see money_currency_info()
*/
function _money_currency_info($currency = NULL, $property = NULL) {
	static $list;
	if (!$list) {
		$list = array(
			'AED' => array(
				'title' => 'UAE dirham',
				'country' => 'AE',
			),
			'AFN' => array(
				'title' => 'Afghan afghani',
        'country' => 'AF',
			),
			'ALL' => array(
				'title' => 'Albanian lek',
				'country' => 'AL',
			),
			'AMD' => array(
				'title' => 'Armenian dram',
				'country' => 'AM',
			),
			'ANG' => array(
				'title' => 'Netherlands Antillean gulden',
				'symbol' => 'EC$',
				'country' => 'AN',
			),
			'AOA' => array(
				'title' => 'Angolan kwanza',
				'country' => 'AO',
			),
			'ARS' => array(
				'title' => 'Argentine peso',
				'country' => 'AR',
			),
			'AUD' => array(
				'title' => 'Australian dollar',
				'symbol' => 'A$',
				'country' => 'TV',
			),
			'AWG' => array(
				'title' => 'Aruban florin',
				'symbol' => '&#402;',
				'country' => 'AW',
			),
			'AZM' => array(
				'title' => 'Azerbaijanian Manat',
				'country' => 'AZ',
			),
			'BAM' => array(
				'title' => 'Bosnia and Herzegovina konvertibilna marka',
				'symbol' => 'KM',
				'country' => 'BA',
			),
			'BBD' => array(
				'title' => 'Barbadian dollar',
				'symbol' => 'Bds$',
				'country' => 'BB',
			),
			'BDT' => array(
				'title' => 'Bangladeshi taka',
				'country' => 'BD',
			),
			'BGN' => array(
				'title' => 'Bulgarian lev',
				'country' => 'BG',
			),
			'BHD' => array(
				'title' => 'Bahraini dinar',
				'decimals' => 3,
				'country' => 'BH',
			),
			'BIF' => array(
				'title' => 'Burundi franc',
				'symbol' => 'FBu',
				'decimals' => 0,
				'country' => 'BI',
			),
			'BMD' => array(
				'title' => 'Bermudian dollar',
				'symbol' => 'BD$',
				'country' => 'BM',
			),
			'BND' => array(
				'title' => 'Brunei dollar',
				'symbol' => 'B$',
				'country' => 'BN',
			),
			'BOB' => array(
				'title' => 'Bolivian boliviano',
				'symbol' => 'Bs.',
				'country' => 'BO',
			),
			'BRL' => array(
				'title' => 'Brazilian real',
				'symbol' => 'R$',
				'country' => 'BR',
			),
			'BSD' => array(
				'title' => 'Bahamian dollar',
				'symbol' => 'B$',
				'country' => 'BS',
			),
			'BTN' => array(
				'title' => 'Bhutanese ngultrum',
				'symbol' => 'Nu.',
				'country' => 'BT',
			),
			'BWP' => array(
				'title' => 'Botswana pula',
				'symbol' => 'P',
				'country' => 'BW',
			),
			'BYR' => array(
				'title' => 'Belarusian ruble',
				'symbol' => 'Br',
				'decimals' => 0,
				'country' => 'BY',
			),
			'BZD' => array(
				'title' => 'Belize dollar',
				'symbol' => 'BZ$',
				'country' => 'BZ',
			),
			'CAD' => array(
				'title' => 'Canadian dollar',
				'symbol' => 'CA$',
				'country' => 'CA',
			),
			'CDF' => array(
				'title' => 'Congolese franc',
				'symbol' => 'F',
				'country' => 'CD',
			),
			'CHF' => array(
				'title' => 'Swiss franc',
				'symbol' => 'Fr.',
				'country' => 'CH',
			),
			'CLP' => array(
				'title' => 'Chilean peso',
				'symbol' => '$',
				'decimals' => 0,
				'country' => 'CL',
			),
			'CNY' => array(
				'title' => 'Chinese renminbi',
				'symbol' => '&#165;',
				'country' => 'CN',
			),
			'COP' => array(
				'title' => 'Colombian peso',
				'symbol' => 'Col$',
				'country' => 'CO',
			),
			'CRC' => array(
				'title' => 'Costa Rican colon',
				'symbol' => '&#8353;',
				'country' => 'CR',
			),
			'CSD' => array(
				'title' => 'Serbian Dinar',
				'country' => 'RS',
			),
			'CUP' => array(
				'title' => 'Cuban Peso',
				'country' => 'CU',
			),
			'CVE' => array(
				'title' => 'Cape Verdean escudo',
				'symbol' => 'Esc',
				'country' => 'CV',
			),
			'CYP' => array(
				'title' => 'Cyprus Pound',
				'country' => 'CY',
			),
			'CZK' => array(
				'title' => 'Czech koruna',
				'symbol' => 'K&#269;',
				'country' => 'CZ',
			),
			'DJF' => array(
				'title' => 'Djiboutian franc',
				'symbol' => 'Fdj',
				'decimals' => 0,
				'country' => 'DJ',
			),
			'DKK' => array(
				'title' => 'Danish krone',
				'symbol' => 'Kr',
				'country' => 'GL',
			),
			'DOP' => array(
				'title' => 'Dominican peso',
				'symbol' => 'RD$',
				'country' => 'DO',
			),
			'DZD' => array(
				'title' => 'Algerian dinar',
				'country' => 'DZ',
			),
			'EEK' => array(
				'title' => 'Estonian kroon',
				'symbol' => 'KR',
				'country' => 'EE',
			),
			'EGP' => array(
				'title' => 'Egyptian pound',
				'symbol' => '&#163;',
				'country' => 'EG',
			),
			'ERN' => array(
				'title' => 'Eritrean nakfa',
				'symbol' => 'Nfa',
				'country' => 'ER',
			),
			'ETB' => array(
				'title' => 'Ethiopian birr',
				'symbol' => 'Br',
				'country' => 'ET',
			),
			'EUR' => array(
				'title' => 'European euro',
				'symbol' => '&#8364;',
				'country' => 'VA',
			),
			'FJD' => array(
				'title' => 'Fijian dollar',
				'symbol' => 'FJ$',
				'country' => 'FJ',
			),
			'FKP' => array(
				'title' => 'Falkland Islands pound',
				'symbol' => '&#163;',
				'country' => 'FK',
			),
			'GBP' => array(
				'title' => 'British pound',
				'symbol' => '&#163;',
				'country' => 'GB',
			),
			'GEL' => array(
				'title' => 'Georgian lari',
				'country' => 'GE',
			),
			'GHC' => array(
				'title' => 'Ghana Cedi',
				'country' => 'GH',
			),
			'GIP' => array(
				'title' => 'Gibraltar pound',
				'symbol' => '&#163;',
				'country' => 'GI',
			),
			'GMD' => array(
				'title' => 'Gambian dalasi',
				'symbol' => 'D',
				'country' => 'GM',
			),
			'GNF' => array(
				'title' => 'Guinean franc',
				'symbol' => 'FG',
				'decimals' => 0,
				'country' => 'GN',
			),
			'GTQ' => array(
				'title' => 'Guatemalan quetzal',
				'symbol' => 'Q',
				'country' => 'GT',
			),
			'GYD' => array(
				'title' => 'Guyanese dollar',
				'symbol' => 'GY$',
				'country' => 'GY',
			),
			'HKD' => array(
				'title' => 'Hong Kong dollar',
				'symbol' => 'HK$',
				'country' => 'HK',
			),
			'HNL' => array(
				'title' => 'Honduran lempira',
				'symbol' => 'L',
				'country' => 'HN',
			),
			'HRK' => array(
				'title' => 'Croatian kuna',
				'symbol' => 'kn',
				'country' => 'HR',
			),
			'HTG' => array(
				'title' => 'Haitian gourde',
				'symbol' => 'G',
				'country' => 'HT',
			),
			'HUF' => array(
				'title' => 'Hungarian forint',
				'symbol' => 'Ft',
				'country' => 'HU',
			),
			'IDR' => array(
				'title' => 'Indonesian rupiah',
				'symbol' => 'Rp',
				'country' => 'ID',
			),
			'ILS' => array(
				'title' => 'Israeli new sheqel',
				'country' => 'PS',
			),
			'INR' => array(
				'title' => 'Indian rupee',
				'country' => 'IN',
			),
			'IQD' => array(
				'title' => 'Iraqi dinar',
				'decimals' => 3,
				'country' => 'IQ',
			),
			'IRR' => array(
				'title' => 'Iranian rial',
				'country' => 'IR',
			),
			'ISK' => array(
				'title' => 'Icelandic kr&oacute;na',
				'symbol' => 'kr',
				'decimals' => 0,
				'country' => 'IS',
			),
			'JMD' => array(
				'title' => 'Jamaican dollar',
				'symbol' => 'J$',
				'country' => 'JM',
			),
			'JOD' => array(
				'title' => 'Jordanian dinar',
				'decimals' => 3,
				'country' => 'JO',
			),
			'JPY' => array(
				'title' => 'Japanese yen',
				'symbol' => '&#165;',
				'decimals' => 0,
				'country' => 'JP',
			),
			'KES' => array(
				'title' => 'Kenyan shilling',
				'symbol' => 'KSh',
				'country' => 'KE',
			),
			'KGS' => array(
				'title' => 'Kyrgyzstani som',
				'country' => 'KG',
			),
			'KHR' => array(
				'title' => 'Cambodian riel',
				'country' => 'KH',
			),
			'KMF' => array(
				'title' => 'Comorian franc',
				'decimals' => 0,
				'country' => 'KM',
			),
			'KPW' => array(
				'title' => 'North Korean won',
				'symbol' => 'W',
				'country' => 'KP',
			),
			'KRW' => array(
				'title' => 'South Korean won',
				'symbol' => 'W',
				'decimals' => 0,
				'country' => 'KR',
			),
			'KWD' => array(
				'title' => 'Kuwaiti dinar',
				'decimals' => 3,
				'country' => 'KW',
			),
			'KYD' => array(
				'title' => 'Cayman Islands dollar',
				'symbol' => 'KY$',
				'country' => 'KY',
			),
			'KZT' => array(
				'title' => 'Kazakhstani tenge',
				'symbol' => 'T',
				'country' => 'KZ',
			),
			'LAK' => array(
				'title' => 'Lao kip',
				'symbol' => 'KN',
				'country' => 'LA',
			),
			'LBP' => array(
				'title' => 'Lebanese lira',
				'country' => 'LB',
			),
			'LKR' => array(
				'title' => 'Sri Lankan rupee',
				'symbol' => 'Rs',
				'country' => 'LK',
			),
			'LRD' => array(
				'title' => 'Liberian dollar',
				'symbol' => 'L$',
				'country' => 'LR',
			),
			'LSL' => array(
				'title' => 'Lesotho loti',
				'symbol' => 'M',
				'country' => 'LS',
			),
			'LTL' => array(
				'title' => 'Lithuanian litas',
				'symbol' => 'Lt',
				'country' => 'LT',
			),
			'LVL' => array(
				'title' => 'Latvian lats',
				'symbol' => 'Ls',
				'country' => 'LV',
			),
			'LYD' => array(
				'title' => 'Libyan dinar',
				'symbol' => 'LD',
				'decimals' => 3,
				'country' => 'LY',
			),
			'MAD' => array(
				'title' => 'Moroccan dirham',
				'country' => 'MA',
			),
			'MDL' => array(
				'title' => 'Moldovan leu',
				'country' => 'MD',
			),
			'MGA' => array(
				'title' => 'Malagasy ariary',
				'symbol' => 'FMG',
				'country' => 'MG',
			),
			'MKD' => array(
				'title' => 'Macedonian denar',
				'country' => 'MK',
			),
			'MNT' => array(
				'title' => 'Mongolian tugrik',
				'symbol' => '&#8366;',
				'country' => 'MN',
			),
			'MOP' => array(
				'title' => 'Macanese pataca',
				'symbol' => 'P',
				'country' => 'MO',
			),
			'MRO' => array(
				'title' => 'Mauritanian ouguiya',
				'symbol' => 'UM',
				'country' => 'MR',
			),
			'MTL' => array(
				'title' => 'Maltese Lira',
				'country' => 'MT',
			),
			'MUR' => array(
				'title' => 'Mauritian rupee',
				'symbol' => 'Rs',
				'country' => 'MU',
			),
			'MVR' => array(
				'title' => 'Maldivian rufiyaa',
				'symbol' => 'Rf',
				'country' => 'MV',
			),
			'MWK' => array(
				'title' => 'Malawian kwacha',
				'symbol' => 'MK',
				'country' => 'MW',
			),
			'MXN' => array(
				'title' => 'Mexican peso',
				'symbol' => '$',
				'country' => 'MX',
			),
			'MYR' => array(
				'title' => 'Malaysian ringgit',
				'symbol' => 'RM',
				'country' => 'MY',
			),
			'MZM' => array(
				'title' => 'Mozambican metical',
				'symbol' => 'MTn',
				'country' => 'MZ',
			),
			'NAD' => array(
				'title' => 'Namibian dollar',
				'symbol' => 'N$',
				'country' => 'NA',
			),
			'NGN' => array(
				'title' => 'Nigerian naira',
				'symbol' => '&#8358;',
				'country' => 'NG',
			),
			'NIO' => array(
				'title' => 'Nicaraguan c&oacute;rdoba',
				'symbol' => 'C$',
				'country' => 'NI',
			),
			'NOK' => array(
				'title' => 'Norwegian krone',
				'symbol' => 'kr',
				'country' => 'SJ',
			),
			'NZD' => array(
				'title' => 'New Zealand dollar',
				'symbol' => 'NZ$',
				'country' => 'TK',
			),
			'OMR' => array(
				'title' => 'Omani rial',
				'decimals' => 3,
				'country' => 'OM',
			),
			'PAB' => array(
				'title' => 'Panamanian balboa',
				'symbol' => 'B./',
				'country' => 'PA',
			),
			'PEN' => array(
				'title' => 'Peruvian nuevo sol',
				'symbol' => 'S/.',
				'country' => 'PE',
			),
			'PGK' => array(
				'title' => 'Papua New Guinean kina',
				'symbol' => 'K',
				'country' => 'PG',
			),
			'PHP' => array(
				'title' => 'Philippine peso',
				'symbol' => '&#8369;',
				'country' => 'PH',
			),
			'PKR' => array(
				'title' => 'Pakistani rupee',
				'symbol' => 'Rs.',
				'country' => 'PK',
			),
			'PLN' => array(
				'title' => 'Polish zloty',
				'country' => 'PL',
			),
			'PYG' => array(
				'title' => 'Paraguayan guarani',
				'decimals' => 0,
				'country' => 'PY',
			),
			'QAR' => array(
				'title' => 'Qatari riyal',
				'symbol' => 'QR',
				'country' => 'QA',
			),
			'RON' => array(
				'title' => 'Romanian leu',
				'symbol' => 'L',
				'country' => 'RO',
			),
			'RUB' => array(
				'title' => 'Russian ruble',
				'symbol' => '&#x20bd;',
				'country' => 'RU',
			),/*
			'RUR' => array(
				'title' => 'Russian ruble',
				'symbol' => 'R',
				'country' => 'RU',
			),*/
			'RWF' => array(
				'title' => 'Rwandan franc',
				'symbol' => 'RF',
				'decimals' => 0,
				'country' => 'RW',
			),
			'SAR' => array(
				'title' => 'Saudi riyal',
				'symbol' => 'SR',
				'country' => 'SA',
			),
			'SBD' => array(
				'title' => 'Solomon Islands dollar',
				'symbol' => 'SI$',
				'country' => 'SB',
			),
			'SCR' => array(
				'title' => 'Seychellois rupee',
				'symbol' => 'SR',
				'country' => 'SC',
			),
			'SDD' => array(
				'title' => 'Sudanese Dinar',
				'country' => 'SD',
			),
			'SEK' => array(
				'title' => 'Swedish krona',
				'symbol' => 'kr',
				'country' => 'SE',
			),
			'SGD' => array(
				'title' => 'Singapore dollar',
				'symbol' => 'S$',
				'country' => 'SG',
			),
			'SHP' => array(
				'title' => 'Saint Helena pound',
				'symbol' => '&#163;',
				'country' => 'SH',
			),
			'SIT' => array(
				'title' => 'Slovenian Tolar',
				'country' => 'SI',
			),
			'SKK' => array(
				'title' => 'Slovak Koruna',
				'country' => 'SK',
			),
			'SLL' => array(
				'title' => 'Sierra Leonean leone',
				'symbol' => 'Le',
				'country' => 'SL',
			),
			'SOS' => array(
				'title' => 'Somali shilling',
				'symbol' => 'Sh.',
				'country' => 'SO',
			),
			'SRD' => array(
				'title' => 'Surinamese dollar',
				'symbol' => 'SRD',
				'country' => 'SR',
			),
			'STD' => array(
				'title' => 'S&atilde;o Tom&eacute; and Pr&iacute;ncipe',
				'symbol' => 'Db',
				'country' => 'ST',
			),
			'SVC' => array(
				'title' => 'El Salvador Colon',
				'country' => 'SV',
			),
			'SYP' => array(
				'title' => 'Syrian pound',
				'country' => 'SY',
			),
			'SZL' => array(
				'title' => 'Swazi lilangeni',
				'symbol' => 'E',
				'country' => 'SZ',
			),
			'THB' => array(
				'title' => 'Thai baht',
				'symbol' => '&#3647;',
				'country' => 'TH',
			),
			'TJS' => array(
				'title' => 'Tajikistani somoni',
				'country' => 'TJ',
			),
			'TMM' => array(
				'title' => 'Turkmenistan Manat',
				'country' => 'TM',
			),
			'TND' => array(
				'title' => 'Tunisian dinar',
				'symbol' => 'DT',
				'decimals' => 3,
				'country' => 'TN',
			),
			'TOP' => array(
				'title' => 'Tonga Pa\'anga',
				'symbol' => 'T$',
				'country' => 'TO',
			),
			'TRY' => array(
				'title' => 'Turkish new lira',
				'symbol' => 'YTL',
				'country' => 'TR',
			),
			'TTD' => array(
				'title' => 'Trinidad and Tobago dollar',
				'symbol' => 'TT$',
				'country' => 'TT',
			),
			'TWD' => array(
				'title' => 'New Taiwan dollar',
				'symbol' => 'NT$',
				'country' => 'TW',
			),
			'TZS' => array(
				'title' => 'Tanzanian shilling',
				'country' => 'TZ',
			),
			'UAH' => array(
				'title' => 'Ukrainian hryvnia',
				'symbol' => '&#8372;',
				'country' => 'UA',
			),
			'UGX' => array(
				'title' => 'Ugandan shilling',
				'symbol' => 'USh',
				'country' => 'UG',
			),
			'USD' => array(
				'title' => 'United States dollar',
				'symbol' => 'US$',
				'country' => 'VI',
			),
			'UYU' => array(
				'title' => 'Uruguayan peso',
				'symbol' => '$U',
				'country' => 'UY',
			),
			'UZS' => array(
				'title' => 'Uzbekistani som',
				'country' => 'UZ',
			),
			'VEB' => array(
				'title' => 'Venezuelan bolivar',
				'symbol' => 'Bs',
				'country' => 'VE',
			),
			'VND' => array(
				'title' => 'Vietnamese dong',
				'symbol' => '&#8363;',
				'country' => 'VN',
			),
			'VUV' => array(
				'title' => 'Vanuatu vatu',
				'symbol' => 'VT',
				'decimals' => 0,
				'country' => 'VU',
			),
			'WST' => array(
				'title' => 'Samoan tala',
				'symbol' => 'WS$',
				'country' => 'WS',
			),
			'XAF' => array(
				'title' => 'Central African CFA franc',
				'symbol' => 'CFA',
				'decimals' => 0,
				'country' => 'GA',
			),
			'XCD' => array(
				'title' => 'East Caribbean dollar',
				'symbol' => 'EC$',
				'country' => 'VC',
			),
			'XDR' => array(
				'title' => 'Special Drawing Rights',
				'symbol' => 'SDR',
				'decimals' => 5,
				'country' => NULL,
			),
			'XOF' => array(
				'title' => 'West African CFA franc',
				'symbol' => 'CFA',
				'decimals' => 0,
				'country' => 'TG',
			),
			'XPF' => array(
				'title' => 'CFP franc',
				'symbol' => 'F',
				'decimals' => 0,
				'country' => 'WF',
			),
			'YER' => array(
				'title' => 'Yemeni rial',
				'country' => 'YE',
			),
			'ZAR' => array(
				'title' => 'South African rand',
				'symbol' => 'R',
				'country' => 'ZA',
			),
			'ZMK' => array(
				'title' => 'Zambian kwacha',
				'symbol' => 'ZK',
				'country' => 'ZM',
			),
			'ZWD' => array(
				'title' => 'Zimbabwean dollar',
				'symbol' => 'Z$',
				'country' => 'ZW',
			),
			'ZWR' => array(
				'title' => 'Zimbabwean dollar',
				'symbol' => 'Z$',
				'decimals' => 0,
				'country' => 'ZW',
			),
			'AZN' => array(
				'title' => 'Azerbaijani manat',
				'decimals' => 0,
				'country' => 'AZ',
			),
			'CUC' => array(
				'title' => 'Cuban peso',
				'symbol' => '$',
				'decimals' => 0,
				'country' => 'CU',
			),
			'GQE' => array(
				'title' => 'Central African CFA franc',
				'symbol' => 'CFA',
				'decimals' => 0,
				'country' => 'GQ',
			),
			'GHS' => array(
				'title' => 'Ghanaian cedi',
				'symbol' => '&#8373;',
				'decimals' => 0,
				'country' => 'GH',
			),
			'RSD' => array(
				'title' => 'Serbian dinar',
				'symbol' => 'din.',
				'decimals' => 0,
				'country' => 'RS',
			),
			'SDG' => array(
				'title' => 'Sudanese pound',
				'decimals' => 0,
				'country' => 'SD',
			),
			'TMT' => array(
				'title' => 'Turkmen manat',
				'symbol' => 'm',
				'decimals' => 0,
				'country' => 'TM',
			),
		);
      foreach ($list as $key => $data) {
        $list[$key] += array(
          'decimals' => 2,
        );
        $list[$key]['title'] = html_entity_decode(t($data['title']));
        $list[$key]['reduction'] = t(strtolower($key), array(), array('context' => 'currency'));
        $list[$key]['symbol'] = empty($data['symbol']) ? $key : html_entity_decode($data['symbol'], ENT_COMPAT, 'UTF-8');
        $list[$key]['full'] = $list[$key]['title'] . ' (' . $list[$key]['symbol'] . ', ' . $list[$key]['reduction'] . ')';
        $list[$key]['id'] = $key;
      }
    drupal_alter('currency_info', $list);
	}
	
	$result = $list;
	if ($property) {
		foreach ($result as $key => $data) {
			$result[$key] = isset($result[$key][$property]) ? $result[$key][$property] : NULL;
		}
	}
  if ($currency) {
    $currency = drupal_strtoupper($currency);
    if (!isset($result[$currency])) {
      if (module_exists('transliteration') && preg_match('/[^A-Z]/u', $currency)) {
        $currency = transliteration_get($currency);
      }
      else {
        $currency = preg_replace('/[^A-Z]/u', '', $currency);
      }
      $currency = $currency ? drupal_substr($currency, 0, 3) : '-';
      $currency = $currency === 'RUR' ? 'RUB' : $currency;
    }
    return $currency && isset($result[$currency]) ? $result[$currency] : NULL;
  }
  else {
    return $result;
  }
	
}


















