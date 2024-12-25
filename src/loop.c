/* mrloop extension for PHP (c) 2024 Lochemem Bruno Michael */
#include "loop.h"

static zend_object *php_mrloop_create_object(zend_class_entry *ce)
{
  php_mrloop_t *obj = zend_object_alloc(sizeof(php_mrloop_t), php_mrloop_ce);
  zend_object_std_init(&obj->std, php_mrloop_ce);

  obj->std.handlers = &php_mrloop_object_handlers;
  obj->loop = NULL;

  return &obj->std;
}
static void php_mrloop_free_object(zend_object *obj)
{
  php_mrloop_t *intern = php_mrloop_from_obj(obj);

  if (intern->loop)
  {
    mr_free(intern->loop);
  }

  zend_object_std_dtor(obj);
  efree(intern);
}

static void php_mrloop_signal_handler(const int sig)
{
  exit(EXIT_SUCCESS);
}
static void php_mrloop_create(INTERNAL_FUNCTION_PARAMETERS)
{
  php_mrloop_t *evloop;

  ZEND_PARSE_PARAMETERS_NONE();

  object_init_ex(return_value, php_mrloop_ce);
  evloop = PHP_MRLOOP_OBJ(return_value);

  mr_loop_t *loop = mr_create_loop(php_mrloop_signal_handler);
  evloop->loop = loop;
}
static void php_mrloop_stop(INTERNAL_FUNCTION_PARAMETERS)
{
  zval *obj;
  php_mrloop_t *this;

  obj = getThis();

  ZEND_PARSE_PARAMETERS_NONE();

  this = PHP_MRLOOP_OBJ(obj);

  mr_stop(this->loop);
}
static void php_mrloop_run(INTERNAL_FUNCTION_PARAMETERS)
{
  zval *obj;
  php_mrloop_t *this;

  obj = getThis();

  ZEND_PARSE_PARAMETERS_NONE();

  this = PHP_MRLOOP_OBJ(obj);

  mr_run(this->loop);
}

