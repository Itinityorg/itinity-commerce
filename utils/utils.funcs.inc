<?php


/**
* Helper func 
* Returns array of (regexps => descriptions) needed for robots detection
* @see _utils_is_robot()
*/
function _utils_crawlers_list() {
  return array(
    'abachobot' => 'AbachoBOT',
    'accoona(-ai-agent)?' => 'Accoona',
    'acoirobot' => 'AcoiRobot',
    'ahrefsbot' => 'AhrefsBot (http://ahrefs.com)',
    'altavista' => 'AltaVista',
    'antabot' => 'antabot (private)',
    'archive_org' => 'Archive.org',
    'ask\s+jeeves' => 'Ask.com/Teoma',
    'aspseek' => 'ASPSeek',
    'baiduspider' => 'Baidu Spider',
    'bingbot' => 'Bing.com',
    'contextad' => 'ContextAd Bot',
    'croccrawler' => 'CrocCrawler',
    'dataprovider\.com' => 'Dataprovider.com',
    'deusu\.de\/robot' => 'DeuSu (https://deusu.de/robot.html)',
    'dumbot' => 'Dumbot',
    'dotbot' => 'DotBot (http://www.opensiteexplorer.org/dotbot)',
    'eltaindexer' => 'EltaIndexer',
    'estyle' => 'eStyle Bot',
    'exabot|biggerbetter' => 'Exabot (BiggerBetter)',
    'facebookexternalhit' => 'Facebook (http://www.facebook.com/externalhit_uatext.php)',
    'fast-webcrawler' => 'FAST-WebCrawler',
    'gamespyhttp' => 'GameSpyHTTP',
    'geonabot' => 'GeonaBot',
    'gigabot' => 'Gigabot',
    'google(bot)?' => 'Google',
    'gsa-crawler' => 'Google Search Appliance',
    'gulper web bot' => 'Gulper Web Bot',
    'ia_archiver' => 'Alexa',
    'idbot' => 'ID-Search Bot',
    'lycos' => 'Lycos spider',
    'jigsaw' => 'W3C CSS Validator',
    'mail\.ru' => 'Mail.ru',
    'megaindex\.ru' => 'MegaIndex.ru', 
    'mihalismbot' => 'Mihalism Bot',
    'mj12bot' => 'MJ12bot (http://www.majestic12.co.uk/bot.php)',
    'msnbot' => 'MSN',
    'msrbot' => 'MSRBOT',
    'obot' => 'oBot (http://filterdb.iss.net/crawler/)',
    'omniExplorer_bot' => 'OmniExplorer Bot',
    'ooyyobot' => 'OoyyoBot',
    'pagebull' => 'PageBull',
    'pinterest' => 'Pinterest',
    'qwantify' => 'Qwant search engine (https://www.qwant.com/)',
    '(stack)?rambler' => 'Rambler',
    'riddler' => 'Riddler (http://riddler.io/about)',
    'sape\.bot' => 'Sape',
    'scooter' => 'Scooter (AltaVista)',
    'scrubby' => 'Scrubby robot',
    'seokicks-robot' => 'SEOkicks Webcrawlern (http://www.seokicks.de/robot.html)',
    'semrushbot' => 'SEMrush Competitive Data (https://semrush.com/bot/)',
    'slackbot' => 'Slackbot-LinkExpanding 1.0 (https://api.slack.com/robots)',
    'statdom\.ru' => 'statdom.ru web technologies crawler (http://statdom.ru/bot.html)',
    'turnitinbot' => 'TurnitIn Bot',
    'wbsearchbot' => 'Ware Bay Best Buys Search',
    'webalta' => 'WebAlta',
    'webalta\s+crawler' => 'WebAlta Crawler',
    'webindex' => 'WebIndex',
    'vkshare' => 'vkShare (http://vk.com/dev/Share)',
    'xenu\s+link\s+sleuth' => 'Xenu Link Sleuth',
    'yahoo|slurp' => 'Yahoo',
    'ya(ndex)?(market|metrika|direct|bot)' => 'Yandex',
    'yandex\.com\/bots' => 'Yandex',
    
    'searchmetricsbot' => 'SearchmetricsBot',
    'ezooms' => 'Ezooms',
    'netcraftsurveyagent' => 'Netcraft Survey Agent',
    'purebot' => 'Purebot (http://www.puritysearch.net)',
    'socketcrawler' => 'socketcrawler',
    'siteexplorer' => 'SiteExplorer (http://siteexplorer.info)',
    
    'spider|crawler|search|robot' => 'Unknown search robot',
    
    'skypeuripreview' => 'Microsoft SkypeUriPreview',
    'telegrambot' => 'TelegramBot (like TwitterBot)',
    // Services:
    'domaincrawler' => 'info@domaincrawler.com; http://www.domaincrawler.com',
    'web-monitoring' => 'Web-Monitoring (http://monoid.nic.ru)',
    'validator\.nu|w3c_validator' => 'W3C Validation Services',
    'pingdom' => 'Pingdom.com bot',
    'compatible;\s*synapse' => 'Apache web service for processing XML documents (http://wiki.apache.org/incubator/SynapseProposal)', 
    
    // System clients:
    'curl\/(\d+\.)+\d+' => 'Curl/x.x.x',
    'wget\/(\d+\.)+\d+' => 'Wget/x.x.x',
    'mod_pagespeed' => 'Apache ModPagespeed',
    'cpython\/' => 'CPython',
    'dycurl\/' => 'PycURL (http://pycurl.io/)',
    'drupal\s+\(\+https?\:\/\/drupal.org\/\)' => 'Drupal engine',
  );
}


