<?php
/**
 Usage: 
 $css = new UtilsCSS('body {margin:0 auto;  }');
 m($css->process()->css); // compressed css
 m($css->process(FALSE)->css); // Clean and formatted css
 
*/


class UtilsCSS {
	
	public $compress = TRUE;
	public $css = '';
	
	private $css_array = array();
	
	public function __construct($css = '') {
		$this->set($css);
	}
	public function __toString() {
		return $this->css;
	}
	function set($css) {
		$this->css = $css;
		$this->css_array = array();
	}

	function process($compress = NULL) {
		$this->compress = isset($compress) ? (bool)$compress : $this->compress;
		return $this->makeArray()->makeCss();
	}

	private function processCSSComment($matches) {
	  static $keep_nextone = FALSE;

	  // Quoted string, keep it.
	  if ($matches[0][0] === "'" || $matches[0][0] === '"') {
			return $matches[0];
	  }
	  // End of IE-Mac hack, keep it.
	  if ($keep_nextone) {
			$keep_nextone = FALSE;
			return $matches[0];
	  }
	  switch (strrpos($matches[0], '\\')) {
			case FALSE:
				// No backslash, strip it.
				return '';

			case drupal_strlen($matches[0]) -3:
				// Ends with \*/ so is a multi line IE-Mac hack, keep the next one also.
				$keep_nextone = TRUE;
				return '/*_\*/';

			default:
				// Single line IE-Mac hack.
				return '/*\_*/';
	  }
	}

	private function makeArray() {
		$this->css_array = array();
		// remove double spaces
		$css = preg_replace('/\s+/', ' ', $this->css); 
		
		$css = preg_replace('/^@charset\s+[\'"](\S*)\b[\'"];/i', '', $css);
		$css = preg_replace_callback("<".'"[^"\\\\]*(?:\\\\.[^"\\\\]*)*"'."|'[^'\\\\]*(?:\\\\.[^'\\\\]*)*'|".'/\*[^*]*\*+(?:[^/*][^*]*\*+)*/'.">Ss", array(__CLASS__, 'processCSSComment') , $css);
		$css = preg_replace('<\s*([@{}:;,]|\)\s|\s\()\s*>S', '\1', $css);
		preg_match_all('/\s*(.+?)\s*\{\s*(.*?)\s*\}/', $css, $matches);
		foreach ($matches[1] as $i => $selector) {
			if (($selector = trim($selector)) && strpos($selector, '@') !== 0) {
				$items = explode(';', $matches[2][$i]);
				$sorted_items = array();
				foreach($items as $idx => $attr) {
					// convert .. url("a/b/c") to url('a/b/c')
					$attr = preg_replace('/url\s*\(\s*"(.*?)"\s*\)/', 'url(\'$1\')', $attr);
					
					if (preg_match('/\!important[\;\s]*$/i', $attr)) {
						$sorted_items['99_' . $idx] = $attr;
					} else {
						$sorted_items['00_' . $idx] = $attr;
					}
				}
				ksort($sorted_items);
			
				foreach($sorted_items as $attr) {
					if ($attr && strlen($attr = trim($attr)) > 0 && count($attr =  explode(':', $attr, 2 /* limit to 2 for keep http:// in property */ )) == 2 && ($attr[0] = trim($attr[0])) && ($attr[1] = trim($attr[1])) !== '') {
						$selector = preg_replace('/\s+/', ' ', $selector);
						$selector = preg_replace('/\s*([\>\:\*\,])\s*/', '$1', $selector); // replace ', ' to ','
						$selector = drupal_map_assoc(explode(',', $selector)); // remove duplicates of selectors
						ksort($selector);
						$selector = trim(implode(',', $selector));
						$this->css_array[$selector][$attr[0]] = $attr[1];
					}
				}
			}
		}
		return $this;
	}

	
/*	@media all and (min-width:641px) and (max-width:1000px) {
		body.one-sidebar.sidebar-first .col-center {
			width:69.88%;
		}
		body.one-sidebar.sidebar-first .col-left {
			width:29.88%;
		}
		
		^\s*@([^\{])*?\{([^\{]*\{[^\}]*\})*?\s*\}
		
	}*/
	
	private function _preg_replace_rgb2hex($m) {
		return $m[1] . '#' . str_pad(dechex($m[2]), 2, '0', STR_PAD_LEFT) . str_pad(dechex($m[3]), 2, '0', STR_PAD_LEFT) . str_pad(dechex($m[4]), 2, '0', STR_PAD_LEFT);
	}
	
