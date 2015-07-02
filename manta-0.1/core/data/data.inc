<?php

/**
 * Storage handler.
 * 
 * @version 0.1
 */
class data {

    private
            /** @var array */
            $data = array();

    /**
     * Creates a storage handler.
     * 
     * @param array $data
     */
    public function __construct($data = array()) {
        $this->load($data);
    }

    /**
     * Fetch data.
     * 
     * @param string $path Storage path, separated by /.
     * @param mixed $default
     * @return mixed
     */
    public function get($path, $default = null) {
        $path = explode('/', $path);
        $tree = $this->data;
        foreach ($path as $leaf) {
            if (!isset($tree[$leaf])) {
                return $default;
            }
            $tree = $tree[$leaf];
        }
        return $tree;
    }

    /**
     * Store data.
     * 
     * @param string $path Storage path, separated by /.
     * @param mixed $value
     * @return boolean
     */
    public function set($path, $value) {
        $appendArray = false;
        if (substr($path, -2) == '[]') {
            $appendArray = true;
            $path = substr($path, 0, -2);
        }
        $path = explode('/', $path);
        $tree = & $this->data;
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
    public function delete($path) {
        if (!is_array($path)) {
            $path = array($path);
        }
        foreach ($path as $p) {
            $p = explode('/', $p);
            $tree = & $this->data;
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
     * Prints complete storeage.
     * 
     * @return array
     */
    public function dump() {
        $backtrace = debug_backtrace(false);
        manta::message($this->data, $backtrace);
    }

    /**
     * Loads data into storage.
     * 
     * @param array $data
     */
    public function load($data) {
        $this->data = $data;
    }

    /**
     * Clears storage.
     */
    public function clear() {
        $this->data = array();
    }

}
