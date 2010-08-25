#!/usr/bin/php
<?php
    require("../src/web/cachelib.php");
    
    $cache = new ISGCache("utf8", "/tmp/test.sock");


    print_r($cache->store("hi there", "kingdom", 30));
    print_r($cache->restore("hi there"));
    
?>
