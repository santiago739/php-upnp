<?php
$br = (php_sapi_name() == "cli")? "\n":"<br>";

/* ########################################################### */
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
/* ########################################################### */


/* ########################################################### */
echo "check: upnp_get_server_port() $br";
var_dump(upnp_get_server_port());
/* ########################################################### */


/* ########################################################### */
echo "check: upnp_get_server_ip_address() $br";
var_dump(upnp_get_server_ip_address());
/* ########################################################### */


/* ########################################################### */
echo "check: upnp_set_max_content_length() $br";
var_dump(upnp_set_max_content_length(512));
/* ########################################################### */


/* ########################################################### */
echo "check: upnp_set_webserver_rootdir() $br";
var_dump(upnp_set_webserver_rootdir('/tmp'));
/* ########################################################### */



var_dump(upnp_error());
?>
