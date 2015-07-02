<?php

/**
 * Session handler.
 * 
 * Uses memcache and database for storage.
 * 
 * @version 0.1
 */
class session extends singleton {
    
    private static
            /** @var integer */
            $lifetime,
            /** @var integer */
            $refreshTime;
    
    private static 
            /** @var boolean */
            $useMemcache,
            /** @var boolean */
            $useDatabase;
    
    private static
            /** @var Memcache */
            $memcache;
    
    
    private static
            /** @var string */
            $initSessionData,
            /** @var string */
            $domain,
            /** @var string */
            $debug = '';
    
    /**
     * Initiate session handler.
     * 
     * @return boolean
     */
    public static function _construct() {
        self::$useDatabase = self::config()->get('database/active');
        self::$useMemcache = self::config()->get('memcache/active');
        self::config()->set('path', rtrim(session_save_path(), '/') . '/' . manta::$namespace);
        self::config()->set('name', manta::$namespace . '_' . self::config()->get('name', session_name()));
        
        // Make sure to end sessions when execution ends
        register_shutdown_function('session_write_close');
        
        // Check if to limit access to subdomain level
        self::$domain = arr('HTTP_HOST', $_SERVER, arr('SERVER_NAME', $_SERVER));
        if (!self::config()->get('cookie/subdomain')) {
            self::$domain = array_reverse(explode('.', self::$domain));
            array_pop(self::$domain);
            self::$domain = '.' . join('.', array_reverse(self::$domain));
        }
        
        // Session cookie settings
        session_set_cookie_params(
                0, // Kill cookie upon exit
                self::config()->get('cookie/path'),
                self::$domain,
                manta::config()->get('secure'),
                self::config()->get('cookie/httponly'));
        session_name(self::config()->get('name'));
        
        // Create path if absent
        if (!file_exists(self::config()->get('path'))) {
            mkdir(self::config()->get('path'), 0770, true);
        }
        
        session_save_path(self::config()->get('path'));
        session_cache_limiter('nocache');
        
        // Memcache
        if (self::$useMemcache) {
            self::$memcache = new Memcache;
            self::$memcache->connect(
                    self::config()->get('memcache/hostname'),
                    self::config()->get('memcache/port'));
        }
        self::$lifetime = self::config()->get('lifetime', intval(ini_get('session.gc_maxlifetime')));
        self::$refreshTime = self::config()->get('refreshTime', 300);
        self::$initSessionData = null;
        
        // Session handler
        if (self::$useMemcache || self::$useDatabase) {
            // We need to close the session if it's been auto started
            // before we can change the session handler.
            session_write_close();
            session_set_save_handler(
                    array('session', 'open'),
                    array('session', 'close'),
                    array('session', 'read'),
                    array('session', 'write'),
                    array('session', 'destroy'),
                    array('session', 'gc'));
        }
        
        // Initiate session
        session_start();
        
        self::debug(self::$debug);
        return true;
    }

    /**
     * Session open.
     * 
     * @return boolean
     */
    public static function open($savePath, $sessionName) {
        self::$debug .= self::debug(sprintf('SESSION OPEN<br>sessionId:   %s<br>sessionName: %s<br>savePath:    %s', session_id(), $sessionName, $savePath), true);
        return true;
    }

    /**
     * Session close.
     * 
     * @return boolean
     */
    public static function close() {
        self::debug(sprintf('SESSION CLOSE'));
        self::$lifetime = null;
        self::$memcache = null;
        self::$initSessionData = null;
        return true;
    }

