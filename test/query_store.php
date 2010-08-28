#!/usr/bin/php
<?php
    require("../src/web/cachelib.php");
    
    $cache = new ISGCache("utf8", "/tmp/test.sock");

    for($i=0; $i< 1000000; $i++)
	$cache->store("hi there".$i, "kingdom", 30);
    
    print_r($cache->restore("hi there123"));
    
?>