/**
* Search crawlers detection
* @see utils.module-->>utils_is_robot() 
*/
function _utils_is_robot() {
  if (user_is_logged_in()) {
    return FALSE;
  }
  static $result = NULL;
  if (!isset($result)) {
    $result = FALSE;
    if (variable_get('utils_is_robot_detection', TRUE)) {
      if (empty($_SERVER['HTTP_USER_AGENT']) || !trim($_SERVER['HTTP_USER_AGENT'])) {
        $result = '---unknowh user agent---';
      }
      else {
        $http_user_agent = substr($_SERVER['HTTP_USER_AGENT'], 0, 300);
        foreach (_utils_crawlers_list() as $regexp => $name) {
          if (preg_match('/([^a-z0-9]|^)' . $regexp . '([^a-z0-9]|$)/i', $http_user_agent)) {
            $result = $name;
            break;
          }
        }
      }
    }
  }
  return $result;
}


/**
* Helper function.
* Returns associative array by primary font name, or possible font names (adaptive degradation) for optional passed primary font name.
*/

function _utils_get_web_fonts($font_name = NULL) {
  $fonts = array(
    //Sans Serif
    'Arial, Helvetica, sans-serif',
    '"Arial Black", Gadget, sans-serif',
    '"Lucida Sans Unicode", "Lucida Grande", sans-serif',
    'Impact,Charcoal, sans-serif',
    'Tahoma, Geneva, sans-serif',
    '"Trebuchet MS", Helvetica, sans-serif',
    'Verdana, Geneva, sans-serif',
    '"MS Sans Serif", Geneva, sans-serif',
    
/*
    'Arial, "Helvetica Neue", Helvetica, sans-serif',
    '"Arial Black", "Arial Bold", Gadget, sans-serif',
    '"Arial Narrow", Arial, sans-serif',
    '"Arial Rounded MT", Arial, Helvetica, sans-serif',
    '"Arial Rounded MT Bold", "Helvetica Rounded", Arial, sans-serif',
    '"Avant Garde", Avantgarde, "Century Gothic", CenturyGothic, "AppleGothic", sans-serif',
    'Calibri, Candara, Segoe, "Segoe UI", Optima, Arial, sans-serif',
    'Candara, Calibri, Segoe, "Segoe UI", Optima, Arial, sans-serif',
    '"Century Gothic", CenturyGothic, AppleGothic, sans-serif',
    '"Franklin Gothic Medium", "Franklin Gothic", "ITC Franklin Gothic", Arial, sans-serif',
    'Futura, "Trebuchet MS", Arial, sans-serif',
    'Geneva, Tahoma, Verdana, sans-serif',
    '"Gill Sans", "Gill Sans MT", Calibri, sans-serif',
    '"Microsoft Sans Serif"',
    '"MS Outlook"',
    '"MS Sans Serif", Geneva, sans-serif',
    '"MS Serif", "New York", serif',
    'Helvetica, "HelveticaNeue-Light", "Helvetica Neue Light", "Helvetica Neue", Helvetica, Arial, "Lucida Grande", sans-serif',
    '"Helvetica Neue", Helvetica, Arial, sans-serif',
    'Impact, Haettenschweiler, "Franklin Gothic Bold", Charcoal, "Helvetica Inserat", "Bitstream Vera Sans Bold", "Arial Black", sans serif',
    '"Lucida Grande", "Lucida Sans Unicode", "Lucida Sans", Geneva, Verdana, sans-serif',
    'Optima, Segoe, "Segoe UI", Candara, Calibri, Arial, sans-serif',
    '"Segoe UI", Frutiger, "Frutiger Linotype’, "Dejavu Sans", "Helvetica Neue", Arial, sans-serif',
    'Tahoma, Verdana, Segoe, sans-serif',
    '"Trebuchet MS", "Lucida Grande", "Lucida Sans Unicode", "Lucida Sans", Tahoma, sans-serif',
    'Verdana, Geneva, sans-serif',
    'Algerian, sans-serif',
*/
    
    //Serif
      'Georgia, serif',
    '"Palatino Linotype", "Book Antiqua", Palatino, serif',
    '"Times New Roman", Times, serif',
    '"MS Serif", "New York", serif',
    
/*
    'Baskerville, "Baskerville Old Face", "Hoefler Text", Garamond, "Times New Roman", serif',
    '"Big Caslon", "Book Antiqua", "Palatino Linotype", Georgia, serif',
    '"Bodoni MT", Didot, "Didot LT STD", "Hoefler Text", Garamond, "Times New Roman", serif',
    '"Book Antiqua", Palatino, "Palatino Linotype", "Palatino LT STD", Georgia, serif',
    '"Calisto MT", "Bookman Old Style", Bookman, "Goudy Old Style", Garamond, "Hoefler Text", "Bitstream Charter", Georgia, serif',
    '"Bookman Old Style", "Book Antiqua", serif',
    'Cambria, Georgia, serif',
    'Didot, "Didot LT STD", "Hoefler Text", Garamond, "Times New Roman", serif',
    'Garamond, Baskerville, "Baskerville Old Face", "Hoefler Text", "Times New Roman", serif',
    'Georgia, Times, "Times New Roman", serif',
    'Times, "Times New Roman", serif',
    '"Goudy Old Style", Garamond, "Big Caslon", "Times New Roman", serif',
    '"Hoefler Text", "Baskerville old face", Garamond, "Times New Roman", serif',
    'Lucida, sans-serif',
    '"Lucida Sans Unicode", "Lucida Grande", sans-serif',
    '"Lucida Bright", Georgia, serif',
    'Palatino, "Palatino Linotype", "Palatino LT STD", "Book Antiqua", Georgia, serif',
    '"Palatino Linotype", "Book Antiqua", Palatino, serif',
    'Perpetua, Baskerville, "Big Caslon", "Palatino Linotype", Palatino, "URW Palladio L", "Nimbus Roman No9 L", serif',
    'Rockwell, "Courier Bold", Courier, Georgia, Times, "Times New Roman", serif',
    '"Rockwell Extra Bold", "Rockwell Bold", monospace',
    '"Times New Roman", Times, Baskerville, Georgia, serif',
    'TimesNewRoman, "Times New Roman", Times, Baskerville, Georgia, serif',
*/
    
    
    
    
    // Monospaced
      '"Courier New", Courier, monospace',
    '"Lucida Console", Monaco, monospace',
/*
    'Fixedsys, System, "andale mono", monospace',
    'System, Fixedsys, monospace',
    '"Small Fonts"',
    '"Andale Mono", AndaleMono, monospace',
    'Consolas, monaco, monospace',
    '"Courier New", Courier, "Lucida Sans Typewriter", "Lucida Typewriter", monospace',
    'Courier, "Lucida Sans Typewriter", monospace',
    '"Lucida Console", "Lucida Sans Typewriter", Monaco, "Bitstream Vera Sans Mono", monospace',
    '"Lucida Sans Typewriter", "Lucida Console", Monaco, "Bitstream Vera Sans Mono", monospace',
    'Monaco, Consolas, "Lucida Console", monospace',
*/    
    //Fantasy
    '"Comic Sans MS", cursive',
    //'Copperplate, "Copperplate Gothic Light", fantasy',
    //'Papyrus, fantasy',
    
/*
    'Copperplate, "Copperplate Gothic Light", fantasy',
    'Papyrus, fantasy',
    '"Comic Sans MS", cursive',
*/
    // Script
    //'"Brush Script MT", cursive',

  );
  
  drupal_alter('utils_get_web_fonts', $fonts);
  $result = array();
  foreach ($fonts as $item) {
    if ($item = preg_split('/[\,]\s*/', preg_replace('/\s+/', ' ', $item))) {
      $result[trim($item[0], '\'"')] = implode(',', $item);
    }
  }
  return $font_name ? (isset($result[$font_name]) ? $result[$font_name] : NULL) : $result;
}




