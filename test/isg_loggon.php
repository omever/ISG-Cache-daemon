#!/usr/local/bin/php
<?php
/* Create a TCP/IP socket. */
$socket = fsockopen("unix:///tmp/test.sock");

fwrite($socket, "<?xml version=\"1.0\"?>\n");
fwrite($socket, "<query>\n<isg_logon pbhk=\"217.113.118.1:262\" login=\"om\" password=\"12345\"/>\n</query>\n\n");
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
