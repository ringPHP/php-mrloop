/* mrloop extension for PHP (c) 2024 Lochemem Bruno Michael */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "src/loop.c"
#include "php_mrloop.h"
#include "mrloop_arginfo.h"

/* {{{ proto Mrloop Mrloop::init() */
PHP_METHOD(Mrloop, init)
{
  php_mrloop_create(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ proto void Mrloop::run() */
PHP_METHOD(Mrloop, run)
{
  php_mrloop_run(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ proto void Mrloop::stop() */
PHP_METHOD(Mrloop, stop)
{
  php_mrloop_stop(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ proto void Mrloop::addTimer( float interval [, callable callback ] ) */
PHP_METHOD(Mrloop, addTimer)
{
  php_mrloop_add_timer(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ proto void Mrloop::addPeriodicTimer( float interval [, callable callback ] ) */
PHP_METHOD(Mrloop, addPeriodicTimer)
{
  php_mrloop_add_periodic_timer(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ proto void Mrloop::tcpServer( int port [, callable callback ] ) */
PHP_METHOD(Mrloop, tcpServer)
{
  php_mrloop_tcp_server_listen(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ proto void Mrloop::addSignal( int signal [, callable callback ] ) */
PHP_METHOD(Mrloop, addSignal)
{
  php_mrloop_add_signal(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ proto void Mrloop::addReadStream( resource stream [, ?int nbytes = null [, callable callback ]] ) */
PHP_METHOD(Mrloop, addReadStream)
{
  php_mrloop_add_read_stream(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ proto void Mrloop::addWriteStream( resource stream [, string contents [, callable callback ]] ) */
PHP_METHOD(Mrloop, addWriteStream)
{
  php_mrloop_add_write_stream(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ proto void Mrloop::writev( int fd [, string contents ] ) */
PHP_METHOD(Mrloop, writev)
{
  php_mrloop_writev(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ proto void Mrloop::futureTick( callable callback ) */
PHP_METHOD(Mrloop, futureTick)
{
  php_mrloop_add_future_tick(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ PHP_MINIT_FUNCTION */
PHP_MINIT_FUNCTION(mrloop)
{
  zend_class_entry ce, exception_ce;

  INIT_NS_CLASS_ENTRY(ce, "ringphp", "Mrloop", class_Mrloop_methods);
  INIT_CLASS_ENTRY(exception_ce, "MrloopException", NULL);

  php_mrloop_ce = zend_register_internal_class(&ce);
  php_mrloop_ce->create_object = php_mrloop_create_object;

  memcpy(&php_mrloop_object_handlers, zend_get_std_object_handlers(), sizeof(php_mrloop_object_handlers));
  php_mrloop_object_handlers.free_obj = php_mrloop_free_object;

#ifdef HAVE_SPL
  php_mrloop_exception_ce = zend_register_internal_class_ex(&exception_ce, spl_ce_RuntimeException);
#else
  php_mrloop_exception_ce = zend_register_internal_class_ex(&exception_ce, zend_exception_get_default());
#endif

  return SUCCESS;
}
/* }}} */

/* {{{ PHP_GINIT_FUNCTION */
static PHP_GINIT_FUNCTION(mrloop)
{
#if defined(ZTS) && defined(COMPILE_DL_MRLOOP)
  ZEND_TSRMLS_CACHE_UPDATE();
#endif

  memset(mrloop_globals, 0, sizeof(*mrloop_globals));
}
/* }}} */

/* {{{ PHP_RINIT_FUNCTION */
PHP_RINIT_FUNCTION(mrloop)
{
#if defined(ZTS) && defined(COMPILE_DL_MRLOOP)
  ZEND_TSRMLS_CACHE_UPDATE();
#endif

  return SUCCESS;
}
/* }}} */

/* {{{ PHP_RSHUTDOWN_FUNCTION */
PHP_RSHUTDOWN_FUNCTION(mrloop)
{
  if (MRLOOP_G(tcp_cb))
  {
    efree(MRLOOP_G(tcp_cb));
  }

  if (MRLOOP_G(sigc) > 0)
  {
    for (size_t idx = 0; idx < MRLOOP_G(sigc); idx++)
    {
      efree(MRLOOP_G(sig_cb)[idx]);
    }
  }

  return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION */
PHP_MINFO_FUNCTION(mrloop)
{
  php_info_print_table_start();
  php_info_print_table_header(2, "mrloop support", "enabled");
  php_info_print_table_header(2, "mrloop version", MRLOOP_VERSION);
  php_info_print_table_header(2, "mrloop author", MRLOOP_AUTHOR);
  php_info_print_table_end();
}
/* }}} */

zend_module_entry mrloop_module_entry = {
  STANDARD_MODULE_HEADER,
  "mrloop",                   /* extension name */
  class_Mrloop_methods,       /* zend_function_entry */
  PHP_MINIT(mrloop),          /* PHP_MINIT - module initialization */
  NULL,                       /* PHP_MSHUTDOWN - module shutdown */
  PHP_RINIT(mrloop),          /* PHP_RINIT - request initialization */
  PHP_RSHUTDOWN(mrloop),      /* PHP_RSHUTDOWN - request shutdown */
  PHP_MINFO(mrloop),          /* PHP_MINFO - module information */
  MRLOOP_VERSION,             /* module version */
  PHP_MODULE_GLOBALS(mrloop), /* PHP_MODULE_GLOBALS - module globals */
  PHP_GINIT(mrloop),          /* PHP_GINIT - globals initialization */
  NULL,
  NULL,
  STANDARD_MODULE_PROPERTIES_EX};

#ifdef COMPILE_DL_MRLOOP
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
#endif
ZEND_GET_MODULE(mrloop)
#endif
