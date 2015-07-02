<?php

class database_query {
    
    public
            /** @var PDOStatement */
            $stmt = null;


    public
            /** @var array */
            $params = array(),
            /** @var array */
            $columns = array(),
            /** @var array */
            $tables = array(),
            /** @var array */
            $group = array(),
            /** @var array */
            $keys = array(),
            /** @var array */
            $order = array(),
            /** @var array */
            $unique = array(),
            /** @var array */
            $where = array(),
            /** @var array */
            $result = array(),
            /** @var array */
            $set = array();
    
    public
            /** @var integer */
            $limit,
            /** @var integer */
            $offset,
            /** @var integer */
            $lastInsertedID,
            /** @var integer */
            $rowCount;
    
    public
            /** @var string */
            $type,
            /** @var string */
            $sql;
    
    public
            /** @var boolean */
            $explain = false,
            /** @var boolean */
            $describe = false;
    
    public function __construct($type) {
        $this->type = $type;
    }
    
    /**
     * Dynamically calls methods in current database driver.
     * 
     * Each call returns the query object if null is returned by the driver method.
     * 
     * @param string $method
     * @param array $arguments
     * @return database_query
     */
    public function __call($method, $arguments) {
        if ($method === 'dump') {
            $backtrace = debug_backtrace(false);
            array_unshift($arguments, $backtrace);
            array_unshift($arguments, $this->type);
        }
        array_unshift($arguments, $this);
        if (method_exists(database::$driver, $method)) {
            $return = call_user_func_array(array(database::$driver, $method), $arguments);
        } else {
            manta::error(
                "Method '$method' does not exist.", "trying to call $method in " . get_class(database::$driver));
        }
        return !is_null($return) ? $return : $this;
    }
    
}