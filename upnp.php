<?php
$br = (php_sapi_name() == "cli")? "\n":"<br>";

/*
if(!extension_loaded('upnp')) {
	dl('upnp.' . PHP_SHLIB_SUFFIX);
}
$module = 'upnp';
$functions = get_extension_funcs($module);
echo "Functions available in the test extension:$br";
foreach($functions as $func) {
    echo $func."$br";
}
echo "$br";
*/

function ctrl_point_callback_event_handler($args, $event_type)
{
	global $br;
	echo "ctrl_point_callback_event_handler() is called $br";
	echo "args: $br";
	var_dump($args);
	var_dump($event_type);
}

function device_callback_event_handler($args, $event_type)
{
	global $br;
	echo "device_callback_event_handler() is called $br";
	echo "args: $br";
	var_dump($args);
	var_dump($event_type);
}

echo "check: upnp_get_server_port() $br";
var_dump(upnp_get_server_port());



echo "check: upnp_get_server_ip_address() $br";
var_dump(upnp_get_server_ip_address());


/*
echo "check: upnp_register_client() $br";
$callback = 'ctrl_point_callback_event_handler';
$args = array('register_client');
var_dump(upnp_register_client($callback, $args));

echo "Start sleep $br";
sleep(10);
echo "Stop sleep $br";


echo "check: upnp_search_async() $br";
$args = array('search_async');
var_dump(upnp_search_async(5, "urn:schemas-upnp-org:device:tvdevice:1", $args));
echo "Start sleep $br";
sleep(10);
echo "Stop sleep $br";


echo "check: upnp_unregister_client() $br";
var_dump(upnp_unregister_client());
*/



//echo "check: upnp_set_max_content_length() $br";
//var_dump(upnp_set_max_content_length(512));



//echo "check: upnp_set_webserver_rootdir() $br";
//var_dump(upnp_set_webserver_rootdir('/tmp'));





echo "check: upnp_register_root_device()";
echo "$br";
$url = 'http://localhost/upnp/tvdevicedesc.xml';
$callback = 'device_callback_event_handler';
$args = array('device_arg');
var_dump(upnp_register_root_device($url, $callback, $args));
var_dump(upnp_error());
//echo "Start sleep $br";
sleep(10);
//echo "Stop sleep $br";



echo "upnp_unregister_root_device()";
echo "$br";
var_dump(upnp_unregister_root_device());


var_dump(upnp_error());
?>
