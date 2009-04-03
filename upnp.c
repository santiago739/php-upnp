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


ZEND_DECLARE_MODULE_GLOBALS(upnp);

ithread_mutex_t DeviceListMutex;

typedef struct _php_upnp_callback_struct { /* {{{ */
    zval *func;
    zval *arg;
#ifdef ZTS
	void ***thread_ctx;
#endif	
} php_upnp_callback_struct;
/* }}} */

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

static void php_upnp_globals_ctor(zend_upnp_globals *upnp_globals TSRMLS_DC) /* {{{ */
{ 
    upnp_globals->ip = NULL;
	upnp_globals->port = 0;
	upnp_globals->ctrlpt_handle = -1;
	upnp_globals->device_handle = -1;
	upnp_globals->error_code = 0;
	upnp_globals->initialized = 0;
	upnp_globals->callbacks_on = 1;
} 
/* }}} */

static void php_upnp_globals_dtor(zend_upnp_globals *upnp_globals TSRMLS_DC) /* {{{ */
{ 
    /* Any resources allocated during initialization 
     * May be freed here */ 
}
/* }}} */

static int php_upnp_initialize(const char *ip, unsigned short port TSRMLS_DC) /* {{{ */
{
	int res;

#ifdef ZTS
php_printf("ZTS enabled\n");
#endif	
#ifndef ZTS
php_printf("ZTS NOT enabled\n");
#endif		
	
	if (UPNP_G(initialized)) {
		return UPNP_E_SUCCESS;
	}
	
	res = UpnpInit(ip, port);
	if (res == UPNP_E_SUCCESS) {
		UPNP_G(initialized) = 1;
	}
	return res;
}
/* }}} */

static int php_upnp_terminate(TSRMLS_D) /* {{{ */
{
	int res;
	
	if (UPNP_G(initialized)) {
		if (UPNP_G(ctrlpt_handle) != -1) {
			UpnpUnRegisterClient(UPNP_G(ctrlpt_handle));
		}
	} 
	
	res = UpnpFinish();

	if (res == UPNP_E_SUCCESS) {
		UPNP_G(initialized) = 0;
	}
	
	return res;
}
/* }}} */

//static void php_upnp_init_globals(zend_upnp_globals *upnp_globals) /* {{{ */
/*{
	upnp_globals->ip = NULL;
	upnp_globals->port = 0;
	upnp_globals->ctrlpt_handle = -1;
	upnp_globals->device_handle = -1;
	upnp_globals->error_code = 0;
	upnp_globals->initialized = 0;
}*/
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

