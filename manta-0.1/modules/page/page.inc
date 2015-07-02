<?php

//define('PATH_PAGES', PATH_SITE . '/pages');
//define('PATH_THEMES', PATH_SITE . '/themes');
//define('PATH_THEMES_SYSTEM', PATH_FRAMEWORK . '/themes');

include('page.template.inc');

/**
 * Page handler.
 * 
 * @version 0.1
 */
class page extends singleton {
    
    public static
            /** @var array */
            $uri,
            /** @var array */
            $classes;
    
    public static
            /** @var string */
            $request;
    
    public static
            /** @var boolean */
            $https;
    
    public static $title;
    public static $breadcrumbs = array();
    public static $theme;
    public static $theme_path;
    public static $stylesheets;
    public static $javascripts;
    public static $content;
    public static $result;

    /**
     * Analyze and set a couple of vital parameters.
     */
    protected static function _construct() {
        self::$https = strtolower(arr('HTTPS', $_SERVER, '')) == 'on'
                || strtolower(arr('HTTP_X_FORWARDED_PROTO', $_SERVER, '')) == 'https'
                ? true
                : false;
        self::$uri = parse_uri(arr('REQUEST_URI', $_SERVER, ''));
        self::$uri['raw'] = array(
            'path' => self::$uri['path']
        );
        if (count(self::$uri['path']) == 0) {
            self::$uri['path'][] = 'start';
        }
        self::$classes = array('html' => array());
        
        // Determine if page request is Regular or API.
        switch (strtolower(arr('SCRIPT_NAME', $_SERVER, ''))) {
            case '/index.php':
                self::$request = 'page';
                break;
            case '/api.php':
                self::$request = 'api';
                break;
        }
        
        // Check SSL presence, redirect if told so.
        if (!self::$https && manta::config()->get('secure')) {
            redirect('https://' . arr('HTTP_HOST', $_SERVER) . arr('REQUEST_URI', $_SERVER), 301);
        }
        
        // Check for maintenance mode.
        self::maintenance();
    }

    /**
     * Load theme settings.
     * 
     * @param string $name Theme name.
     */
    public static function theme($name) {
        $name = strtolower($name);
        $theme_path_site = path(array(PATH_WEBROOT, 'site', manta::config()->get('site'), 'themes'));
        $theme_path = path(array(PATH_FRAMEWORK, 'themes'));
        dpr($theme_path_site, 1);
        self::$theme_path = (file_exists(PATH_THEMES . "/$name/$name.info")
            ? PATH_THEMES . "/$name"
            : (file_exists(PATH_THEMES_SYSTEM . "/$name")
                ? PATH_THEMES_SYSTEM . "/$name"
                : null));
        if (self::$theme_path === null) {
            dpr("Can't find theme '$name'", 2);
        }
        self::$theme = parse_ini_file(self::$theme_path . "/$name.info", true);
        if (!isset(self::$theme['regions']['content'])) {
            dpr("Required region 'content' missing for theme '$name'", 2);
        }
        foreach (self::$theme['regions'] as $name => &$region) {
            $region = array();
        }
        self::$stylesheets = self::$theme['stylesheets'];
        self::$stylesheets['all'] = isset(self::$stylesheets['all']) ? self::$stylesheets['all'] : array();
        self::$stylesheets['screen'] = isset(self::$stylesheets['screen']) ? self::$stylesheets['screen'] : array();
        self::$stylesheets['print'] = isset(self::$stylesheets['print']) ? self::$stylesheets['print'] : array();
        self::$javascripts['files'] = self::$theme['javascripts'];
        self::$javascripts['settings'] = new data();
        self::$javascripts['inline'] = '';
    }

    /**
     * Store or retrieve javascript code and scripts.
     * 
     * @param array|string $path Array if $type is 'settings', string if not.
     *                           Returns concatenated string of queued scripts if null.
     * @param string $type
     * @return string Concatenated string with all script tags if $path is null.
     */
    public static function javascripts($path = null, $type = 'file') {
        // Fetch data.
        if ($path === null) {
            $js_files = self::$javascripts['files'];
            $concatenated = '';
            foreach ($js_files as $js) {
                $concatenated .= "<script type='text/javascript' src='$js'></script>";
            }
            self::$javascripts['files'] = array();
            return $concatenated . "\n";
        }
        
        // Store a new script or code.
        else {
            switch ($type) {
                case 'settings':
                    self::$javascripts['settings']->set(key($path), current($path));
                    break;
                case 'inline':
                    self::$javascripts['inline'] .= $path;
                    break;
                default:
                    // Requested script is already in queue.
                    if (in_array($path, self::$javascripts['files'])) {
                        return;
                    }
                    self::$javascripts['files'][] = $path;
                    break;
            }
        }
    }

    /**
     * Store or retrieve stylesheet links.
     * 
     * @param string $stylesheet Returns concatenated string of queued stylesheets if null.
     * @param string $media Possible values are: 'all', 'screen', 'print'.
     * @return string Concatenated string with all stylesheet links if $stylesheet is null.
     */
    public static function stylesheets($stylesheet = null, $media = 'all') {
        // Verify media type.
        $media_types = array('all', 'screen', 'print');
        if (!in_array($media, $media_types)) {
            manta::error('Trying to load stylesheet with wrong media type.', array('type' => $media), false);
            return;
        }
        
        // Fetch data.
        if ($stylesheet === null) {
            $stylesheets = self::$stylesheets;
            $concatenated = '';
            foreach ($media_types as $media_type) {
                foreach ($stylesheets[$media_type] as $stylesheet) {
                    $concatenated .= "<link rel='stylesheet' href='$stylesheet' media='$media_type' />";
                }
            }
            self::$stylesheets = array(
              'all' => array(),
              'screen' => array(),
              'print' => array(),
            );
            return $concatenated . "\n";
        }
        
        // Store a new stylesheet link.
        else {
            // Requested stylesheet is already in queue.
            if (in_array($stylesheet, self::$stylesheets[$media])) {
                return;
            }
            self::$stylesheets[$media][] = $stylesheet;
        }
    }

