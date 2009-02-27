<?php
$br = (php_sapi_name() == "cli")? "\n":"<br>";

$callback = 'ctrl_point_callback_event_handler';
$TvDeviceType = "urn:schemas-upnp-org:device:tvdevice:1";

function ctrl_point_callback_event_handler($args, $event_type, $event)
{
	global $br;
	
	echo "=========================================================$br";
	echo "[CALL]: ctrl_point_callback_event_handler() $br";
	echo "---------------------------------------------------------$br";

	echo "args: ";
	print_r($args);
	echo $br;

	echo "event_type: ";
	echo $event_type;
	echo $br;

	echo "event: ";
	echo $event;
	echo $br;
	
	echo "=========================================================$br";
	echo $br.$br;
}

function show_error()
{
	global $br;
	echo '[ERROR]: ' . upnp_error() . $br;
}

/* ########################################################### */
echo "[CALL]: upnp_register_client() $br";
$args = array('register_client');
$res = upnp_register_client($callback, $args);
echo "[RESULT]: ";
var_dump($res);
if (!$res)
{
	show_error();
	die('Failed to register client');
}
/* ########################################################### */


/* ########################################################### */
//echo "check: upnp_search_async() $br";
$args = array('search_async');

//$rc = upnp_search_async(5, $TvDeviceType, $callback, $args);
//var_dump($rc);
/* ########################################################### */

while(1)
{

}

/* ########################################################### */
echo "check: upnp_unregister_client() $br";
var_dump(upnp_unregister_client());
/* ########################################################### */



var_dump();
?>
