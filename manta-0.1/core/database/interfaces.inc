<?php

/**
 * This file is actually never included.
 * It's only to learn NetBeans about various query methods.
 * Time to ctrl+w.
 * 
 * @ignore
 */

interface _if_query extends database_query {
    /**
     * Dump query.
     * 
     * @return this
     */
    function dump();
    
    /**
     * Set tables.
     * <ul>
     *      <li>array $tables
     *          <ul>
     *              <li><b>['name']</b>     <i>string</i> Table name</li>
     *              <li>['prefix']          <i>string</i> Table prefix</li>
     *              <li>['index']           <i>string</i> Index hint</li>
     *              <li>['alias']           <i>string</i> Define table alias</li>
     *              <li>['join']            <i>array</i> Joins
     *                  <ul>
     *                      <li><b>array</b>
     *                          <ul>
     *                              <li><b>['type']</b>     <i>string</i> Join type</li>
     *                              <li><b>['name']</b>     <i>string</i> Table name</li>
     *                              <li><b>['on']</b>       <i>string</i> Join on</li>
     *                              <li>['prefix']          <i>string</i> Table prefix</li>
     *                              <li>['index']           <i>string</i> Index hint</li>
     *                              <li>['alias']           <i>string</i> Define table alias</li>
     *                          </ul>
     *                      </li>
     *                  </ul>
     *              </li>
     *          </ul>
     *      </li>
     * </ul>
     * @param string|array $tables (See above)
     * @return this
     */
    function tables($tables);
    
    /**
     * Bind param to query.
     * 
     * @param string $name
     * @param mixed $value
     * @param integer $type
     * @return this
     */
    function param($name, $value, $type);
    
    /**
     * Execute query.
     * 
     * @param string $fetch_class
     * @return this
     */
    function execute($fetch_class);
}

interface _if_query_select extends _if_query {
    /**
     * Set columns.
     * 
     * @param string|array $columns
     * @return this
     */
    function columns($columns);
    
    /**
     * Set where.
     * 
     * @param string $column
     * @param string $operator
     * @param mixed $value
     * @param string $prefix
     * @param string $suffix
     * @return this
     */
    function where($column, $operator, $value, $prefix, $suffix);
    
    /**
     * Returns an array containing all of the result set rows.
     * 
     * @param string $fetch_class
     * @return array
     */
    function fetchAll($fetch_class);
    
    /**
     * Set order.
     * 
     * @param string $column
     * @param string $direction
     * @param string $prefix
     * @param string $suffix
     * @return this
     */
    function order($column, $direction, $prefix, $suffix);
    
    /**
     * Set limit.
     * 
     * @param integer $limit
     * @return this
     */
    function limit($limit);
    
    /**
     * Set offset.
     * 
     * @param integer $offset
     * @return this
     */
    function offset($offset);
    
    /**
     * Set flag to obtain execution plan information.
     * 
     * @return this
     */
    function explain();
    
    /**
     * Set flag to obtain table structure information.
     * 
     * @return this
     */
    function describe();
    
    /**
     * Return item by index from result set.
     * 
     * @param integer $index
     * @return stdClassExt|false
     */
    function fetch($index);
}

interface _if_query_insert extends _if_query {
    /**
     * Set columns.
     * 
     * @param string|array $columns
     * @return this
     */
    function columns($columns);
}

interface _if_query_update extends _if_query {
    /**
     * Set where.
     * 
     * @param string $column
     * @param string $operator
     * @param mixed $value
     * @param string $prefix
     * @param string $suffix
     * @return this
     */
    function where($column, $operator, $value, $prefix, $suffix);
    
    /**
     * Set limit.
     * 
     * @param integer $limit
     * @return this
     */
    function limit($limit);
    
    /**
     * Set order.
     * 
     * @param string $column
     * @param string $direction
     * @param string $prefix
     * @param string $suffix
     * @return this
     */
    function order($column, $direction, $prefix, $suffix);
    
    /**
     * Set set.
     * 
     * @param string $column
     * @param mixed $value
     * @return this
     */
    function set($column, $value);
}

interface _if_query_replace extends _if_query {
    /**
     * Set set.
     * 
     * @param string $column
     * @param mixed $value
     * @return this
     */
    function set($column, $value);
}

interface _if_query_delete extends _if_query {
    /**
     * Set where.
     * 
     * @param string $column
     * @param string $operator
     * @param mixed $value
     * @param string $prefix
     * @param string $suffix
     * @return this
     */
    function where($column, $operator, $value, $prefix, $suffix);
    
    /**
     * Set limit.
     * 
     * @param integer $limit
     * @return this
     */
    function limit($limit);
    
    /**
     * Set order.
     * 
     * @param string $column
     * @param string $direction
     * @param string $prefix
     * @param string $suffix
     * @return this
     */
    function order($column, $direction, $prefix, $suffix);
}