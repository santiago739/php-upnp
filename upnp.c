/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2007 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

/* $Id: header,v 1.16.2.1.2.1 2007/01/01 19:32:09 iliaa Exp $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_upnp.h"

#include "upnp/upnp.h"

ZEND_DECLARE_MODULE_GLOBALS(upnp)

typedef struct _php_upnp_callback_struct { /* {{{ */
    zval *func;
    zval *arg;
} php_upnp_callback_struct;
/* }}} */

static int le_upnp_discovery;

static int php_upnp_initialized = 0;
static int php_upnp_error_code = 0;

static UpnpClient_Handle php_upnp_ctrlpt_handle = -1;
static UpnpDevice_Handle php_upnp_device_handle = -1;

static int php_upnp_callback_mutex = 1;


#ifdef COMPILE_DL_UPNP
ZEND_GET_MODULE(upnp)
#endif

/* {{{ PHP_INI */
PHP_INI_BEGIN()
	STD_PHP_INI_ENTRY("upnp.ip", NULL, PHP_INI_ALL, OnUpdateString, ip, zend_upnp_globals, upnp_globals)
	STD_PHP_INI_ENTRY("upnp.port", "0", PHP_INI_ALL, OnUpdateLong, port, zend_upnp_globals, upnp_globals)
	STD_PHP_INI_ENTRY("upnp.enabled", "0", PHP_INI_ALL, OnUpdateBool, enabled, zend_upnp_globals, upnp_globals)
PHP_INI_END()
/* }}} */

static void php_upnp_init_globals(zend_upnp_globals *upnp_globals) /* {{{ */
{
	upnp_globals->ip = NULL;
	upnp_globals->port = 0;
}

/* }}} */

static void php_upnp_callback_event_free(php_upnp_callback_struct *callback) /* {{{ */
{
	if (!callback) {
		return;
	}

	zval_ptr_dtor(&callback->func);
	if (callback->arg) {
		zval_ptr_dtor(&callback->arg);
	}
	efree(callback);
}
/* }}} */

static const char *php_upnp_err2str(int code) /* {{{ */
{
	switch (code) {
		case UPNP_E_SUCCESS:
			return "no error";
		case UPNP_E_INVALID_HANDLE:
			return "invalid handle";
		case UPNP_E_INVALID_PARAM:
			return "invalid parameter";
		case UPNP_E_OUTOF_HANDLE:
			return "out of handles";
		case UPNP_E_OUTOF_MEMORY:
			return "out of memory";
		case UPNP_E_INIT:
			return "UPnP has already been initialized";
		case UPNP_E_BUFFER_TOO_SMALL:
			return "buffer too small";
		case UPNP_E_INVALID_DESC:
			return "invalid description document";
		case UPNP_E_INVALID_URL:
			return "invalid URL";
		case UPNP_E_INVALID_SID:
			return "invalid SID";
		case UPNP_E_INVALID_DEVICE:
			return "invalid device";
		case UPNP_E_INVALID_SERVICE:
			return "invalide service";
		case UPNP_E_BAD_RESPONSE:
			return "bad response";
		case UPNP_E_BAD_REQUEST:
			return "bad request";
		case UPNP_E_INVALID_ACTION:
			return "invalid action";
		case UPNP_E_FINISH:
			return "UPnP is not initialized yet";
		case UPNP_E_INIT_FAILED:
			return "initialization failed";
		case UPNP_E_URL_TOO_BIG:
			return "URL too big";
		case UPNP_E_BAD_HTTPMSG:
			return "invalid HTTP message";
		case UPNP_E_ALREADY_REGISTERED:
			return "client or device is already registered";
		case UPNP_E_NETWORK_ERROR:
			return "network error";
		case UPNP_E_SOCKET_WRITE:
			return "error writing to a socket";
		case UPNP_E_SOCKET_READ:
			return "error reading from a socket";
		case UPNP_E_SOCKET_BIND:
			return "socket bind failed";
		case UPNP_E_SOCKET_CONNECT:
			return "socket connect failed";
		case UPNP_E_OUTOF_SOCKET:
			return "out of sockets";
		case UPNP_E_LISTEN:
			return "listen failed";
		case UPNP_E_TIMEDOUT:
			return "timed out";
		case UPNP_E_SOCKET_ERROR:
			return "socket error";
		case UPNP_E_FILE_WRITE_ERROR:
			return "file write error";
		case UPNP_E_CANCELED:
			return "operation was cancelled";
		case UPNP_E_EVENT_PROTOCOL:
			return "event protocol error";
		case UPNP_E_SUBSCRIBE_UNACCEPTED:
			return "subcription rejected";
		case UPNP_E_UNSUBSCRIBE_UNACCEPTED:
			return "unsubscription rejected";
		case UPNP_E_NOTIFY_UNACCEPTED:
			return "notification rejected";
		case UPNP_E_INVALID_ARGUMENT:
			return "invalid argument";
		case UPNP_E_FILE_NOT_FOUND:
			return "file not found";
		case UPNP_E_FILE_READ_ERROR:
			return "file read error";
		case UPNP_E_EXT_NOT_XML:
			return "wrong file extension (not .xml)";
		case UPNP_E_NO_WEB_SERVER:
			return "no web server";
		case UPNP_E_OUTOF_BOUNDS:
			return "out of bounds";
		case UPNP_E_NOT_FOUND:
			return "invalid SOAP response";
		case UPNP_E_INTERNAL_ERROR:
			return "internal error occured";
		case UPNP_SOAP_E_INVALID_ACTION:
			return "SOAP invalid action";
		case UPNP_SOAP_E_INVALID_ARGS:
			return "SOAP invalid arguments";
		case UPNP_SOAP_E_OUT_OF_SYNC:
			return "SOAP out of sync";
		case UPNP_SOAP_E_INVALID_VAR:
			return "SOAP invalid var";
		case UPNP_SOAP_E_ACTION_FAILED:
			return "SOAP action failed";
		default:
			return "unknown UPnP error code returned";
	}
}
/* }}} */

