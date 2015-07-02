<?php
// TODO: Better comments for utilities.inc

// TODO: Create debug module? environment script?
if (true) {
    error_reporting(-1);
    ini_set('display_errors', true);
    ini_set('display_startup_errors', true);
}
ini_set('magic_quotes_runtime', '0');
ini_set('session.use_cookies', '1');
ini_set('session.use_only_cookies', '1');
ini_set('session.use_trans_sid', '0');
ini_set('session.cookie_httponly', '1');

// TODO: Localization module
setlocale(LC_ALL, 'C');
date_default_timezone_set('Europe/Stockholm');

/**
 * Debug variable through print_r().
 * 
 * @param mixed $var
 * @param bool|int $die Discard buffer if $die equals 2
 * @param bool $return
 * @param string $key
 * @return string 
 */
define('KILL', 1);
define('KILL_CLEAN', 2);
function dpr($var, $die = false, $return = false, $key = null, $backtrace = null) {
    if ($key && !isset($_GET[$key])) {
        return;
    }
    $backtrace = $backtrace !== null ? $backtrace : debug_backtrace(false);
    $caller = array_shift($backtrace);
    $trace_box = '';
    foreach ($backtrace as $index => $trace) {
        if (arr('file', $trace) || arr('line', $trace)) {
            $trace_box .= sprintf('<div>&#8627; %s:%s</div>', str_replace(implode('/', array_slice(explode('/', PATH_FRAMEWORK), 0, -1)), '', arr('file', $trace, '')), arr('line', $trace));
        }
    }
    $caller_info = sprintf('<div style="color:#36c;font-size:1em;border-bottom:1px solid #ddd;margin-bottom:3px;padding-bottom:3px;"><div style="cursor:pointer" onclick="(this.nextSibling.style.display=this.nextSibling.style.display==\'none\'?\'\':\'none\')">%s:%s</div><div style="display:none;color:#3c6;font-size:1em;line-height:1;">%s</div></div>', str_replace(implode('/', array_slice(explode('/', PATH_FRAMEWORK), 0, -1)), '', arr('file', $caller, '')), arr('line', $caller), $trace_box);
    $dump = sprintf('<pre style="border:1px solid #ccc;background-color:#f0f0f0;padding:7px;">%s%s</pre>', $caller_info, print_r($var, true));
    if ($die) {
        if ($die === KILL_CLEAN && ob_get_length()) {
            ob_end_clean();
        }
        exit($dump);
    }
    if ($return) {
        return $dump;
    }
    print $dump;
}

/**
 * Debug variable through var_dump().
 * 
 * @param mixed $var
 * @param bool|int $die Discard buffer if $die equals 2
 * @param bool $return
 * @return string 
 */
function xxxxxxdvd($var, $die = false, $return = false) {
    $trace = debug_backtrace(false);
    $caller = array_shift($trace);
    $caller_info = sprintf('<div style="color:#36c;font-size:0.9em;">> %s:%s</div>', substr($caller['file'], strrpos($caller['file'], '/') + 1), $caller['line']);

    ob_start();
    var_dump($var);
    $dump = ob_get_clean();
    $dump = '<pre style="border:1px solid #ccc; background-color:#f0f0f0; padding:7px;">' . $caller_info . $dump . '</pre>';
    if ($die) {
        if ($die === 2 && ob_get_length()) {
            ob_end_clean();
        }
        exit($dump);
    }
    if ($return) {
        return $dump;
    }
    print $dump;
}

/**
 * Return a parsed uri component.
 * 
 * array(
 *   uri => string,
 *   path => array,
 *   query => array
 * )
 * 
 * @param string $uri
 * @return array
 */
function parse_uri($uri) {
    $arr = parse_url($uri);
    $arr['uri'] = $uri;
    $arr['path'] = explode('/', trim($arr['path'], '/'));
    $arr['query'] = isset($arr['query']) ? explode('&', $arr['query']) : array();
    $arr['path'] = (count($arr['path']) == 1 && $arr['path'][0] == '') ? array() : $arr['path'];
    return $arr;
}

/**
 * Redirect request.
 * 
 * Codes:
 * 301: Moved Permanently
 * 302: Found
 * 303: See Other (use after POST)
 * 304: Not Modified
 * 305: Use Proxy
 * 307: Temporary Redirect
 * 
 * @param string $uri
 * @param int $code 
 */
