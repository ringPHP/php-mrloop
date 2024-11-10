/* mrloop extension for PHP (c) 2024 Lochemem Bruno Michael */
#ifndef __LOOP_H__
#define __LOOP_H__

#include "php.h"
#include "ext/spl/spl_exceptions.h"
#include "ext/standard/info.h"
#include "ext/standard/php_array.h"
#include "ext/standard/php_string.h"
#include "ext/standard/php_var.h"
#include "zend_exceptions.h"
#include "php_network.h"
#include "php_streams.h"
#include "sys/file.h"
#include "signal.h"
#include "mrloop.c"
#include "picohttpparser.c"

/* for compatibility with older PHP versions */
#ifndef ZEND_PARSE_PARAMETERS_NONE
#define ZEND_PARSE_PARAMETERS_NONE() \
  ZEND_PARSE_PARAMETERS_START(0, 0)  \
  ZEND_PARSE_PARAMETERS_END()
#endif

#define DEFAULT_STREAM_BUFF_LEN 8192
#define DEFAULT_CONN_BUFF_LEN 65536
#define DEFAULT_HTTP_HEADER_LIMIT 100
#define PHP_MRLOOP_TIMER 1
#define PHP_MRLOOP_PERIODIC_TIMER 2
#define PHP_MRLOOP_FUTURE_TICK 3

struct php_mrloop_t;
struct php_mrloop_cb_t;
struct php_mrloop_conn_t;
typedef struct php_mrloop_t php_mrloop_t;
typedef struct php_mrloop_cb_t php_mrloop_cb_t;
typedef struct php_mrloop_conn_t php_mrloop_conn_t;

typedef struct iovec php_iovec_t;
typedef struct addrinfo php_addrinfo_t;
typedef struct sockaddr_in php_sockaddr_t;
typedef struct phr_header phr_header_t;
typedef struct stat php_stat_t;

/* userspace-bound event loop object */
struct php_mrloop_t
{
  /* event loop instance */
  mr_loop_t *loop;
  /* PHP object */
  zend_object std;
};

/* TCP client connection object */
struct php_mrloop_conn_t
{
  /* client socket file descriptor */
  int fd;
  /* client socket address */
  char *addr;
  /* data sent over client socket */
  char buffer[DEFAULT_CONN_BUFF_LEN];
  /* client socket port */
  size_t port;
  /* scatter-gather I/O primitives */
  php_iovec_t iov;
};

/* mrloop callback object */
struct php_mrloop_cb_t
{
  /* PHP callback interface */
  zend_fcall_info fci;
  /* PHP callback cache */
  zend_fcall_info_cache fci_cache;
  /* arbitrary data relevant to callback */
  void *data;
  /* ancillary numeric data (signal or otherwise) relevant to callback */
  int signal;
};

zend_object_handlers php_mrloop_object_handlers;

static inline php_mrloop_t *php_mrloop_from_obj(zend_object *obj)
{
  return (php_mrloop_t *)((char *)obj - XtOffsetOf(php_mrloop_t, std));
}

#define PHP_MRLOOP_OBJ(zv) php_mrloop_from_obj(Z_OBJ_P(zv));

/* {{{ ZEND_BEGIN_MODULE_GLOBALS */
ZEND_BEGIN_MODULE_GLOBALS(mrloop)
/* TCP server callback */
php_mrloop_cb_t *tcp_cb;
/* signal callback */
php_mrloop_cb_t *sig_cb[3];
/* signal callback count */
size_t sigc;
ZEND_END_MODULE_GLOBALS(mrloop)
/* }}} */

ZEND_DECLARE_MODULE_GLOBALS(mrloop)

/* ext-mrloop globals accessor */
#ifdef ZTS
#define MRLOOP_G(v) ZEND_MODULE_GLOBALS_ACCESSOR(mrloop, v)
#else
#define MRLOOP_G(v) (mrloop_globals.v)
#endif

static PHP_GINIT_FUNCTION(mrloop);

/* creates mrloop object in PHP userspace */
static zend_object *php_mrloop_create_object(zend_class_entry *ce);
/* frees PHP userspace-residing mrloop object */
static void php_mrloop_free_object(zend_object *obj);

