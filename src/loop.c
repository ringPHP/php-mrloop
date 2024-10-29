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

  zval_ptr_dtor(&result);
  efree(cb);

  return type == PHP_MRLOOP_TIMER ? 0 : 1;
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

static void php_mrloop_readv_cb(void *data, int res)
{
  php_mrloop_cb_t *cb;
  php_iovec_t *iov;
  zval args[2], result;

  cb = (php_mrloop_cb_t *)data;
  iov = (php_iovec_t *)cb->data;

  ZVAL_STRING(&args[0], (char *)iov->iov_base);
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
static void php_mrloop_file_readv(INTERNAL_FUNCTION_PARAMETERS)
{
  zval *obj;
  php_mrloop_t *this;
  php_mrloop_cb_t *cb;
  php_iovec_t *iov;
  zend_string *file;
  zend_fcall_info fci;
  zend_fcall_info_cache fci_cache;
  zend_long nbytes, offset;
  bool nbytes_null, offset_null;
  int fd;
  size_t fnbytes, foffset;
  php_stat_t sb;

  obj = getThis();
  nbytes_null = true;
  offset_null = true;
  fci = empty_fcall_info;
  fci_cache = empty_fcall_info_cache;

  ZEND_PARSE_PARAMETERS_START(4, 4)
  Z_PARAM_STR(file)
  Z_PARAM_LONG_OR_NULL(nbytes, nbytes_null)
  Z_PARAM_LONG_OR_NULL(offset, offset_null)
  Z_PARAM_FUNC(fci, fci_cache)
  ZEND_PARSE_PARAMETERS_END();

  this = PHP_MRLOOP_OBJ(obj);

  fd = open(ZSTR_VAL(file), O_RDONLY | O_NONBLOCK);

  if (fd < 0)
  {
    char *error = strerror(errno);
    close(fd);

    PHP_MRLOOP_THROW(error);

    mr_stop(this->loop);

    return;
  }

  if (fstat(fd, &sb) < 0)
  {
    char *error = strerror(errno);
    close(fd);

    PHP_MRLOOP_THROW(error);

    mr_stop(this->loop);

    return;
  }

  fnbytes = (size_t)(nbytes_null == false ? nbytes : sb.st_size);
  foffset = (size_t)(offset_null == false ? offset : 0);

  iov = emalloc(sizeof(php_iovec_t));
  iov->iov_base = emalloc(fnbytes);
  iov->iov_len = fnbytes;

  cb = emalloc(sizeof(php_mrloop_cb_t));
  PHP_CB_TO_MRLOOP_CB(cb, fci, fci_cache);

  cb->data = iov;

  mr_readvcb(this->loop, fd, iov, 1, foffset, cb, php_mrloop_readv_cb);
  mr_flush(this->loop);

  return;
}
static void php_mrloop_proc_readv(INTERNAL_FUNCTION_PARAMETERS)
{
  zval *obj;
  php_mrloop_t *this;
  php_mrloop_cb_t *cb;
  php_iovec_t *iov;
  zend_string *command;
  zend_fcall_info fci;
  zend_fcall_info_cache fci_cache;
  zend_long nbytes;
  bool nbytes_null;
  int fd;
  size_t fnbytes;
  FILE *pfd;

  obj = getThis();
  nbytes_null = true;
  fci = empty_fcall_info;
  fci_cache = empty_fcall_info_cache;

  ZEND_PARSE_PARAMETERS_START(3, 3)
  Z_PARAM_STR(command)
  Z_PARAM_LONG_OR_NULL(nbytes, nbytes_null)
  Z_PARAM_FUNC(fci, fci_cache)
  ZEND_PARSE_PARAMETERS_END();

  this = PHP_MRLOOP_OBJ(obj);
  pfd = popen(ZSTR_VAL(command), "r");

  if ((fd = fileno(pfd)) < 0)
  {
    close(fd);
    pclose(pfd);

    char *error = strerror(errno);
    PHP_MRLOOP_THROW(error);

    mr_stop(this->loop);

    return;
  }

  if (fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK) < 0)
  {
    close(fd);
    pclose(pfd);

    char *error = strerror(errno);
    PHP_MRLOOP_THROW(error);

    mr_stop(this->loop);

    return;
  }

  fnbytes = (size_t)(nbytes_null == false ? nbytes : DEFAULT_SHELL_BUFF_LEN);

  iov = emalloc(sizeof(php_iovec_t));
  iov->iov_base = emalloc(fnbytes);
  iov->iov_len = fnbytes;

  cb = emalloc(sizeof(php_mrloop_cb_t));
  PHP_CB_TO_MRLOOP_CB(cb, fci, fci_cache);

  cb->data = iov;

  mr_readvcb(this->loop, fd, iov, 1, 0, cb, php_mrloop_readv_cb);
  mr_flush(this->loop);
  pclose(pfd);

  return;
}

