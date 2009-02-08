<?php
$br = (php_sapi_name() == "cli")? "\n":"<br>";

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

echo "upnp_get_server_port()";
echo "$br";
var_dump(upnp_get_server_port());

echo "upnp_get_server_ip_address()";
echo "$br";
var_dump(upnp_get_server_ip_address());

/*
echo "upnp_register_client()";
echo "$br";
var_dump(upnp_register_client());

echo "upnp_unregister_client()";
echo "$br";
var_dump(upnp_unregister_client());
*/

echo "upnp_set_max_content_length()";
echo "$br";
var_dump(upnp_set_max_content_length(512));

echo "upnp_set_webserver_rootdir()";
echo "$br";
var_dump(upnp_set_webserver_rootdir(''));

echo "upnp_register_root_device()";
echo "$br";
var_dump(upnp_register_root_device('tvdevicedesc.xml'));

echo "upnp_unregister_root_device()";
echo "$br";
var_dump(upnp_unregister_root_device());

var_dump(upnp_error());
?>
