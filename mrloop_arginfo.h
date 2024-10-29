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

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_Mrloop_freadv, 0, 0, 4)
ZEND_ARG_TYPE_INFO(0, file, IS_STRING, 0)
ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, nbytes, IS_LONG, 0, "null")
ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, offset, IS_LONG, 0, "null")
ZEND_ARG_TYPE_INFO(0, callback, IS_CALLABLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_Mrloop_preadv, 0, 0, 3)
ZEND_ARG_TYPE_INFO(0, command, IS_STRING, 0)
ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, nbytes, IS_LONG, 0, "null")
ZEND_ARG_TYPE_INFO(0, callback, IS_CALLABLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_Mrloop_pwritev, 0, 0, 3)
ZEND_ARG_TYPE_INFO(0, command, IS_STRING, 0)
ZEND_ARG_TYPE_INFO(0, contents, IS_STRING, 0)
ZEND_ARG_TYPE_INFO(0, callback, IS_CALLABLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_Mrloop_fwritev, 0, 0, 4)
ZEND_ARG_TYPE_INFO(0, command, IS_STRING, 0)
ZEND_ARG_TYPE_INFO(0, contents, IS_STRING, 0)
ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flags, IS_LONG, 0, "null")
ZEND_ARG_TYPE_INFO(0, callback, IS_CALLABLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_Mrloop_tcpServer, 0, 0, 2)
ZEND_ARG_TYPE_INFO(0, port, IS_LONG, 0)
ZEND_ARG_TYPE_INFO(0, callback, IS_CALLABLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_Mrloop_parseHttp, 0, 0, 2)
ZEND_ARG_TYPE_INFO(0, request, IS_STRING, 0)
ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, header_limit, IS_LONG, 0, "100")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_Mrloop_addSignal, 0, 0, 2)
ZEND_ARG_TYPE_INFO(0, signal, IS_LONG, 0)
ZEND_ARG_TYPE_INFO(0, callback, IS_CALLABLE, 0)
ZEND_END_ARG_INFO()

ZEND_METHOD(Mrloop, init);
ZEND_METHOD(Mrloop, stop);
ZEND_METHOD(Mrloop, run);
ZEND_METHOD(Mrloop, addTimer);
ZEND_METHOD(Mrloop, addPeriodicTimer);
ZEND_METHOD(Mrloop, freadv);
ZEND_METHOD(Mrloop, preadv);
ZEND_METHOD(Mrloop, pwritev);
ZEND_METHOD(Mrloop, fwritev);
ZEND_METHOD(Mrloop, tcpServer);
ZEND_METHOD(Mrloop, parseHttp);
ZEND_METHOD(Mrloop, addSignal);

static const zend_function_entry class_Mrloop_methods[] = {
  PHP_ME(Mrloop, init, arginfo_class_Mrloop_init, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(Mrloop, stop, arginfo_class_Mrloop_stop, ZEND_ACC_PUBLIC)
      PHP_ME(Mrloop, run, arginfo_class_Mrloop_run, ZEND_ACC_PUBLIC)
        PHP_ME(Mrloop, addTimer, arginfo_class_Mrloop_addTimer, ZEND_ACC_PUBLIC)
          PHP_ME(Mrloop, addPeriodicTimer, arginfo_class_Mrloop_addPeriodicTimer, ZEND_ACC_PUBLIC)
            PHP_ME(Mrloop, freadv, arginfo_class_Mrloop_freadv, ZEND_ACC_PUBLIC)
              PHP_ME(Mrloop, preadv, arginfo_class_Mrloop_preadv, ZEND_ACC_PUBLIC)
                PHP_ME(Mrloop, pwritev, arginfo_class_Mrloop_pwritev, ZEND_ACC_PUBLIC)
                  PHP_ME(Mrloop, fwritev, arginfo_class_Mrloop_fwritev, ZEND_ACC_PUBLIC)
                    PHP_ME(Mrloop, tcpServer, arginfo_class_Mrloop_tcpServer, ZEND_ACC_PUBLIC)
                      PHP_ME(Mrloop, parseHttp, arginfo_class_Mrloop_parseHttp, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
                        PHP_ME(Mrloop, addSignal, arginfo_class_Mrloop_addSignal, ZEND_ACC_PUBLIC)
                          PHP_FE_END};
