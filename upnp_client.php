<?php

declare(ticks=1);
pcntl_signal(SIGINT, "ctrl_point_sig_int");

function ctrl_point_sig_int($signal)
{
	fputs(STDERR, "\nSignal SIGINT was caught\n");

	echo "=========================================================\n";
	echo "[CALL]: upnp_unregister_client() \n";
	echo "---------------------------------------------------------\n";

	$res = upnp_unregister_client();
	echo "[RESULT]: ";
	var_dump($res);
	echo "=========================================================\n\n\n";

	exit();
}

function ctrl_point_callback_event_handler($args, $event_type, $event)
{
	//$subs_id = 0;
	$services = array();

	$tv_device_type = "urn:schemas-upnp-org:device:tvdevice:1";
	
	echo "=========================================================\n";
	echo "[CALL]: ctrl_point_callback_event_handler() \n";
	echo "---------------------------------------------------------\n";

	echo "args: ";
	print_r($args);

	printf("EventType: %s (%d)\n", upnp_get_event_type_name($event_type), $event_type);
	printf("Event: %s\n", $event);
	
	$event_data = upnp_get_resource_data($event, $event_type);
	echo "event_data: ";
	print_r($event_data);
	
	$location = $event_data['location'];
	printf("EventLocation: %s\n", $location);

	$url = parse_url($location);

	if ($url['scheme'] != 'http')
	{
		echo "=========================================================\n\n\n";
		return false;
	}

	$xml = new SimpleXMLElement($location, NULL, TRUE);
	printf("deviceType: %s\n", $xml->device->deviceType);

	if ($xml->device->deviceType == $tv_device_type)
	{
		foreach ($xml->device->serviceList->service as $key=>$service)
		{
			//$event_sub_url = $xml->device->serviceList->service[0]->eventSubURL;
			$event_sub_url = $service->eventSubURL;
			
			printf("eventSubURL: %s\n", $event_sub_url);
			
			$time_out = 5;
			$services[$key]['subsc_url'] = sprintf('%s://%s:%d%s', $url['scheme'], $url['host'], $url['port'], $event_sub_url);
			

			printf("\nSubscribing to EventURL %s ...\n", $services[$key]['subsc_url']);

			$services[$key]['subs_id'] = ctrl_point_subscribe($services[$key]['subsc_url'], $time_out);

			if ($services[$key]['subs_id'])
			{
				printf("Subscribed to EventURL with SID=%s\n\n", $services[$key]['subs_id']);
				
				//sleep(6);
				/*
				echo "\n[CALL]: upnp_renew_subscription()\n";
				$res = upnp_renew_subscription($subs_id, 5);
				echo "[RESULT]: ";
				var_dump($res);

				sleep(5);

				echo "\n[CALL]: upnp_unsubscribe()\n";
				$res = upnp_unsubscribe($subs_id);
				echo "[RESULT]: ";
				var_dump($res);
				*/
			}
			else
			{
				show_error();
			}
		}
	}	
	
	echo "=========================================================\n\n\n";
}

function ctrl_point_callback_event_handler_async($args, $event_type, $event)
{
	$subs_id = 0;

	$tv_device_type = "urn:schemas-upnp-org:device:tvdevice:1";
	
	echo "=========================================================\n";
	echo "[CALL]: ctrl_point_callback_event_handler_async() \n";
	echo "---------------------------------------------------------\n";

	echo "args: ";
	print_r($args);
	printf("EventType: %s\n", $event_type);
	printf("Event: %s\n", $event);

	//$url = parse_url($event);

	$event_data = upnp_get_resource_data($event, $event_type);
	$location = $event_data['location'];
	printf("EventLocation: %s\n", $location);

	$url = parse_url($location);

	if ($url['scheme'] != 'http')
	{
		echo "=========================================================\n\n\n";
		return false;
	}

	$xml = new SimpleXMLElement($location, NULL, TRUE);
	$event_sub_url = $xml->device->serviceList->service[0]->eventSubURL;
	printf("deviceType: %s\n", $xml->device->deviceType);
	printf("eventSubURL: %s\n", $event_sub_url);

	if (($xml->device->deviceType == $tv_device_type) && ($subs_id == 0))
	{
		$subsc_url = sprintf('%s://%s:%d%s', $url['scheme'], $url['host'], $url['port'], $event_sub_url);
		$time_out = 5;

		printf("Subscribing to EventURL %s ...\n", $subsc_url);

		$res = ctrl_point_subscribe_async($subsc_url, $time_out);

		if ($res)
		{
			printf("Async subscribing...\n");
			var_dump($res);
		}
		else
		{
			show_error();
		}
	}	
		
	
	echo "=========================================================\n\n\n";
}