static int php_upnp_initialize(const char *ip, unsigned short port) /* {{{ */
{
	int res;

	if (php_upnp_initialized) {
		return UPNP_E_SUCCESS;
	}

	res = UpnpInit(ip, port);
	if (res == UPNP_E_SUCCESS) {
		php_upnp_initialized = 1;
	}
	return res;
}
/* }}} */

static int php_upnp_terminate(void) /* {{{ */
{
	int res;

	if (php_upnp_initialized) {
		res = UpnpFinish();
	} else {
		return UPNP_E_SUCCESS;
	}

	if (res == UPNP_E_SUCCESS) {
		php_upnp_initialized = 0;
	}
	return res;
}
/* }}} */

static const char *php_upnp_get_event_type_name(Upnp_EventType EventType) /* {{{ */
{
	switch (EventType) {
		case UPNP_DISCOVERY_ADVERTISEMENT_ALIVE:
			return "UPNP_DISCOVERY_ADVERTISEMENT_ALIVE";
		case UPNP_DISCOVERY_ADVERTISEMENT_BYEBYE:
			return "UPNP_DISCOVERY_ADVERTISEMENT_BYEBYE";
		case UPNP_DISCOVERY_SEARCH_RESULT:
			return "UPNP_DISCOVERY_SEARCH_RESULT";
		case UPNP_DISCOVERY_SEARCH_TIMEOUT:
			return "UPNP_DISCOVERY_SEARCH_TIMEOUT";
		case UPNP_CONTROL_ACTION_REQUEST:
			return "UPNP_CONTROL_ACTION_REQUEST";
		case UPNP_CONTROL_ACTION_COMPLETE:
			return "UPNP_CONTROL_ACTION_COMPLETE";
		case UPNP_CONTROL_GET_VAR_REQUEST:
			return "UPNP_CONTROL_GET_VAR_REQUEST";
		case UPNP_CONTROL_GET_VAR_COMPLETE:
			return "UPNP_CONTROL_GET_VAR_COMPLETE";
		case UPNP_EVENT_SUBSCRIPTION_REQUEST:
			return "UPNP_EVENT_SUBSCRIPTION_REQUEST";
		case UPNP_EVENT_RECEIVED:
			return "UPNP_EVENT_RECEIVED";
		case UPNP_EVENT_RENEWAL_COMPLETE:
			return "UPNP_EVENT_RENEWAL_COMPLETE";
		case UPNP_EVENT_SUBSCRIBE_COMPLETE:
			return "UPNP_EVENT_SUBSCRIBE_COMPLETE";
		case UPNP_EVENT_UNSUBSCRIBE_COMPLETE:
			return "UPNP_EVENT_UNSUBSCRIBE_COMPLETE";
		case UPNP_EVENT_AUTORENEWAL_FAILED:
			return "UPNP_EVENT_AUTORENEWAL_FAILED";
		case UPNP_EVENT_SUBSCRIPTION_EXPIRED:
			return "UPNP_EVENT_SUBSCRIPTION_EXPIRED";
		default:
			return "unknown event type";
    }
}
/* }}} */