function redirect($uri, $status_code = 302) {
    send_header($status_code, true);
    // Again, why are we doing this? :)
    session_write_close();
    session_regenerate_id(true);
    header('Cache-Control: no-store, no-cache, must-revalidate, post-check=0, pre-check=0');
    header('Location: ' . $uri);
    exit(0);
}

/**
 * Send header.
 * 
 * @param int $status_code
 */
function send_header($status_code, $check_sent = false) {
    $status_codes = array (
        100 => 'Continue',
        101 => 'Switching Protocols',
        102 => 'Processing',
        200 => 'OK',
        201 => 'Created',
        202 => 'Accepted',
        203 => 'Non-Authoritative Information',
        204 => 'No Content',
        205 => 'Reset Content',
        206 => 'Partial Content',
        207 => 'Multi-Status',
        300 => 'Multiple Choices',
        301 => 'Moved Permanently',
        302 => 'Found',
        303 => 'See Other',
        304 => 'Not Modified',
        305 => 'Use Proxy',
        307 => 'Temporary Redirect',
        400 => 'Bad Request',
        401 => 'Unauthorized',
        402 => 'Payment Required',
        403 => 'Forbidden',
        404 => 'Not Found',
        405 => 'Method Not Allowed',
        406 => 'Not Acceptable',
        407 => 'Proxy Authentication Required',
        408 => 'Request Timeout',
        409 => 'Conflict',
        410 => 'Gone',
        411 => 'Length Required',
        412 => 'Precondition Failed',
        413 => 'Request Entity Too Large',
        414 => 'Request-URI Too Long',
        415 => 'Unsupported Media Type',
        416 => 'Requested Range Not Satisfiable',
        417 => 'Expectation Failed',
        422 => 'Unprocessable Entity',
        423 => 'Locked',
        424 => 'Failed Dependency',
        426 => 'Upgrade Required',
        500 => 'Internal Server Error',
        501 => 'Not Implemented',
        502 => 'Bad Gateway',
        503 => 'Service Unavailable',
        504 => 'Gateway Timeout',
        505 => 'HTTP Version Not Supported',
        506 => 'Variant Also Negotiates',
        507 => 'Insufficient Storage',
        509 => 'Bandwidth Limit Exceeded',
        510 => 'Not Extended',
    );
    if (isset($status_codes[$status_code])) {
        $status_string = "$status_code {$status_codes[$status_code]}";
        if ($check_sent && headers_sent()) {
            manta::error('Headers can not be modified, output already started.', null, KILL);
        }
        header("{$_SERVER['SERVER_PROTOCOL']} $status_string", true, $status_code);
    }
}

/**
 * usort() function to sort by weight.
 * 
 * @param array $a
 * @param array $b
 */
function xxxxxxxxxsort_object_weight($a, $b) {
    if ($a['weight'] == $b['weight']) {
        return 0;
    }
    return ($a['weight'] < $b['weight']) ? -1 : 1;
}

/**
 * Get microtime.
 * 
 * @param int $e Precision
 * @return float
 */
function getmicrotime($e = 7) {
    list($a, $b) = explode(' ', microtime());
    return bcadd($a, $b, $e);
}

/**
 * Byte formatting.
 * 
 * @param int $bytes
 * @param string $unit
 * @param int $precision
 * @return string
 */
function byte_format($bytes, $unit = '', $precision = 2) {
    $units = array(
      'B' => 0,
      'KB' => 1,
      'MB' => 2,
      'GB' => 3,
      'TB' => 4,
      'PB' => 5,
      'EB' => 6,
      'ZB' => 7,
      'YB' => 8);

    $value = 0;
    if ($bytes > 0) {
        if (!array_key_exists($unit, $units)) {
            $pow = floor(log($bytes) / log(1024));
            $unit = array_search($pow, $units);
        }
        $value = ($bytes / pow(1024, floor($units[$unit])));
    }
    if (!is_numeric($precision) || $precision < 0) {
        $precision = 2;
    }

    return sprintf('%.' . $precision . 'f ' . $unit, $value);
}

/**
 * Print a link.
 * 
 * Returns current uri if $link is null.
 * 
 * @param string $link
 * @uses i18n
 * @uses framework
 * @return string
 */