function _utils_backup_drupal_database($output_dir, $mysqldump_bin = NULL, $output_filename = NULL, $db_name = NULL, $db_user = NULL, $db_pass = NULL, $db_host = NULL, $db_port = NULL, $gzip = FALSE) {
  $output_dir_original = $output_dir;
  if (
    (!$output_filename || (!is_file($output_dir . DIRECTORY_SEPARATOR . $output_filename) && !is_dir($output_dir . DIRECTORY_SEPARATOR . $output_filename))) 
    && !is_file($output_dir) && (is_dir($output_dir) || (!is_file($output_dir) && mkdir($output_dir, 0775, TRUE)))
    && ($output_dir = drupal_realpath($output_dir)) && is_dir($output_dir) && is_writable($output_dir)) {

    // General options /////////////////
    //executable paths
    $mysqldump_bin = $mysqldump_bin ? $mysqldump_bin : (preg_match('/^Apache.*?\(Win/i', $_SERVER['SERVER_SOFTWARE']) ? '..\\mysql\\bin\\mysqldump' : '/usr/bin/mysqldump');
    $gzip_bin_dir = '';  
    // Find tables and delete data from there before dumping
    $exclude_tables_data_regexp = '^cache($|_)';
    // mysqldump options set
    $mysqldump_opts = array(
      '--skip-comments',
      '--skip-add-locks',
      '--skip-disable-keys',
      '--quick',
      );
    // Database name for dump, for using in dump filename (when $output_filename is not specified) and calling mysqldump
    $db_name = isset($db_name) ? $db_name : (isset($GLOBALS['databases']['default']['default']['database']) ? $GLOBALS['databases']['default']['default']['database'] : '');
    // Database user, password, host and port parameters, for mysqldump
    $db_user = isset($db_user) ? ($db_user ? $db_user : '') : (isset($GLOBALS['databases']['default']['default']['username']) ? $GLOBALS['databases']['default']['default']['username'] : '');
    $db_pass = isset($db_pass) ? ($db_pass ? $db_pass : '') : (isset($GLOBALS['databases']['default']['default']['password']) ? $GLOBALS['databases']['default']['default']['password'] : '');
    $db_host = isset($db_host) ? ($db_host ? $db_host : 'localhost') : (isset($GLOBALS['databases']['default']['default']['host']) ? $GLOBALS['databases']['default']['default']['host'] : 'localhost');
    $db_port = isset($db_port) ? ($db_port ? $db_port : 3306) : (isset($GLOBALS['databases']['default']['default']['port']) ? $GLOBALS['databases']['default']['default']['port'] : 3306);
    
    $mysqldump_opts[] = '--host=' . $db_host;
    $mysqldump_opts[] = '--port=' . $db_port;
    
    // Timestamp, for using in dump filename (when $output_filename not is specified)
    $time_format = 'Y-m-d_H-i-s';
    // Security key, for using in dump filename (when $output_filename is not specified)
    $secure_key = sha1(microtime() . serialize($_COOKIE) . serialize($GLOBALS['user']) . ip_address());
    ////////////////////////////////////

    $t = REQUEST_TIME;
    if (!$output_filename) {
      
      // If file allready found, we increment new file name
      while (is_file($output_dir . DIRECTORY_SEPARATOR . ($output_filename = $db_name . '.' . date($time_format, $t) . ($secure_key ? '.' . $secure_key : '') . '.sql')) || is_dir($output_dir . DIRECTORY_SEPARATOR . $output_filename)) {
        $t++;
      }
    }

    if (!is_file($output_dir . DIRECTORY_SEPARATOR . $output_filename) && !is_dir($output_dir . DIRECTORY_SEPARATOR . $output_filename)) {
      if (0 && $exclude_tables_data_regexp) {
        foreach(db_query('SHOW TABLES WHERE tables_in_' . $db_name . ' REGEXP(:exclude_tables_data_regexp)', array(':exclude_tables_data_regexp' => $exclude_tables_data_regexp))->fetchCol() as $table) {
          db_query('DELETE FROM {' . $table . '}');
        }
      }
      file_create_htaccess($output_dir, TRUE);
      // Prepend stub line for Syphex Dumper compability
      file_put_contents($output_dir . DIRECTORY_SEPARATOR . $output_filename, '#SKD101|' . $db_name . '||' . date('Y.m.d H:i:s', $t) . '|1' . "\n");

      // Run mysqldump
      unset($out);
      drupal_set_time_limit(1800);
      exec(
        $mysqldump_bin . 
        ($db_user ? ' -u' . $db_user : '') . ' ' . 
        ($db_pass ? '-p' . $db_pass : '') . ' ' . 
        implode(' ', $mysqldump_opts) . 
        ' ' . $db_name .
        '>>"' . $output_dir . DIRECTORY_SEPARATOR . $output_filename . '"',
        $out,
        $ret
        );
      
      if (!$ret) {
        if ($gzip) { // Use gzip for syphex dumper compability
          $rp = drupal_realpath($output_dir_original . '/' . $output_filename);
          if ($gz = gzopen($rp . '.gz', 'wb9')) {
            if ($fp = fopen($rp, 'rb')) {
              while (!feof($fp)) {
                $s = fread($fp, 1024 * 512);
                gzwrite($gz, $s);
              }
              fclose($fp);      
            }
            gzclose($gz);
            @unlink($output_dir_original . '/' . $output_filename);
            $output_filename .= '.gz';  
          }          
        }
        return $output_dir_original .'/' . $output_filename;
      } else {
        @unlink($output_dir . DIRECTORY_SEPARATOR . $output_filename);
      }
    }
  }
}



