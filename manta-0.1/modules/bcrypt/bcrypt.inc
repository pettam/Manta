<?php

/**
 * Hashing algorithm, scalable with hardware.
 * 
 * Heavily influenced by http://stackoverflow.com/a/6337021
 * 
 * It's damn slow, and therefor more secure.
 * Each $rounds step increases the time required by 2.
 * PHP version >= 5.5-DEV doesn't need this mumbo-jumbo but instead
 * should use the built in function "password_hash()".
 * PHP version >= 5.3.7 < 5.5-DEV needs a compatibility library installed.
 * PHP version < 5.3.7 may use crypt() on steroids, below.
 * 
 * @version 0.1
 */
class bcrypt extends singleton {

    private static $rounds;
    private static $random_state;

    protected static function _construct($rounds = 12) {
        // Verify that php-mcrypt is installed.
        if (CRYPT_BLOWFISH != 1) {
            throw new Exception('bcrypt not supported in this installation. See http://php.net/crypt');
        }
        !is_int($rounds) && $rounds = 12;
        self::$rounds = $rounds;
    }

    public static function hash($input) {
        $hash = '';
        while (strlen($hash) <= 13) {
            $hash = crypt($input, self::get_salt());
        }
        return $hash;
    }

    public static function verify($input, $existing_hash) {
        $hash = crypt($input, $existing_hash);
        return ($hash === $existing_hash);
    }

    private static function get_salt() {
        $salt = sprintf('$2a$%02d$', self::$rounds);
        $bytes = self::get_random_bytes(16);
        $salt .= self::encode_bytes($bytes);
        return $salt;
    }

    private static function get_random_bytes($count) {
        $bytes = '';
        if (function_exists('openssl_random_pseudo_bytes') &&
            (strtoupper(substr(PHP_OS, 0, 3)) !== 'WIN')) { // OpenSSL slow on Win
            $bytes = openssl_random_pseudo_bytes($count);
        }
        if ($bytes === '' && is_readable('/dev/urandom') &&
            ($h_rand = @fopen('/dev/urandom', 'rb')) !== FALSE) {
            $bytes = fread($h_rand, $count);
            fclose($h_rand);
        }
        if (strlen($bytes) < $count) {
            $bytes = '';
            if (self::$random_state === null) {
                self::$random_state = microtime();
                if (function_exists('getmypid')) {
                    self::$random_state .= getmypid();
                }
            }
            for ($i = 0; $i < $count; $i += 16) {
                self::$random_state = md5(microtime() . self::$random_state);
                if (PHP_VERSION >= '5') {
                    $bytes .= md5(self::$random_state, true);
                } else {
                    $bytes .= pack('H*', md5(self::$random_state));
                }
            }
            $bytes = substr($bytes, 0, $count);
        }
        return $bytes;
    }

    private static function encode_bytes($input) {
        // The following is code from the PHP Password Hashing Framework
        $itoa64 = './ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789';
        $output = '';
        $i = 0;
        do {
            $c1 = ord($input[$i++]);
            $output .= $itoa64[$c1 >> 2];
            $c1 = ($c1 & 0x03) << 4;
            if ($i >= 16) {
                $output .= $itoa64[$c1];
                break;
            }
            $c2 = ord($input[$i++]);
            $c1 |= $c2 >> 4;
            $output .= $itoa64[$c1];
            $c1 = ($c2 & 0x0f) << 2;
            $c2 = ord($input[$i++]);
            $c1 |= $c2 >> 6;
            $output .= $itoa64[$c1];
            $output .= $itoa64[$c2 & 0x3f];
        } while (1);
        return $output;
    }

}

bcrypt::getInstance();