static void php_upnp_event_discovery_dtor(zend_rsrc_list_entry *rsrc TSRMLS_DC) 
{ 
    struct Upnp_Discovery *d_event = (struct Upnp_Discovery *)rsrc->ptr;
	
	efree(d_event); 
	//pefree(d_event, 1); 
}

static int php_upnp_callback_event_handler(Upnp_EventType EventType, void *Event, void *Cookie) /* {{{ */
{
	zval *args[3];
	zval retval;
	php_upnp_callback_struct *callback = (php_upnp_callback_struct *)Cookie;
	
	if (!callback || (php_upnp_callback_mutex == 0)) {
		return 1;
	}
	
	php_upnp_callback_mutex = 0;
		
	args[0] = callback->arg;
	args[0]->refcount++;
	
	MAKE_STD_ZVAL(args[1]);
	//ZVAL_STRING(args[1], estrdup(php_upnp_get_event_type_name(EventType)), 1);
	ZVAL_LONG(args[1], EventType); 
	
	MAKE_STD_ZVAL(args[2]);
	
	switch (EventType) {
		case UPNP_DISCOVERY_ADVERTISEMENT_BYEBYE:
		case UPNP_DISCOVERY_ADVERTISEMENT_ALIVE:
		case UPNP_DISCOVERY_SEARCH_RESULT:
		{
			struct Upnp_Discovery *d_event, *d_event_copy;
			char *hash_key;
			int hash_key_len, hash_exists = 0;
			list_entry *existing_event; 
			
			d_event = (struct Upnp_Discovery *)Event;
			
			hash_key_len = spprintf(&hash_key, 0, 
            	"UpnpDiscoveryEvent:%s", d_event->Location); 
			if (zend_hash_find(&EG(persistent_list), hash_key, 
					hash_key_len + 1, (void **)&existing_event) == SUCCESS) { 
				ZEND_REGISTER_RESOURCE(args[2], existing_event->ptr, le_upnp_discovery); 
				efree(hash_key); 
				hash_exists = 1;
			} 
			
			if (!hash_exists)
			{
				if (!d_event) { 
					return 1;
				} 
				
				d_event_copy = emalloc(sizeof(struct Upnp_Discovery));
				*d_event_copy = *d_event;
				
				list_entry le; 

				ZEND_REGISTER_RESOURCE(args[2], d_event_copy, le_upnp_discovery); 

				le.type = le_upnp_discovery; 
				le.ptr = d_event_copy; 

				zend_hash_update(&EG(persistent_list), hash_key, hash_key_len + 1, 
					(void*)&le, sizeof(list_entry), NULL); 

				efree(hash_key); 
			}
			
			break;
		}
			
		case UPNP_DISCOVERY_SEARCH_TIMEOUT:
			break;
			
		case UPNP_EVENT_SUBSCRIBE_COMPLETE:
		case UPNP_EVENT_UNSUBSCRIBE_COMPLETE:
		case UPNP_EVENT_RENEWAL_COMPLETE:
		{
			struct Upnp_Event_Subscribe *es_event = (struct Upnp_Event_Subscribe *)Event;
			
			ZVAL_STRING(args[2], estrdup(es_event->Sid), 1);
			
			break;
		}
			
		default:
			ZVAL_STRING(args[2], estrdup("other EventType recieved"), 0);
		break;
	}

	if (call_user_function(EG(function_table), NULL, callback->func, &retval, 3, args TSRMLS_CC) == SUCCESS) {
		zval_dtor(&retval);
	}
	
	zval_ptr_dtor(&(args[0]));
	zval_ptr_dtor(&(args[1])); 
	zval_ptr_dtor(&(args[2])); 
		
	php_upnp_callback_mutex = 1;
	
	return 0;
}
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(upnp)
{
	ZEND_INIT_MODULE_GLOBALS(upnp, php_upnp_init_globals, NULL);
	REGISTER_INI_ENTRIES();

	//if (UPNP_G(enabled)) {
		/* UpnpInit() should be called once per process */
		php_upnp_initialize(UPNP_G(ip), UPNP_G(port));
	//}
	
	le_upnp_discovery = zend_register_list_destructors_ex(
							NULL, php_upnp_event_discovery_dtor, "UPNP Event", 
							module_number); 
	
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(upnp)
{
	/* shut it down if initialized */
	if (UPNP_G(enabled)) {
		php_upnp_terminate();
		//php_upnp_callback_event_free();
	}

	UNREGISTER_INI_ENTRIES();
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(upnp)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "UPnP support", "enabled");
	php_info_print_table_end();

	DISPLAY_INI_ENTRIES();
}
/* }}} */