function xxxxxxxxxxxxl($link = null) {
    $lang = i18n::$language['langcode'];
//  $link = ($link === null) ? framework::$uri['uri'] : "/$lang/" . ltrim($link, '/');
    $link = ($link === null) ? framework::$uri['uri'] : '/' . ltrim($link, '/');

    return $link;
}

/**
 * Check if request is a XMLHttpRequest.
 * 
 * @return bool
 */
function xml_http_request() {
    return arr('HTTP_X_REQUESTED_WITH', $_SERVER, false) == 'XMLHttpRequest';
}

/**
 * Get a random floating number.
 * 
 * @param int $min
 * @param int $max
 * @return float
 */
function mt_randfloat($min = 0, $max = 1) {
    return $min + mt_rand() / mt_getrandmax() * ($max - $min);
}

/**
 * Generate a random identifier.
 * 
 * @param string $prefix
 * @param int $length
 * @return string
 */
function uniqueid($prefix = '', $length = 32) {
    $id = chr(floor((mt_randfloat() * 25) + 65));
    do {
        if (($code = floor((mt_randfloat() * 42) + 48)) && ($code < 58 || $code > 64)) {
            $id .= chr($code);
        }
    } while (mb_strlen($id) < $length);
    return $prefix . $id;
}

/**
 * Ends and cleans every output buffer.
 */
function ob_clear() {
    while (ob_get_status()) {
        ob_end_clean();
    }
}

/**
 * Fix for PHP<5.4 to enable unicode characters in json_encode.
 * 
 * @param mixed $var
 * @return string
 */
function json_encode_php53($var) {
    $json = preg_replace_callback('/\\\\u([0-9a-f]{4})/i', function($matches) {
            return mb_convert_encoding(pack('H*', $matches[1]), 'UTF-8', 'UTF-16');
        }, json_encode($var, JSON_NUMERIC_CHECK));
    return $json;
}

/**
 * Json encode a variable.
 * 
 * @param mixed $var
 * @return string
 */
function json($var) {
//  return json_encode($var, JSON_UNESCAPED_UNICODE | JSON_UNESCAPED_SLASHES | JSON_NUMERIC_CHECK);
    return json_encode_php53($var);
}

/**
 * Generate a remote procedure call response.
 * 
 * 
 * @param mixed $response
 * @param int $result JSON status begins with JSON_STATUS_
 * @param array $keys
 * @param int $err_code
 * @param array $err_data
 * @link http://www.jsonrpc.org/specification
 * @uses framework
 */
function xxxxxxxxxxxxxxxjsonrpc($response, $result, $keys = array(), $err_code = -1, $err_data = null) {
    // Remove output buffer if any
    ob_clear();

    header('Expires: ' . gmdate('r', 0));
    header('Content-type: application/json');
    $jsonrpc = array(
      'jsonrpc' => '2.0',
      'id' => framework::$rpc['id'],
    );
    switch ($result) {
        case JSON_STATUS_PARTIAL:
        case JSON_STATUS_SUCCESS:
            $keys = array_flip($keys);
            $jsonrpc['result'] = array($keys, $response);
            break;
        case JSON_STATUS_ERROR:
            $jsonrpc['error'] = array(
              'code' => $err_code,
              'message' => $response,
            );
            if ($err_data !== null) {
                $jsonrpc['error']['data'] = $err_data;
            }
            break;
    }
    print json($jsonrpc);
    exit;
}

/**
 * Check if current request belongs to specific form.
 * 
 * @param string $key
 * @return bool
 */
function xxxxxxxxxxxxxxis_form($key) {
    return (arr('REQUEST_METHOD', $_SERVER) == 'POST' && (
        // Standard upload
        (arr('key', $_REQUEST) == $key) ||
        // Plupload
        (arr('HTTP_LISTA_KEY', $_SERVER) == $key)
        ));
}

/**
 * Check for API request.
 * 
 * @param string $key
 * @uses framework
 * @return array
 */
