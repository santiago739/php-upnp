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

static int le_upnp;
static int php_upnp_initialized = 0;
static int php_upnp_error_code = 0;
static UpnpClient_Handle php_upnp_ctrlpt_handle = -1;
static UpnpDevice_Handle php_upnp_device_handle = -1;


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

static int php_upnp_ctrl_point_callback_event_handler(Upnp_EventType EventType, void *Event, void *Cookie) /* {{{ */
{
/*    switch ( EventType ) {
            
    }
*/
    return 0;
}

static int php_upnp_device_callback_event_handler(Upnp_EventType EventType, void *Event, void *Cookie) /* {{{ */
{
/*    switch ( EventType ) {
            
    }
*/
    return 0;
}

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(upnp)
{
	ZEND_INIT_MODULE_GLOBALS(upnp, php_upnp_init_globals, NULL);
	REGISTER_INI_ENTRIES();

	if (UPNP_G(enabled)) {
		/* UpnpInit() should be called once per process */
		php_upnp_initialize(UPNP_G(ip), UPNP_G(port));
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
		php_upnp_terminate();
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
    
    if (ZEND_NUM_ARGS() != 0) {
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
    
    if (ZEND_NUM_ARGS() != 0) {
		return;
	}
	
    if (!php_upnp_initialized) {
		RETURN_FALSE;
	}

	ip = UpnpGetServerIpAddress();
	if (ip) {
		RETURN_STRING(ip, 0);
	}
    RETURN_FALSE;
}
/* }}} */

/* {{{ */
PHP_FUNCTION(upnp_register_client)
{
	if (ZEND_NUM_ARGS() != 0) {
		return;
	}

	if (!php_upnp_initialized) {
		RETURN_FALSE;
	}
	
	php_upnp_error_code = UpnpRegisterClient(php_upnp_ctrl_point_callback_event_handler,
                            &php_upnp_ctrlpt_handle, &php_upnp_ctrlpt_handle);
    if (php_upnp_error_code != UPNP_E_SUCCESS) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Error registering control point: %d", php_upnp_error_code);
        UpnpFinish();
		RETURN_FALSE;
    }

	RETURN_TRUE;
}
/* }}} */

/* {{{ */
PHP_FUNCTION(upnp_unregister_client)
{
	if (ZEND_NUM_ARGS() != 0) {
		return;
	}

	if (!php_upnp_initialized) {
		RETURN_FALSE;
	}

	php_upnp_error_code = UpnpUnRegisterClient(php_upnp_ctrlpt_handle);

	if (php_upnp_error_code != UPNP_E_SUCCESS) {
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ */
PHP_FUNCTION(upnp_set_max_content_length)
{
	size_t length;

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
PHP_FUNCTION(upnp_set_webserver_rootdir)
{
	char* root_dir;
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
PHP_FUNCTION(upnp_register_root_device)
{
	char* desc_url;
	int desc_url_len;

	if (!php_upnp_initialized) {
		RETURN_FALSE;
	}
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &desc_url, &desc_url_len) == FAILURE) {
		return;
	}

	php_upnp_error_code = UpnpRegisterRootDevice(desc_url, php_upnp_device_callback_event_handler,
                            &php_upnp_device_handle, &php_upnp_device_handle);
    if (php_upnp_error_code != UPNP_E_SUCCESS) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Error registering the rootdevice: %d", php_upnp_error_code);
        UpnpFinish();
		RETURN_FALSE;
    }

	RETURN_TRUE;
}
/* }}} */

/* {{{ */
PHP_FUNCTION(upnp_unregister_root_device)
{
	if (ZEND_NUM_ARGS() != 0) {
		return;
	}

	if (!php_upnp_initialized) {
		RETURN_FALSE;
	}

	php_upnp_error_code = UpnpUnRegisterRootDevice(php_upnp_device_handle);

	if (php_upnp_error_code != UPNP_E_SUCCESS) {
		RETURN_FALSE;
	}
	RETURN_TRUE;
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
	PHP_FE(upnp_set_max_content_length, NULL)
	PHP_FE(upnp_set_webserver_rootdir, NULL)
	PHP_FE(upnp_register_root_device, NULL)
	PHP_FE(upnp_unregister_root_device, NULL)
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