/* {{{ */
PHP_FUNCTION(upnp_errcode)
{
	RETURN_LONG(php_upnp_error_code);
}
/* }}} */

/* {{{ */
PHP_FUNCTION(upnp_error)
{
	char* error = NULL;
	error = estrdup(php_upnp_err2str(php_upnp_error_code));
	RETURN_STRING(error, 0);
}
/* }}} */

/* {{{ */
PHP_FUNCTION(upnp_get_server_port) 
{
	unsigned short port;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "") == FAILURE) {
		return;
	}

	if (!php_upnp_initialized) {
		RETURN_FALSE;
	}
	
	port = UpnpGetServerPort();
	RETURN_LONG(port);
}
/* }}} */

/* {{{ */
PHP_FUNCTION(upnp_get_server_ip_address) 
{
	char *ip = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "") == FAILURE) {
		return;
	}
	
    if (!php_upnp_initialized) {
		RETURN_FALSE;
	}

	ip = UpnpGetServerIpAddress();
	if (ip) {
		RETURN_STRING(ip, 1);
	}
	RETURN_FALSE;
}
/* }}} */

/* {{{ */
PHP_FUNCTION(upnp_register_client)
{
	zval *zcallback, *zarg;
	char *callback_name;
	php_upnp_callback_struct *callback;

	if (!php_upnp_initialized) {
		RETURN_FALSE;
	}
	
	if ((php_upnp_ctrlpt_handle != -1) || (php_upnp_device_handle != -1)) {
		RETURN_FALSE;
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz", &zcallback, &zarg) == FAILURE) {
		return;
	}

	if (!zend_is_callable(zcallback, 0, &callback_name)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "'%s' is not a valid callback", callback_name);
		efree(callback_name);
		RETURN_FALSE;
	}
	efree(callback_name); 
	
	
	zval_add_ref(&zcallback);
	if (zarg) {
		zval_add_ref(&zarg);
	} else {
		ALLOC_INIT_ZVAL(zarg);
	}

	callback = emalloc(sizeof(php_upnp_callback_struct));
	callback->func = zcallback;
	callback->arg = zarg;
	
	
	php_upnp_error_code = UpnpRegisterClient(php_upnp_callback_event_handler,
							callback, &php_upnp_ctrlpt_handle);
	
	if (php_upnp_error_code != UPNP_E_SUCCESS) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Error registering control point: %d", php_upnp_error_code);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ */
PHP_FUNCTION(upnp_unregister_client)
{
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "") == FAILURE) {
		return;
	}

	if (!php_upnp_initialized) {
		RETURN_FALSE;
	}
	
	if (php_upnp_ctrlpt_handle == -1) {
		RETURN_FALSE;
	}

	php_upnp_error_code = UpnpUnRegisterClient(php_upnp_ctrlpt_handle);

	if (php_upnp_error_code != UPNP_E_SUCCESS) {
		RETURN_FALSE;
	}
	
	php_upnp_ctrlpt_handle = -1; 
	
	RETURN_TRUE;
}
/* }}} */

