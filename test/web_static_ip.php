#!/usr/local/bin/php
<?php
    require("../src/web/cachelib.php");
    
    $cache = new ISGCache();
    if($cache->connect()) {
	echo "Connect ok\n";
    } else {
	echo "Connect failed\n";
    }
    
    print_r($cache->getStaticIP("192.168.226.189", "217.113.112.11"));
    $cache->disconnect();
?>