function _utils_zip_files($files, $archive_name, $strip_from_path = DRUPAL_ROOT, $excluded_files_regexp = '') {
  drupal_set_time_limit(1800); // 30 min max
  if (is_string($files)) {
    $files = array($files);
  }
  if (!is_dir(dirname($archive_name))) {
    mkdir(dirname($archive_name), 0755, TRUE);
  }
  $original_output_dir = dirname($archive_name);
  $archive_name = str_replace('\\', '/', drupal_realpath(dirname($archive_name)) . '/' . basename($archive_name));
  
  $strip_from_path = trim(preg_replace(/*remove windows drive letetr (D:)*/'/^[a-z]\:/i', '', str_replace('\\', '/', drupal_realpath($strip_from_path))), '/');
  
  if (is_dir(dirname($archive_name))) {
    $zip = new ZipArchive();
    $files_added = FALSE;
    
    if ($zip->open($archive_name, ZIPARCHIVE::CREATE) === TRUE) {
    
      foreach ($files as $fn) {
        $fn = str_replace('\\', '/', drupal_realpath($fn));
        if (is_dir($fn)) {
          foreach (array_keys(file_scan_directory($fn, '//', array('nomask' => '/\?/'))) as $f) {
            $f = str_replace('\\', '/', drupal_realpath($f));
            $fl = trim(preg_replace('/^[a-z]\:/i', '', $f), '/');
            
            if ($strip_from_path && strpos($fl, $strip_from_path) === 0) {
              $fl = trim(drupal_substr($fl, drupal_strlen($strip_from_path)), '/');
            }
            if ($fl) {
              $files_added = TRUE;
              if (!$excluded_files_regexp || !preg_match($excluded_files_regexp, $f)) {
                $zip->addFile($f, $fl);//$zip->addEmptyDir(dirname($fl));
              } else {
                
              }
            }

          }
        } else {
          $fl = trim(preg_replace('/^[a-z]\:/i', '', $fn), '/');
          if ($strip_from_path && strpos($fl, $strip_from_path) === 0) {
            $fl = trim(drupal_substr($fl, drupal_strlen($strip_from_path)), '/');
          }
          if ($fl) {
            $files_added = TRUE;
            if (!$excluded_files_regexp || !preg_match($excluded_files_regexp, $fn)) {
              $zip->addFile($fn, $fl);//  $zip->addEmptyDir(dirname($fl));
            } else {
              
            }
          }
          
        }
      }
      $zip->close();
      if (!$files_added) {
        @unlink($archive_name);
        return FALSE;
      } else {
        return $original_output_dir . '/' . basename($archive_name);
      }
    }
  }

}