/* {{{ */
PHP_FUNCTION(upnp_register_root_device)
{
	zval *zcallback, *zarg;
	char *callback_name, *desc_url;
	int desc_url_len;
	php_upnp_callback_struct *callback;
	
	if (!php_upnp_initialized) {
		RETURN_FALSE;
	}
	
	if ((php_upnp_ctrlpt_handle != -1) || (php_upnp_device_handle != -1)) {
		RETURN_FALSE;
	}
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "szz", &desc_url, &desc_url_len, &zcallback, &zarg) == FAILURE) {
		return;
	}
	
	if (!zend_is_callable(zcallback, 0, &callback_name)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "'%s' is not a valid callback", callback_name);
		efree(callback_name);
		RETURN_FALSE;
	}
	efree(callback_name); 

	zval_add_ref(&zcallback);
	if (zarg) {
		zval_add_ref(&zarg);
	} else {
		ALLOC_INIT_ZVAL(zarg);
	}
	
	/*
	php_upnp_callback = emalloc(sizeof(php_upnp_callback_struct));
	php_upnp_callback->callback = zcallback;
	php_upnp_callback->arg = zarg; 
	*/
	
	callback = emalloc(sizeof(php_upnp_callback_struct));
	callback->func = zcallback;
	callback->arg = zarg;

	php_upnp_error_code = UpnpRegisterRootDevice(desc_url, php_upnp_callback_event_handler,
							callback, &php_upnp_device_handle);
	if (php_upnp_error_code != UPNP_E_SUCCESS) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Error registering the rootdevice: %d", php_upnp_error_code);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ */
PHP_FUNCTION(upnp_register_root_device_ext)
{
	zval *zcallback, *zarg;
	char *callback_name, *desc;
	int desc_len, config_base_url;
	long desc_type, buffer_len = 0;
	Upnp_DescType upnp_desc_type;
	php_upnp_callback_struct *callback;
	
	if (!php_upnp_initialized) {
		RETURN_FALSE;
	}
	
	if ((php_upnp_ctrlpt_handle != -1) || (php_upnp_device_handle != -1)) {
		RETURN_FALSE;
	}
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "szz|l", &desc, &desc_len, &zcallback, &zarg, &desc_type) == FAILURE) {
		return;
	}
	
	if (!zend_is_callable(zcallback, 0, &callback_name)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "'%s' is not a valid callback", callback_name);
		efree(callback_name);
		RETURN_FALSE;
	}
	efree(callback_name); 

	zval_add_ref(&zcallback);
	if (zarg) {
		zval_add_ref(&zarg);
	} else {
		ALLOC_INIT_ZVAL(zarg);
	}
	
	/*
	php_upnp_callback = emalloc(sizeof(php_upnp_callback_struct));
	php_upnp_callback->callback = zcallback;
	php_upnp_callback->arg = zarg; 
	*/
	
	callback = emalloc(sizeof(php_upnp_callback_struct));
	callback->func = zcallback;
	callback->arg = zarg;
	
	switch (desc_type) {
		case 1:
			upnp_desc_type = UPNPREG_URL_DESC;
			config_base_url = 0;
			break;
		case 2:
			upnp_desc_type = UPNPREG_FILENAME_DESC;
			config_base_url = 1;
			break;
		case 3:
			upnp_desc_type = UPNPREG_BUF_DESC;
			buffer_len = desc_len;
			config_base_url = 1;
			break;
		default:
			upnp_desc_type = UPNPREG_URL_DESC;
			config_base_url = 0;
	}

	php_upnp_error_code = UpnpRegisterRootDevice2(upnp_desc_type, desc, 
							buffer_len, config_base_url, 
							php_upnp_callback_event_handler,
							callback, &php_upnp_device_handle);
	
	if (php_upnp_error_code != UPNP_E_SUCCESS) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Error registering the rootdevice: %s (error code: %d)", 
						 php_upnp_err2str(php_upnp_error_code), php_upnp_error_code);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ */
PHP_FUNCTION(upnp_unregister_root_device)
{
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "") == FAILURE) {
		return;
	}

	if (!php_upnp_initialized) {
		RETURN_FALSE;
	}
	
	if (php_upnp_device_handle == -1) {
		RETURN_FALSE;
	}

	php_upnp_error_code = UpnpUnRegisterRootDevice(php_upnp_device_handle);

	if (php_upnp_error_code != UPNP_E_SUCCESS) {
		RETURN_FALSE;
	}
	
	php_upnp_device_handle = -1;
	
	RETURN_TRUE;
}
/* }}} */

