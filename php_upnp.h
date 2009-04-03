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

#ifndef PHP_UPNP_H
#define PHP_UPNP_H

#include "upnp/upnp.h"
#include "upnp/upnptools.h"
#include "upnp/ithread.h"

extern zend_module_entry upnp_module_entry;
#define phpext_upnp_ptr &upnp_module_entry

#ifdef ZTS
#include "TSRM.h"
#endif

ZEND_BEGIN_MODULE_GLOBALS(upnp)
	char *ip;
	long port;
	zend_bool enabled;
	UpnpClient_Handle ctrlpt_handle;
	UpnpDevice_Handle device_handle;
	int error_code;
	int initialized;
	int callbacks_on;
ZEND_END_MODULE_GLOBALS(upnp)

#ifdef ZTS
#define UPNP_G(v) TSRMG(upnp_globals_id, zend_upnp_globals *, v)
#else
#define UPNP_G(v) (upnp_globals.v)
#endif

#endif	/* PHP_UPNP_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