static int php_upnp_callback_event_handler(Upnp_EventType EventType, void *Event, void *Cookie) /* {{{ */
{
	if (!UPNP_G(callbacks_on)) {
		return 1;
	}
	
	ithread_mutex_lock( &DeviceListMutex );
	
	printf("START callback, THREAD: %p, EventType: %d, Event: %p, Cookie: %p\n", pthread_self(), EventType, Event, Cookie);
	printf("EventType: %s (%d)\n", php_upnp_get_event_type_name(EventType), EventType);
	
	zval *args[3];
	zval retval;
	php_upnp_callback_struct *callback = (php_upnp_callback_struct *)Cookie;
	
	TSRMLS_FETCH_FROM_CTX(callback ? callback->thread_ctx : NULL);
	
	if (!callback) {
		return 1;
	}
	
	/*
	MAKE_STD_ZVAL(args[0]);
	ZVAL_LONG(args[1], EventType); 
	
	if (call_user_function(EG(function_table), NULL, callback->func, &retval, 1, args TSRMLS_CC) == SUCCESS) {
		zval_dtor(&retval);
	}
	
	zval_ptr_dtor(&(args[0]));
	*/
	
	/*
	zval *zcallback;
	MAKE_STD_ZVAL(zcallback);     
	ZVAL_STRING(zcallback, "ctrl_point_callback_event_handler", 1); 
	ZEND_SET_SYMBOL(EG(active_symbol_table), "callback", zcallback); 
	
	if (call_user_function(EG(function_table), NULL, zcallback, &retval, 0, NULL TSRMLS_CC) == SUCCESS) {
		zval_dtor(&retval);
	}
	*/
	
	MAKE_STD_ZVAL(args[0]);
	ZVAL_LONG(args[0], EventType); 
	
	MAKE_STD_ZVAL(args[1]);
	array_init(args[1]); 
	
	switch (EventType) {
		case UPNP_DISCOVERY_ADVERTISEMENT_ALIVE:
		case UPNP_DISCOVERY_SEARCH_RESULT:
		case UPNP_DISCOVERY_ADVERTISEMENT_BYEBYE:
		{
			struct Upnp_Discovery *d_event = (struct Upnp_Discovery *)Event;
			
			if (d_event) {
				add_assoc_long(args[1], "err_code", d_event->ErrCode);
				add_assoc_long(args[1], "expires", d_event->Expires);
				add_assoc_string(args[1], "device_id", d_event->DeviceId, 1); 
				add_assoc_string(args[1], "device_type", d_event->DeviceType, 1); 
				add_assoc_string(args[1], "service_type", d_event->ServiceType, 1); 
				add_assoc_string(args[1], "service_ver", d_event->ServiceVer, 1); 
				add_assoc_string(args[1], "location", d_event->Location, 1); 
				add_assoc_string(args[1], "os", d_event->Os, 1); 
				add_assoc_string(args[1], "date", d_event->Date, 1); 
				add_assoc_string(args[1], "ext", d_event->Ext, 1);
			}
			//efree(d_event);
			
			break;
		}
			
		case UPNP_EVENT_SUBSCRIBE_COMPLETE:
		case UPNP_EVENT_UNSUBSCRIBE_COMPLETE:
		case UPNP_EVENT_RENEWAL_COMPLETE:
		{
			struct Upnp_Event_Subscribe *es_event = (struct Upnp_Event_Subscribe *)Event;
			
			if (es_event) {
				add_assoc_long(args[1], "err_code", es_event->ErrCode);
				add_assoc_string(args[1], "publisher_url", es_event->PublisherUrl, 1); 
				add_assoc_string(args[1], "sid", es_event->Sid, 1); 
				add_assoc_long(args[1], "time_out", es_event->TimeOut);
			}
			//efree(es_event);
			
			break;
		}
			
		case UPNP_EVENT_RECEIVED:
		{
			struct Upnp_Event *e_event = (struct Upnp_Event *)Event;
			
			if (e_event) {
				add_assoc_long(args[1], "event_key", e_event->EventKey);
				add_assoc_string(args[1], "sid", e_event->Sid, 1); 
				//add_assoc_string(args[2], "changed_variables", ixmlDocumenttoString(e_event->ChangedVariables), 1);
			}
			//efree(e_event);
			
			break;
		}
			
		case UPNP_CONTROL_ACTION_COMPLETE:
		{
			struct Upnp_Action_Complete *a_event = (struct Upnp_Action_Complete *)Event;
			
			if (a_event) {
				add_assoc_long(args[1], "err_code", a_event->ErrCode);
				add_assoc_string(args[1], "ctrl_url", a_event->CtrlUrl, 1);
				//add_assoc_string(args[2], "action_request", ixmlDocumenttoString(a_event->ActionRequest), 1);
				//add_assoc_string(args[2], "action_result", ixmlDocumenttoString(a_event->ActionResult), 1);
			}
			
			break;
		}
	}
	
	args[2] = callback->arg;
	//args[2]->refcount++;
	//Z_ADDREF_P(callback->arg);
	callback->arg->refcount++;
	
	if (call_user_function(EG(function_table), NULL, callback->func, &retval, 3, args TSRMLS_CC) == SUCCESS) {
		zval_dtor(&retval);
	}
	
	zval_ptr_dtor(&(args[0]));
	zval_ptr_dtor(&(args[1]));
	
	//sleep(5);
	printf("FINISH callback, THREAD: %p, EventType: %d, Event: %p, Cookie: %p\n\n\n", pthread_self(), EventType, Event, Cookie);
	
	ithread_mutex_unlock( &DeviceListMutex );
	
	return 0;
}
/* }}} */