static int php_mrloop_timer_cb(void *data)
{
  php_mrloop_cb_t *cb = (php_mrloop_cb_t *)data;
  zval result;
  int type;

  cb->fci.retval = &result;
  cb->fci.param_count = 0;
  cb->fci.params = NULL;

  type = cb->signal;

  if (zend_call_function(&cb->fci, &cb->fci_cache) == FAILURE)
  {
    efree(cb);
    mr_stop((mr_loop_t *)cb->data);

    PHP_MRLOOP_THROW("There is an error in your callback");
    zval_ptr_dtor(&result);

    return 0;
  }

  // add explicit timer cancellation to periodic timer
  if (type == PHP_MRLOOP_PERIODIC_TIMER && Z_TYPE(result) == IS_LONG && Z_LVAL(result) == 0)
  {
    zval_ptr_dtor(&result);
    efree(cb);

    return 0;
  }

  zval_ptr_dtor(&result);
  efree(cb);

  return type == PHP_MRLOOP_TIMER || type == PHP_MRLOOP_FUTURE_TICK ? 0 : 1;
}
static void php_mrloop_add_timer(INTERNAL_FUNCTION_PARAMETERS)
{
  zval *obj;
  double interval;
  php_mrloop_t *this;
  php_mrloop_cb_t *cb;
  zend_fcall_info fci;
  zend_fcall_info_cache fci_cache;

  fci = empty_fcall_info;
  fci_cache = empty_fcall_info_cache;
  obj = getThis();

  ZEND_PARSE_PARAMETERS_START(2, 2)
  Z_PARAM_DOUBLE(interval)
  Z_PARAM_FUNC(fci, fci_cache)
  ZEND_PARSE_PARAMETERS_END();

  this = PHP_MRLOOP_OBJ(obj);
  cb = emalloc(sizeof(php_mrloop_cb_t));
  PHP_CB_TO_MRLOOP_CB(cb, fci, fci_cache);

  cb->signal = PHP_MRLOOP_TIMER;
  cb->data = this->loop;

  mr_call_after(this->loop, php_mrloop_timer_cb, (interval * 1000), cb);

  return;
}
static void php_mrloop_add_periodic_timer(INTERNAL_FUNCTION_PARAMETERS)
{
  zval *obj;
  double interval;
  php_mrloop_t *this;
  php_mrloop_cb_t *cb;
  zend_fcall_info fci;
  zend_fcall_info_cache fci_cache;

  fci = empty_fcall_info;
  fci_cache = empty_fcall_info_cache;
  obj = getThis();

  ZEND_PARSE_PARAMETERS_START(2, 2)
  Z_PARAM_DOUBLE(interval)
  Z_PARAM_FUNC(fci, fci_cache)
  ZEND_PARSE_PARAMETERS_END();

  this = PHP_MRLOOP_OBJ(obj);
  cb = emalloc(sizeof(php_mrloop_cb_t));
  PHP_CB_TO_MRLOOP_CB(cb, fci, fci_cache);

  cb->signal = PHP_MRLOOP_PERIODIC_TIMER;
  cb->data = this->loop;

  mr_add_timer(this->loop, interval, php_mrloop_timer_cb, cb);

  return;
}
static void php_mrloop_add_future_tick(INTERNAL_FUNCTION_PARAMETERS)
{
  zval *obj;
  php_mrloop_cb_t *cb;
  php_mrloop_t *this;
  zend_fcall_info fci;
  zend_fcall_info_cache fci_cache;

  fci = empty_fcall_info;
  fci_cache = empty_fcall_info_cache;
  obj = getThis();

  ZEND_PARSE_PARAMETERS_START(1, 1)
  Z_PARAM_FUNC(fci, fci_cache)
  ZEND_PARSE_PARAMETERS_END();

  this = PHP_MRLOOP_OBJ(obj);
  cb = emalloc(sizeof(php_mrloop_cb_t));
  PHP_CB_TO_MRLOOP_CB(cb, fci, fci_cache);

  cb->signal = PHP_MRLOOP_FUTURE_TICK;
  cb->data = this->loop;

  mr_call_soon(this->loop, php_mrloop_timer_cb, cb);

  return;
}

static void php_mrloop_readv_cb(void *data, int res)
{
  if (res < 0)
  {
    PHP_MRLOOP_THROW(strerror(-res));
  }

  php_mrloop_cb_t *cb;
  php_iovec_t *iov;
  zval args[2], result;

  cb = (php_mrloop_cb_t *)data;
  iov = (php_iovec_t *)cb->data;

  char next[(size_t)iov->iov_len];
  php_strncpy(next, iov->iov_base, iov->iov_len + 1);

  ZVAL_STRING(&args[0], next);
  ZVAL_LONG(&args[1], res);

  cb->fci.retval = &result;
  cb->fci.param_count = 2;
  cb->fci.params = args;

  if (zend_call_function(&cb->fci, &cb->fci_cache) == FAILURE)
  {
    PHP_MRLOOP_THROW("There is an error in your callback");
  }

  zval_ptr_dtor(&result);
  efree(cb->data);
  efree(cb);

  return;
}
static void php_mrloop_writev_cb(void *data, int res)
{
  if (res < 0)
  {
    PHP_MRLOOP_THROW(strerror(-res));
  }

  php_mrloop_cb_t *cb = (php_mrloop_cb_t *)data;
  zval args[1], result;
  ZVAL_LONG(&args[0], res);

  cb->fci.retval = &result;
  cb->fci.param_count = 1;
  cb->fci.params = args;

  if (zend_call_function(&cb->fci, &cb->fci_cache) == FAILURE)
  {
    PHP_MRLOOP_THROW("There is an error in your callback");
  }

  zval_ptr_dtor(&result);
  efree(cb->data);
  efree(cb);

  return;
}