function xxxxxxxxxxxxapi($key) {
    if (arr('SCRIPT_NAME', $_SERVER) != '/api.php' || strpos(arr('CONTENT_TYPE', $_SERVER, ''), 'application/json') === false) {
        return false;
    } else {
        if (framework::$rpc === null) {
            framework::$rpc = array_intersect_key(
                json_decode(file_get_contents('php://input'), true), array_flip(array(
              'jsonrpc', 'method', 'id', 'params'
            )));

            if (isset(framework::$rpc['params']) && count(framework::$rpc['params'])) {
                $params = array();
                foreach (framework::$rpc['params'] as $name => $value) {
                    // Break parts
                    $matches = array();
                    if (preg_match_all('/^[^\[]*?|(?:[^\[\]]+?)/Ui', $name, $matches)) {
                        $match = $matches[0];
                        if (!isset($params[$match[0]])) {
                            $params[$match[0]] = array();
                        }
                        $id = count($match) > 1 && is_numeric($match[1]) ? $match[1] : 'n/a';
                        if (!isset($params[$match[0]][$id])) {
                            $params[$match[0]][$id] = array();
                        }
                        $params[$match[0]][$id]['id'] = $id;
                        $params[$match[0]][$id]['key'] = $match[0];

                        /* Examples:
                         * field       = value
                         * field[part] = value
                         * field[id]   = value
                         */
                        if (count($match) <= 2) {
                            if (count($match) === 1 || is_numeric($match[1])) {
                                $params[$match[0]][$id]['value'] = $value;
                            } else {
                                if (!isset($params[$match[0]][$id]['parts'])) {
                                    $params[$match[0]][$id]['parts'] = array();
                                }
                                $params[$match[0]][$id]['parts'][$match[1]] = $value;
                            }
                        }

                        /* Examples:
                         * field[id][part] = value
                         */ else {
                            $params[$match[0]][$match[1]]['parts'][$match[2]] = $value;
                        }
                    }
                }
                framework::$rpc['params'] = $params;
            }
        }

        if (isset(framework::$rpc['id']) && framework::$rpc['id'] === $key) {
            return true;
        } else {
            return false;
        }
    }
}

/**
 * Check value.
 * 
 * Verifies $var, with data from $check.
 * 
 * @param mixed $var
 * @param mixed $check
 * @param mixed $default
 * @return mixed
 */
function check($var, $check, $default = null) {
    // if $var is an array
    if (is_array($var)) {
        if (is_array($check)) {
            // verify if key of $check exists in $var
            // and that value of $var is valid by $check
            $key = key($check);
            if (isset($var[$key]) && in_array($var[$key], $check[$key])) {
                return $var[$key];
            }
        } else {
            // verify if $check exists in $var
            if (isset($var[$check])) {
                return $var[$check];
            }
        }
    }

    // if $check is an array
    elseif (is_array($check)) {
        // verify if value of $var is valid by $check
        if (in_array($var, $check)) {
            return $var;
        }
    }

    // neither $var nor $check is an array,
    // compare $var and $check
    elseif ($var === $check) {
        return $var;
    }

    return $default;
}

/**
 * Get parameter from $_POST.
 * 
 * @param string $key
 * @param mixed $default
 * @return mixed
 */
function form($key, $default = null) {
    return arr($key, $_POST, $default);
}

/**
 * Get parameter from $_GET.
 * 
 * @param string $key
 * @param mixed $default
 * @return mixed
 */
function get($key, $default = null) {
    return arr($key, $_GET, $default);
}

/**
 * Get visitor's IP number.
 * 
 * @return string
 */
function ip() {
    return arr('REMOTE_ADDR', $_SERVER, 'n/a');
}

/**
 * Get visitor's user agent.
 * 
 * @return string
 */
function ua() {
    return arr('HTTP_USER_AGENT', $_SERVER, 'n/a');
}

/**
 * Get parameter from $array.
 * 
 * @param string $key
 * @param array $array
 * @param mixed $default
 * @return mixed
 */
function arr($key, $array, $default = null) {
    return isset($array[$key]) ? $array[$key] : $default;
}

/**
 * Build a path.
 * 
 * @param array $parts
 */
function path(array $parts, $prefix = DIRECTORY_SEPARATOR, $suffix = DIRECTORY_SEPARATOR) {
    return $prefix . join(DIRECTORY_SEPARATOR, $parts) . $suffix;
}

/**
 * Extended standard class.
 * 
 * Overloading methods and properties.
 */
class stdClassExt extends stdClass {
    public function __call($method, $arguments) {
        $return = null;
        if (method_exists($this, $method)) {
            $return = call_user_func_array(array($this, $method), $arguments);
        }
        return $return;
    }
    public function __get($name) {
        $return = null;
        if (array_key_exists($name, $this)) {
            $return = $this->$name;
        }
        return $return;
    }
}