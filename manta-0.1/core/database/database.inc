<?php

include('driver.inc');
include('query.inc');

/**
 * Database handler.
 * 
 * @version 0.1
 */
class database extends singleton {
    
    public static
            /** @var PDO */
            $dbh = null;
    
    public static
            /** @var database_driver */
            $driver = null;

    protected static function _construct() {
        // Load database driver.
        if (file_exists(PATH_FRAMEWORK . '/core/database/driver.' . self::config()->get('driver') . '.inc')) {
            include('driver.' . self::config()->get('driver') . '.inc');
            $driver = '_database_driver_' . self::config()->get('driver');
            self::$driver = new $driver();
        } else {
            manta::error('Database driver missing.', array('driver' => self::config()->get('driver')), KILL_CLEAN);
        }
    }
    
    protected static function _destruct() {
        // Verify that all transactions has been taken care of.
        if (self::inTransaction()) {
            // Rollback last transaction.
            self::rollbackTransaction();
            
            manta::error('Database transaction was not closed properly.', null, KILL);
        }
    }
    
    /**
     * Fetch data.
     * 
     * @param string|array $columns
     * @return _if_query_select
     */
    public static function select($columns) {
        $query = self::$driver->query(__FUNCTION__);
        $query->columns($columns);
        return $query;
    }
    
    /**
     * Insert data.
     * 
     * @param string|array $columns
     * @return _if_query_insert
     */
    public static function insert($columns) {
        $query = self::$driver->query(__FUNCTION__);
        $query->columns($columns);
        return $query;
    }
    
    /**
     * Update data.
     * 
     * @return _if_query_update
     */
    public static function update() {
        $query = self::$driver->query(__FUNCTION__);
        return $query;
    }
    
    /**
     * Replace data.
     * 
     * @return _if_query_replace
     */
    public static function replace() {
        $query = self::$driver->query(__FUNCTION__);
        return $query;
    }
    
    /**
     * Delete data.
     * 
     * @return _if_query_delete
     */
    public static function delete() {
        $query = self::$driver->query(__FUNCTION__);
        return $query;
    }
    
    /**
     * Begin transaction.
     */
    public static function beginTransaction() {
        try {
            self::$dbh->beginTransaction();
        } catch (PDOException $ex) {
            $backtrace = debug_backtrace(false);
            manta::error($ex->getMessage(), null, KILL_CLEAN, $backtrace);
        }
    }
    
    /**
     * Commit transaction.
     */
    public static function commitTransaction() {
        try {
            self::$dbh->commit();
        } catch (PDOException $ex) {
            $backtrace = debug_backtrace(false);
            manta::error($ex->getMessage(), null, KILL_CLEAN, $backtrace);
        }
    }
    
    /**
     * Rollback transaction.
     */
    public static function rollbackTransaction() {
        try {
            self::$dbh->rollBack();
        } catch (PDOException $ex) {
            $backtrace = debug_backtrace(false);
            manta::error($ex->getMessage(), null, KILL_CLEAN, $backtrace);
        }
    }
    
    /**
     * Check if there is an active transaction.
     * 
     * @return boolean
     */
    public static function inTransaction() {
        return self::$dbh && self::$dbh->inTransaction();
    }

}

database::getInstance();