    /**
     * Session read.
     * 
     * Tries to read from memcache.
     * If no match, try database storage.
     * Verifies session expiration on match.
     * 
     * @param string $sessionId
     * @return mixed
     */
    public static function read($sessionId) {
        self::$debug .= self::debug(sprintf('SESSION READ<br>sessionId: %s', $sessionId), true);
        $now = time();
        $data = '';
        $expiration = null;
        
        // Memcache
        self::$useMemcache && $data = self::$memcache->get($sessionId);
        self::$useMemcache && self::$debug .= self::debug(sprintf('MEMCACHE FETCH: %s', $data !== false ? 'hit' : 'miss'), true);
        
        // Fetch from database if necessary
        // Miss from memcache returns boolean false!
        if ($data === false && self::$useDatabase) {
            $query = database::select('data, expiration')
                    ->tables('session')
                    ->where('id', '=', ':sessionId')
                    ->param(':sessionId', $sessionId)
                    ->limit(1)
                    ->execute();
            self::$debug .= self::debug(sprintf('DB FETCH: %s', $query->result ? 'hit' : 'miss'), true);
            
            // Hit from database.
            if ($query->result) {
                $data = $query->fetch(0)->data;
                $expiration = $query->fetch(0)->expiration;
                
                // Update expiration timestamp in database if there's been more
                // than self::$refreshTime since last update
                // We need the self::$lifetime check because the expiration 
                // timestamp is stored with it
                if ($now - self::$refreshTime > $expiration - self::$lifetime) {
                    self::updateDbExpiration($sessionId, $now, $data);
                }
            }
        }
        
        // Update expiration timestamp in database if necessary
        elseif (self::$useDatabase) {
            // In here, we are sure that memcache is activated
            $expiration = self::$memcache->get('db-' . $sessionId, $expiration);
            if ($expiration) {
                // Update expiration timestamp in database if there's been more
                // than self::$refreshTime since last update
                // We need the self::$lifetime check because the expiration 
                // timestamp is stored with it
                if ($now - self::$refreshTime > $expiration - self::$lifetime) {
                    self::updateDbExpiration($sessionId, $now, $data);
                }
            } else {
                // Set database expiration timestamp if absent
                self::updateDbExpiration($sessionId, $now, $data);
            }
        }
        
        // Extend expiration timestamp for memcache storage by writing to it
        // Miss from memcache returns boolean false!
        self::$useMemcache && $data !== false && self::$memcache->set($sessionId, $data, false, self::$lifetime);
        
        // Transform boolean false data to empty string
        if ($data === false) {
            $data = '';
        }
        
        // In case of Memcached restart, we need to force an update
        if (self::$useMemcache
                && self::$useDatabase 
                && self::$memcache->get('db-' . $sessionId) === false) {
            self::updateDbExpiration($sessionId, $now, $data);
        }
        
        // Set initial data
        if (self::$initSessionData === null) {
            // Fetch initial data
            self::$initSessionData = $data;
        }
        
        return $data;
    }

    /**
     * Update expiration timestamp in database record.
     * 
     * @param string $sessionId
     * @param integer $now
     */
    private static function updateDbExpiration($sessionId, $now, $data) {
        if (!$now) {
            $now = time();
        }
        self::$debug .= self::debug(sprintf('DB EXPIRE UPDATE'), true);
        $expiration = $now + self::$lifetime;
        
        // By using replace, we can recover data that has accidently been
        // deleted from the database.
        database::replace()
                ->tables('session')
                ->set('id', ':sessionId')
                ->set('expiration', ':expiration')
                ->set('data', ':data')
                ->param(':sessionId', $sessionId)
                ->param(':expiration', $expiration, PDO::PARAM_INT)
                ->param(':data', $data)
                ->execute();
//        database::update()
//                ->tables('session')
//                ->set('expiration', ':expiration')
//                ->where('id', '=', ':sessionId')
//                ->param(':sessionId', $sessionId)
//                ->param(':expiration', $expiration, PDO::PARAM_INT)
//                ->limit(1)
//                ->execute();
        
        // Update database expiration time in memcache
        self::$useMemcache && self::$memcache->set('db-' . $sessionId, $expiration, false, self::$lifetime);
    }

    /**
     * Session write.
     * 
     * This will run when session ends.
     * 
     * @param string $sessionId
     * @param mixed $data
     * @return boolean
     */
    public static function write($sessionId, $data) {
        $now = time();
        $expiration = $now + self::$lifetime;
        $result = true;
        
        // Update memcache if used
        self::$useMemcache && $result = self::$memcache->set($sessionId, $data, false, self::$lifetime);
        
        // Update database only if data has changed
        if (self::$useDatabase && self::$initSessionData !== $data) {
            $query = database::replace()
                    ->tables('session')
                    ->set('id', ':sessionId')
                    ->set('expiration', ':expiration')
                    ->set('data', ':data')
                    ->param(':sessionId', $sessionId)
                    ->param(':expiration', $expiration, PDO::PARAM_INT)
                    ->param(':data', $data)
                    ->execute();
            $result = $query->rowCount ? true : false;
            self::debug(sprintf('DB UPDATE'));
            
            // Update database expiration time in memcache
            self::$useMemcache && self::$memcache->set('db-' . $sessionId, $expiration, false, self::$lifetime);
        }
        return $result;
    }

