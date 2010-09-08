#!/usr/bin/php
<?php
    require("../src/web/cachelib.php");
    
    $cache = new ISGCache("utf8", "/tmp/test.sock");


    print_r($cache->sql("SELECT FIRST_NAME, LAST_NAME, MIDDLE_NAME, FIRST_NAME||LAST_NAME||MIDDLE_NAME as S from USERS where ID_USER=:id_user", array('id_user' => 2)));
?>
