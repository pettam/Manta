<?php

/**
 * Module install.
 * 
 * @version 0.1
 */
abstract class module_install extends singleton {
    
    /** @var string */
    protected static $module_name;

    protected function _construct() {
        self::$module_name = preg_replace(
            array('/^_/', '/_install$/'),
            array('', ''),
            __CLASS__);
        if (!$this->check()) {
            $this->install();
        }
    }

    abstract protected function check();
    
    abstract protected function install();

}
