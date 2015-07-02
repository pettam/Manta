<?php

/**
 * Singleton initiator.
 * 
 * @version 0.1
 */
abstract class singleton {

    private static $_instances = array();
    
    /** @var array */
    private static $config = array();

    public function __destruct() {
        // Call destructor method, if any
        if (method_exists($this, '_destruct')) {
            call_user_func(array($this, '_destruct'));
        }
    }

    protected function __construct($options) {
        // Load module config, if any
        $reflection = new ReflectionClass($this);
        $ini_path = dirname($reflection->getFileName())
                . DIRECTORY_SEPARATOR
                . $reflection->name
                . '.ini';
        if (file_exists($ini_path)) {
            self::$config[get_called_class()] = new data(parse_ini_file($ini_path, true));
        }
        
        // Call constructor method, if any
        if (method_exists($this, '_construct')) {
            call_user_func(array($this, '_construct'), $options);
        }
    }
    
    /**
     * @return data
     */
    public static function config() {
        return self::$config[get_called_class()];
    }

    protected function __clone() {
        
    }

    public static function getInstance($options = array()) {
        $class = get_called_class();
        if (!isset(self::$_instances[$class])) {
            self::$_instances[$class] = new $class($options);
        }
        return self::$_instances[$class];
    }
    
    public static function debug($variable, $return = false) {
        if (isset(self::$config[get_called_class()])
                && self::$config[get_called_class()]->get('debug', false)) {
            $backtrace = debug_backtrace(false);
            $return = dpr($variable, false, $return, null, $backtrace);
            if ($return) {
                return $return;
            }
        }
    }

}
