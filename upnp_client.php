<?php

global $services, $subscribed;
$services = array();
$subscribed = false;

define('TV_DEVICE_TYPE', 'urn:schemas-upnp-org:device:tvdevice:1');
define('TIME_OUT', 20);

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

function ctrl_point_callback_event_handler($event_type, $event_data, $args)
{
	echo "=========================================================\n";
	echo "[CALL]: ctrl_point_callback_event_handler() \n";
	echo "---------------------------------------------------------\n";
	
	printf("PHP EventType: %s (%d)\n", upnp_get_event_type_name($event_type), $event_type);
	
	/*echo "[EVENT_DATA]: ";
	var_dump($event_data);
	unset($event_type);
	unset($event_data);
	var_dump($event_type);
	var_dump($event_data);*/

	$time_out = -1;
	for ($i=0; $i<=$time_out; $i++) {
		echo ".";
		sleep(1);	
	}
	echo "\n";
	//return;

	

	//echo "[ARGS]: ";
	//var_dump($args);
	
	switch ($event_type)
	{
		case 4:
			ctrl_point_subscribe($event_data);
			break;

		case 9:
			ctrl_point_event_recieved($event_data);
			break;

		default:
			break;
	}
	
	echo "=========================================================\n\n\n";
}

function ctrl_point_subscribe($event_data)
{
	global $services;

	echo "\n[CALL]: ctrl_point_subscribe()\n";

	//sleep(5);
	//exit;

	$location = $event_data['location'];
	printf("EventLocation: %s\n", $location);

	$url = parse_url($location);

	if ($url['scheme'] != 'http')
	{
		return false;
	}

	$xml = new SimpleXMLElement($location, NULL, TRUE);
	printf("deviceType: %s\n", $xml->device->deviceType);

	if ($xml->device->deviceType == TV_DEVICE_TYPE)
	{
		printf("Found Tv device: \n");
	
		$i = 0;
		foreach ($xml->device->serviceList->service as $key=>$service)
		{
			printf("Found service: %s\n", $service->serviceType);
			if ($services[$i]['subscribed'] == 'yes') {
				printf("Already subscribed\n");
				$i++;
				continue;
			}
			if ($services[$i]['subscribed'] == 'in process') {
				printf("Subscribe in process...\n");
				$i++;
				continue;
			}

			$event_sub_url = $service->eventSubURL;
			printf("eventSubURL: %s\n", $event_sub_url);
			$event_control_url = $service->controlURL;
			printf("controlURL: %s\n", $event_control_url);
			$service_host = sprintf('%s://%s:%d', $url['scheme'], $url['host'], $url['port']);

			$services[$i]['service_type'] = $service->serviceType;
			$services[$i]['subsc_url'] = sprintf('%s%s', $service_host, $event_sub_url);
			$services[$i]['control_url'] = sprintf('%s%s', $service_host, $event_control_url);
			
			printf("\nSubscribing to EventURL %s ...\n", $services[$i]['subsc_url']);
			//sleep(2); // ctrl-c -> segfault 

			if (!($i % 2))
			{
				$services[$i]['subs_id'] = upnp_subscribe($services[$i]['subsc_url'], TIME_OUT);

				if ($services[$i]['subs_id'])
				{
					printf("Subscribed to EventURL with SID=%s\n\n", $services[$i]['subs_id']);
					$services[$i]['subscribed'] = 'yes';
				}
				else
				{
					show_error();
					$services[$i]['subscribed'] = 'no';
				}
			}
			else
			{
				$callback = 'ctrl_point_subscribe_callback_event_handler';
				$args = array('subscribe_async', 'index' => $i);

				$res = upnp_subscribe_async($services[$i]['subsc_url'], TIME_OUT, $callback, $args);

				if ($res)
				{
					printf("Async subscribing...\n");
					$services[$i]['subscribed'] = 'in process';
					var_dump($res);
				}
				else
				{
					$services[$i]['subscribed'] = 'no';
					show_error();
				}
			}
			$i++;
		}
	}
}

function ctrl_point_subscribe_callback_event_handler($event_type, $event_data, $args)
{
	echo "=========================================================\n";
	echo "[CALL]: ctrl_point_subscribe_callback_event_handler() \n";
	echo "---------------------------------------------------------\n";

	global $services;

	printf("PHP EventType: %s (%d)\n", upnp_get_event_type_name($event_type), $event_type);
	
	echo "event_data: ";
	print_r($event_data);

	echo "args: ";
	print_r($args);

	if ($event_data['sid'])
	{
		$services[$args['index']]['subs_id'] = $event_data['sid'];
		$services[$args['index']]['subscribed'] = 'yes';
		printf("Subscribed to EventURL with SID=%s\n", $event_data['sid']);
	}
	else
	{
		$services[$args['index']]['subscribed'] = 'no';
		show_error();
	}
	
	echo "=========================================================\n\n\n";
}

