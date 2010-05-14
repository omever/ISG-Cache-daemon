<?php

class ISGCache
{
    public $socketPath = '/tmp/l.sock';
    private $socket = null;
    private $parser = null;
    private $rv = FALSE;
    private $tmp = 0;
    private $level = 0;
    private $current = null;
    
    function connect()
    {
	if(isset($this) && get_class($this) == 'ISGCache') {
	    $this->socket = fsockopen("unix://" . $this->socketPath);
	    if($this->socket === FALSE) {
		fwrite(2, "Error connecting to cache socket\n");
		return false;
	    }
	    return true;
	} else {
	    fwrite(2, "Connect should be called from instance\n");
	    return false;
	}
    }
    
    function disconnect()
    {
	if(isset($this) && get_class($this) == 'ISGCache') {
	    fclose($this->socket);
	    return true;
	} else {
	    fwrite(2, "Disconnect should be called from instance\n");
	    return false;
	}
    }
    
    function tag_open($parser, $tag, $attributes)
    {
	$this->level++;
	if($this->level == 2) {
	    if($this->rv === FALSE) {
		$this->rv = array();
	    }
	    
	    $this->rv[$this->tmp] = array();
	}
	if($this->level == 3) {
	    $this->rv[$this->tmp][$tag][] = "";
	    $this->current = $tag;
	}
    }
    
    function tag_close($parser, $tag)
    {
	$this->level--;
	if($this->level == 1) {
	    $i = count($this->rv[$this->tmp][$this->current]) - 1;
	    $this->rv[$this->tmp][$this->current][$i] = trim($this->rv[$this->tmp][$this->current][$i]);
	    $this->tmp++;
	    $this->current = null;
	}
    }
    
    function cdata($parser, $cdata)
    {
	if($this->current != null) {
	    $i = count($this->rv[$this->tmp][$this->current]) - 1;
	    $this->rv[$this->tmp][$this->current][$i] .= iconv('utf8', 'koi8-r', $cdata);
	}
    }
    
    function query($xml) 
    {
	fwrite($this->socket, $xml);
	fflush($this->socket);
	
	$this->parser = xml_parser_create("UTF-8");
	xml_parser_set_option($this->parser, XML_OPTION_SKIP_WHITE, 1);
	xml_set_object($this->parser, $this);
	xml_set_element_handler($this->parser, "tag_open", "tag_close");
	xml_set_character_data_handler($this->parser, "cdata");
	
	while(1) {
	    $data = fgets($this->socket);
	    if(!$data) {
    		break;
	    }
	    
	    echo $data;
	    if(xml_parse($this->parser, $data, FALSE) == 0) {
		fwrite(STDERR, "Error parsing result while parsing stream at line ".
				xml_get_current_line_number($this->parser).
				" column ".
				xml_get_current_column_number($this->parser).
				": " . xml_error_string(xml_get_error_code($this->parser)) . "\n");
		fwrite(STDERR, $data);
		$this->rv = FALSE;
		break;
	    }
	}
	xml_parse($this->parser, "", TRUE);
	xml_parser_free($this->parser);
    }

    function getOnlineStatus($pbhk)
    {
	$dom = new DOMDocument("1.0", "UTF-8");
	$dom->formatOutput = true;
	
	$root = $dom->createElement("query");
	$dom->appendChild($root);
	
	$status = $dom->createElement("status");
	$root->appendChild($status);
	
	$attrib = $dom->createAttribute("pbhk");
	$status->appendChild($attrib);
	
	$attrib->appendChild($dom->createTextNode($pbhk));
	
	$this->query($dom->saveXML());

	return $this->rv;
    }

    function getIsgServices($pbhk)
    {
	$dom = new DOMDocument("1.0", "UTF-8");
	$dom->formatOutput = true;
	
	$root = $dom->createElement("query");
	$dom->appendChild($root);
	
	$status = $dom->createElement("get_isg_services");
	$root->appendChild($status);
	
	$attrib = $dom->createAttribute("pbhk");
	$status->appendChild($attrib);
	
	$attrib->appendChild($dom->createTextNode($pbhk));
	
	$this->query($dom->saveXML());

	return $this->rv;
    }

    function isgPing($pbhk)
    {
	$dom = new DOMDocument("1.0", "UTF-8");
	$dom->formatOutput = true;
	
	$root = $dom->createElement("query");
	$dom->appendChild($root);
	
	$status = $dom->createElement("isg_ping");
	$root->appendChild($status);
	
	$attrib = $dom->createAttribute("pbhk");
	$status->appendChild($attrib);
	
	$attrib->appendChild($dom->createTextNode($pbhk));
	
	$this->query($dom->saveXML());

	return $this->rv;
    }

    function isgLoggon($pbhk, $login, $password)
    {
	$dom = new DOMDocument("1.0", "UTF-8");
	$dom->formatOutput = true;
	
	$root = $dom->createElement("query");
	$dom->appendChild($root);
	
	$status = $dom->createElement("isg_logon");
	$root->appendChild($status);
	
	$attrib = $dom->createAttribute("pbhk");
	$status->appendChild($attrib);
	$attrib->appendChild($dom->createTextNode($pbhk));
	
	$attrib = $dom->createAttribute("login");
	$status->appendChild($attrib);
	$attrib->appendChild($dom->createTextNode($login));
	
	$attrib = $dom->createAttribute("password");
	$status->appendChild($attrib);
	$attrib->appendChild($dom->createTextNode($password));
	
	$this->query($dom->saveXML());

	return $this->rv;
    }

    function isgLogout($pbhk)
    {
	$dom = new DOMDocument("1.0", "UTF-8");
	$dom->formatOutput = true;
	
	$root = $dom->createElement("query");
	$dom->appendChild($root);
	
	$status = $dom->createElement("isg_logout");
	$root->appendChild($status);
	
	$attrib = $dom->createAttribute("pbhk");
	$status->appendChild($attrib);
	$attrib->appendChild($dom->createTextNode($pbhk));
	
	$this->query($dom->saveXML());

	return $this->rv;
    }

    function getStaticIP($user_ip, $server_ip)
    {

	$dom = new DOMDocument("1.0", "UTF-8");
	$dom->formatOutput = true;
	
	$root = $dom->createElement("query");
	$dom->appendChild($root);
	
	$status = $dom->createElement("get_static_ip");
	$root->appendChild($status);
	
	$attrib = $dom->createAttribute("ip");
	$status->appendChild($attrib);
	$attrib->appendChild($dom->createTextNode($user_ip));
	
	$attrib = $dom->createAttribute("server");
	$status->appendChild($attrib);
	$attrib->appendChild($dom->createTextNode($server_ip));
	
	$this->query($dom->saveXML());

	return $this->rv;
    }
}

?>