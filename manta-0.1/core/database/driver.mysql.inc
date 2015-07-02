<?php

class _database_driver_mysql extends database_driver {
    
    /**
     * Connect to database.
     */
    protected function connect() {
        try {
            database::$dbh = new PDO(
                'mysql'
                . ':host=' . database::config()->get('hostname')
                . ';dbname=' . database::config()->get('database'),
                database::config()->get('username'),
                database::config()->get('password'),
                array(
                    PDO::MYSQL_ATTR_FOUND_ROWS => true,
                    PDO::ATTR_PERSISTENT => true,
                ));
            // Set error mode to throw exceptions.
            database::$dbh->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
        } catch (PDOException $e) {
            manta::error('Problem connecting to database.', $e->getMessage(), KILL_CLEAN);
            exit;
        }
    }
    
    /**
     * Disconnect from database.
     */
    protected function disconnect() {
        database::$dbh = null;
    }
    
    /**
     * Set columns.
     * 
     * @param database_query $query
     * @param string|array $columns
     */
    public function columns(database_query $query, $columns) {
        if (!is_array($columns)) {
            $columns = explode(',', $columns);
        }
        $query->columns = array_merge($query->columns, $columns);
    }
    
    /**
     * Set tables.
     * 
     * @param database_query $query
     * @param string|array $tables
     */
    public function tables(database_query $query, $tables) {
        if (!is_array($tables)) {
            $tables = array(
                'name' => $tables,
            );
        }
        $tables['prefix'] = arr('prefix', $tables, database::config()->get('table_prefix'));
        $tables['index'] = arr('index', $tables, null);
        $tables['alias'] = arr('alias', $tables, null);
        if (($join = arr('join', $tables))) {
            if (isset($join['type'])) {
                $join = array($join);
            }
            foreach ($join as &$j) {
                if (($missing = array_diff_key(array_flip(array('type', 'name', 'on')), $j))) {
                    $missing = array_flip($missing);
                    manta::error('Missing vital keys in join statement. Missing keys: ' . join(', ', $missing), array('database table' => $tables, 'missing keys' => $missing), KILL);
                }
                $j['prefix'] = arr('prefix', $j, database::config()->get('table_prefix'));
                $j['index'] = arr('index', $j, null);
                $j['alias'] = arr('alias', $j, null);
            }
            $tables['join'] = $join;
        }
        if (!arr('name', $tables)) {
            manta::error('Missing tablename in from statement.', array('database table' => $tables), KILL);
        } else if (preg_match('/\s/', $tables['name'])) {
            manta::error('Only a clean (no spaces) table name is permitted. Advanced from clause shall use the array structure.', array('database table', $tables), KILL);
        }
        $query->tables[] = $tables;
    }
    
    /**
     * Set where.
     * 
     * @param database_query
     * @param string $column
     * @param string $operator
     * @param string $prefix
     * @param string $suffix
     * @param mixed $value
     */
    public function where(database_query $query, $column, $operator = null, $value = null, $prefix = null, $suffix = null) {
        $where = is_array($column) ? $column : array('column' => $column);
        $operator && $where['operator'] = $operator;
        $value && $where['value'] = $value;
        $prefix && $where['prefix'] = $prefix;
        $suffix && $where['suffix'] = $suffix;
        if (($missing = array_diff_key(array_flip(array('column', 'operator', 'value')), $where))) {
            $missing = array_flip($missing);
            manta::error('Missing vital keys in where clause. Missing keys: ' . join(', ', $missing), array('where clause' => $where, 'missing keys' => $missing), KILL_CLEAN);
        }
        $query->where[] = $where;
    }
    
    /**
     * Set set.
     * 
     * @param database_query
     * @param string $column
     * @param mixed $value
     */
    public function set(database_query $query, $column, $value = null) {
        $set = is_array($column) ? $column : array('column' => $column);
        $value && $set['value'] = $value;
        if (($missing = array_diff_key(array_flip(array('column', 'value')), $set))) {
            $missing = array_flip($missing);
            manta::error('Missing vital keys in set clause. Missing keys: ' . join(', ', $missing), array('set clause' => $set, 'missing keys' => $missing), KILL_CLEAN);
        }
        $query->set[] = $set;
    }
    
    /**
     * Set order.
     * 
     * @param database_query
     * @param string $column
     * @param string $direction
     */
    public function order(database_query $query, $column, $direction = 'asc', $prefix = null, $suffix = null) {
        $order = is_array($column) ? $column : array('column' => $column);
        $direction && $order['direction'] = $direction;
        $prefix && $order['prefix'] = $prefix;
        $suffix && $order['suffix'] = $suffix;
        if (($missing = array_diff_key(array_flip(array('column')), $order))) {
            $missing = array_flip($missing);
            manta::error('Missing vital keys in order clause. Missing keys: ' . join(', ', $missing), array('order clause' => $order, 'missing keys' => $missing), KILL_CLEAN);
        }
        $query->order[] = $order;
    }
    
    /**
     * Set limit.
     * 
     * @param database_query $query
     * @param integer $limit
     */
    public function limit(database_query $query, $limit) {
        $query->limit = $limit;
    }
    
    /**
     * Set offset.
     * 
     * @param database_query $query
     * @param integer $offset
     */
    public function offset(database_query $query, $offset) {
        $query->offset = $offset;
    }
    
    /**
     * Bind params to query.
     * 
     * @param database_query $query
     * @param string $name
     * @param mixed $value
     * @param integer $type
     */
    public function param(database_query $query, $name, $value, $type = null) {
        if (is_null($type)) {
            $type = is_numeric($value) ? PDO::PARAM_INT : PDO::PARAM_STR;
        }
        $query->params[$name] = array($value, $type);
    }
    