	private function makeCss() {
		$this->css = '';
		foreach ($this->css_array as $selector => $items) {
				if ($selector = trim(preg_replace('/\,\s*/', ',' . ($this->compress ? '' : ' '), $selector))) {
					$optimized_items = array();
					if ($items) {
						foreach ($items as $prop => $val) {
							if ($prop = trim($prop)) {
								if ($this->compress) {
									$prop = strtolower(preg_replace('/\s+/', ' ', trim($prop)));
									$val = preg_replace('/\s+/', ' ', trim($val)); // remove double spaces
									if (!preg_match('/url\s*\(/', $val)) { // Skip next optimizations of values cotains urls
										$val = trim(preg_replace('/(^|\s)(0)(px|pt|em|%)(\s|$)/i', '$1$2$4', $val)); // convert '0px' to '0'
										$val = trim(preg_replace('/(^|\s)(0+)?\.(0)+(\s|$)/', '$1$3$4', $val)); // replace '0.0' to '0'
										$val = trim(preg_replace('/(^|\s)0\./', '$1.', $val)); // convert '.. 0.5 ..' to '.. .5 ..'
										$val = trim(preg_replace('/^([\-]*[0-9\.]+[a-z%]*\s+)\1\1\1$/i', '$1', $val. ' ')); // convert '3px 3px 3px 3px' to '3px'
										$val = trim(preg_replace('/^([\-]*[0-9\.]+[a-z%]*\s+)\1$/i', '$1', $val. ' ')); // convert '3px 3px' to '3px'
										$val = trim(preg_replace('/([^\w]rgb)a\s*(\()\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*,\s*(?:0+|0+?\.0+|\.0+)?\s*(\))/i', '$1$2$3,$4,$5$6', $val)); // convert non transparent rgba (11, 12, 13, 0.0000) to rgb(11,12,13)
										
									//	$val = preg_replace('/(^|\s)rgb\s*\(\s*(\d{1,3})\s*,\s*(\d{1,3})\s*,\s*(\d{1,3})\s*\)/ie', "'$1#' . utils_rgb_to_hex(array('$2', '$3', '$4',))", $val); // Convert rgb(11, 22, 33) color to #ABCDEF
										
										$val = preg_replace_callback('/(^|\s)rgb\s*\(\s*(\d{1,3})\s*,\s*(\d{1,3})\s*,\s*(\d{1,3})\s*\)/i', array(__CLASS__, '_preg_replace_rgb2hex'), $val); // Convert rgb(11, 22, 33) color to #ABCDEF
										
										// Optimize HTML colors ('#FF66DD -> #F6D)
										$val = preg_replace('/(^|\s)\#([a-f0-9])\2([a-f0-9])\3([a-f0-9])\4(\s|$|!)/i', '$1#$2$3$4$5', $val);
										// Convert color names to hex values
										foreach (array('black' => '000', 'fuchsia' => 'f0f', 'yellow' => 'ff0', 'white' => 'fff') as $from => $to) {
											$val = preg_replace('/(^|\s)' . $from . '(\s|$|!)/i', '$1#' . $to . '$2', $val);
										}
										// the colors that are shorter than the hex representation of them
										foreach (array('808080' => 'gray', '008000' => 'green', '000080' => 'navy', '808000' => 'olive', '008080' => 'teal') as $from => $to) {
											$val = preg_replace('/(^|\s)#' . $from . '(\s|$|!)/', '$1' . $to . '$2', $val);
										}
									}
									
									// compress 'bold' to '700', and others...
									if ($prop === 'font-weight' || $prop === 'font') {
										$val = ($val === 'bold') ? '700' : ($val === 'normal' ? '400' : ($val === 'lighter' ? '200' : $val));
									}
								}
								$optimized_items[$prop] = ($this->compress ? $prop . ':' : "" . $prop . ': ') . $val;					
							}
						}
						// TODO: replace {margin-left:1px; ... margin:0;} to {..margin:0;}
					}
					if ($optimized_items) {
						$this->css .= $selector . ($this->compress ? '{' . implode(';', $optimized_items) . '}' : " {\n\t" . implode(";\n\t", $optimized_items) . ";\n}\n\n");
					}
				}
		}
		return $this;
	}
}


