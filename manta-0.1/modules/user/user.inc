<?php

include('models.inc');

/**
 * User.
 * 
 * @version 0.1
 */
class user extends singleton {

    public static
            /** @var model_user */
            $current;

    protected static function _construct() {
        self::challenge_current();
    }

    private static function challenge_current() {
        $current = new stdClassExt();
        $challenge = false;
        if (($current->id = session::get('user/uid', 0))) {
            if (($user_model = model::user($current->id)) && $user_model->id) {
                $challenge = true;
                $current = $user_model;
                self::$classes['html'][] = 'user-online';
            }
        }
        if ($current->id && !$challenge) {
            $current->id = 0;
            session::set('user', null);
        }
        self::$current = $current;
    }

}

user::getInstance();
