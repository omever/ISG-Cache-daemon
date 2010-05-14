#!/usr/local/bin/php
<?php
/* Create a TCP/IP socket. */
$socket = fsockopen("unix:///tmp/test.sock");

fwrite($socket, "<?xml version=\"1.0\"?>\n");
fwrite($socket, "<query>\n<status pbhk=\"S217.113.122.2\"/>\n</query>\n\n");
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
    return $result;
}


?>
