<?php
require("/u01/app/web/cachelib.php");

function user_balance_view($session_id){
$start_time_session = date("H:i:s d.m");
$arr_user_balance = array();
$arr_balance_param = array("<tr class=\"cell\"><td class=\"msg\">баланс (в руб):</td><td>%.2f</td><td>%.2f</td></tr>",
"<tr class=\"cell\"><td class=\"msg\">трафик (в Мб):</td><td>%.2f</td><td>%.2f</td></tr>",
"<tr class=\"cell\"><td class=\"msg\">время (в сек):</td><td>%-s</td><td>%-s</td></tr>");

$cache = new ISGCache();
if(!$cache->connect()) {
    echo "<tr class=\"cell\"><td class=\"msg\"><b>Извините, данные ещё не получены.</b></td></tr>\n";
    return 0;
}

    $rv = $cache->getOnlineStatus('S'.$session_id);
    $cache->disconnect();

if (count($rv) > 0)
{
	$arr_balance = $rv[0];
	$user_plan_name =$arr_balance["PLAN"][0]; //isg_user_plan($arr_balance["USER_ID"]);
}
else
{
	echo "<tr class=\"cell\"><td class=\"msg\"><b>Извините, данные ещё не получены.</b></td></tr>\n";
	return 0;
}

echo "<tr class=\"cell\"><td class=\"msg\" colspan=3><b>Тарифный план &laquo;",$user_plan_name,"&raquo;</b></td></tr>\n";
echo "<tr class=\"cell\"><td class=\"msg\" style=\"text-align:center;\" colspan=3><b>Счёт</b></td></tr>\n";
echo "<tr class=\"cell\"><td><br></td><td class=\"msg\" valign=\"top\">состояние<br>на начало сессии<br><font size=-1>(на ",($arr_balance["START_TIME"][0]),")</font</td><td class=\"msg\" valign=\"top\">израсходованно<br>в текущей сессии<br><font size=-1>(на ",($arr_balance["LAST_TIME"][0]),")</font></td></tr>\n";
printf("<tr class=\"cell\"><td class=\"msg\">баланс (в руб):</td><td>%.2f</td><td>%.2f</td></tr>", $arr_balance["MB"][0], $arr_balance["MV"][0]);
printf("<tr class=\"cell\"><td class=\"msg\">трафик (в Мб):</td><td>%.2f</td><td>%.2f</td></tr>", $arr_balance["BB"][0], $arr_balance["BV"][0]);
printf("<tr class=\"cell\"><td class=\"msg\">время (в сек):</td><td>%-s</td><td>%-s</td></tr>", $arr_balance["TB"][0], $arr_balance["TV"][0]);
return 3;	# количество столбцов
}

function isg_services_auto($isg_session_id){
    $arr_services = array();
    $cache = new ISGCache();

    if(!$cache->connect()) {
	return $arr_services;
    }

    $rv = $cache->getIsgServices($isg_session_id);
    if(is_array($rv) && count($rv)>0) {
	foreach($rv as $val) {
	    $arr_services[] = $val["ISG_SERVICE_ID"][0];
	}
    }
    $cache->disconnect();
    
    return $arr_services;
}

function isg_auto_logon($user_ip, $server_ip, &$error_strings){
    $cache = new ISGCache();

    if(!$cache->connect()) {
	return $arr_services;
    }

    $rv = $cache->getStaticIP($user_ip, $server_ip);
    $cache->disconnect();
    
    $login = "static_ip_".$user_ip;
    if (count($rv[0]) > 0) 
	$rv = coa_command_exec("loggon", $error_strings, $login, "static_ip");
    return count($rv[0]);
}

function coa_command_exec($coa_command, &$arr_services, $account_name = NULL, $access_string = NULL){
    $arr_octet = explode(".", $_SERVER['REMOTE_ADDR']);
    if (($arr_octet[0] == 217 && $arr_octet[1] == 113 && $arr_octet[2] == 222 && ($arr_octet[3] & 248) == 248)
	|| ($arr_octet[0] == 217 && $arr_octet[1] == 113 && $arr_octet[2] == 118 && ($arr_octet[3] & 252) == 0)
	|| ($arr_octet[0] == 217 && $arr_octet[1] == 113 && $arr_octet[2] == 117 && ($arr_octet[3] & 248) == 0)
	)
    {
	$coa_session_id = $_SERVER['REMOTE_ADDR'].':'.($_SERVER['REMOTE_PORT'] >> 5);
    }
    else $coa_session_id = $_SERVER['REMOTE_ADDR'];

    $retval = -1;
    $cache = new ISGCache();
    if(!$cache->connect()) {
        echo "<tr class=\"cell\"><td class=\"msg\"><b>Извините, данные ещё не получены.</b></td></tr>\n";
        $retval = -1;
    } else {
	if($coa_command == "ping") {
	    $rv = $cache->isgPing($coa_session_id);
	    $retval = $rv[0]["CISCO-COMMAND-CODE"][0];
	    if($retval == 1) {
		$arr_services["Framed-Address"] = $rv[0]["FRAMED-IP-ADDRESS"][0];
		$arr_services["NAS-Port-Id"] = $rv[0]["NAS-PORT-ID"][0];
		$retval = $coa_session_id;
	    } else {
		if(array_key_exists("FRAMED-IP-ADDRESS", $rv[0]) && array_key_exists("NAS-PORT-ID", $rv[0])) {
		    $arr_services["Framed-Address"] = trim($rv[0]["FRAMED-IP-ADDRESS"][0]);

		    $arr_radius = explode(":", trim($rv[0]["NAS-PORT-ID"][0]));
		    if (count($arr_radius) > 1) $i = 1;
		    else $i = 0;
		    $arr_services["NAS-Port-Id"] = $arr_radius[$i];
		}
	    }
	} else if($coa_command == "loggon") {
	    if($account_name) {
		$rv = $cache->isgLoggon($coa_session_id, $account_name, $access_string);
		$retval = $rv[0]["CODE"][0] == "CoA-ACK";
		if($retval) {
		    $arr_services = sprintf("Сеанс авторизован, %-s", $account_name);
		}
	    }
	} else if($coa_command == "logout") {
	    $rv = $cache->isgLogout($coa_session_id);
	    $retval = $rv[0]["CODE"][0] == "CoA-ACK";
	}
	$cache->disconnect();
    }

    return $retval;
}

?>