static void *php_mrloop_tcp_client_setup(int fd, char **buffer, int *bsize)
{
  php_mrloop_conn_t *conn;
  php_sockaddr_t addr;
  socklen_t socklen;
  char ip_str[INET_ADDRSTRLEN];

  conn = emalloc(sizeof(php_mrloop_conn_t));
  conn->fd = fd;
  conn->buffer = ecalloc(1, MRLOOP_G(tcp_buff_size));
  *buffer = conn->buffer;
  *bsize = MRLOOP_G(tcp_buff_size);

  socklen = sizeof(php_sockaddr_t);

  if (getpeername(fd, (struct sockaddr *)&addr, &socklen) > -1)
  {
    inet_ntop(AF_INET, &addr.sin_addr, ip_str, INET_ADDRSTRLEN);

    conn->addr = ecalloc(1, INET_ADDRSTRLEN);
    php_strncpy(conn->addr, ip_str, INET_ADDRSTRLEN);

    conn->port = (size_t)addr.sin_port;
  }

  return (void *)conn;
}
static int php_mrloop_tcp_server_recv(void *conn, int fd, ssize_t nbytes, char *buffer)
{
  php_mrloop_conn_t *client = (php_mrloop_conn_t *)conn;
  mr_loop_t *loop = (mr_loop_t *)MRLOOP_G(tcp_cb)->data;

  if (nbytes == 0)
  {
    mr_close(loop, client->fd);
    efree(client->addr);
    efree(client->buffer);
    efree(client);

    return 1;
  }

  zval args[2], result;
  ZVAL_STRING(&args[0], buffer);

  array_init(&args[1]);
  add_assoc_string(&args[1], "client_addr", (char *)client->addr);
  add_assoc_long(&args[1], "client_port", client->port);
  add_assoc_long(&args[1], "client_fd", dup(client->fd));

  MRLOOP_G(tcp_cb)->fci.retval = &result;
  MRLOOP_G(tcp_cb)->fci.param_count = 2;
  MRLOOP_G(tcp_cb)->fci.params = args;

  if (zend_call_function(&MRLOOP_G(tcp_cb)->fci, &MRLOOP_G(tcp_cb)->fci_cache) == FAILURE)
  {
    PHP_MRLOOP_THROW("There is an error in your callback");
    zval_ptr_dtor(&result);

    return 1;
  }

  if (Z_TYPE(result) == IS_STRING)
  {
    client->iov.iov_base = Z_STRVAL(result);
    client->iov.iov_len = Z_STRLEN(result);

    mr_writev(loop, client->fd, &(client->iov), 1);
    mr_flush(loop);
  }

  zval_ptr_dtor(&result);

  return 1;
}
static void php_mrloop_tcp_server_listen(INTERNAL_FUNCTION_PARAMETERS)
{
  zval *obj;
  php_mrloop_t *this;
  zend_fcall_info fci;
  zend_fcall_info_cache fci_cache;
  zend_long port, max_conn, nbytes;
  bool max_conn_null, nbytes_null;
  size_t nconn, fnbytes;

  obj = getThis();
  fci = empty_fcall_info;
  fci_cache = empty_fcall_info_cache;
  max_conn_null = true;
  nbytes_null = true;

  ZEND_PARSE_PARAMETERS_START(4, 4)
  Z_PARAM_LONG(port)
  Z_PARAM_LONG_OR_NULL(max_conn, max_conn_null)
  Z_PARAM_LONG_OR_NULL(nbytes, nbytes_null)
  Z_PARAM_FUNC(fci, fci_cache)
  ZEND_PARSE_PARAMETERS_END();

  this = PHP_MRLOOP_OBJ(obj);

  fnbytes = (size_t)(nbytes_null == true ? DEFAULT_CONN_BUFF_LEN : nbytes);
  MRLOOP_G(tcp_buff_size) = fnbytes;

  MRLOOP_G(tcp_cb) = emalloc(sizeof(php_mrloop_cb_t));
  PHP_CB_TO_MRLOOP_CB(MRLOOP_G(tcp_cb), fci, fci_cache);
  MRLOOP_G(tcp_cb)->data = this->loop;

  nconn = (size_t)(max_conn_null == true ? PHP_MRLOOP_MAX_TCP_CONNECTIONS : (max_conn == 0 ? PHP_MRLOOP_MAX_TCP_CONNECTIONS : max_conn));

#ifdef MRLOOP_H
  mr_tcp_server(this->loop, (int)port, nconn, php_mrloop_tcp_client_setup, php_mrloop_tcp_server_recv);
#else
  mr_tcp_server(this->loop, (int)port, php_mrloop_tcp_client_setup, php_mrloop_tcp_server_recv);
#endif

  return;
}