static void php_mrloop_writev_cb(void *data, int res)
{
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
static void php_mrloop_proc_writev(INTERNAL_FUNCTION_PARAMETERS)
{
  zval *obj;
  php_mrloop_t *this;
  php_mrloop_cb_t *cb;
  php_iovec_t *iov;
  zend_string *command, *contents;
  zend_fcall_info fci;
  zend_fcall_info_cache fci_cache;
  int fd;
  FILE *pfd;
  size_t nbytes;

  obj = getThis();
  fci = empty_fcall_info;
  fci_cache = empty_fcall_info_cache;

  ZEND_PARSE_PARAMETERS_START(3, 3)
  Z_PARAM_STR(command)
  Z_PARAM_STR(contents)
  Z_PARAM_FUNC(fci, fci_cache)
  ZEND_PARSE_PARAMETERS_END();

  this = PHP_MRLOOP_OBJ(obj);
  pfd = popen(ZSTR_VAL(command), "w");

  if ((fd = fileno(pfd)) < 0)
  {
    close(fd);
    pclose(pfd);

    char *error = strerror(errno);
    PHP_MRLOOP_THROW(error);

    mr_stop(this->loop);

    return;
  }

  if (fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK) < 0)
  {
    close(fd);
    pclose(pfd);

    char *error = strerror(errno);
    PHP_MRLOOP_THROW(error);

    mr_stop(this->loop);

    return;
  }

  nbytes = ZSTR_LEN(contents);

  iov = emalloc(sizeof(php_iovec_t));
  iov->iov_base = emalloc(nbytes);
  iov->iov_len = nbytes;

  strcpy(iov->iov_base, ZSTR_VAL(contents));

  cb = emalloc(sizeof(php_mrloop_cb_t));
  PHP_CB_TO_MRLOOP_CB(cb, fci, fci_cache);

  cb->data = iov;

  mr_writevcb(this->loop, fd, iov, 1, cb, php_mrloop_writev_cb);
  mr_flush(this->loop);
  pclose(pfd);

  return;
}
static void php_mrloop_file_writev(INTERNAL_FUNCTION_PARAMETERS)
{
  zval *obj;
  php_mrloop_t *this;
  php_mrloop_cb_t *cb;
  php_iovec_t *iov;
  zend_string *file, *contents;
  zend_fcall_info fci;
  zend_fcall_info_cache fci_cache;
  int fd;
  zend_long flags;
  size_t nbytes;
  bool flags_null;

  obj = getThis();
  fci = empty_fcall_info;
  fci_cache = empty_fcall_info_cache;
  flags_null = true;

  ZEND_PARSE_PARAMETERS_START(4, 4)
  Z_PARAM_STR(file)
  Z_PARAM_STR(contents)
  Z_PARAM_LONG_OR_NULL(flags, flags_null)
  Z_PARAM_FUNC(fci, fci_cache)
  ZEND_PARSE_PARAMETERS_END();

  this = PHP_MRLOOP_OBJ(obj);

  // contains FILE_APPEND; might also contain LOCK_EX
  if (flags == 8 || flags == 10)
  {
    fd = open(ZSTR_VAL(file), O_APPEND | O_RDWR | O_NONBLOCK);
  }
  else
  {
    fd = open(ZSTR_VAL(file), O_CREAT | O_WRONLY | O_TRUNC | O_NONBLOCK, S_IRWXU);
  }

  if (fd < 0)
  {
    char *error = strerror(errno);
    close(fd);

    PHP_MRLOOP_THROW(error);

    return;
  }

  // contains LOCK_EX
  if (flags == 2 || flags == 10)
  {
    // apply advisory lock
    if (flock(fd, LOCK_EX | LOCK_NB) < 0)
    {
      char *error = strerror(errno);
      close(fd);

      PHP_MRLOOP_THROW(error);

      return;
    }
  }

  nbytes = ZSTR_LEN(contents);
  iov = emalloc(sizeof(php_iovec_t));
  iov->iov_base = emalloc(nbytes);
  iov->iov_len = nbytes;

  strcpy(iov->iov_base, ZSTR_VAL(contents));

  cb = emalloc(sizeof(php_mrloop_cb_t));
  PHP_CB_TO_MRLOOP_CB(cb, fci, fci_cache);

  cb->data = iov;

  mr_writevcb(this->loop, fd, iov, 1, cb, php_mrloop_writev_cb);
  mr_flush(this->loop);

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
  *buffer = conn->buffer;
  *bsize = DEFAULT_CONN_BUFF_LEN;

  socklen = sizeof(php_sockaddr_t);

  if (getpeername(fd, &addr, &socklen) > -1)
  {
    inet_ntop(AF_INET, &addr.sin_addr, ip_str, INET_ADDRSTRLEN);

    conn->addr = ecalloc(1, strlen(ip_str));
    strcpy(conn->addr, ip_str);

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
    efree(client);

    return 1;
  }

  zval args[2], result;
  ZVAL_STRING(&args[0], buffer);

  array_init(&args[1]);
  add_assoc_string(&args[1], "client_addr", (char *)client->addr);
  add_assoc_long(&args[1], "client_port", client->port);

  MRLOOP_G(tcp_cb)->fci.retval = &result;
  MRLOOP_G(tcp_cb)->fci.param_count = 2;
  MRLOOP_G(tcp_cb)->fci.params = args;

  if (zend_call_function(&MRLOOP_G(tcp_cb)->fci, &MRLOOP_G(tcp_cb)->fci_cache) == FAILURE ||
      Z_TYPE(result) != IS_STRING)
  {
    PHP_MRLOOP_THROW("There is an error in your callback");
    zval_ptr_dtor(&result);

    return 1;
  }

  client->iov.iov_base = Z_STRVAL(result);
  client->iov.iov_len = Z_STRLEN(result);

  mr_writev(loop, client->fd, &(client->iov), 1);
  mr_flush(loop);
  zval_ptr_dtor(&result);

  return 1;
}
static void php_mrloop_tcp_server_listen(INTERNAL_FUNCTION_PARAMETERS)
{
  zval *obj;
  php_mrloop_t *this;
  zend_fcall_info fci;
  zend_fcall_info_cache fci_cache;
  zend_long port;

  obj = getThis();
  fci = empty_fcall_info;
  fci_cache = empty_fcall_info_cache;

  ZEND_PARSE_PARAMETERS_START(2, 2)
  Z_PARAM_LONG(port)
  Z_PARAM_FUNC(fci, fci_cache)
  ZEND_PARSE_PARAMETERS_END();

  this = PHP_MRLOOP_OBJ(obj);

  MRLOOP_G(tcp_cb) = emalloc(sizeof(php_mrloop_cb_t));
  PHP_CB_TO_MRLOOP_CB(MRLOOP_G(tcp_cb), fci, fci_cache);
  MRLOOP_G(tcp_cb)->data = this->loop;

  mr_tcp_server(this->loop, (int)port, php_mrloop_tcp_client_setup, php_mrloop_tcp_server_recv);

  return;
}
static void php_mrloop_http_parser(INTERNAL_FUNCTION_PARAMETERS)
{
  zend_string *request;
  int http_parser, http_minor_version;
  zval retval, retval_headers;
  char *method, *path;
  size_t header_count, method_len, path_len, buffer_len;
  zend_long header_limit = DEFAULT_HTTP_HEADER_LIMIT;

  ZEND_PARSE_PARAMETERS_START(1, 2)
  Z_PARAM_STR(request)
  Z_PARAM_OPTIONAL
  Z_PARAM_LONG(header_limit)
  ZEND_PARSE_PARAMETERS_END();

  buffer_len = ZSTR_LEN(request);
  phr_header_t headers[(size_t)header_limit];
  char buffer[(size_t)buffer_len];

  header_count = sizeof(headers) / sizeof(headers[0]);
  strcpy(buffer, ZSTR_VAL(request));

  http_parser = phr_parse_request(
    buffer, buffer_len, (const char **)&method, &method_len, (const char **)&path,
    &path_len, &http_minor_version, headers, &header_count, 0);

  if (http_parser < 0)
  {
    PHP_MRLOOP_THROW("There is an error in the HTTP syntax");
    RETURN_NULL();
  }

  array_init(&retval);
  array_init(&retval_headers);

  char *body, tmp_method[method_len], tmp_path[path_len];
  body = buffer + http_parser;

  sprintf(tmp_method, "%.*s", (int)method_len, method);
  sprintf(tmp_path, "%.*s", (int)path_len, path);

  add_assoc_string(&retval, "path", tmp_path);
  add_assoc_string(&retval, "method", tmp_method);
  add_assoc_string(&retval, "body", body);

  // initialize array for response headers
  for (size_t idx = 0; idx < header_count; idx++)
  {
    if (headers[idx].name && headers[idx].value)
    {
      char tmp_header_name[headers[idx].name_len], tmp_header_value[headers[idx].value_len];
      sprintf(tmp_header_name, "%.*s", (int)headers[idx].name_len, headers[idx].name);
      sprintf(tmp_header_value, "%.*s", (int)headers[idx].value_len, headers[idx].value);

      add_assoc_string(&retval_headers, tmp_header_name, tmp_header_value);
    }
  }

  add_assoc_zval(&retval, "headers", &retval_headers);

  RETURN_ZVAL(&retval, 0, 1);
}

static void php_mrloop_signal_cb(int sig)
{
  zval result;
  int fsignal;

  MRLOOP_G(sig_cb)->fci.retval = &result;
  MRLOOP_G(sig_cb)->fci.param_count = 0;
  MRLOOP_G(sig_cb)->fci.params = NULL;

  fsignal = MRLOOP_G(sig_cb)->signal;

  if (fsignal == sig)
  {
    if (zend_call_function(&MRLOOP_G(sig_cb)->fci, &MRLOOP_G(sig_cb)->fci_cache) == FAILURE)
    {
      PHP_MRLOOP_THROW("There is an error in your callback");
    }

    zval_ptr_dtor(&result);
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

  MRLOOP_G(sig_cb) = emalloc(sizeof(php_mrloop_cb_t));
  PHP_CB_TO_MRLOOP_CB(MRLOOP_G(sig_cb), fci, fci_cache);
  MRLOOP_G(sig_cb)->signal = (int)php_signal;

  signal(SIGINT, php_mrloop_signal_cb);
  signal(SIGHUP, php_mrloop_signal_cb);
  signal(SIGTERM, php_mrloop_signal_cb);

  return;
}