function _utils_get_str_relevancy($s1, $s2) {
  $results = &drupal_static(__FUNCTION__);
  $key = sha1($s1) . sha1($s2);
  if (!isset($results[$key])) {
    $score = $results[$key] = 0;
    
    if ($s1 === $s2) {
      $score += 8;
    } elseif ($s1 == $s2) {
      $score += 7;
    } elseif (($s1 = drupal_strtolower($s1)) == ($s2 = drupal_strtolower($s2))) {
      $score += 6;
    } elseif (($s1 = trim(preg_replace('/[^\pL0-9]/u', ' ', $s1))) == ($s2 = trim(preg_replace('/[^\pL0-9]/u', ' ', $s2)))) {
      $score += 5;
    } elseif (module_exists('transliteration') && ($s1 = transliteration_clean_filename($s1)) == ($s2 = transliteration_clean_filename($s2))) {
      $score += 4;
    } elseif (($s1m = metaphone($s1)) == ($s2m = metaphone($s2))) {
      $score += 3;
      } else {
        similar_text($s1m, $s2m, $p);
        if ($p > 30) {
          $strpos1 = strpos($s2, $s1);
          $strpos2 = strpos($s1, $s2);
          $score += 2 + $p / 1000 + ($strpos1 === 0 ? 0.1 : ($strpos2 === 0) ? 0.09 : 0);  
        } else {
          similar_text($s1m, $s2m, $p);
          if ($p > 30) {
            $strpos1 = strpos($s2m, $s1m);
            $strpos2 = strpos($s1m, $s2m);
            $score += 1 + $p / 1000 + ($strpos1 === 0 ? 0.1 : ($strpos2 === 0) ? 0.09 : 0);            
            } else {
              $l = levenshtein($s1, $s2);
              if ($l / drupal_strlen($s1) < 0.1 && $l / drupal_strlen($s2) < 0.1) {
                $score += $l / (drupal_strlen($s1) + drupal_strlen($s2));
              }            
            }
        }
      }    
    $results[$key] =  $score;
  }
  return $results[$key];
}


