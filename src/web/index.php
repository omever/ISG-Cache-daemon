<?php
ignore_user_abort(1);
ini_set("display_errors", 0);
include ("/u01/app/web/php_new/utils.inc");
//include ("/u01/app/web/php_new/login.inc");
require("../isgcoalib.php");
$session_id = 0;
$arr_isg_services = "";
$url_server = "start.infolada.ru";	# portal.tlt.ru
if (array_key_exists("SID", $_GET)) session_id($_GET['SID']);
session_start();
header("Cache-Control: no-store, no-cache, must-revalidate");	# for HTTP/1.1
header("Pragma: no-cache");					# for HTTP/1.0
header("Connection: close");
header("Proxy-Connection: close");
header("Expires: Thu, 01 Dec 1994 16:00:00 GMT");
$_SESSION["isgid"] = $_SERVER['REMOTE_ADDR'].':'.($_SERVER['REMOTE_PORT'] >> 5);
$url_local = 'http://'.$url_server.'/';#($_SERVER['PHP_SELF']);
if (array_key_exists("HTTP_HOST", $_SERVER) && $_SERVER['HTTP_HOST'] != $url_server)
{
	$url_local .= '?SID='.(session_id());
	header("Refresh: 1; URL=".$url_local);
	if (array_key_exists("HTTPS", $_SERVER)) $url_local = 'https://';
	else $url_local = 'http://';
	$url_local .= $_SERVER['HTTP_HOST'].$_SERVER['REQUEST_URI'];
	$_SESSION["target"] = $url_local;
	exit();
}
# Параметр сессии 'target', существует короткое время, до ПЕРВОЙ АВТОМАТИЧЕСКОЙ ПЕРЕЗАГРУЗКИ страницы.
# Оставлять этот параметр для дальнейшего использования нежелательно, так как возможна подмена идентификатора сессии,
# который передаётся на страницу методом GET.
if (!(empty($_SESSION["target"])))
{
	$url_local = $_SESSION["target"];
	unset($_SESSION["target"]);
}
if (array_key_exists("cmd", $_POST))
{
	if ($_POST["cmd"] == "logout")
	{
		header("Refresh: 3; URL=".($url_local));
		header("Content-Type: text/html; charset=koi8-r");
		echo "<html>
			<head>
			<title>Доступ в Интернет для персонала</title>
			</head>\n";
		echo "<body><center>Сеанс завершён</center></body></html>\n\n";
		flush();
		coa_command_exec("logout", $arr_isg_services);
		return;
	}
	else
	{
		header("Refresh: 2; URL=".($_POST["host"]));
		$login = (array_key_exists("un", $_POST)) ? (preg_replace("/\s/", "", $_POST["un"])) : NULL ;
		$paswd = (array_key_exists("pw", $_POST)) ? (preg_replace("/\s/", "", $_POST["pw"])) : NULL ;
		$session_id = coa_command_exec($_POST["cmd"], $arr_isg_services, $login, $paswd);
	}
}
else
{
	$session_id = coa_command_exec("ping", $arr_isg_services);
	if ($session_id == 0 && is_array($arr_isg_services) && array_key_exists("Framed-Address", $arr_isg_services) && array_key_exists("NAS-Port-Id", $arr_isg_services))
	{
		$session_id = isg_auto_logon($arr_isg_services["Framed-Address"], $arr_isg_services["NAS-Port-Id"], $arr_isg_services);
		if ($session_id != 0) header("Refresh: 2; URL=".$url_local);
	}
}
?>
<?php
require ("/u01/app/web/inc/config.php"); 
include ($inc_path."home/isgmeta.php"); 
include ($inc_path."home/head.php"); 
?>
<table width="100%"  border="0" cellspacing="0" cellpadding="0">
	<tr>
		<td width="250" valign="top">
			<!-- MENU --><? /* include ($inc_path."home/menu.php"); */ ?><!-- END MENU -->
		</td>
		<td width="*%" valign="top">
