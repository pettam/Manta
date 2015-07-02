<?php

$_model_settings_user = parse_ini_file('model.user.ini', true);

class model_user {

    public $logname = 'model_user';
    public $uid;
    public $user;

    public function __construct($uid = null) {
        if ($uid) {
            $this->get_user($uid);
        }
    }

    /**
     * @todo Use new query system
     * @see challenge_login
     */
    public function fetch_users(array $options = array()) {
        $result = array();
        $options['fetch_mode'] = isset($options['fetch_mode']) ? $options['fetch_mode'] : PDO::FETCH_OBJ;
        if (($query = database::query('
            SELECT
                u.uid,
                u.user
            FROM
                user AS u
          '))) {
            $properties = array();
            foreach ($query[0] as $prop => $val) {
                $properties[] = $prop;
            }
            foreach ($query as &$user) {
                if ($options['fetch_mode'] !== PDO::FETCH_OBJ) {
                    $user = array_values((array) $user);
                }
                $result[] = $user;
            }
        }
        return $result;
    }

    /**
     * @todo Use new query system
     * @see challenge_login
     */
    private function get_user($uid) {
        if ($query = database::query('
            SELECT
                u.uid,
                u.user
            FROM
                user AS u
            WHERE
                u.uid = :uid
          ', array(
              'uid' => $uid,
            ))) {
            $properties = array();
            foreach ($query[0] as $prop => $val) {
                $this->{$prop} = $val;
                $properties[] = $prop;
            }
        }
    }

    public static function challenge_login($user, $pass) {
        $result = false;
        $query = database::select('id, password')
                ->tables('user')
                ->where('username', '=', ':username')
                ->where('status', '=', 1, 'and')
                ->param(':username', $user)
                ->limit(1)
                ->execute();
        $query_result = $query->fetch();
        if ($query_result && bcrypt::verify(
                user::config()->get('salt') . $query_result->uid . $pass,
                $query_result->pass)) {
            $result = (int) $query_result->id;
        }
        return $result;
    }

}