function ctrl_point_subscribe_callback_event_handler($args, $event_type, $event)
{
	//$subs_id = $event;

	echo "=========================================================\n";
	echo "[CALL]: ctrl_point_subscribe_callback_event_handler() \n";
	echo "---------------------------------------------------------\n";

	echo "args: ";
	print_r($args);
	printf("EventType: %s\n", $event_type);
	printf("Event: %s\n", $event);

	$event_data = upnp_get_resource_data($event, $event_type);
	$subs_id = $event_data['sid'];

	if ($subs_id)
	{
		printf("Subscribed to EventURL with SID=%s\n", $subs_id);
		sleep(6);

		echo "\n[CALL]: upnp_renew_subscription_async()\n";
		$callback = 'ctrl_point_renew_subscribe_callback_event_handler';
		$args = array('renewsubscribe_async');
		$time_out = 5;
		$res = upnp_renew_subscription_async($subs_id, $time_out, $callback, $args);
		echo "[RESULT]: ";
		var_dump($res);

		echo "\n[CALL]: upnp_unsubscribe_async()\n";
		$callback = 'ctrl_point_unsubscribe_callback_event_handler';
		$args = array('unsubscribe_async');
		$res = upnp_unsubscribe_async($subs_id, $callback, $args);
		echo "[RESULT]: ";
		var_dump($res);
	}
	else
	{
		show_error();
	}
	
	echo "=========================================================\n\n\n";
}

function ctrl_point_renew_subscribe_callback_event_handler($args, $event_type, $event)
{
	echo "=========================================================\n";
	echo "[CALL]: ctrl_point_renew_subscribe_callback_event_handler() \n";
	echo "---------------------------------------------------------\n";

	echo "args: ";
	print_r($args);
	printf("EventType: %s\n", $event_type);
	printf("Event: %s\n", $event);

	$event_data = upnp_get_resource_data($event, $event_type);
	echo "event_data: ";
	print_r($event_data);

	echo "=========================================================\n\n\n";
}

function ctrl_point_unsubscribe_callback_event_handler($args, $event_type, $event)
{
	echo "=========================================================\n";
	echo "[CALL]: ctrl_point_unsubscribe_callback_event_handler() \n";
	echo "---------------------------------------------------------\n";

	echo "args: ";
	print_r($args);
	printf("EventType: %s\n", $event_type);
	printf("Event: %s\n", $event);
	
	$event_data = upnp_get_resource_data($event, $event_type);
	echo "event_data: ";
	print_r($event_data);

	echo "=========================================================\n\n\n";
}

function ctrl_point_subscribe($url, $time_out)
{
	echo "\n[CALL]: upnp_subscribe()\n";
	$res = upnp_subscribe($url, $time_out);
	return $res;
	
}

function ctrl_point_subscribe_async($url, $time_out)
{
	$callback = 'ctrl_point_subscribe_callback_event_handler';
	$args = array('subscribe_async');

	echo "\n[CALL]: upnp_subscribe_async()\n";
	$res = upnp_subscribe_async($url, $time_out, $callback, $args);
	return $res;
	
}

function show_error()
{
	echo "\n[ERROR] " . upnp_error() . "\n";
}

/* ########################################################### */
echo "=========================================================\n";
echo "[CALL]: upnp_register_client() \n";
echo "---------------------------------------------------------\n";

$callback = 'ctrl_point_callback_event_handler';
//$callback = 'ctrl_point_callback_event_handler_async';
$args = array('register_client');
$res = upnp_register_client($callback, $args);
echo "[RESULT]: ";
var_dump($res);
if (!$res)
{
	show_error();
	die("Failed to register client\n");
}
echo "=========================================================\n\n\n";
/* ########################################################### */


/* ########################################################### */
/*echo "=========================================================\n";
echo "[CALL]: upnp_search_async() \n";
echo "---------------------------------------------------------\n";

$tv_device_type = "urn:schemas-upnp-org:device:tvdevice:1";
$callback = 'ctrl_point_callback_event_handler';
$args = array('search_async');

$res= upnp_search_async(5, $tv_device_type, $callback, $args);
echo "[RESULT]: ";
var_dump($res);
if (!$res)
{
	show_error();
}
echo "=========================================================\n\n\n";*/
/* ########################################################### */



while(true)
{
	sleep(1);
}





?>
