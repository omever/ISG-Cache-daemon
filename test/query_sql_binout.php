#!/usr/bin/php
<?php
    require("../src/web/cachelib.php");
    
    $cache = new ISGCache("utf8", "/tmp/test.sock");


//    print_r($cache->sql("SELECT photo_size, price FROM photo_price order by price"));

    if (0 === "hi there") {
	echo "ПРИВЕТ БЛЯДЕ\n";
    } else {
	echo "АВОТХУЙ\n";
    }
?>
