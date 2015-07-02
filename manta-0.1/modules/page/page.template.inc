<?php

abstract class page_template {

    public $uri;
    public $name;
    protected $accepted_display_challenge;
    protected $path = array();
    protected $titles = array();

    public function __construct($uri) {
        $this->uri = $uri;
        $this->name = $uri['path'][0];
        page::$classes['html'][] = "page_{$this->name}";
        ob_start();
        $this->challenge_request();
        call_user_func(array($this, 'page'));
        if ($this->accepted_display_challenge) {
            page::$classes['html'][] = "page_display_{$this->accepted_display_challenge}";
            call_user_func(array($this, $this->accepted_display_challenge));
        }
        if ($this->path) {
            $current = $this->path[0];
            if (isset($this->titles[$current['link']])) {
                page::$title = $this->titles[$current['link']];
            }
            $breadcrumbs = array_reverse($this->path);
            foreach ($breadcrumbs as $crumb) {
                if (isset($this->titles[$crumb['link']])) {
                    $crumb['title'] = $this->titles[$crumb['link']];
                }
                page::breadcrumbs($crumb['title'], $crumb['link']);
            }
        }
        if (ob_get_length()) {
            $this->send();
        } else {
            ob_end_clean();
        }
    }

    private function challenge_request() {
        $parts = $this->uri['path'];
        while (count($parts)) {
            $name = 'display__' . implode('__', $parts);
            if (method_exists($this, $name)) {
                if (!$this->accepted_display_challenge) {
                    $this->accepted_display_challenge = $name;
                }
                $this->path[] = array(
                  'name' => $parts[count($parts) - 1],
                  'link' => implode('/', $parts),
                );
            }
            array_pop($parts);
        }
    }

    public function send($region = 'content') {
        $content = ob_get_clean();
        page::$theme['regions'][$region][] = $content;
    }

    public function template($name, $args = array()) {
        $i18n = array();
        if (file_exists(PATH_PAGES . "/{$this->name}/$name.tpl.i18n")) {
            $i18n = parse_ini_file(PATH_PAGES . "/{$this->name}/$name.tpl.i18n");
        }
        extract($args);
        print '<div class="page-' . $this->name . '-template-' . $name . ' clearfix">';
        if ((include(PATH_PAGES . "/{$this->name}/$name.tpl.php")) != 1) {
            dpr('Failed to load page template.', 2);
            log::write(__FILE__, __LINE__, "Failed to load page '{$this->name}' template '$name'.", page::$logname);
        }
        print '</div>';
    }

    public function display($name) {
        $i18n = array();
        if (file_exists(PATH_PAGES . "/{$this->name}/$name.display.i18n")) {
            $i18n = parse_ini_file(PATH_PAGES . "/{$this->name}/$name.display.i18n");
        }
        if ((include(PATH_PAGES . "/{$this->name}/$name.display.php")) != 1) {
            dpr('Failed to load page display.', 2);
            log::write(__FILE__, __LINE__, "Failed to load page '{$this->name}' display '$name'.", page::$logname);
        }
    }

    abstract protected function page();
}