static int php_upnp_try_lock_by_timeout(int timeout, const char* func_name) /* {{{ */
{
	struct timespec delay;
	int irc, time_counter = 0;
	
	while (time_counter < timeout ) {
		delay.tv_sec = 0;
		delay.tv_nsec = 1000000;  /* 1 milli sec */

		irc = pthread_mutex_trylock(&DeviceListMutex);
		if (!irc)  {
			/* we now own the mutex  */
			printf("locked %s, THREAD: %p\n", func_name, pthread_self());
			//break;
			return 0;
		}
		else {
			/* check whether somebody else has the mutex */
			//if (irc == EPERM ) {
			if (irc == EBUSY ) {
				/* sleep for delay time */
				nanosleep(&delay, NULL);
				time_counter++ ;
			} else {
				/* error  */
				printf("failed to lock %s, THREAD: %p\n", func_name, pthread_self());
				return 1;
			}
		}
	}
	printf("failed to lock %s after timeout, THREAD: %p\n", func_name, pthread_self());
	return 1;
	
}
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(upnp)
{
	ithread_mutex_init(&DeviceListMutex, 0);
	
	//ZEND_INIT_MODULE_GLOBALS(upnp, php_upnp_init_globals, NULL);
#ifdef ZTS     
	ts_allocate_id(&upnp_globals_id, 
                sizeof(zend_upnp_globals), 
                (ts_allocate_ctor)php_upnp_globals_ctor, 
                (ts_allocate_dtor)php_upnp_globals_dtor); 
#else 
	php_upnp_globals_ctor(&upnp_globals TSRMLS_CC); 
#endif 
	
	REGISTER_INI_ENTRIES();

	if (UPNP_G(enabled)) {
		/* UpnpInit() should be called once per process */
		php_upnp_initialize(UPNP_G(ip), UPNP_G(port) TSRMLS_CC);
	}
	
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(upnp)
{
	/* shut it down if initialized */
	if (UPNP_G(enabled)) {
		php_upnp_terminate(TSRMLS_C);
	}
	
#ifndef ZTS     
	php_upnp_globals_dtor(&upnp_globals TSRMLS_CC); 
#endif 

	UNREGISTER_INI_ENTRIES();
	
	ithread_mutex_destroy(&DeviceListMutex);
	
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
PHP_FUNCTION(upnp_start_callbacks)
{
	UPNP_G(callbacks_on) = 1;
	RETURN_TRUE;
}
/* }}} */

/* {{{ */
PHP_FUNCTION(upnp_stop_callbacks)
{
	UPNP_G(callbacks_on) = 0;
	RETURN_TRUE;
}
/* }}} */

/* {{{ */
PHP_FUNCTION(upnp_errcode)
{
	RETURN_LONG(UPNP_G(error_code));
}
/* }}} */

/* {{{ */
PHP_FUNCTION(upnp_error)
{
	char* error = NULL;
	error = estrdup(php_upnp_err2str(UPNP_G(error_code)));
	RETURN_STRING(error, 0);
}
/* }}} */

/* {{{ */
PHP_FUNCTION(upnp_get_event_type_name)
{
	long event_type;
	char* event_type_name = NULL;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &event_type) == FAILURE) {
		return;
	}
	
	event_type_name = estrdup(php_upnp_get_event_type_name((Upnp_EventType)event_type));
	
	RETURN_STRING(event_type_name, 0);
}
/* }}} */

