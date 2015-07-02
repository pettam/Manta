<?php

/**
 * Cookie handler.
 * 
 * @version 0.1
 */
class cookie extends singleton {
    
    public static
            /** @var boolean */
            $httpOnly;
    
    public static
            /** @var string */
            $domain;

    public static function _construct() {
        self::$httpOnly = self::config()->get('httponly', false);
        self::$domain = arr('HTTP_HOST', $_SERVER, arr('SERVER_NAME', $_SERVER));
        if (!self::config()->get('subdomain')) {
            self::$domain = array_reverse(explode('.', self::$domain));
            array_pop(self::$domain);
            self::$domain = '.' . join('.', array_reverse(self::$domain));
        }
    }

    /**
     * Delete cookie.
     * 
     * @param string $name
     */
    public static function delete($name = null) {
        if (($cookies = arr('HTTP_COOKIE', $_SERVER)) !== null) {
            // Cookies like session and google analytics should not be touched.
            $exceptions = array(session_name(), '__utma', '__utmz', '__utmb', '__utmc', '__utmv');
            $cookies = explode(';', $cookies);
            foreach ($cookies as $cookie) {
                $parts = explode('=', $cookie);
                $parts[0] = trim($parts[0]);
                if (!in_array($parts[0], $exceptions) && (!$name || $name == $parts[0])) {
                    setcookie($parts[0], '', time() - 1000);
                    setcookie($parts[0], '', time() - 1000, '/');
                    setcookie($parts[0], '', time() - 1000, '/', self::$domain, manta::config()->get('secure'), self::$httpOnly);
                    if (isset($_COOKIE[$parts[0]])) {
                        unset($_COOKIE[$parts[0]]);
                    }
                }
            }
        }
    }
    
    /**
     * Clear cookies.
     */
    public static function clear() {
        self::delete();
    }

    /**
     * Get cookie.
     * 
     * @param string $name
     * @param mixed $default
     * @return mixed
     */
    public static function get($name, $default = null) {
        $cookie = arr($name, $_COOKIE, $default);
        return $cookie === $default ? $cookie : json_decode($cookie, true);
    }

    /**
     * Set cookie.
     * 
     * @param string $name
     * @param mixed $value
     * @param integer $days
     * @return boolean
     */
    public static function set($name, $value, $days = 0) {
        $cookie = json($value);
        $days = $days === 0 ? $days : time() + (60 * 60 * 24 * $days);
        setcookie($name, $cookie, $days, '/', self::$domain, manta::config()->get('secure'), self::$httpOnly);
        $_COOKIE[$name] = $value;
        return true;
    }

    /**
     * Prints complete storage.
     * 
     * @return array
     */
    public static function dump() {
        $backtrace = debug_backtrace(false);
        manta::message($_COOKIE, $backtrace);
    }

}

cookie::getInstance();
