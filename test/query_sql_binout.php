#!/usr/bin/php
<?php
    ini_set('display_errors', 1); 
    require("../src/web/cachelib.php");
    
    $cache = new ISGCache("utf8", "/tmp/test.sock");

    echo "hi there\n";
    print_r($cache->sql("SELECT FIRST_NAME, LAST_NAME, MIDDLE_NAME, FIRST_NAME||LAST_NAME||MIDDLE_NAME as S from USERS wheraasdfe ID_USER=:id_user", array('id_user' => 2)));
    echo "Error: " . $cache->getError() . "\n";
?>
