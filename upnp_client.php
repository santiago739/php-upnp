<?php
$br = (php_sapi_name() == "cli")? "\n":"<br>";

function ctrl_point_callback_event_handler($args, $event_type, $event)
//function ctrl_point_callback_event_handler($args, $event_type)
{
	global $br;
	
	echo "=========================================================$br";
	echo "ctrl_point_callback_event_handler() is called $br";
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

/* ########################################################### */
echo "check: upnp_register_client() $br";
$callback = 'ctrl_point_callback_event_handler';
$args = array('register_client');
var_dump(upnp_register_client($callback, $args));

//echo "Start sleep $br";
//sleep(10);
//echo "Stop sleep $br";
/* ########################################################### */


/* ########################################################### */
echo "check: upnp_search_async() $br";
$callback = 'ctrl_point_callback_event_handler';
$args = array('search_async');
$TvDeviceType = "urn:schemas-upnp-org:device:tvdevice:1";

$rc = upnp_search_async(5, $TvDeviceType, $callback, $args);
var_dump($rc);
//echo "Start sleep $br";
//sleep(10);
//echo "Stop sleep $br";
/* ########################################################### */

while(1)
{

}

/* ########################################################### */
echo "check: upnp_unregister_client() $br";
var_dump(upnp_unregister_client());
/* ########################################################### */



var_dump(upnp_error());
?>