function ctrl_point_unsubscribe($ind, $async=false)
{
	echo "=========================================================\n";
	echo "[CALL]: ctrl_point_unsubscribe($ind, $async) \n";
	echo "---------------------------------------------------------\n";

	global $services;

	if ($async) {
		echo "\n[CALL]: upnp_unsubscribe_async()\n";
		$callback = 'ctrl_point_unsubscribe_callback_event_handler';
		$args = array('unsubscribe_async', 'index' => $ind);
		$res = upnp_unsubscribe_async($services[$ind]['subs_id'], $callback, $args);
		if ($res)
		{
			printf("Async unsubscribing...\n");
			$services[$ind]['subscribed'] = 'in process';
			var_dump($res);
		}
		else
		{
			$services[$ind]['subscribed'] = 'no';
			show_error();
		}
	} else {
		echo "\n[CALL]: upnp_unsubscribe()\n";

		$res = upnp_unsubscribe($services[$ind]['subs_id']);
		if ($res)
		{
			printf("Unsubscribed from EventURL with SID=%s\n\n", $services[$ind]['subs_id']);
			$services[$ind]['subscribed'] = 'no';
			ctrl_point_search_async();
		}
		else
		{
			show_error();
		}
	}

	echo "=========================================================\n\n\n";
}

function ctrl_point_unsubscribe_callback_event_handler($event_type, $event_data, $args)
{
	echo "=========================================================\n";
	echo "[CALL]: ctrl_point_unsubscribe_callback_event_handler() \n";
	echo "---------------------------------------------------------\n";

	global $services;

	printf("PHP EventType: %s (%d)\n", upnp_get_event_type_name($event_type), $event_type);
	
	echo "event_data: ";
	print_r($event_data);
	if ($event_data['err_code'] == 0)
	{
		$services[$args['index']]['subscribed'] = 'no';
		printf("Unsubscribed from EventURL with SID=%s\n\n", $services[$args['index']]['subs_id']);
	}

	echo "=========================================================\n\n\n";
}

function ctrl_point_renew_subscription($ind, $async=false)
{
	echo "=========================================================\n";
	echo "[CALL]: ctrl_point_renew_subscription($ind, $async) \n";
	echo "---------------------------------------------------------\n";

	global $services;

	if ($async) {
		echo "\n[CALL]: upnp_renew_subscription_async()\n";
		$callback = 'ctrl_point_renew_subscribe_callback_event_handler';
		$args = array('renew_subscribe_async', 'index' => $ind);
		$res = upnp_renew_subscription_async($services[$ind]['subs_id'], TIME_OUT, $callback, $args);
		if ($res)
		{
			printf("Async renew subscription...\n");
			$services[$ind]['subscribed'] = 'in process';
		}
		else
		{
			$services[$ind]['subscribed'] = 'no';
			show_error();
		}
	} else {
		echo "\n[CALL]: upnp_renew_subscription()\n";

		$res = upnp_renew_subscription($services[$ind]['subs_id'], TIME_OUT);
		if ($res)
		{
			printf("Renew subscription from EventURL with SID=%s\n\n", $services[$ind]['subs_id']);
			$services[$ind]['subscribed'] = 'yes';
		}
		else
		{
			show_error();
		}
	}
	echo "=========================================================\n\n\n";
}

function ctrl_point_renew_subscribe_callback_event_handler($event_type, $event_data, $args)
{
	echo "=========================================================\n";
	echo "[CALL]: ctrl_point_renew_subscribe_callback_event_handler() \n";
	echo "---------------------------------------------------------\n";
	
	global $services;

	printf("PHP EventType: %s (%d)\n", upnp_get_event_type_name($event_type), $event_type);
	
	echo "event_data: ";
	print_r($event_data);
	if ($event_data['err_code'] == 0)
	{
		$services[$args['index']]['subscribed'] = 'no';
		printf("Renew subscription from EventURL with SID=%s\n\n", $services[$args['index']]['subs_id']);
	}

	echo "=========================================================\n\n\n";
}