    /**
     * Session destroy.
     * 
     * @param string $sessionId
     * @return boolean
     */
    public static function destroy($sessionId) {
        self::$useMemcache && self::$memcache->delete($sessionId);
        self::$useMemcache && self::$useDatabase && self::$memcache->delete('db-' . $sessionId);
        self::$useDatabase && $query = database::delete()
                ->tables('session')
                ->where('id', '=', ':sessionId')
                ->param(':sessionId', $sessionId)
                ->limit(1)
                ->execute();
        self::debug(sprintf('SESSION DESTROYED'));
        self::$useDatabase && self::debug(sprintf('DB DELETE'));
        
        // Delete session cookie
        $cookieparams = session_get_cookie_params();
        setcookie(
                session_name(),
                '',
                0,
                $cookieparams['path'],
                $cookieparams['domain'],
                $cookieparams['secure'],
                $cookieparams['httponly']
        );
        
        return true;
    }

    /**
     * Garbage collector.
     * 
     * The GC runs on a random schedule set by php.ini
     * and removes all session data from database
     * where expiration timestamp has expired.
     * 
     * Memchache uses it's own garbage collector.
     * 
     * @return boolean
     */
    public static function gc($maxLifeTime) {
        if (self::$useDatabase) {
            $query = database::select('id')
                    ->tables('session')
                    ->where('expiration', '<', ':now')
                    ->param(':now', time(), PDO::PARAM_INT)
                    ->execute();
            foreach ($query->result as $row) {
                self::destroy($row->id);
            }
            manta::log('GC: Found and deleted ' . count($query->result) . ' db entries.');
        }
        return true;
    }

    /**
     * Fetch a session value.
     * 
     * @param string $path Session path, separated by /.
     * @param mixed $default
     * @return mixed
     */
    public static function get($path, $default = null) {
        self::debug(sprintf('GET SESSION DATA<br>path: %s', $path));
        $path = explode('/', $path);
        $tree = $_SESSION;
        foreach ($path as $leaf) {
            if (!isset($tree[$leaf])) {
                return $default;
            }
            $tree = $tree[$leaf];
        }
        return $tree;
    }

    /**
     * Set a session value.
     * 
     * @param string $path Session path, separated by /.
     * @param mixed $value
     * @return boolean
     */
    public static function set($path, $value) {
        self::debug(sprintf('SET SESSION DATA<br>path: %s<br>value: %s', $path, $value));
        $appendArray = false;
        if (substr($path, -2) == '[]') {
            $appendArray = true;
            $path = substr($path, 0, -2);
        }
        $path = explode('/', $path);
        $tree = & $_SESSION;
        foreach ($path as $leaf) {
            if (!isset($tree[$leaf]) || !is_array($tree[$leaf])) {
                $tree[$leaf] = array();
            }
            $tree = & $tree[$leaf];
        }
        if ($appendArray) {
            if (!is_array($tree)) {
                $tree = array();
            }
            $tree[] = $value;
        } else {
            $tree = $value;
        }
        return true;
    }
    
    /**
     * Delete data.
     * 
     * @param string|array $path Storage path, separated by /.
     */
    public static function delete($path) {
        if (!is_array($path)) {
            $path = array($path);
        }
        foreach ($path as $p) {
            $p = explode('/', $p);
            $tree = & $_SESSION;
            for ($i = 0, $m = count($p); $i < $m; ++$i) {
                if (!isset($tree[$p[$i]])) {
                    break;
                }
                if ($i < $m - 1) {
                    $tree = & $tree[$p[$i]];
                } else {
                    unset($tree[$p[$i]]);
                }
            }
        }
    }

    /**
     * Prints complete storage.
     * 
     * @return array
     */
    public static function dump() {
        $backtrace = debug_backtrace(false);
        manta::message($_SESSION, $backtrace);
    }

}

session::getInstance();