/* {{{ */
PHP_FUNCTION(upnp_set_max_content_length)
{
	long length;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &length) == FAILURE) {
		return;
	}

	php_upnp_error_code = UpnpSetMaxContentLength(length);
	
	if (php_upnp_error_code != UPNP_E_SUCCESS) {
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ */
PHP_FUNCTION(upnp_search_async)
{
	char *target = NULL;
	int time_mx, target_len;
	zval *zcallback, *zarg;
	char *callback_name;
	php_upnp_callback_struct *callback;
	
	if (!php_upnp_initialized) {
		RETURN_FALSE;
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "lszz", &time_mx, &target, &target_len, &zcallback, &zarg) == FAILURE) {
  		return;
  	}
	
	if (!zend_is_callable(zcallback, 0, &callback_name)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "'%s' is not a valid callback", callback_name);
		efree(callback_name);
		RETURN_FALSE;
	}
	efree(callback_name); 
	
	
	zval_add_ref(&zcallback);
	if (zarg) {
		zval_add_ref(&zarg);
	} else {
		ALLOC_INIT_ZVAL(zarg);
	}

	callback = emalloc(sizeof(php_upnp_callback_struct));
	callback->func = zcallback;
	callback->arg = zarg;
	
	php_upnp_error_code = UpnpSearchAsync(php_upnp_ctrlpt_handle, time_mx, target, callback);
	
	RETURN_TRUE;
}
/* }}} */

/* {{{ */
PHP_FUNCTION(upnp_send_advertisement)
{
	long exp;
	
	if (php_upnp_device_handle == -1) {
		RETURN_FALSE;
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &exp) == FAILURE) {
		return;
	}

	php_upnp_error_code = UpnpSendAdvertisement(php_upnp_device_handle, exp);
	
	if (php_upnp_error_code != UPNP_E_SUCCESS) {
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ */
PHP_FUNCTION(upnp_set_max_subscriptions)
{
	int max_subsc;
	
	if (php_upnp_device_handle == -1) {
		RETURN_FALSE;
	}
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &max_subsc) == FAILURE) {
		return;
	}

	php_upnp_error_code =  UpnpSetMaxSubscriptions(php_upnp_device_handle, max_subsc);

	if (php_upnp_error_code != UPNP_E_SUCCESS) {
		RETURN_FALSE;
	}
	
	RETURN_TRUE;
}
/* }}} */

/* {{{ */
PHP_FUNCTION(upnp_set_max_subscription_timeout)
{
	int max_time_out;
	
	if (php_upnp_device_handle == -1) {
		RETURN_FALSE;
	}
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &max_time_out) == FAILURE) {
		return;
	}

	php_upnp_error_code =  UpnpSetMaxSubscriptionTimeOut(php_upnp_device_handle, max_time_out);

	if (php_upnp_error_code != UPNP_E_SUCCESS) {
		RETURN_FALSE;
	}
	
	RETURN_TRUE;
}
/* }}} */

/* {{{ */
PHP_FUNCTION(upnp_renew_subscription)
{
	char *subs_id;
	int subs_id_len, time_out;
	
	if (php_upnp_ctrlpt_handle == -1) {
		RETURN_FALSE;
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sl", &subs_id, &subs_id_len, &time_out) == FAILURE) {
		return;
	}

	php_upnp_error_code = UpnpRenewSubscription(php_upnp_ctrlpt_handle, &time_out, subs_id);
	
	if (php_upnp_error_code != UPNP_E_SUCCESS) {
		RETURN_FALSE;
	}
	
	RETURN_TRUE;
}
/* }}} */

/* {{{ */
PHP_FUNCTION(upnp_renew_subscription_async)
{
	char *subs_id, *callback_name;
	int subs_id_len, time_out;
	zval *zcallback, *zarg;
	php_upnp_callback_struct *callback;
	
	if (php_upnp_ctrlpt_handle == -1) {
		RETURN_FALSE;
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "slzz", &subs_id, &subs_id_len, &time_out, &zcallback, &zarg) == FAILURE) {
		return;
	}
	
	if (!zend_is_callable(zcallback, 0, &callback_name)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "'%s' is not a valid callback", callback_name);
		efree(callback_name);
		RETURN_FALSE;
	}
	efree(callback_name); 
	
	zval_add_ref(&zcallback);
	if (zarg) {
		zval_add_ref(&zarg);
	} else {
		ALLOC_INIT_ZVAL(zarg);
	}

	callback = emalloc(sizeof(php_upnp_callback_struct));
	callback->func = zcallback;
	callback->arg = zarg;

	php_upnp_error_code = UpnpRenewSubscriptionAsync(php_upnp_ctrlpt_handle, time_out, subs_id,
							php_upnp_callback_event_handler, callback);
	
	if (php_upnp_error_code != UPNP_E_SUCCESS) {
		RETURN_FALSE;
	}
	
	RETURN_TRUE;
}
/* }}} */