    /**
     * Returns an array containing all of the result set rows.
     * 
     * @param database_query $query
     * @param string $fetch_class
     * @return array|false False if result set is empty.
     */
    public function fetchAll(database_query $query, $fetch_class = 'stdClassExt') {
        if (empty($query->result)) {
            $query->result = $query->stmt->fetchAll(PDO::FETCH_CLASS, $fetch_class);
        }
        return $query->result ? $query->result : false;
    }
    
    /**
     * Execute query.
     * 
     * @param database_query $query
     * @param string $fetch_class
     */
    public function execute(database_query $query, $fetch_class = 'stdClassExt') {
        $sql = '';
        
        // Try:catch
        try {
            switch ($query->type) {
                default:
                    throw new PDOException('Query type not defined in execute function.');
                    break;
                case 'select':
                    $sql .= $query->explain ? 'explain' : ($query->describe ? 'describe' : '');
                    $sql .= ' select ' . join(', ', $query->columns);
                    $sql .= ' from ' . self::buildTables($query);
                    $sql .= self::buildWhere($query);
                    $sql .= self::buildOrder($query);
                    $sql .= self::buildLimit($query);
                    $sql .= self::buildOffset($query);
                    break;
                case 'insert':
                    $sql .= ' insert into ' . self::buildTables($query);
                    $sql .= ' (' . join(',', $query->columns) . ')';
                    $sql .= ' values (' . self::buildValues($query) . ')';
                    break;
                case 'update':
                    $sql .= ' update ' . self::buildTables($query);
                    $sql .= self::buildSet($query);
                    $sql .= self::buildWhere($query);
                    $sql .= self::buildOrder($query);
                    $sql .= self::buildLimit($query);
                    break;
                case 'replace':
                    $sql .= ' replace into ' . self::buildTables($query);
                    $sql .= self::buildSet($query);
                    break;
                case 'delete':
                    $sql .= ' delete from ' . self::buildTables($query);
                    $sql .= self::buildWhere($query);
                    $sql .= self::buildOrder($query);
                    $sql .= self::buildLimit($query);
                    break;
            }
            $query->sql = $sql;
            
            // Create statement
            $query->stmt = database::$dbh->prepare($query->sql);

            // Bind params
            foreach ($query->params as $param => &$value) {
                $query->stmt->bindParam($param, $value[0], $value[1]);
            }

            // Execute query
            $query->stmt->execute();
            $query->rowCount = $query->stmt->rowCount();
            
            switch ($query->type) {
                case 'select':
                    $query->fetchAll($fetch_class);
                    break;
                case 'insert':
                    $query->lastInsertedID = database::$dbh->lastInsertId();
                    $query->result = (bool) $query->rowCount;
                    break;
                case 'update':
                case 'replace':
                case 'delete':
                    $query->result = (bool) $query->rowCount;
                    break;
            }
        } catch (PDOException $ex) {
            $backtrace = debug_backtrace(false);
            $backtrace = array_splice($backtrace, 2);
            manta::error($ex->getMessage(), array('type' => $query->type,'sql' => $query->sql), KILL_CLEAN, $backtrace);
        }
    }
    
    private static function buildTables(database_query $query) {
        $result = array();
        foreach ($query->tables as $tables) {
            $table = arr('prefix', $tables, '') . arr('name', $tables);
            if (($alias = arr('alias', $tables))) {
                $table .= " as $alias";
            }
            if (($index = arr('index', $tables))) {
                $table .= " $index";
            }
            if (($join = arr('join', $tables))) {
                foreach ($join as $j) {
                    $alias = arr('alias', $j);
                    $table .= sprintf(' %s join %s%s%s %s on %s',
                        $j['type'],
                        arr('prefix', $j, ''),
                        $j['name'],
                        $alias ? " as $alias" : '',
                        arr('index', $j, ''),
                        $j['on']);
                }
            }
            $result[] = $table;
        }
        return join(',', $result);
    }
    
    private static function buildWhere(database_query $query) {
        $result = array();
        foreach ($query->where as $where) {
            $prefix = arr('prefix', $where, '');
            $suffix = arr('suffix', $where, '');
            $result[] = sprintf('%s%s %s %s%s',
                $prefix ? "$prefix " : '',
                arr('column', $where, ''),
                arr('operator', $where, ''),
                arr('value', $where, ''),
                $suffix ? " $suffix " : '');
        }
        return $result ? ' where ' . join(' ', $result) : '';
    }
    
    private static function buildSet(database_query $query) {
        $result = array();
        foreach ($query->set as $set) {
            $result[] = sprintf('%s = %s',
                arr('column', $set, ''),
                arr('value', $set, ''));
        }
        return $result ? ' set ' . join(', ', $result) : '';
    }
    
    private static function buildValues(database_query $query) {
        $result = array();
        foreach ($query->params as $key => $param) {
            $result[] = $key;
        }
        return join(',', $result);
    }
    
    private static function buildLimit(database_query $query) {
        return $query->limit ? ' limit ' . $query->limit : '';
    }
    
    private static function buildOffset(database_query $query) {
        return $query->offset ? ' offset ' . $query->offset : '';
    }
    
    private static function buildOrder(database_query $query) {
        $result = array();
        foreach ($query->order as $order) {
            $prefix = arr('prefix', $order, '');
            $suffix = arr('suffix', $order, '');
            $result[] = sprintf(' %s%s %s%s',
                $prefix ? "$prefix " : '',
                arr('column', $order, ''),
                arr('direction', $order, ''),
                $suffix ? " $suffix" : '');
        }
        return $result ? ' order by' . join(' ', $result) : '';
    }
    
}