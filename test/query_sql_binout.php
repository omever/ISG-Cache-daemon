#!/usr/bin/php
<?php
    require("../src/web/cachelib.php");
    
    $cache = new ISGCache();
    if($cache->connect()) {
	echo "Connect ok\n";
    } else {
	echo "Connect failed\n";
    }
    
    print_r($cache->sql("BEGIN TEST_OM(:OUT); END;"));
    $cache->disconnect();
?>
