<?php
$br = (php_sapi_name() == "cli")? "\n":"<br>";

function device_callback_event_handler($args, $event_type)
{
	global $br;
	echo "device_callback_event_handler() is called $br";
	echo "args: $br";
	var_dump($args);
	var_dump($event_type);
}

/* ########################################################### */
echo "check: upnp_register_root_device()";
echo "$br";
$url = 'http://localhost/upnp/tvdevicedesc.xml';
$callback = 'device_callback_event_handler';
$args = array('device_arg');
var_dump(upnp_register_root_device($url, $callback, $args));
var_dump(upnp_error());
echo "Start sleep $br";
sleep(1);
echo "Stop sleep $br";
/* ########################################################### */


/* ########################################################### */
echo "upnp_send_advertisement()";
echo "$br";
var_dump(upnp_send_advertisement(5));
/* ########################################################### */


/* ########################################################### */
echo "upnp_unregister_root_device()";
echo "$br";
var_dump(upnp_unregister_root_device());
/* ########################################################### */


/* ########################################################### */
echo "check: upnp_register_root_device_ext()";
echo "$br";
$url = 'http://localhost/upnp/tvdevicedesc.xml';
$file = '/var/www/upnp/tvdevicedesc.xml';
$desc = file_get_contents($file);
$callback = 'device_callback_event_handler';
$args = array('device_arg_ext');
var_dump(upnp_register_root_device_ext($desc, $callback, $args, 3));
var_dump(upnp_error());
echo "Start sleep $br";
sleep(1);
echo "Stop sleep $br";
/* ########################################################### */


/* ########################################################### */
echo "upnp_unregister_root_device()";
echo "$br";
var_dump(upnp_unregister_root_device());
/* ########################################################### */

?>