function ctrl_point_get_var_status($ind, $param_name, $async=false)
{
	echo "=========================================================\n";
	echo "[CALL]: ctrl_point_get_var_status($ind, $param_name, $async) \n";
	echo "---------------------------------------------------------\n";

	global $services;

	if ($async) {
		echo "\n[CALL]: upnp_get_service_var_status_async()\n";
		$callback = 'ctrl_point_get_service_var_status_callback_event_handler';
		$args = array('get_service_var_status_async', 'index' => $ind);
		$res = upnp_get_service_var_status_async($services[$ind]['control_url'], $param_name, $callback, $args);
		if ($res)
		{
			printf("Async get service var status...\n");
			var_dump($res);
		}
		else
		{
			show_error();
		}
	} else {
		echo "\n[CALL]: upnp_get_service_var_status()\n";

		$res = upnp_get_service_var_status($services[$ind]['control_url'], $param_name);
	
		if ($res)
		{
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

function ctrl_point_get_service_var_status_callback_event_handler($event_type, $event_data, $args)
{
	echo "=========================================================\n";
	echo "[CALL]: ctrl_point_get_service_var_status_callback_event_handler() \n";
	echo "---------------------------------------------------------\n";
	
	global $services;

	printf("PHP EventType: %s (%d)\n", upnp_get_event_type_name($event_type), $event_type);
	
	echo "event_data: ";
	print_r($event_data);
	/*
	if ($event_data['err_code'] == 0)
	{
		$services[$args['index']]['subscribed'] = 'no';
		printf("Renew subscription from EventURL with SID=%s\n\n", $services[$args['index']]['subs_id']);
	}
	*/

	echo "=========================================================\n\n\n";
}

function ctrl_point_send_action($ind, $action_name, $param_name, $param_val, $async=false)
{
	echo "=========================================================\n";
	echo "[CALL]: ctrl_point_send_action($ind, $action_name, $param_name, $param_val, $async) \n";
	echo "---------------------------------------------------------\n";

	global $services;

	if ($async) {
		echo "\n[CALL]: upnp_send_action_async()\n";
		$callback = 'ctrl_point_send_action_callback_event_handler';
		$args = array('send_action_async', 'index' => $ind);
		$res = upnp_send_action_async($services[$ind]['control_url'], $services[$ind]['service_type'], 
					$action_name, $param_name, $param_val, $callback, $args);
		if ($res)
		{
			printf("Async send action...\n");
		}
		else
		{
			show_error();
		}
	} else {
		echo "\n[CALL]: upnp_send_action()\n";

		$res = upnp_send_action($services[$ind]['control_url'], $services[$ind]['service_type'], 
					$action_name, $param_name, $param_val);
		if ($res)
		{
			echo "result: ";
			print_r($res);
			printf("Action %s with %s=%s was sended \n\n", $action_name, $param_name, $action_name);
		}
		else
		{
			show_error();
		}
	}

	echo "=========================================================\n\n\n";
}

function ctrl_point_send_action_callback_event_handler($event_type, $event_data, $args)
{
	echo "=========================================================\n";
	echo "[CALL]: ctrl_point_send_action_callback_event_handler() \n";
	echo "---------------------------------------------------------\n";
	
	global $services;

	printf("PHP EventType: %s (%d)\n", upnp_get_event_type_name($event_type), $event_type);
	
	echo "event_data: ";
	print_r($event_data);
/*
	if ($event_data['err_code'] == 0)
	{
		$services[$args['index']]['subscribed'] = 'no';
		printf("Renew subscription from EventURL with SID=%s\n\n", $services[$args['index']]['subs_id']);
	}
*/
	echo "=========================================================\n\n\n";
}

function ctrl_point_event_recieved($event_data)
{
	echo "\n[CALL]: ctrl_point_event_recieved()\n";

	global $services;

	echo "event_data: ";
	print_r($event_data);

	foreach ($services as $key=>$service)
	{
		if ($service['subs_id'] == $event_data['sid'])
		{
			$services[$key]['changed_variables'] = $event_data['changed_variables'];
			break;
		}
	}
}

function ctrl_point_search_async()
{
	echo "=========================================================\n";
	echo "[CALL]: ctrl_point_search_async() \n";
	echo "---------------------------------------------------------\n";

	echo "\n[CALL]: upnp_search_async()\n";
	$callback = 'ctrl_point_callback_event_handler';
	$args = array('search_async');
	$res = upnp_search_async(TIME_OUT, TV_DEVICE_TYPE, $callback, $args);
	if ($res)
	{
		printf("Async search...\n");
	}
	else
	{
		show_error();
	}
	

	echo "=========================================================\n\n\n";
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

$callback = 'ctrl_point_callback_event_handler';
$args = array('search_async');

$res= upnp_search_async(20, TV_DEVICE_TYPE, $callback, $args);
echo "[RESULT]: ";
var_dump($res);
if (!$res)
{
	show_error();
}
echo "=========================================================\n\n\n";*/
/* ########################################################### */

//sleep(10);

while(true) {
	sleep(20);
	
	//sleep(10);
	ctrl_point_renew_subscription(0);
	//sleep(1);

	ctrl_point_send_action(0, "SetVolume", "Volume", 6);

	ctrl_point_get_var_status(0, "SetVolume");

	sleep(20);
	ctrl_point_unsubscribe(0);

	ctrl_point_renew_subscription(1, true);

	sleep(5);
	ctrl_point_send_action(1, "SetColor", "Color", 3, true);

	sleep(10);
	ctrl_point_get_var_status(1, "Color", true);

	sleep(10);

	ctrl_point_unsubscribe(1, true);
	
}





?>
