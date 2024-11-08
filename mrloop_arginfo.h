/* mrloop extension for PHP (c) 2024 Lochemem Bruno Michael */
ZEND_BEGIN_ARG_INFO_EX(arginfo_class_Mrloop_init, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_Mrloop_run, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_Mrloop_stop, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_Mrloop_addTimer, 0, 0, 2)
ZEND_ARG_TYPE_INFO(0, interval, IS_DOUBLE, 0)
ZEND_ARG_TYPE_INFO(0, callback, IS_CALLABLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_Mrloop_addPeriodicTimer, 0, 0, 2)
ZEND_ARG_TYPE_INFO(0, interval, IS_DOUBLE, 0)
ZEND_ARG_TYPE_INFO(0, callback, IS_CALLABLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_Mrloop_tcpServer, 0, 0, 2)
ZEND_ARG_TYPE_INFO(0, port, IS_LONG, 0)
ZEND_ARG_TYPE_INFO(0, callback, IS_CALLABLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_Mrloop_parseHttpRequest, 0, 0, 2)
ZEND_ARG_TYPE_INFO(0, request, IS_STRING, 0)
ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, header_limit, IS_LONG, 0, "100")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_Mrloop_parseHttpResponse, 0, 0, 2)
ZEND_ARG_TYPE_INFO(0, response, IS_STRING, 0)
ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, header_limit, IS_LONG, 0, "100")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_Mrloop_addSignal, 0, 0, 2)
ZEND_ARG_TYPE_INFO(0, signal, IS_LONG, 0)
ZEND_ARG_TYPE_INFO(0, callback, IS_CALLABLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_Mrloop_addReadStream, 0, 0, 3)
ZEND_ARG_TYPE_INFO(0, stream, IS_RESOURCE, 0)
ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, nbytes, IS_LONG, 0, "null")
ZEND_ARG_TYPE_INFO(0, callback, IS_CALLABLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_Mrloop_addWriteStream, 0, 0, 3)
ZEND_ARG_TYPE_INFO(0, stream, IS_RESOURCE, 0)
ZEND_ARG_TYPE_INFO(0, contents, IS_STRING, 0)
ZEND_ARG_TYPE_INFO(0, callback, IS_CALLABLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_Mrloop_writev, 0, 0, 2)
ZEND_ARG_TYPE_INFO(0, fd, IS_LONG, 0)
ZEND_ARG_TYPE_INFO(0, contents, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_METHOD(Mrloop, init);
ZEND_METHOD(Mrloop, stop);
ZEND_METHOD(Mrloop, run);
ZEND_METHOD(Mrloop, addTimer);
ZEND_METHOD(Mrloop, addPeriodicTimer);
ZEND_METHOD(Mrloop, tcpServer);
ZEND_METHOD(Mrloop, parseHttpRequest);
ZEND_METHOD(Mrloop, addSignal);
ZEND_METHOD(Mrloop, addReadStream);
ZEND_METHOD(Mrloop, addWriteStream);
ZEND_METHOD(Mrloop, parseHttpResponse);
ZEND_METHOD(Mrloop, writev);

static const zend_function_entry class_Mrloop_methods[] = {
  PHP_ME(Mrloop, init, arginfo_class_Mrloop_init, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(Mrloop, stop, arginfo_class_Mrloop_stop, ZEND_ACC_PUBLIC)
      PHP_ME(Mrloop, run, arginfo_class_Mrloop_run, ZEND_ACC_PUBLIC)
        PHP_ME(Mrloop, addTimer, arginfo_class_Mrloop_addTimer, ZEND_ACC_PUBLIC)
          PHP_ME(Mrloop, addPeriodicTimer, arginfo_class_Mrloop_addPeriodicTimer, ZEND_ACC_PUBLIC)
            PHP_ME(Mrloop, tcpServer, arginfo_class_Mrloop_tcpServer, ZEND_ACC_PUBLIC)
              PHP_ME(Mrloop, parseHttpRequest, arginfo_class_Mrloop_parseHttpRequest, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
                PHP_ME(Mrloop, addSignal, arginfo_class_Mrloop_addSignal, ZEND_ACC_PUBLIC)
                  PHP_ME(Mrloop, addReadStream, arginfo_class_Mrloop_addReadStream, ZEND_ACC_PUBLIC)
                    PHP_ME(Mrloop, addWriteStream, arginfo_class_Mrloop_addWriteStream, ZEND_ACC_PUBLIC)
                      PHP_ME(Mrloop, parseHttpResponse, arginfo_class_Mrloop_parseHttpResponse, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
                        PHP_ME(Mrloop, writev, arginfo_class_Mrloop_writev, ZEND_ACC_PUBLIC)
                          PHP_FE_END};