/* {{{ */
PHP_FUNCTION(upnp_subscribe)
{
	char *url;
	int url_len, time_out;
	Upnp_SID subs_id;
	
	if (php_upnp_ctrlpt_handle == -1) {
		RETURN_FALSE;
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sl", &url, &url_len, &time_out) == FAILURE) {
		return;
	}

	php_upnp_error_code = UpnpSubscribe(php_upnp_ctrlpt_handle, url, &time_out, subs_id);
	
	if (php_upnp_error_code != UPNP_E_SUCCESS) {
		RETURN_FALSE;
	}
	
	RETURN_STRING(subs_id, 1);
}
/* }}} */

/* {{{ */
PHP_FUNCTION(upnp_subscribe_async)
{
	char *url, *callback_name;
	int url_len, time_out;
	zval *zcallback, *zarg;
	php_upnp_callback_struct *callback;
	
	if (php_upnp_ctrlpt_handle == -1) {
		RETURN_FALSE;
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "slzz", &url, &url_len, &time_out, &zcallback, &zarg) == FAILURE) {
		return;
	}

	if (!zend_is_callable(zcallback, 0, &callback_name)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "'%s' is not a valid callback", callback_name);
		efree(callback_name);
		RETURN_FALSE;
	}
	efree(callback_name); 
	
	zval_add_ref(&zcallback);
	if (zarg) {
		zval_add_ref(&zarg);
	} else {
		ALLOC_INIT_ZVAL(zarg);
	}

	callback = emalloc(sizeof(php_upnp_callback_struct));
	callback->func = zcallback;
	callback->arg = zarg;
	
	
	php_upnp_error_code = UpnpSubscribeAsync(php_upnp_ctrlpt_handle, 
							url, time_out, php_upnp_callback_event_handler,
							callback);
	
	if (php_upnp_error_code != UPNP_E_SUCCESS) {
		RETURN_FALSE;
	}
	
	RETURN_TRUE;
}
/* }}} */

/* {{{ */
PHP_FUNCTION(upnp_unsubscribe)
{
	char *subs_id;
	int subs_id_len;
	
	if (php_upnp_ctrlpt_handle == -1) {
		RETURN_FALSE;
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &subs_id, &subs_id_len) == FAILURE) {
		return;
	}

	php_upnp_error_code = UpnpUnSubscribe(php_upnp_ctrlpt_handle, subs_id);
	
	if (php_upnp_error_code != UPNP_E_SUCCESS) {
		RETURN_FALSE;
	}
	
	RETURN_TRUE;
}
/* }}} */

/* {{{ */
PHP_FUNCTION(upnp_unsubscribe_async)
{
	char *subs_id, *callback_name;
	int subs_id_len;
	zval *zcallback, *zarg;
	php_upnp_callback_struct *callback;
	
	if (php_upnp_ctrlpt_handle == -1) {
		RETURN_FALSE;
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "szz", &subs_id, &subs_id_len, &zcallback, &zarg) == FAILURE) {
		return;
	}

	if (!zend_is_callable(zcallback, 0, &callback_name)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "'%s' is not a valid callback", callback_name);
		efree(callback_name);
		RETURN_FALSE;
	}
	efree(callback_name); 
	
	zval_add_ref(&zcallback);
	if (zarg) {
		zval_add_ref(&zarg);
	} else {
		ALLOC_INIT_ZVAL(zarg);
	}

	callback = emalloc(sizeof(php_upnp_callback_struct));
	callback->func = zcallback;
	callback->arg = zarg;
	
	
	php_upnp_error_code = UpnpUnSubscribeAsync(php_upnp_ctrlpt_handle, 
							subs_id, php_upnp_callback_event_handler, callback);
	
	if (php_upnp_error_code != UPNP_E_SUCCESS) {
		RETURN_FALSE;
	}
	
	RETURN_TRUE;
}
/* }}} */

