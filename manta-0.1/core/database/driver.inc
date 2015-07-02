<?php

/**
 * Abstract class for database drivers.
 */
abstract class database_driver {
    
    protected static
            /** @var array */
            $queries = array();
    
    public function __construct() {
        $this->connect();
        database::config()->delete(array(
            'username',
            'password',
            'hostname',
            'database'));
    }
    
    public function __destruct() {
        $this->disconnect();
    }
    
    /**
     * Build new query.
     * 
     * @param string $type
     * @return database_query
     */
    public function query($type) {
        self::$queries[] = new database_query($type);
        return end(self::$queries);
    }
    
    /**
     * Examine database query.
     * 
     * @param database_query $query
     * @param string $type
     * @param array $backtrace
     */
    public function dump(database_query $query, $type, $backtrace) {
        $result = new stdClass();
        $visible_variables = 'sql,params,tables,rowCount,result';
        switch ($type) {
            case 'select':
                $visible_variables .= ',explain,describe,columns,where,order,limit,offset';
                break;
            case 'insert':
                $visible_variables .= ',lastInsertedID';
                break;
            case 'update':
                $visible_variables .= ',set,where,order,limit';
                break;
            case 'replace':
                $visible_variables .= ',set,lastInsertedID';
                break;
            case 'delete':
                $visible_variables .= ',where,order,limit';
                break;
        }
        $variables = get_class_vars(get_class($query));
        if (!is_null($visible_variables)) {
            $variables = array_intersect_key(
                $variables,
                array_flip(explode(',', $visible_variables))
            );
        }
        foreach ($variables as $key => $val) {
            if (isset($query->$key) && (!empty($query->$key) || $query->$key === null || $query->$key === false)) {
                $result->$key = $query->$key;
            }
        }
        manta::message($result, $backtrace);
    }
    
    /**
     * Set flag to obtain execution plan information.
     * 
     * @param database_query $query
     */
    public function explain(database_query $query) {
        $query->explain = true;
    }
    
    /**
     * Set flag to obtain table structure information.
     * 
     * @param database_query $query
     */
    public function describe(database_query $query) {
        $query->describe = true;
    }
    
    /**
     * Fetch queries.
     * 
     * @param integer $id
     * @return array
     */
    public function queries($id = null) {
        $result = self::$queries;
        if ($id !== null && is_int($id)) {
            $result = isset(self::$queries[$id]) ? self::$queries[$id] : null;
        }
        return $result;
    }
    
    /**
     * Return item by index from result set.
     * 
     * @param database_query $query
     * @param integer $index
     * @return stdClassExt|false
     */
    public function fetch(database_query $query, $index = 0) {
        return arr($index, $query->result, false);
    }
    
    abstract protected function connect();
    abstract protected function disconnect();
    abstract public function columns(database_query $query, $columns);
    abstract public function tables(database_query $query, $tables);
    abstract public function where(database_query $query, $column, $operator = null, $value = null, $prefix = null, $suffix = null);
    abstract public function set(database_query $query, $column, $value = null);
    abstract public function order(database_query $query, $columns, $direction = 'asc', $prefix = null, $suffix = null);
    abstract public function limit(database_query $query, $limit);
    abstract public function offset(database_query $query, $offset);
    abstract public function param(database_query $query, $name, $value, $type = null);
    abstract public function fetchAll(database_query $query, $fetch_class = null);
    abstract public function execute(database_query $query, $fetch_class = 'stdClassExt');
    
}