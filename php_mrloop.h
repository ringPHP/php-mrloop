/* mrloop extension for PHP (c) 2024 Lochemem Bruno Michael */
#ifndef PHP_MRLOOP_H
#define PHP_MRLOOP_H

extern zend_module_entry mrloop_module_entry;

#define MRLOOP_VERSION "0.1.0"
#define MRLOOP_AUTHOR "Lochemem Bruno Michael"

#if defined(ZTS) && defined(COMPILE_DL_MRLOOP)
ZEND_TSRMLS_CACHE_EXTERN()
#endif

#endif /* PHP_MRLOOP_H */
