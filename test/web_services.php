#!/usr/local/bin/php
<?php
    require("../src/web/cachelib.php");
    
    $cache = new ISGCache();
    if($cache->connect()) {
	echo "Connect ok\n";
    } else {
	echo "Connect failed\n";
    }
    
    print_r($cache->getIsgServices("217.113.118.1:2012"));
    $cache->disconnect();
?>
