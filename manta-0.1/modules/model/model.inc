<?php

/**
 * Model skeleton.
 * 
 * @version 0.1
 */
class model extends singleton {

    public static function __callStatic($name, $args = array()) {
        if (!class_exists('model_' . $name)) {
            if (!function_exists("model_initiate_$name")) {
                $backtrace = debug_backtrace(false);
                manta::error('Model absent.', array('model absent', $name), KILL, $backtrace);
            }
            call_user_func("model_initiate_$name");
        }
        return call_user_func_array(array(new ReflectionClass("model_$name"), 'newInstance'), $args);
    }

}

model::getInstance();