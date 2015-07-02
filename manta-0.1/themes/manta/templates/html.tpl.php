<!DOCTYPE html>
<!--[if lt IE 7]>      <html lang="<?php print i18n::$language['code'] ?>" class="<?php print $classes_html ?> no-js lt-ie9 lt-ie8 lt-ie7"> <![endif]-->
<!--[if IE 7]>         <html lang="<?php print i18n::$language['code'] ?>" class="<?php print $classes_html ?> no-js lt-ie9 lt-ie8"> <![endif]-->
<!--[if IE 8]>         <html lang="<?php print i18n::$language['code'] ?>" class="<?php print $classes_html ?> no-js lt-ie9"> <![endif]-->
<!--[if IE 9]>         <html lang="<?php print i18n::$language['code'] ?>" class="<?php print $classes_html ?> no-js lt-ie10"> <![endif]-->
<!--[if gt IE 9]><!--> <html lang="<?php print i18n::$language['code'] ?>" class="<?php print $classes_html ?> no-js"> <!--<![endif]-->
    <head>
        <meta charset="utf-8" />
        <meta http-equiv="X-UA-Compatible" content="IE=edge,chrome=1">
        <meta name="msapplication-config" content="none" />
        <title><?php print page::$title ? page::$title . ' - ' : ''  ?><?php print manta::$config['site_name'] ?></title>
        <meta name="description" content="">
        <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0">
        <meta http-equiv="cache-control" content="max-age=0" />
        <meta http-equiv="cache-control" content="no-cache" />
        <meta http-equiv="expires" content="0" />
        <meta http-equiv="expires" content="Tue, 01 Jan 1980 1:00:00 GMT" />
        <meta http-equiv="pragma" content="no-cache" />
        <?php print $stylesheets ?>
        <?php print $javascripts ?>
    </head>
    <body>
        <?php print $content ?>
    </body>
</html>