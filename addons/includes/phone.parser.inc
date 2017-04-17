<?php

/**
* This class used by addons_parse_phone() function (@see element.phone.inc)
*/

class PhonesParser {

  private static $_instance = NULL;

  private $_countriesCodesArrayFile;
  private $_regionalArrayFiles;

  private $_countriesCodesArray = array();
  private $_regionalArrays = array();

  private function __construct() {
    $this->_countriesCodesArrayFile = drupal_get_path('module', 'addons') . '/lib/inputmask_multi/phone-codes.json';
    $this->_regionalArrayFiles = array(
      '7' => drupal_get_path('module', 'addons') . '/lib/inputmask_multi/phones-ru.json',
      '1' => drupal_get_path('module', 'addons') . '/lib/inputmask_multi/phones-us.json',
    );

    $this->loadCountriesArrayData();
    $this->loadRegionalArraysData();
  }

  public function __clone() {
    throw new BadMethodCallException(sprintf('Class %s cannot be instantiated explicitly.', __CLASS__));
  }

  public static function getInstance() {
    if (is_null(self::$_instance)) {
      self::$_instance = new PhonesParser();
    }
    return self::$_instance;
  }

  public function parsePhoneNumber($phoneNumber) {

    $result = array(
      'country_code' => '',
      'region_code' => '',
      'phone' => $phoneNumber . '',
    );
    // convert russian 11 digit cellular 89999999999 to +79999999999
    $phoneNumber = preg_replace('/^\s*8([\-\(\)]*)((\d[\-\(\)]*){10})[\s\-\(\)]*$/u', '+7$1$2', $phoneNumber);
    // if $phoneNumber does not start with "+" - we can't parse it correctly and return as is
    if (!preg_match('/^\s*\+/', $phoneNumber)) {
      return $result;
    }

    $phoneNumber = preg_replace('/\D/u', '', $phoneNumber); // remove all except digits

    // try to determine country code
    $countryCodeLength = NULL;
    foreach ($this->_countriesCodesArray as $countryCode) {
      if (strpos($phoneNumber, $countryCode) === 0) {
        $result['country_code'] = $countryCode;
        $countryCodeLength = strlen($countryCode);
        break;
      }
    }

    // if we found country code - try to determine regional code
    $regionCodeLength = NULL;
    if (!is_null($countryCodeLength)) {
      if (strpos($phoneNumber, '79') === 0) { // crunch for russian mobiles
        $result['region_code'] = substr($phoneNumber, 1, 3);
        $regionCodeLength = 3;
      }
      elseif (array_key_exists($result['country_code'], $this->_regionalArrays)) {
        foreach ($this->_regionalArrays[$result['country_code']] as $regionCode) {
          if (strpos($phoneNumber, $regionCode, $countryCodeLength) === $countryCodeLength) {
            $result['region_code'] = $regionCode;
            $regionCodeLength = strlen($regionCode);
            break;
          }
        }
      }
    }

    // the rest on given $phoneNumber is phone (without country code and region code)
    $result['phone'] = substr($phoneNumber, $countryCodeLength + $regionCodeLength);

    return $result;

  }


  private function loadCountriesArrayData() {
    if (!@is_file($this->_countriesCodesArrayFile)) {
      throw new Exception('Cannot load coutries array data.');
    }
    $countriesCodesMap = @json_decode(@file_get_contents($this->_countriesCodesArrayFile));
    if (!$countriesCodesMap) {
      throw new Exception('Cannot parse coutries array from JSON.');
    }

    foreach ($countriesCodesMap as $countryData) {
      $mask = $countryData->mask;
      $matches = array();
      preg_match('/^\+(\d+)/', $mask, $matches);
      $this->_countriesCodesArray[] = $matches[1];
    }

    $this->_countriesCodesArray = array_unique(array_filter($this->_countriesCodesArray));
    natsort($this->_countriesCodesArray);
    $this->_countriesCodesArray = array_reverse($this->_countriesCodesArray);
  }

  private function loadRegionalArraysData() {

    $regionalMaps = array();

    foreach ($this->_regionalArrayFiles as $coutryCode => $regionalFilename) {

      if (!@is_file($regionalFilename)) {
        throw new Exception(sprintf('Cannot load regional array data for country code %s.', $coutryCode));
      }
      $regionalMaps[$coutryCode] = @json_decode(@file_get_contents($regionalFilename));
      if (!$regionalMaps[$coutryCode]) {
        throw new Exception(sprintf('Cannot parse regional array data for country code %s from JSON.', $coutryCode));
      }

      $this->_regionalArrays[$coutryCode] = array();
      foreach ($regionalMaps[$coutryCode] as $regionData) {
        $mask = $regionData->mask;
        $matches = array();
        preg_match('/\(([^)]+)\)/', $mask, $matches);
        if ($matches[1] !== '9##') { // crunch for russian mobile phones
          $this->_regionalArrays[$coutryCode][] = $matches[1];
        }
        else {
          $this->_regionalArrays[$coutryCode][] = '900';
        }
      }

      $this->_regionalArrays[$coutryCode] = array_unique(array_filter($this->_regionalArrays[$coutryCode]));
      natsort($this->_regionalArrays[$coutryCode]);
      $this->_regionalArrays[$coutryCode] = array_reverse($this->_regionalArrays[$coutryCode]);

    }

  }

}