static void php_mrloop_signal_cb(int sig)
{
  for (size_t idx = 0; idx < MRLOOP_G(sigc); idx++)
  {
    if (MRLOOP_G(sig_cb)[idx] == NULL)
    {
      break;
    }

    php_mrloop_cb_t *cb = MRLOOP_G(sig_cb)[idx];

    if (cb->signal == sig)
    {
      zval result;

      cb->fci.retval = &result;
      cb->fci.param_count = 0;
      cb->fci.params = NULL;

      if (zend_call_function(&cb->fci, &cb->fci_cache) == FAILURE)
      {
        PHP_MRLOOP_THROW("There is an error in your callback");
      }

      zval_ptr_dtor(&result);

      break;
    }
  }

  exit(EXIT_SUCCESS);
}
static void php_mrloop_add_signal(INTERNAL_FUNCTION_PARAMETERS)
{
  zend_long php_signal;
  zend_fcall_info fci;
  zend_fcall_info_cache fci_cache;

  ZEND_PARSE_PARAMETERS_START(2, 2)
  Z_PARAM_LONG(php_signal)
  Z_PARAM_FUNC(fci, fci_cache)
  ZEND_PARSE_PARAMETERS_END();

  MRLOOP_G(sigc)++;
  size_t next = MRLOOP_G(sigc) - 1;

  MRLOOP_G(sig_cb)[next] = emalloc(sizeof(php_mrloop_cb_t));
  PHP_CB_TO_MRLOOP_CB(MRLOOP_G(sig_cb)[next], fci, fci_cache);
  MRLOOP_G(sig_cb)[next]->signal = (int)php_signal;

  signal(SIGINT, php_mrloop_signal_cb);
  signal(SIGHUP, php_mrloop_signal_cb);
  signal(SIGTERM, php_mrloop_signal_cb);

  return;
}

