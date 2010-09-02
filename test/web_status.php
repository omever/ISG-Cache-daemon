#!/usr/local/bin/php
<?php
    require("../src/web/cachelib.php");
    
    $cache = new ISGCache();
    if($cache->connect()) {
	echo "Connect ok\n";
    } else {
	echo "Connect failed\n";
    }
    
    print_r($cache->isgPing("217.113.122.7"));
    $cache->disconnect();
?>