    /**
     * Store or retrieve breadcrumbs.
     * 
     * @param string $title Returns concatenated string of all breadcrumbs if null.
     * @param string $link
     * @return string Concatenated string with all breadcrumbs if $title is null.
     */
    public static function breadcrumbs($title = null, $link = null) {
        // Fetch data.
        if ($title === null) {
            $result = '<nav id="breadcrumbs"><ul>';
            foreach (self::$breadcrumbs as $crumb) {
                $result .= '<li><a href="' . $crumb['link'] . '">' . $crumb['title'] . '</a></li>';
            }
            $result .= '</ul></nav>';
            return $result;
        }
        
        // Store a new breadcrumb.
        else {
            self::$breadcrumbs[] = array(
              'title' => $title,
              'link' => $link,
            );
        }
    }

    /**
     * Build page request.
     */
    public static function build() {
        $regions = & self::$theme['regions'];
        foreach ($regions as $region => &$content) {
            $content = implode('', $content);
        }
        ob_start();
        include_once(self::$theme_path . '/templates/page.tpl.php');
        self::$content = ob_get_clean();
    }

    /**
     * Finalize page request.
     * 
     * Fetches resources and loads the html template of active theme for later output.
     */
    public static function finalize() {
        $stylesheets = self::stylesheets();
        $javascripts = self::javascripts();
        $classes_html = implode(' ', self::$classes['html']);
        self::$classes['html'] = array();
        ob_start();
        $content = self::$content;
        include_once(self::$theme_path . '/templates/html.tpl.php');
        self::$result = ob_get_clean();
    }

    /**
     * Render page request.
     * 
     * Adds some resources (javascript) before printing result from page request.
     * Closes the session.
     */
    public static function render() {
        // Make javascript aware of uri data.
        self::javascripts(array('uri' => self::$uri), 'settings');
        
        // Make javascript aware of breadcrumbs.
        self::javascripts(array('breadcrumbs' => self::$breadcrumbs), 'settings');
        
        // Inject inline javascript.
        $js_inline = '<script type="text/javascript">/*<![CDATA[*/';
        $js_inline .= 'var _manta_settings = ' . json(self::$javascripts['settings']->dump()) . ';';
        $js_inline .= self::$javascripts['inline'];
        $js_inline .= '/*]]>*/</script>';
        self::$result = str_replace('<head>', "<head>$js_inline", self::$result);
        
        // Close and render.
        session_write_close();
        print self::$result;
    }

    /**
     * Checks if site is under maintenance mode.
     * 
     * If so, stop execution and inform visitor.
     */
    private static function maintenance() {
        // Checks if site is locked by request method, Regular or API.
        if (self::config()->get('maintenance/locked/' . self::$request)) {
            $client_ip_parts = explode('.', ip());
            $client_ip[] = ip();
            $client_ip[] = "{$client_ip_parts[0]}.{$client_ip_parts[1]}.{$client_ip_parts[2]}.%";
            $client_ip[] = "{$client_ip_parts[0]}.{$client_ip_parts[1]}.%.%";
            
            // Check visitor IP address if in list of exceptions.
            // If so, let visitor pass.
            if (is_array(self::config()->get('maintenance/exceptions'))) {
                foreach ($client_ip as $ip) {
                    if (in_array($ip, self::config()->get('maintenance/exceptions'))) {
                        return;
                    }
                }
            }
            
            // Inform visitor, either in Regular way or API method.
            if (self::$request === 'api') {
                if (ob_get_length()) {
                    ob_end_clean();
                }
                exit(json_encode(array(
                  'maintenance' => true,
                  'message' => 'Sidan är offline.',
                )));
            } else {
                manta::message('Site in maintenance mode.', null, KILL_CLEAN);
            }
        }
    }

    /**
     * Initiate page request.
     * 
     * @param array $uri
     * @return boolean
     */
    public static function request(array $uri) {
        self::sanitize_uri($uri);
        if (count($uri['path']) < 1 || !file_exists(PATH_PAGES . "/{$uri['path'][0]}/{$uri['path'][0]}.page.php")) {
            $uri['path'][0] = '404';
        }
        if (!class_exists("page_{$uri['path'][0]}")) {
            include_once(PATH_PAGES . "/{$uri['path'][0]}/{$uri['path'][0]}.page.php");
        }
        call_user_func_array(
            array(new ReflectionClass("page_{$uri['path'][0]}"), 'newInstance'), array($uri));
        return ($uri['path'][0] == '404') ? false : true;
    }

    /**
     * Washes uri request to alphanumeric characters.
     * 
     * @param array $uri
     */
    private static function sanitize_uri(&$uri) {
        foreach ($uri['path'] as &$path) {
            $path = preg_replace('/[^a-z0-9]/i', '_', $path);
        }
    }

}

page::getInstance();
