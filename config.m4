dnl ext-mrloop config.m4 file

dnl PHP_ARG_ENABLE([mrloop],
dnl   [for mrloop support],
dnl   [AS_HELP_STRING([--enable-mrloop],
dnl     [include mrloop support])])

PHP_ARG_WITH([mrloop],
  [for mrloop library],
  [AS_HELP_STRING([--with-mrloop],
    [specify path to mrloop library])],
  [no])

PHP_ARG_WITH([picohttp],
  [for picohttp library],
  [AS_HELP_STRING([--with-picohttp],
    [specify path to picohttp library])],
  [no])

if test "$PHP_MRLOOP" != "no"; then
  dnl add PHP version check
  PHP_VERSION=$($PHP_CONFIG --vernum)
  AC_MSG_CHECKING([PHP version])
  if test $PHP_VERSION -lt 80100; then
    AC_MSG_ERROR([ext-mrloop requires PHP 8.1+])
  else
    AC_MSG_RESULT([ok])
  fi

  HEADER_INSTALL_DIRS="/usr/local/lib /usr/lib"
  URING_OBJ="liburing.so"

  AC_MSG_CHECKING([for liburing object file])
  for iter in $HEADER_INSTALL_DIRS; do
    if test -s "$iter/$URING_OBJ"; then
      URING_SO="$iter/$URING_OBJ"
      AC_MSG_RESULT(found $URING_SO)
    fi
  done

  if test -z "$URING_SO"; then
    AC_MSG_RESULT(liburing is not properly installed)
    AC_MSG_ERROR(Please install liburing)
  fi

  AC_MSG_CHECKING([for mrloop package])
  if test -s "$PHP_MRLOOP/mrloop.c"; then
    AC_MSG_RESULT(found mrloop package)
  else
    AC_MSG_RESULT(mrloop is not downloaded)
    AC_MSG_ERROR(Please download mrloop)
  fi

  AC_MSG_CHECKING([for picohttpparser package])
  if test -s "$PHP_PICOHTTP/picohttpparser.c"; then
    AC_MSG_RESULT(found picohttpparser package)
  else
    AC_MSG_RESULT(picohttpparser is not downloaded)
    AC_MSG_ERROR(Please download picohttpparser)
  fi

  CFLAGS="-g -O3 -luring -I$PHP_MRLOOP/ -I$PHP_PICOHTTP/"
  AC_DEFINE(HAVE_MRLOOP, 1, [ Have mrloop support ])

  PHP_NEW_EXTENSION(mrloop, php_mrloop.c, $ext_shared)
fi