/* {{{ */
PHP_FUNCTION(upnp_set_webserver_rootdir)
{
	char *root_dir;
	int root_dir_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &root_dir, &root_dir_len) == FAILURE) {
		return;
	}

	php_upnp_error_code = UpnpSetWebServerRootDir(root_dir);
	
	if (php_upnp_error_code != UPNP_E_SUCCESS) {
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ */
PHP_FUNCTION(upnp_get_resource_data)
{
	int event_type;
	zval *event;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rl", &event, &event_type) == FAILURE) {
		return;
	}
	
	switch (event_type) {
		case UPNP_DISCOVERY_ADVERTISEMENT_BYEBYE:
		case UPNP_DISCOVERY_ADVERTISEMENT_ALIVE:
		case UPNP_DISCOVERY_SEARCH_RESULT:
		{
			struct Upnp_Discovery *d_event;
			
			ZEND_FETCH_RESOURCE(d_event, struct Upnp_Discovery *, &event, -1, "UPNP Event", le_upnp_discovery); 
			
			if (d_event) {
				array_init(return_value); 
				add_assoc_long(return_value, "err_code", d_event->ErrCode);
				add_assoc_long(return_value, "expires", d_event->Expires);
				add_assoc_string(return_value, "device_id", d_event->DeviceId, 1); 
				add_assoc_string(return_value, "device_type", d_event->DeviceType, 1); 
				add_assoc_string(return_value, "service_type", d_event->ServiceType, 1); 
				add_assoc_string(return_value, "service_ver", d_event->ServiceVer, 1); 
				add_assoc_string(return_value, "location", d_event->Location, 1); 
				add_assoc_string(return_value, "os", d_event->Os, 1); 
				add_assoc_string(return_value, "date", d_event->Date, 1); 
				add_assoc_string(return_value, "ext", d_event->Ext, 1); 
				
				return; 
			}
			
			break;
		}
			
		case UPNP_DISCOVERY_SEARCH_TIMEOUT:
			break;
			
		case UPNP_EVENT_SUBSCRIBE_COMPLETE:
		case UPNP_EVENT_UNSUBSCRIBE_COMPLETE:
		case UPNP_EVENT_RENEWAL_COMPLETE:
		{
			struct Upnp_Event_Subscribe *es_event;
			
			//ZVAL_STRING(args[2], estrdup(es_event->Sid), 1);
			
			break;
		}
			
		default:
			break;
	}
	
	
	RETURN_FALSE;
}
/* }}} */

/* {{{ upnp_functions[]
 */
const zend_function_entry upnp_functions[] = {
	PHP_FE(upnp_errcode, NULL)
	PHP_FE(upnp_error, NULL)
	PHP_FE(upnp_get_server_port, NULL)
	PHP_FE(upnp_get_server_ip_address, NULL)
	PHP_FE(upnp_register_client, NULL)
	PHP_FE(upnp_unregister_client, NULL)
	PHP_FE(upnp_register_root_device, NULL)
	PHP_FE(upnp_register_root_device_ext, NULL)	
	PHP_FE(upnp_unregister_root_device, NULL)
	PHP_FE(upnp_set_max_content_length, NULL)
	PHP_FE(upnp_search_async, NULL)
	PHP_FE(upnp_send_advertisement, NULL)
	PHP_FE(upnp_set_max_subscriptions, NULL)
	PHP_FE(upnp_set_max_subscription_timeout, NULL)
	PHP_FE(upnp_renew_subscription, NULL)
	PHP_FE(upnp_renew_subscription_async, NULL)
	PHP_FE(upnp_subscribe, NULL)
	PHP_FE(upnp_subscribe_async, NULL)
	PHP_FE(upnp_unsubscribe, NULL)
	PHP_FE(upnp_unsubscribe_async, NULL)
	PHP_FE(upnp_set_webserver_rootdir, NULL)
	PHP_FE(upnp_get_resource_data, NULL)
	{NULL, NULL, NULL}
};
/* }}} */

/* {{{ upnp_module_entry
 */
zend_module_entry upnp_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"upnp",
	upnp_functions,
	PHP_MINIT(upnp),
	PHP_MSHUTDOWN(upnp),
	NULL,
	NULL,
	PHP_MINFO(upnp),
#if ZEND_MODULE_API_NO >= 20010901
	"0.1",
#endif
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
