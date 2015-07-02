<?php

include('install.inc');

class modules extends singleton {
    
    public static
        /** @var array */
        $modules = array(
            'queue' => array(),
            'loaded' => array(),
            'failed' => array()
        );

    protected static function _construct() {
        self::fetch_core();
        self::load(true);
    }

    /**
     * Fetch info files for core modules, mark them as loaded.
     */
    private static function fetch_core() {
        $dir = new DirectoryIterator(PATH_FRAMEWORK . '/core');
        foreach ($dir as $fileinfo) {
            if ($fileinfo->isDir() && !$fileinfo->isDot()) {
                if (file_exists($fileinfo->getPathname() . '/' . $fileinfo->getFilename() . '.info')) {
                    $module_ini = parse_ini_file($fileinfo->getPathname() . '/' . $fileinfo->getFilename() . '.info', true);
                    if (isset($module_ini['disabled']) && $module_ini['disabled']) {
                        continue;
                    }
                    $module_ini['filename'] = $fileinfo->getFilename();
                    $module_ini['pathname'] = $fileinfo->getPathname();
                    $module_ini['requires'] = self::module_split_list($module_ini['requires']);
                    $module_ini['dependencies'] = self::module_split_list($module_ini['dependencies']);
                    self::$modules['loaded'][$module_ini['filename']] = $module_ini;
                    self::$modules['queue'][$module_ini['filename']] = $module_ini;
                } else {
                    self::$modules['failed'][$fileinfo->getFilename()] = null;
                }
            }
        }
        
        // Fetch framework information.
        $module_ini = parse_ini_file(PATH_FRAMEWORK . '/manta.info', true);
        $module_ini += array(
            'filename' => 'manta',
            'pathname' => PATH_FRAMEWORK,
        );
        $module_ini['requires'] = self::module_split_list($module_ini['requires']);
        $module_ini['dependencies'] = self::module_split_list($module_ini['dependencies']);
        self::$modules['loaded']['manta'] = $module_ini;
        self::$modules['queue']['manta'] = $module_ini;
    }
    
    /**
     * Load modules.
     * 
     * @param boolean $system
     */
    private static function load($system = false) {
        $path = $system
            ? PATH_FRAMEWORK . '/modules'
            : PATH_WEBROOT . '/site/' . manta::config()->get('site') . '/modules';
        $dir = new DirectoryIterator($path);
        foreach ($dir as $fileinfo) {
            if ($fileinfo->isDir() && !$fileinfo->isDot()) {
                if (file_exists($fileinfo->getPathname() . '/' . $fileinfo->getFilename() . '.info')) {
                    $module_ini = parse_ini_file($fileinfo->getPathname() . '/' . $fileinfo->getFilename() . '.info', true);
                    if (isset($module_ini['disabled']) && $module_ini['disabled']) {
                        continue;
                    }
                    $module_ini['filename'] = $fileinfo->getFilename();
                    $module_ini['pathname'] = $fileinfo->getPathname();
                    $module_ini['requires'] = self::module_split_list($module_ini['requires']);
                    $module_ini['dependencies'] = self::module_split_list($module_ini['dependencies']);
                    self::$modules['queue'][$module_ini['filename']] = $module_ini;
                } else {
                    self::$modules['failed'][$fileinfo->getFilename()] = null;
                }
            }
        }
        
        // Handle failures.
        if (!empty(self::$modules['failed'])) {
            manta::error('Failed to load one or more module.', self::$modules['failed'], KILL_CLEAN);
        }
        
        // Parse module queue, check required.
        while (self::parse_queue('requires')) {}
        
        // Handle modules that wasn't loaded due to requires.
        if (!empty(self::$modules['queue'])) {
            manta::error('Could not load one or more modules due to missing module requirements.', self::$modules['queue'], KILL_CLEAN);
        }
        
        // Parse module queue, check dependencies.
        self::$modules['queue'] = self::$modules['loaded'];
        while (self::parse_queue('dependencies')) {}
        
        // Handle modules that wasn't loaded due to dependencies.
        if (!empty(self::$modules['queue'])) {
            manta::error('Could not load one or more modules due to missing module dependencies.', self::$modules['queue'], KILL_CLEAN);
        }
    }
    
    /**
     * Parse module queue.
     * 
     * @param string $check_type 'requires' or 'dependencies'.
     * @return int Number of modules loaded.
     */
    private static function parse_queue($check_type) {
        $load_count = 0;
        foreach (self::$modules['queue'] as $name => $module) {
            if (self::module_check($module[$check_type])) {
                $load_count++;
                if ($check_type === 'requires'
                        && !isset(self::$modules['loaded'][$name])) {
                    self::debug(sprintf('MODULE LOAD: %s', $name));
                    include_once("{$module['pathname']}/{$module['filename']}.inc");
                    self::$modules['loaded'][$name] = $module;
                }
                unset(self::$modules['queue'][$name]);
            }
        }
        return $load_count;
    }
    
    /**
     * Split string to array.
     * 
     * Helper function to handle required and dependency values.
     * 
     * @param string $list
     * @return array
     */
    private static function module_split_list($list) {
        $list = str_replace(' ', '', $list);
        $list = empty($list) ? array() : explode(',', $list);
        foreach ($list as &$a) {
            $arr = explode('-', $a);
            $a = array(
              'name' => $arr[0],
              'minor' => $arr[1],
              'major' => isset($arr[2]) ? $arr[2] : null,
            );
        }
        return $list;
    }

    /**
     * Check if modules is loaded.
     * 
     * @param array $check
     * @return boolean
     */
    private static function module_check($check) {
        $result = true;
        foreach ($check as $module) {
            if (!isset(self::$modules['loaded'][$module['name']])) {
                $result = false;
                break;
            } else {
                if (empty($module['major'])) {
                    if (self::$modules['loaded'][$module['name']]['version'] != $module['minor']) {
                        $result = false;
                        break;
                    }
                } elseif (self::$modules['loaded'][$module['name']]['version'] < $module['minor'] || self::$modules['loaded'][$module['name']]['version'] > $module['major']) {
                    $result = false;
                    break;
                }
            }
        }
        return $result;
    }

}

modules::getInstance();