function _utils_relevance_search($search_value, $in_array, $return_all_variants) {

  $result = array();
  foreach ($in_array as $key => $v) {
    $score = _utils_get_str_relevancy($v, $search_value) + (is_int($key) ? 0 : (_utils_get_str_relevancy($key, $search_value) / 1000));
    $result[$key] = $score;
  }
  arsort($result, SORT_NUMERIC);
  $result = array_filter($result);
  return $return_all_variants ? $result : key($result);
}
/*array (
  'INQ' => 'äþéì3',
  'CMQ' => 'ñì3',
  'DMQ' => 'äì3',
  'MMQ' => 'ìì3',
  'LTR' => 'ë',
  'FTQ' => 'ôóò3',
  'MTQ' => 'ì3',
  'YDQ' => 'ÿðä3',
  'MLT' => 'ìë',
  'V' => 'U',
  'kV' => 'KU',
  'QAN' => 'ö',
  'SAN' => 'ïîëãîäà',
  'SEC' => 'ñ',
  'MON' => 'ì',
  'HUR' => '÷',
  'DAY' => 'ñóò',
  'WEE' => 'íåä',
  'ANN' => 'óíö',
  'MIN' => 'ìèí',
  'DCE' => '°C',
  'DFH' => '°Ô',
  'DKE' => 'K',
  'km/h' => 'êì/÷',
  'mi/h' => 'óçåë/÷àñ',
  'mph' => 'ìèëè/÷àñ',
  'm/s' => 'ì/ñ',
  'SATM' => 'àòì',
  'BR' => 'á',
  'TRR' => 'ìì ðò. ñò',
  'PSC' => 'Ïà',
  'AHR' => 'À.÷',
  'HPW' => 'ë.ñ.',
  'KCALH' => 'êêàë/÷',
  'KWTT' => 'êÂò',
  'WAM' => 'Â.À',
  'WTT' => 'Âò',
  'px' => 'ïèêñ.',
  'MQH' => 'êóá.ì/÷',
  'NWTM' => 'Í*ì',
  'FPS' => 'êàäðîâ/ñ',
  'CDMQ' => 'êä/ì2',
  'dp' => 'äï',
  'DTN' => 'ö',
  'MGM' => 'ìã',
  'ONC' => 'óíö',
  'KGM' => 'êã',
  'GRM' => 'ã',
  'GLL' => 'ãàë',
  'BRR' => 'áàðë',
  'TNE' => 'ò',
  'FOT' => 'ôóò',
  'NMI' => 'ìèëÿ',
  'INH' => 'â',
  'DMT' => 'äì',
  'MTR' => 'ì',
  'CMT' => 'ñì',
  'YRD' => 'ÿðä',
  'MMT' => 'ìì',
  'UNT' => 'U',
  'KMT' => 'êì',
  'TB' => 'ÒÁ',
  'B' => 'Á',
  'bit' => 'á',
  'kB' => 'êÁ',
  'GB' => 'ÃÁ',
  'MB' => 'ÌÁ',
  'KHZ' => 'êÃö',
  'MHZ' => 'ÌÃö',
  'GHZ' => 'ÃÃö',
  'HZ' => 'Ãö',
  'KGF' => 'êãñ',
  'NWT' => 'Í',
  'KWH' => 'êÂò/÷',
  'KCAL' => 'êêàë',
  'JOL' => 'Äæ',
  'CAL' => 'êàë',
  'A' => 'A',
  'kA' => 'êÀ',
  'mlA' => 'ìÀ',
  'MA' => 'ÌÀ',
  'µA' => 'ìêÀ',
  'TKM' => 'ò.êì',
  'PCE' => 'øò',
  'NPT' => '÷àñòü',
  'NPR' => 'ïàð',
  'SET' => 'íàáîð',
  'NMP' => 'óïàê',
  'BX' => 'ÿù.',
  'NPL' => 'ðóë',
  'HD' => 'ãîë.',
  'MD' => '÷åë.äí',
  'MH' => '÷åë.÷',
  'MIL' => 'òûñ. øò',
  'LEF' => 'ë.',
  'CEN' => '100 øò',
  'NAR' => 'èçä',
  'NBB' => 'áîá.',
  'NCL' => 'ýëåì',
  'BTL' => 'áóò.',
  'Gb/s' => 'Ãá/c',
  'Tb/s' => 'Òá/c',
  'kb/s' => 'êá/c',
  'Mb/s' => 'Ìá/c',
  'b/s' => 'á/c',
  'YDK' => 'ÿðä2',
  'ARE' => 'à',
  'DMK' => 'äì2',
  'FTK' => 'ôóò2',
  'HAR' => 'ãà',
  'INK' => 'äþéì2',
  'CMK' => 'ñì2',
  'KMK' => 'êì2',
  'MMK' => 'ìì2',
  'MTK' => 'ì2',
  'SOT' => 'ñîò',
)*/