/* {{{ */
PHP_FUNCTION(upnp_get_server_port) 
{
	unsigned short port;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "") == FAILURE) {
		return;
	}

	if (!UPNP_G(initialized)) {
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
	
    if (!UPNP_G(initialized)) {
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
	
	if (!UPNP_G(initialized)) {
		RETURN_FALSE;
	}
	
	if ((UPNP_G(ctrlpt_handle) != -1) || (UPNP_G(device_handle) != -1)) {
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
	TSRMLS_SET_CTX(callback->thread_ctx);
	
	UPNP_G(error_code) = UpnpRegisterClient(php_upnp_callback_event_handler,
							callback, &UPNP_G(ctrlpt_handle));
	
	if (UPNP_G(error_code) != UPNP_E_SUCCESS) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Error registering control point: %d", UPNP_G(error_code));
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

	if (UPNP_G(ctrlpt_handle) == -1) {
		RETURN_FALSE;
	}

	UPNP_G(error_code) = UpnpUnRegisterClient(UPNP_G(ctrlpt_handle));

	if (UPNP_G(error_code) != UPNP_E_SUCCESS) {
		RETURN_FALSE;
	}
	
	UPNP_G(ctrlpt_handle) = -1;
	
	RETURN_TRUE;
}
/* }}} */

/* {{{ */
PHP_FUNCTION(upnp_subscribe)
{
	char *url;
	int url_len, time_out;
	Upnp_SID subs_id;
	
	if (UPNP_G(ctrlpt_handle) == -1) {
		RETURN_FALSE;
	}
	
	int try_lock = -1;
	printf("try upnp_subscribe, THREAD: %p\n", pthread_self());
	/*try_lock = pthread_mutex_trylock(&DeviceListMutex);
	printf("try_lock = %d\n", try_lock);
	if (try_lock != 0)
	{
		printf("Failed to lock in upnp_subscribe\n");
		RETURN_FALSE;
	}
	printf("Locked in upnp_subscribe\n");*/

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sl", &url, &url_len, &time_out) == FAILURE) {
		//ithread_mutex_unlock(&DeviceListMutex);
		return;
	}

	UPNP_G(error_code) = UpnpSubscribe(UPNP_G(ctrlpt_handle), url, &time_out, subs_id);
	
	if (UPNP_G(error_code) != UPNP_E_SUCCESS) {
		//ithread_mutex_unlock(&DeviceListMutex);
		RETURN_FALSE;
	}
	//ithread_mutex_unlock(&DeviceListMutex);
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
	
	if (UPNP_G(ctrlpt_handle) == -1) {
		RETURN_FALSE;
	}
	
	int try_lock = -1;
	printf("try upnp_subscribe_async, THREAD: %p\n", pthread_self());
	/*try_lock = pthread_mutex_trylock(&DeviceListMutex);
	printf("try_lock = %d\n", try_lock);
	if (try_lock != 0)
	{
		printf("Failed to lock in upnp_subscribe_async\n");
		RETURN_FALSE;
	}
	printf("Locked in upnp_subscribe_async\n");*/

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "slzz", &url, &url_len, &time_out, &zcallback, &zarg) == FAILURE) {
		//ithread_mutex_unlock(&DeviceListMutex);
		return;
	}

	if (!zend_is_callable(zcallback, 0, &callback_name)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "'%s' is not a valid callback", callback_name);
		efree(callback_name);
		//ithread_mutex_unlock(&DeviceListMutex);
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
	
	UPNP_G(error_code) = UpnpSubscribeAsync(UPNP_G(ctrlpt_handle), 
							url, time_out, php_upnp_callback_event_handler,
							callback);
	
	if (UPNP_G(error_code) != UPNP_E_SUCCESS) {
		//ithread_mutex_unlock(&DeviceListMutex);
		RETURN_FALSE;
	}
	//ithread_mutex_unlock(&DeviceListMutex);
	RETURN_TRUE;
}
/* }}} */

