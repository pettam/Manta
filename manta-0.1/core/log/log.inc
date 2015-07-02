<?php

define('PATH_LOG', implode('/', array_slice(explode('/', PATH_WEBROOT), 0, -1)) . '/log');

/**
 * Logger.
 * 
 * Log to filesystem and/or database.
 * 
 * @version 0.1
 * @todo Database log
 */
class log extends singleton {
    
    protected static function _construct() {
        if (self::config()->get('file')) {
            // Create log directory if absent.
            if (!file_exists(PATH_LOG)) {
                mkdir(PATH_LOG, 0755, true);
            }
        }
    }

    /**
     * Write to framework log.
     * 
     * @param string $file
     * @param string $line
     * @param mixed $message
     * @param mixed $error
     * @param string $logname
     */
    public static function write($file, $line, $message, $error, $logname = null) {
        !$logname && $logname = 'common';
        $file = str_replace(PATH_ROOT, '', $file);
        $message = json(array(time(), $file, $line, $message, $error)) . "\n";
        if (self::config()->get('file')) {
            file_put_contents(PATH_LOG . "/$logname.log", $message, FILE_APPEND);
        }
        if (self::config()->get('database')) {
            // Store in database ...
        }
        return $file;
    }

    /**
     * Read from framework log.
     * 
     * @param string $file
     * @param string $line
     * @param string $message
     * @param string $log
     */
    public static function read($file, $line, $message, $log = 'common') {
        
    }

}

log::getInstance();
