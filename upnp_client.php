<?php
$br = (php_sapi_name() == "cli")? "\n":"<br>";

function ctrl_point_callback_event_handler($args, $event_type)
{
	global $br;
	echo "ctrl_point_callback_event_handler() is called $br";
	echo "args: $br";
	var_dump($args);
	var_dump($event_type);
}


/* ########################################################### */
echo "check: upnp_register_client() $br";
$callback = 'ctrl_point_callback_event_handler';
$args = array('register_client');
var_dump(upnp_register_client($callback, $args));

echo "Start sleep $br";
sleep(10);
echo "Stop sleep $br";
/* ########################################################### */


/* ########################################################### */
echo "check: upnp_search_async() $br";
$args = array('search_async');
var_dump(upnp_search_async(5, "urn:schemas-upnp-org:device:tvdevice:1", $args));
echo "Start sleep $br";
sleep(10);
echo "Stop sleep $br";
/* ########################################################### */


/* ########################################################### */
echo "check: upnp_unregister_client() $br";
var_dump(upnp_unregister_client());
/* ########################################################### */



var_dump(upnp_error());
?>