/* {{{ */
PHP_FUNCTION(upnp_unsubscribe)
{
	char *subs_id;
	int subs_id_len;
	
	if (UPNP_G(ctrlpt_handle) == -1) {
		RETURN_FALSE;
	}
	
	printf("try upnp_unsubscribe, THREAD: %p\n", pthread_self());
	if (php_upnp_try_lock_by_timeout(200, "upnp_unsubscribe")) {
		return;	
	}
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &subs_id, &subs_id_len) == FAILURE) {
		ithread_mutex_unlock(&DeviceListMutex);
		return;
	}

	UPNP_G(error_code) = UpnpUnSubscribe(UPNP_G(ctrlpt_handle), subs_id);
	
	if (UPNP_G(error_code) != UPNP_E_SUCCESS) {
		ithread_mutex_unlock(&DeviceListMutex);
		RETURN_FALSE;
	}
	ithread_mutex_unlock(&DeviceListMutex);
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
	
	printf("try upnp_unsubscribe_async, THREAD: %p\n", pthread_self());
	
	if (UPNP_G(ctrlpt_handle) == -1) {
		RETURN_FALSE;
	}
	
	if (php_upnp_try_lock_by_timeout(200, "upnp_unsubscribe_async")) {
		return;	
	}
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "szz", &subs_id, &subs_id_len, &zcallback, &zarg) == FAILURE) {
		return;
	}

	if (!zend_is_callable(zcallback, 0, &callback_name)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "'%s' is not a valid callback", callback_name);
		efree(callback_name);
		ithread_mutex_unlock(&DeviceListMutex);
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
	
	UPNP_G(error_code) = UpnpUnSubscribeAsync(UPNP_G(ctrlpt_handle), 
							subs_id, php_upnp_callback_event_handler, callback);
	
	if (UPNP_G(error_code) != UPNP_E_SUCCESS) {
		ithread_mutex_unlock(&DeviceListMutex);
		RETURN_FALSE;
	}
	ithread_mutex_unlock(&DeviceListMutex);
	RETURN_TRUE;
}
/* }}} */

/* {{{ */
PHP_FUNCTION(upnp_send_action)
{
	char *action_url, *service_type, *action_name, *param_name, *param_val;
	int action_url_len, service_type_len, action_name_len, param_name_len, param_val_len;
	IXML_Document *action_node = NULL, *resp_node = NULL;
	
	if (UPNP_G(ctrlpt_handle) == -1) {
		RETURN_FALSE;
	}
	
	printf("try upnp_send_action, THREAD: %p\n", pthread_self());
	
	if (php_upnp_try_lock_by_timeout(200, "upnp_send_action")) {
		return;	
	}
	
	//int try_lock = -1;
	
	/*try_lock = pthread_mutex_trylock(&DeviceListMutex);
	printf("try_lock = %d\n", try_lock);
	if (try_lock != 0)
	{
		printf("Failed to lock in upnp_send_action\n");
		RETURN_FALSE;
	}
	printf("Locked in upnp_send_action\n"); */
	//ithread_mutex_lock(&DeviceListMutex);
	
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sssss", 
		&action_url, &action_url_len, &service_type, &service_type_len, 
		&action_name, &action_name_len, &param_name, &param_name_len, 
		&param_val, &param_val_len) == FAILURE) {
		ithread_mutex_unlock(&DeviceListMutex);
		return;
	}
	
	if (UpnpAddToAction(&action_node, action_name, service_type, param_name, param_val) != UPNP_E_SUCCESS ) {
		ithread_mutex_unlock(&DeviceListMutex);
		RETURN_FALSE;
	}
	
	if (action_node) {
		UPNP_G(error_code) = UpnpSendAction(UPNP_G(ctrlpt_handle), action_url, 
								service_type, NULL, action_node, &resp_node);
        ixmlDocument_free(action_node);
		
		if (UPNP_G(error_code) == UPNP_E_SUCCESS) {
			ithread_mutex_unlock(&DeviceListMutex);
			//RETURN_STRING(ixmlDocumenttoString(resp_node), 1);
			RETURN_TRUE;
		}
	}
	ithread_mutex_unlock(&DeviceListMutex);
	
	RETURN_FALSE;
}
/* }}} */

