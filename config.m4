dnl $Id$
PHP_ARG_WITH(upnp, for UPnP support,
[  --with-upnp             Include UPnP support])

if test "$PHP_UPNP" != "no"; then
  SEARCH_PATH="/usr/local /usr"
  SEARCH_FOR="/include/upnp/upnp.h"

  dnl looking for the headers
  
  if test -r $PHP_UPNP/; then
    AC_MSG_CHECKING([for UPnP headers in $PHP_UPNP])
    if test -r $PHP_UPNP/$SEARCH_FOR; then
      UPNP_DIR=$PHP_UPNP
      AC_MSG_RESULT(found in $PHP_UPNP)
    fi
  else
    AC_MSG_CHECKING([for UPnP headers in default path])
    for i in $SEARCH_PATH ; do
      if test -r $i/$SEARCH_FOR; then
        UPNP_DIR=$i
        AC_MSG_RESULT(found in $i)
      fi
    done
  fi
  
  if test -z "$UPNP_DIR"; then
    AC_MSG_RESULT([not found])
    AC_MSG_ERROR([Please reinstall UPnP distribution])
  fi

  dnl add $UPNP_DIR/include to include path
  PHP_ADD_INCLUDE($UPNP_DIR/include)

  LIBNAME=upnp
  LIBSYMBOL=UpnpGetServerPort

  dnl check if the lib is correct and use --with-libdir to specify libdir name (lib/lib64)
  PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  [
    PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $UPNP_DIR/$PHP_LIBDIR, UPNP_SHARED_LIBADD)
    AC_DEFINE(HAVE_UPNPLIB,1,[ ])
  ],[
    AC_MSG_ERROR([wrong UPnP lib version or lib not found])
  ],[
    -L$UPNP_DIR/$PHP_LIBDIR -lm -ldl
  ])
  
  PHP_SUBST(UPNP_SHARED_LIBADD)
  PHP_NEW_EXTENSION(upnp, upnp.c, $ext_shared)
fi
