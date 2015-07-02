<?php
define('PATH_WEBROOT', getcwd());
define('PATH_FRAMEWORK', dirname(__FILE__));
define('PATH_ROOT', implode('/', array_slice(explode('/', PATH_WEBROOT), 0, -1)));

// Load core modules
include('core/utilities/utilities.inc');
include('core/singleton/singleton.inc');
include('core/data/data.inc');

/**
 * Manta framework.
 * 
 * @version 0.1
 * @author Mathias Petersson <post@mathiaspetersson.se>
 * @todo Lots and lots of stuff...
 */
class manta extends singleton {
    
    public static
            /** @var string */
            $namespace;
    
    protected static function _construct() {
        self::$namespace = strtolower(preg_replace('/[^a-z0-9\-]/i', '_', self::config()->get('namespace', self::config()->get('site_name'))));
    }

    protected static function _destruct() {
        
    }
    
    /**
     * Error.
     * 
     * @param string $message
     * @param mixed $error
     * @param integer $line
     * @param string $file
     * @param boolean $exit
     */
    public static function error($message, $error, $exit = KILL, array $backtrace = null) {
        if (is_array($error) || is_object($error)) {
            $error = json($error);
        }
        !$backtrace && $backtrace = debug_backtrace(false);
        $file = arr('file', $backtrace[0], __FILE__);
        $line = arr('line', $backtrace[0], __LINE__);
        $logname = isset($backtrace[1]) ? arr('class', $backtrace) : null;
        log::write($file, $line, $message, $error, $logname);
        dpr($message, $exit, false, null, $backtrace);
    }
    
    /**
     * Log.
     * 
     * Uses name of caller class for logname. If no class name is found,
     * common.log will be used.
     * 
     * @param mixed $message
     * @param array $backtrace
     */
    public static function log($message, array $backtrace = null) {
        !$backtrace && $backtrace = debug_backtrace(false);
        $file = arr('file', $backtrace[0], __FILE__);
        $line = arr('line', $backtrace[0], __LINE__);
        $logname = isset($backtrace[1]) ? arr('class', $backtrace) : null;
        log::write($file, $line, $message, null, $logname);
    }
    
    /**
     * Message.
     * 
     * @param string $message
     */
    public static function message($message, array $backtrace = null, $exit = false) {
        !$backtrace && $backtrace = debug_backtrace(false);
        dpr($message, $exit, false, null, $backtrace);
    }

    /**
     * Bootstrap.
     */
    public static function bootstrap() {
        self::process();
        self::render();
    }

    /**
     * Process request.
     */
    public static function process() {
        page::theme(self::$config['theme_name']);
        page::request(page::$uri);
        page::build();
        page::finalize();
    }

    /**
     * Render request.
     */
    public static function render() {
        page::render();
    }

}

manta::getInstance();

// Load even more core modules
include('core/log/log.inc');
include('core/database/database.inc');
include('core/session/session.inc');
include('core/modules/modules.inc');