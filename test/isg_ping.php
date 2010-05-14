#!/usr/local/php-fcgi/bin/php
<?php
/* Create a TCP/IP socket. */
$socket = fsockopen("unix:///tmp/l.sock");

fwrite($socket, "<?xml version=\"1.0\"?>\n");
fwrite($socket, "<query>\n<isg_ping pbhk=\"I4004059\"/>\n</query>\n\n");
fflush($socket);
if (!$socket) {
    return;
} else {
    $result = "";
    while(1) {
	$data = fgets($socket);
	if(!$data) {
    	    break;
	}
	$result .= $data;
    }
    fclose($socket);
    print $result;
}


?>
