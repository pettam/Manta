<?php

/**
 * Menu.
 * 
 * @version 0.1
 * @todo Finish...
 */
class menu extends singleton {
    
    public static
            /** @var array */
            $menus;
    
    private static 
            /** @var boolean */
            $useDatabase;
    
    protected static function _construct() {
        self::$useDatabase = self::config()->get('database/active', false);
    }
    
}

menu::getInstance();
