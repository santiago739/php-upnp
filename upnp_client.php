<?php
$br = (php_sapi_name() == "cli")? "\n":"<br>";

$callback = 'ctrl_point_callback_event_handler';


function ctrl_point_callback_event_handler($args, $event_type, $event)
{
	$subs_id = 0;

	$tv_device_type = "urn:schemas-upnp-org:device:tvdevice:1";
	
	echo "=========================================================\n";
	echo "[CALL]: ctrl_point_callback_event_handler() \n";
	echo "---------------------------------------------------------\n";

	echo "args: ";
	print_r($args);

	printf("EventType: %s\n", $event_type);
	printf("Event: %s\n", $event);

	$url = parse_url($event);

	if ($url['scheme'] != 'http')
	{
		echo "=========================================================\n\n\n";
		return false;
	}

	$xml = new SimpleXMLElement($event, NULL, TRUE);
	$event_sub_url = $xml->device->serviceList->service[0]->eventSubURL;
	printf("deviceType: %s\n", $xml->device->deviceType);
	printf("eventSubURL: %s\n", $event_sub_url);

	if (($xml->device->deviceType == $tv_device_type) && ($subs_id == 0))
	{
		$subsc_url = sprintf('%s://%s:%d%s', $url['scheme'], $url['host'], $url['port'], $event_sub_url);
		$time_out = 10;

		printf("Subscribing to EventURL %s ...\n", $subsc_url);

		$subs_id = ctrl_point_subscribe($subsc_url, $time_out);

		if ($subs_id)
		{
			printf("Subscribed to EventURL with SID=%s\n", $subs_id);
			sleep(5);
			echo "[CALL]: upnp_unsubscribe()\n";
			$res = upnp_unsubscribe($subs_id);
			echo "[RESULT]: ";
			var_dump($res);
		}
		else
		{
			show_error();
		}
	}			
	
	echo "=========================================================\n\n\n";
}

function ctrl_point_subscribe($url, $time_out)
{
	echo "[CALL]: upnp_subscribe()\n";
	$res = upnp_subscribe($url, $time_out);
	return $res;
	
}

function show_error()
{
	echo "[ERROR] " . upnp_error() . "\n";
}

/* ########################################################### */
echo "=========================================================\n";
echo "[CALL]: upnp_register_client() \n";
echo "---------------------------------------------------------\n";

$args = array('register_client');
$res = upnp_register_client($callback, $args);
echo "[RESULT]: ";
var_dump($res);
if (!$res)
{
	show_error();
	die('Failed to register client');
}
echo "=========================================================\n\n\n";
/* ########################################################### */


/* ########################################################### */
//echo "check: upnp_search_async() $br";
//$args = array('search_async');

//$rc = upnp_search_async(5, $TvDeviceType, $callback, $args);
//var_dump($rc);
/* ########################################################### */




while(true)
{
	sleep(1);
}

/* ########################################################### */
echo "check: upnp_unregister_client() $br";
var_dump(upnp_unregister_client());
/* ########################################################### */



var_dump();
?>