/* {{{ */
PHP_FUNCTION(upnp_send_action_async)
{
	char *action_url, *service_type, *action_name, *param_name, *param_val, *callback_name;
	int action_url_len, service_type_len, action_name_len, param_name_len, param_val_len;
	IXML_Document *action_node = NULL;
	zval *zcallback, *zarg;
	php_upnp_callback_struct *callback;
	
	if (UPNP_G(ctrlpt_handle) == -1) {
		RETURN_FALSE;
	}
	
	//int try_lock = -1;
	
	/*try_lock = pthread_mutex_trylock(&DeviceListMutex);
	printf("try_lock = %d\n", try_lock);
	if (try_lock != 0)
	{
		printf("Failed to lock in upnp_send_action\n");
		RETURN_FALSE;
	}
	printf("Locked in upnp_send_action\n");*/
	//printf("START upnp_send_action_async, THREAD: %p\n", pthread_self());
	
	printf("try upnp_send_action_async, THREAD: %p\n", pthread_self());
	
	if (php_upnp_try_lock_by_timeout(200, "upnp_send_action_async")) {
		return;	
	}
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ssssszz", 
		&action_url, &action_url_len, &service_type, &service_type_len, 
		&action_name, &action_name_len, &param_name, &param_name_len, 
		&param_val, &param_val_len, &zcallback, &zarg) == FAILURE) {
		ithread_mutex_unlock(&DeviceListMutex);
		return;
	}
	
	if (!zend_is_callable(zcallback, 0, &callback_name)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "'%s' is not a valid callback", callback_name);
		efree(callback_name);
		ithread_mutex_unlock(&DeviceListMutex);
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
	
	if (UpnpAddToAction(&action_node, action_name, service_type, param_name, param_val) != UPNP_E_SUCCESS ) {
		ithread_mutex_unlock(&DeviceListMutex);
		RETURN_FALSE;
	}
	
	if (action_node) {
		UPNP_G(error_code) = UpnpSendActionAsync(UPNP_G(ctrlpt_handle), action_url, 
								service_type, NULL, action_node, php_upnp_callback_event_handler, callback);
        ixmlDocument_free(action_node);
		
		if (UPNP_G(error_code) == UPNP_E_SUCCESS) {
			ithread_mutex_unlock(&DeviceListMutex);
			RETURN_TRUE;
		}
	}
	
	ithread_mutex_unlock(&DeviceListMutex);
	RETURN_FALSE;
}
/* }}} */

/* {{{ upnp_functions[]
 */
const zend_function_entry upnp_functions[] = {
	PHP_FE(upnp_start_callbacks, NULL)
	PHP_FE(upnp_stop_callbacks, NULL)
	PHP_FE(upnp_errcode, NULL)
	PHP_FE(upnp_error, NULL)
	PHP_FE(upnp_get_event_type_name, NULL)
	PHP_FE(upnp_get_server_port, NULL)
	PHP_FE(upnp_get_server_ip_address, NULL)
	PHP_FE(upnp_register_client, NULL)
	PHP_FE(upnp_unregister_client, NULL)
	PHP_FE(upnp_subscribe, NULL)
	PHP_FE(upnp_subscribe_async, NULL)
	PHP_FE(upnp_unsubscribe, NULL)
	PHP_FE(upnp_unsubscribe_async, NULL)
	PHP_FE(upnp_send_action, NULL)
	PHP_FE(upnp_send_action_async, NULL)
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