/* callback specified during creation of event loop */
static void php_mrloop_signal_handler(const int sig);
/* creates Mrloop object in PHP userspace */
static void php_mrloop_create(INTERNAL_FUNCTION_PARAMETERS);
/* explicitly stops event loop subsumed in Mrloop object */
static void php_mrloop_stop(INTERNAL_FUNCTION_PARAMETERS);
/* runs event loop subsumed in Mrloop object */
static void php_mrloop_run(INTERNAL_FUNCTION_PARAMETERS);

/* mrloop-bound callback specified during invocation of timer-related functions */
static int php_mrloop_timer_cb(void *data);
/* executes a specified action after a specified amount of time */
static void php_mrloop_add_timer(INTERNAL_FUNCTION_PARAMETERS);
/* executes a specified action in perpetuity with each successive execution occurring after a specified time interval */
static void php_mrloop_add_periodic_timer(INTERNAL_FUNCTION_PARAMETERS);
/* schedules the execution of a specified action for the next event loop tick */
static void php_mrloop_add_future_tick(INTERNAL_FUNCTION_PARAMETERS);

/* mrloop-bound callback specified during invocation of vectorized read function */
static void php_mrloop_readv_cb(void *data, int res);
/* mrloop-bound callback specified during invocation of vectorized write function */
static void php_mrloop_writev_cb(void *data, int res);

/* initializes client connection context for TCP server */
static void *php_mrloop_tcp_client_setup(int fd, char **buffer, int *bsize);
/* processes incoming TCP connections and issues responses to clients */
static int php_mrloop_tcp_server_recv(void *conn, int fd, ssize_t nbytes, char *buffer);
/* starts a TCP server */
static void php_mrloop_tcp_server_listen(INTERNAL_FUNCTION_PARAMETERS);
/* parses an HTTP request */
static void php_mrloop_parse_http_request(INTERNAL_FUNCTION_PARAMETERS);
/* parses an HTTP response */
static void php_mrloop_parse_http_response(INTERNAL_FUNCTION_PARAMETERS);

/* mrloop-bound callback specified during invocation of signal handlers */
static void php_mrloop_signal_cb(int sig);
/* executes specified action in the event that a specified signal is detected */
static void php_mrloop_add_signal(INTERNAL_FUNCTION_PARAMETERS);

/* funnels file descriptor in readable stream into event loop and thence executes a non-blocking read operation */
static void php_mrloop_add_read_stream(INTERNAL_FUNCTION_PARAMETERS);
/* funnels file descriptor in writable stream into event loop and thence executes a non-blocking write operation */
static void php_mrloop_add_write_stream(INTERNAL_FUNCTION_PARAMETERS);
/* performs vectorized non-blocking write operation on a specified file descriptor */
static void php_mrloop_writev(INTERNAL_FUNCTION_PARAMETERS);

zend_class_entry *php_mrloop_ce, *php_mrloop_exception_ce;

#define PHP_MRLOOP_THROW(message) zend_throw_exception(php_mrloop_exception_ce, message, 0);

/* wraps PHP function in mrloop callback-bound structure */
#define PHP_CB_TO_MRLOOP_CB(mrloop_cb, php_fci, php_fci_cache)                  \
  memcpy(&mrloop_cb->fci, &php_fci, sizeof(zend_fcall_info));                   \
  memcpy(&mrloop_cb->fci_cache, &php_fci_cache, sizeof(zend_fcall_info_cache)); \
  Z_TRY_ADDREF(mrloop_cb->fci.function_name);                                   \
  if (php_fci.object)                                                           \
  {                                                                             \
    GC_ADDREF(mrloop_cb->fci.object);                                           \
  }

/* extract file descriptor from PHP stream */
#define PHP_STREAM_TO_FD(fd_stream, fd_resource, fd)                           \
  if ((fd_stream = (php_stream *)zend_fetch_resource_ex(                       \
           fd_resource, NULL, php_file_le_stream()))) {                        \
    if (php_stream_cast(fd_stream,                                             \
                        PHP_STREAM_AS_FD | PHP_STREAM_CAST_INTERNAL,           \
                        (void *)&fd, 1) == FAILURE ||                          \
        fd < 0) {                                                              \
      PHP_MRLOOP_THROW("Passed resource without file descriptor");             \
      RETURN_NULL();                                                           \
    }                                                                          \
  }                                                                            \
  if (fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK) < 0) {            \
    close(fd);                                                                 \
    char *error = strerror(errno);                                             \
    PHP_MRLOOP_THROW(error);                                                   \
    mr_stop(this->loop);                                                       \
  }

#endif