<style type="text/css">
.cell {background:#F2F4F9;}
.msg, .msg-err, .msg-ok {text-align:left; color: black; padding: 7px;}
.msg-err {color:#990000;}
.msg-ok {color:green;}
.inp {border:1px solid #999999; color:#333333;}
</style>
		<div id="content">
			<!-- CONTENT -->
			<h1>Вход в Интернет</h1>
			<p>Для начала работы сеанса доступа в сеть Интернет, введите имя и пароль<br>пользователя выделенной линии от компании &laquo;ИнфоЛада&raquo; и нажмите кнопку &laquo;Начать сеанс&raquo;.</p>
			<p>Для завершения сеанса нажмите кнопку &laquo;Завершить сеанс&raquo;</p><br>

<?

echo "<table border=0><form method=\"post\" action=\"http://",($url_server),($_SERVER['PHP_SELF']),"\" enctype=\"multipart/form-data\" id=\"ISGform\">\n";
echo "<input type=\"hidden\" name=\"host\" value=\"",$url_local,"\">";
if ($session_id > 0)
{
	if (is_array($arr_isg_services))
	{
		echo "<input type=\"hidden\" name=\"cmd\" value=\"logout\" id=\"CoAcmd\">";
		echo "<input type=\"hidden\" name=\"un\" id=\"ISGsvc\">";
		$cols = 1;
		if (($cols = user_balance_view($session_id)) > 0)
		{
#			echo "<tr class=\"cell\"><td align=\"center\" colspan=3><br></td></tr>\n";
			$arr_auto_services = isg_services_auto($session_id);
			krsort($arr_isg_services, SORT_STRING);

/*
			foreach ($arr_isg_services as $k => $service_status)
			{
				if (!is_array($arr_isg_services[$k])) continue;
				if ($k == 1)
				{
					$arr_all_services = array_intersect($arr_isg_services[1], $arr_auto_services);
					$header_services = "Активированные сервисы";
					$service_button = "<button type=button onClick=\"document.getElementById('CoAcmd').value='off'; document.getElementById('ISGsvc').value='%-s'; document.getElementById('ISGform').submit();\" title=\"Деактивировать сервис\">Отключить</button>";
				}
				elseif ($k == 0)
				{
					$arr_all_services = array_diff($arr_isg_services[0], $arr_isg_services[1]);
					$header_services = "Неактивированные сервисы";
					$service_button = "<button type=button onClick=\"document.getElementById('CoAcmd').value='on'; document.getElementById('ISGsvc').value='%-s'; document.getElementById('ISGform').submit();\" title=\"Активировать сервис\">Подключить</button>";
				}
				else $arr_all_services = array();
				if (count($arr_all_services) > 0)
				{
					echo "<tr class=\"cell\"><td class=\"msg\" colspan=",$cols,"><b>",$header_services,"</b></td></tr>\n";
					sort($arr_all_services);
					foreach ($arr_all_services as $isg_service)
					{
						echo "<tr class=\"cell\">
						<td width=\"50%\">",$isg_service,"</td>";
						echo "<td align=\"center\" colspan=",($cols-1),">";
						printf($service_button, $isg_service);
						echo "</td></tr>\n";
					}
				}
			}
*/
		}
		
		echo "<tr class=\"cell\"><td align=\"center\" colspan=",$cols,">
			<input type=\"submit\" value=\"Завершить сеанс\"></td></tr>\n";
	}
	else echo "<tr class=\"cell\"><td class=\"msg\">",$arr_isg_services,"</td></tr>\n";
}
elseif ($session_id < 0)
{
	echo "<tr class=\"cell\"><td class=\"msg-err\"><b>",$arr_isg_services,"</b></td></tr>\n";
}
else
{
	echo "<input type=\"hidden\" name=\"cmd\" value=\"loggon\">";
	echo "<tr>";
	# При подключении/отключении сервисов, функция coa_command_exec ВСЕГДА возвращает число больше нуля.
	if (array_key_exists("un", $_POST))
	{
		echo "<tr><td class=\"msg-err\"><b>Ошибка авторизации!</b></td></tr>\n";
	}
	else
	{
#		echo "<tr class=\"cell\">
#			<td class=\"msg\" colspan=2>Для выхода в интернет, введите имя и пароль.</td>
#		</tr>\n";
		echo "<tr>
			<td align=\"right\"><b>Имя:</b>&nbsp;&nbsp;&nbsp;</td>
			<td align=\"left\"><input type=\"text\" name=\"un\" size=15 maxlength=128></td>
		</tr>
		<tr>
			<td align=\"right\"><b>Пароль:</b>&nbsp;&nbsp;&nbsp;</td>
			<td align=\"left\"><input type=\"password\" name=\"pw\" size=15 maxlength=128></td>
		</tr>
		<tr>
			<td align=\"left\"><br></td>
			<td align=\"left\"><input type=\"submit\" value=\"Начать сеанс\"></td>
		</tr>\n";
	}
}
echo "</table></form>\n";
?>
	<!--br /><b>ВНИМАНИЕ!<br />С 01.02.2009г. изменяются параметры действия тарифных планов Интернет для клиентов физических лиц.</b><br />
	<a href="http://start.infolada.ru/information.php">Подробнее...</a-->
	</td></tr>
</table>
<? include ($inc_path."footer_homenet.php"); ?>

</body>
</html>
