<?php


function _utils_m($func_get_args) {
	if (arg(0) . '/' . arg(1) . '/' . arg(2) === 'js/admin_menu/cache') { //skip admin menu ajax queries
		return;
	}
	static $supress = FALSE;
	if (!$supress) {
		$supress = TRUE;
		$microtime = preg_replace('/^0\.(\d+)\s(\d+)/', '$2.$1', microtime());
		$debug = debug_backtrace();
		// check it's database query (m() called from q())
		if ($is_q = ($debug[2]['function'] === 'q')) {
			unset($debug[0]);
			$debug = array_values($debug);
		}
		static $msg_idx = 0;
		$msg_idx++;
		$debug_trace = array();
		$i = 0;

		foreach (array_reverse($debug) as $item) {
			if (count($debug_trace) == count($debug) - 2) {
				break;
			}
			$data = array();
			foreach (array('class', 'function', 'line', 'file',) as $key) {
				if (!empty($item[$key])) {
					$data[] = 
          ($key === 'function' && !empty($item['type']) ? $item['type'] : '') .
					 '<strong>' . $key . '</strong> <em>'.
					 $item[$key] .
					 (
             $key === 'function' 
                ? 
              '(' .
              implode(', ', array_map(
                function($a) {
                  return isset($a) ? (empty($a) ? 'empty ' : ''). '<strong title="'. check_plain(drupal_substr(@var_export($a, TRUE), 0, 1000)) . '">' . gettype($a) . '</strong>' : 'NULL';
                },
                !empty($item['args']) ? $item['args'] : array())) . 
             ')' 
                : 
             ''
           ) .
					 '</em>';
				}
			}
			$debug_trace[] = str_repeat('&nbsp;&nbsp;&nbsp;', $i) . (++$i - 1 ? '&#8627;&nbsp;' : '') . ' ' . implode(', ', $data);
		}
		$debug_trace = implode('<br />', $debug_trace);

		foreach ($func_get_args as $idx => $arg) {
      $nesting_found = FALSE;
			if (!is_scalar($arg)) {
				$var = print_r($arg, 1);
				if (!($nesting_found = preg_match('/\n\s*\*RECURSION\*\n/', $var))) { // If not contains "Nesting level too deep - recursive dependency?" error, then use var_export
					$var = var_export($arg, 1);
				}
			}
      elseif (is_float($arg)) {
        $var = stripos($arg.'', 'E') !== FALSE ? $arg . ' ('.preg_replace('/\.(0+|)$/', '', number_format($arg, 16, '.', '')).')' : $arg;
      }
      else {
        $var = !$is_q ? var_export($arg, 1) : $arg;
      }

		$lines = count(explode("\n", $var)) + 3;
		drupal_set_message(
        (
        !$idx
          ?
        ($msg_idx == 1 ? 'URI: ' . check_plain($_SERVER['REQUEST_URI']) : '') . '<h3 style="color:#0E1DA3;font-size:1.1em;padding:.1em;margin:0;">-- Dump #' . $msg_idx . '--</h3>
        <div style="font-size:.9em;" class="desciption">Called from: '.
        (!empty($debug[2]['function']) ? ' <strong><em>'.$debug[2]['function'].'() {...}</em></strong> function ' : '').
        'at line '.$debug[1]['line'].' in file <em>'.$debug[1]['file'].'</em>.<br />Microtime: ' . $microtime . '</div>'
          :
        ''
        ) .
        (count($func_get_args) > 1 ? '<label style="font-size:.9em;">var'. ($idx + 1) . ':</label>' : '') .
        ($nesting_found ? '<small style="color:#933">Notice: <em><strong>Nesting level too deep</strong></em> problem found, references was replaced to "*RECURSION*(reference id=..)".</small>' : '' ).
        '<textarea onkeypress="
          switch (event.keyCode) {
            case 119: // toggle wordwrap on W key is pressed
              jQuery(this).attr(\'wrap\', jQuery(this).attr(\'wrap\') === \'off\' ? \'on\' : \'off\')
            break;
          }
          " readonly="readonly" wrap="off" ' .
        'style="' . 
        'height:' . ($lines > 50 ? 50 : ($lines > 1 ? $lines : 3)) . 'em; ' . 
        'border:1px solid #4F4; ' . 
        'margin:0 auto; padding:.5em; width:96%;' . 
        'font-size:11px;font-family:\'Courier New\', \'Console\', monospace;' . 
        'background:#500;color:#DD6;">' .
				//check_plain($var)
				htmlspecialchars($var, ENT_QUOTES, 'cp1251') .
				'</textarea>' .
				(
        $idx == count($func_get_args) - 1 && $debug_trace 
          ? 
        '<div style="width:96.5%;margin-bottom:2em;padding:.3em 0 .3em .5em;background:#EEEEAA;font-size:.9em;"><strong>Call trace:</strong><br/>' . $debug_trace . '</div>'
          : 
        ''
        )
				,
				'debug" style="font-size:13px;background:#FFFFBB;border:2px solid #F00;padding:.1em;margin:0 auto;'/*<<hack*/);
		}
		$supress = FALSE;
	}

  return end($func_get_args); // return last variable for usability

}