static void php_mrloop_add_read_stream(INTERNAL_FUNCTION_PARAMETERS)
{
  zval *res, *obj;
  php_mrloop_t *this;
  php_mrloop_cb_t *cb;
  php_iovec_t *iov;
  zend_fcall_info fci;
  zend_fcall_info_cache fci_cache;
  zend_long nbytes, vcount, offset;
  bool nbytes_null, vcount_null, offset_null;
  int fd; // php_socket_t fd;
  php_stream *stream;
  size_t fnbytes, fvcount, foffset;

  obj = getThis();
  nbytes_null = true;
  vcount_null = true;
  offset_null = true;
  fci = empty_fcall_info;
  fci_cache = empty_fcall_info_cache;
  fd = -1;

  ZEND_PARSE_PARAMETERS_START(5, 5)
  Z_PARAM_RESOURCE(res)
  Z_PARAM_LONG_OR_NULL(nbytes, nbytes_null)
  Z_PARAM_LONG_OR_NULL(vcount, vcount_null)
  Z_PARAM_LONG_OR_NULL(offset, offset_null)
  Z_PARAM_FUNC(fci, fci_cache)
  ZEND_PARSE_PARAMETERS_END();

  this = PHP_MRLOOP_OBJ(obj);

  // convert resource to PHP stream
  PHP_STREAM_TO_FD(stream, res, fd);

  fnbytes = (size_t)(nbytes_null == true ? DEFAULT_STREAM_BUFF_LEN : nbytes);
  fvcount = (size_t)(vcount_null == true ? DEFAULT_VECTOR_COUNT : vcount);
  foffset = (size_t)(offset_null == true ? DEFAULT_READV_OFFSET : offset);

  iov = emalloc(sizeof(php_iovec_t));
  iov->iov_base = ecalloc(1, fnbytes);
  iov->iov_len = fnbytes;

  cb = emalloc(sizeof(php_mrloop_cb_t));
  PHP_CB_TO_MRLOOP_CB(cb, fci, fci_cache);

  cb->data = iov;

  mr_readvcb(this->loop, fd, iov, fvcount, foffset, cb, php_mrloop_readv_cb);
  mr_flush(this->loop);

  return;
}
static void php_mrloop_add_write_stream(INTERNAL_FUNCTION_PARAMETERS)
{
  zval *res, *obj;
  zend_string *contents;
  php_mrloop_t *this;
  php_mrloop_cb_t *cb;
  php_iovec_t *iov;
  zend_fcall_info fci;
  zend_fcall_info_cache fci_cache;
  zend_long vcount;
  bool vcount_null;
  int fd;
  php_stream *stream;
  size_t nbytes, fvcount;

  obj = getThis();
  fci = empty_fcall_info;
  fci_cache = empty_fcall_info_cache;
  fd = -1;
  vcount_null = true;

  ZEND_PARSE_PARAMETERS_START(4, 4)
  Z_PARAM_RESOURCE(res)
  Z_PARAM_STR(contents)
  Z_PARAM_LONG_OR_NULL(vcount, vcount_null)
  Z_PARAM_FUNC(fci, fci_cache)
  ZEND_PARSE_PARAMETERS_END();

  this = PHP_MRLOOP_OBJ(obj);

  PHP_STREAM_TO_FD(stream, res, fd);

  nbytes = ZSTR_LEN(contents);
  iov = emalloc(sizeof(php_iovec_t));
  iov->iov_base = ecalloc(1, nbytes);
  iov->iov_len = nbytes;

  php_strncpy(iov->iov_base, ZSTR_VAL(contents), nbytes + 1);

  cb = emalloc(sizeof(php_mrloop_cb_t));
  PHP_CB_TO_MRLOOP_CB(cb, fci, fci_cache);

  cb->data = iov;

  fvcount = (size_t)(vcount_null == true ? DEFAULT_VECTOR_COUNT : vcount);

  mr_writevcb(this->loop, fd, iov, fvcount, cb, php_mrloop_writev_cb);
  mr_flush(this->loop);

  return;
}
static void php_mrloop_writev(INTERNAL_FUNCTION_PARAMETERS)
{
  zend_string *contents;
  php_iovec_t iov;
  php_mrloop_t *this;
  zval *obj, *res;
  size_t nbytes;
  php_stream *stream;
  int fd;

  obj = getThis();
  fd = -1;

  ZEND_PARSE_PARAMETERS_START(2, 2)
  Z_PARAM_ZVAL(res)
  Z_PARAM_STR(contents)
  ZEND_PARSE_PARAMETERS_END();

  this = PHP_MRLOOP_OBJ(obj);

  if (Z_TYPE_P(res) == IS_RESOURCE)
  {
    PHP_STREAM_TO_FD(stream, res, fd);
  }
  else if (Z_TYPE_P(res) == IS_LONG)
  {
    fd = Z_LVAL_P(res);

    if (fcntl(fd, F_GETFD) < 0)
    {
      PHP_MRLOOP_THROW(strerror(errno));
      mr_stop(this->loop);

      return;
    }
  }
  else
  {
    PHP_MRLOOP_THROW("Detected invalid file descriptor");
    RETURN_NULL();
  }

  nbytes = ZSTR_LEN(contents);
  iov.iov_base = ZSTR_VAL(contents);
  iov.iov_len = nbytes;

  mr_writev(this->loop, fd, &iov, 1);
  mr_flush(this->loop);
}

static size_t php_strncpy(char *dst, char *src, size_t nbytes)
{
  const char *osrc = src;
  size_t nleft = nbytes;

  // copy as many bytes as will fit
  if (nleft != 0)
  {
    while (--nleft != 0)
    {
      if ((*dst++ = *src++) == '\0')
        break;
    }
  }

  // not enough room in dst, add null byte and traverse rest of src
  if (nleft == 0)
  {
    if (nbytes != 0)
      *dst = '\0';
    while (*src++)
      ;
  }

  return (src - osrc - 1);
}
