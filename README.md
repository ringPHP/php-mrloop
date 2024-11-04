# ext-mrloop

A PHP port of an event loop designed to harness the powers of `io_uring`.

## Rationale

PHP has, in recent years, seen an emergence of eventware built atop potent multiplexing functions like `epoll()`, `poll()`, and `select()`. In event loops like `libev`, `libuv`, and `libevent` are powerful abstractions of the aforelisted functions for interleaving I/O inside a single process. Eponymous PHP extensions have, in the years since the popularization of the non-blocking I/O paradigm, been created to avail users of the language of the many potencies of the aforestated libraries. `io_uring` is yet another async system call API, and mrloop is an event loop built atop its interface. The former, which the latter is written atop—presents an enhancive proposition: unlike the commonplace non-blocking I/O APIs, it is a universal I/O interface with a small system call footprint. `io_uring` is, internally, a ring buffer—a structure the operations on which are of an **O(1)** complexity—whose mechanism is registration of a file descriptor on a submission queue and subsequent retrieval of output from a completion queue. It confers performance benefits that are useful to any project capable of interacting with its API and is thus useful to those for whom PHP is a preferred programming language.

## Requirements

- PHP 8 or newer
- Linux Kernel 5.4.1 or newer
- [mrloop](https://github.com/markreedz/mrloop)
- [liburing](https://github.com/axboe/liburing)
- [picohttpparser](https://github.com/h2o/picohttpparser)

## Installation

It is important to have all the aforelisted requirements at the ready before attempting to install `ext-mrloop`. The directives in the snippet to follow should allow you to build the extension's shared object file (`mrloop.so`).

```sh
$ git clone https://github.com/markreedz/mrloop.git <mrloop-dir>
$ git clone https://github.com/h2o/picohttpparser.git <picohttp-dir>
$ git clone https://github.com/ace411/php-uring-loop.git <dir>
$ cd <dir>
$ ./configure --with-mrloop=<mrloop-dir> --with-picohttp=<picohttp-dir>
$ make && sudo make install
```

After successfully building the shared object, proceed to operationalize the extension by adding the line `extension=mrloop` to your `php.ini` file. If you prefer to perform the said operationalization via command line interface, the following should suffice.

```sh
$ printf "\nextension=mrloop\n" >> "$(php-config --ini-path)/php.ini"
```

## API Synopsis

```php

namespace ringphp;

class Mrloop
{

  /* public methods */
  public static init(): Mrloop
  public addReadStream(
    resource $stream,
    ?int $nbytes,
    callable $callback,
  ): void
  public addWriteStream(
    resource $stream,
    string $contents,
    callable $callback,
  ): void
  public tcpServer(int $port, callable $callback): void
  public static parseHttpRequest(string $request, int $headerlimit = 100): iterable
  public static parseHttpResponse(string $response, int $headerlimit = 100): iterable
  public addTimer(float $interval, callable $callback): void
  public addPeriodicTimer(float $interval, callable $callback): void
  public addSignal(int $signal, callable $callback): void
  public run(): void
  public stop(): void
}
```

- [`Mrloop::init`](#mrloopinit)
- [`Mrloop::addReadStream`](#mrloopaddreadstream)
- [`Mrloop::addWriteStream`](#mrloopaddwritestream)
- [`Mrloop::tcpServer`](#mrlooptcpserver)
- [`Mrloop::parseHttpRequest`](#mrloopparsehttprequest)
- [`Mrloop::parseHttpResponse`](#mrloopparsehttpresponse)
- [`Mrloop::addTimer`](#mrloopaddtimer)
- [`Mrloop::addPeriodicTimer`](#mrloopaddperiodictimer)
- [`Mrloop::addSignal`](#mrloopaddsignal)
- [`Mrloop::run`](#mrlooprun)
- [`Mrloop::stop`](#mrloopstop)

### `Mrloop::init`

```php
public static Mrloop::init(): Mrloop
```

Initializes the event loop.

- Initializing the loop is one step in operationalizing it. A follow-up call to the `run()` function is required to start the loop.

**Parameter(s)**

None.

**Return value(s)**

The function returns a `Mrloop` object in which the event loop is subsumed.

```php
if (!\extension_loaded('mrloop')) {
  echo "Please install ext-mrloop to continue\n";

  exit(1);
}

use ringphp\Mrloop;

$loop = Mrloop::init();

$loop->addTimer(
  1.2,
  function () {
    echo "Hello, user\n";
  },
);

$loop->run();
```

The example above will produce output similar to that in the snippet to follow.

```
Hello, user

```

### `Mrloop::addReadStream`

```php
public Mrloop::addReadStream(
  resource $stream,
  ?int $nbytes,
  callable $callback,
): void
```

Funnels file descriptor in readable stream into event loop and thence executes a non-blocking read operation.

**Parameter(s)**

- **stream** (resource) - A userspace-defined readable stream.
  > The file descriptor in the stream is internally given a non-blocking disposition.
- **nbytes** (int|null) - The number of bytes to read.
  > Specifying `null` will condition the use of an 8KB buffer.
- **callback** (callable) - The binary function through which the file's contents and read result code are propagated.

**Return value(s)**

The function does not return anything.

```php
use ringphp\MrLoop;

$loop = Mrloop::init();

$loop->addReadStream(
  $fd = \fopen('/path/to/file', 'r'),
  null,
  function (string $contents, int $res) use ($fd) {
    if ($res === 0) {
      echo \sprintf("%s\n", $contents);

      \fclose($fd);
    }
  },
);

$loop->run();
```

The example above will produce output similar to that in the snippet to follow.

```
File contents...

```

### `Mrloop::addWriteStream`

```php
public Mrloop::addWriteStream(
  resource $stream,
  string $contents,
  callable $callback,
): void
```

Funnels file descriptor in writable stream into event loop and thence executes a non-blocking write operation.

**Parameter(s)**

- **stream** (resource) - A userspace-defined writable stream.
  > The file descriptor in the stream is internally given a non-blocking disposition.
- **contents** (string) - The contents to write to the file descriptor.
- **callback** (callable) - The unary function through which the number of written bytes is propagated.

**Return value(s)**

The function does not return anything.

```php
use ringphp\Mrloop;

$loop = MrLoop::init();

$file = '/path/to/file';

$loop->addWriteStream(
  $fd = \fopen($file, 'w'),
  "file contents...\n",
  function (int $nbytes) use ($fd, $file) {
    echo \sprintf("Wrote %d bytes to %s\n", $nbytes, $file);

    \fclose($fd);
  },
);

$loop->run();
```

The example above will produce output similar to that in the snippet to follow.

```
Wrote 18 bytes to /path/to/file

```

### `Mrloop::tcpServer`

```php
public Mrloop::tcpServer(int $port, callable $callback): void
```

Instantiates a simple TCP server.

**Parameter(s)**

- **port** (int) - The port on which to listen for incoming connections.
- **callback** (callable) - The binary function with which to define a response to a client.
  > Refer to the segment to follow for more information on the callback.
  - **Callback parameters**
    - **message** (string) - The message sent via client socket to the server.
    - **client** (iterable) - An array containing client socket information.
      - **client_addr** (string) - The client IP address.
      - **client_port** (integer) - The client socket port.

**Return value(s)**

The function does not return anything.

```php
use ringphp\Mrloop;

$loop = Mrloop::init();

$loop->tcpServer(
  8080,
  function (string $message, iterable $client) {
    // print access log
    echo \sprintf(
      "%s %s:%d %s\n",
      (
        (new \DateTimeImmutable())
          ->format(\DateTimeImmutable::ATOM)
      ),
      $client['client_addr'],
      $client['client_port'],
      $message,
    );

    return \strtoupper($message);
  },
);

$loop->run();
```

The example above will produce output similar to that in the snippet to follow.

```
2022-09-24T22:26:56+00:00 127.0.0.1:66521 foo
2022-09-24T22:26:59+00:00 127.0.0.1:67533 bar

```

### `Mrloop::parseHttpRequest`

```php
public static Mrloop::parseHttpRequest(
  string $request,
  int $headerlimit = 100,
): iterable
```

Parses an HTTP request.

> This is a function that utilizes the `picohttpparser` API.

**Parameter(s)**

- **request** (string) - The HTTP request to parse.
- **headerlimit** (int) - The number of headers to parse.
  > The default limit is `100`.

**Return value(s)**

The parser will throw an exception in the event that an invalid HTTP request is encountered and will output a hashtable with the contents enumerated below otherwise.

- **body** (string) - The request body.
- **headers** (iterable) - An associative array containing request headers.
- **method** (string) - The request method.
- **path** (string) - The request path.

```php
use ringphp\Mrloop;

$loop = Mrloop::init();

$loop->tcpServer(
  8080,
  function (mixed ...$args) {
    [$message,]  = $args;
    $response    = static fn (
      string $message,
      int $code    = 200,
      string $mime = 'text/plain',
    ) =>
      \sprintf(
        "HTTP/1.1 %d %s\r\ncontent-type: %s\r\ncontent-length: %d\r\n\r\n%s\r\n",
        $code,
        ($code === 200 ? 'OK' : 'Internal Server Error'),
        $mime,
        \strlen($message),
        $message,
      );

    try {
      $request = Mrloop::parseHttpRequest($message);

      return $response('Hello, user');
    } catch (\Throwable $err) {
      return $response(
        'HTTP parser error',
        500,
      );
    }
  },
);

$loop->run();
```

The example above will produce output similar to that in the snippet to follow.

```
Listening on port 8080

```

### `Mrloop::parseHttpResponse`

```php
public static Mrloop::parseHttpResponse(
  string $response,
  int $headerlimit = 100,
): iterable
```

Parses an HTTP response.

> This function also utilizes the `picohttpparser` API.

**Parameter(s)**

- **response** (string) - The HTTP response to parse.
- **headerlimit** (int) - The number of headers to parse.
  > The default limit is `100`.

**Return value(s)**

The parser will throw an exception in the event that an invalid HTTP response is encountered and will output a hashtable with the contents enumerated below otherwise.

- **body** (string) - The response body.
- **headers** (iterable) - An associative array containing response headers.
- **status** (int) - The response status code.
- **reason** (string) - The response reason phrase.

```php
use ringphp\Mrloop;

$loop = Mrloop::init();

$loop->addWriteStream(
  $sock = \stream_socket_client('tcp://www.example.com:80'),
  "GET / HTTP/1.0\r\nHost: www.example.com\r\nAccept: */*\r\n\r\n",
  function ($nbytes) use ($loop, $sock) {
    $loop->addReadStream(
      $sock,
      null,
      function ($data, $res) use ($sock, $loop) {
        var_dump(Mrloop::parseHttpResponse($data));

        \fclose($sock);
      },
    );
  },
);

$loop->run();
```

The example above will produce output similar to that in the snippet to follow.

```
array(4) {
  ["reason"]=>
  string(2) "OK"
  ["status"]=>
  int(200)
  ["body"]=>
  string(1256) "<!doctype html>
<html>
<head>
    <title>Example Domain</title>

    <meta charset="utf-8" />
    <meta http-equiv="Content-type" content="text/html; charset=utf-8" />
    <meta name="viewport" content="width=device-width, initial-scale=1" />
    <style type="text/css">
    body {
        background-color: #f0f0f2;
        margin: 0;
        padding: 0;
        font-family: -apple-system, system-ui, BlinkMacSystemFont, "Segoe UI", "Open Sans", "Helvetica Neue", Helvetica, Arial, sans-serif;

    }
    div {
        width: 600px;
        margin: 5em auto;
        padding: 2em;
        background-color: #fdfdff;
        border-radius: 0.5em;
        box-shadow: 2px 3px 7px 2px rgba(0,0,0,0.02);
    }
    a:link, a:visited {
        color: #38488f;
        text-decoration: none;
    }
    @media (max-width: 700px) {
        div {
            margin: 0 auto;
            width: auto;
        }
    }
    </style>
</head>

<body>
<div>
    <h1>Example Domain</h1>
    <p>This domain is for use in illustrative examples in documents. You may use this
    domain in literature without prior coordination or asking for permission.</p>
    <p><a href="https://www.iana.org/domains/example">More information...</a></p>
</div>
</body>
</html>
"
  ["headers"]=>
  array(13) {
    ["Accept-Ranges"]=>
    string(5) "bytes"
    ["Age"]=>
    string(6) "506325"
    ["Cache-Control"]=>
    string(14) "max-age=604800"
    ["Content-Type"]=>
    string(24) "text/html; charset=UTF-8"
    ["Date"]=>
    string(29) "Wed, 30 Oct 2024 15:37:43 GMT"
    ["Etag"]=>
    string(17) ""3147526947+gzip""
    ["Expires"]=>
    string(29) "Wed, 06 Nov 2024 15:37:43 GMT"
    ["Last-Modified"]=>
    string(29) "Thu, 17 Oct 2019 07:18:26 GMT"
    [""]=>
    string(16) "ECAcc (dcd/7D5A)"
    ["Vary"]=>
    string(15) "Accept-Encoding"
    ["X-Cache"]=>
    string(3) "HIT"
    ["Content-Length"]=>
    string(4) "1256"
    ["Connection"]=>
    string(5) "close"
  }
}

```

### `Mrloop::addTimer`

```php
public Mrloop::addTimer(float $interval, callable $callback): void
```

Executes a specified action after a specified amount of time.

**Parameter(s)**

- **interval** (float) - The amount of time (in seconds) to wait before executing a specified action.
- **callback** (callable) - The function in which the specified action due for execution after the aforestated interval has elapsed is defined.

**Return value(s)**

The function does not return anything.

```php
use ringphp\Mrloop;

$loop = Mrloop::init();

$loop->addTimer(
  2.0,
  function () {
    echo "Hello, user\n";
  },
);

$loop->run();
```

The example above will produce output similar to that in the snippet to follow.

```
Hello, user

```

### `Mrloop::addPeriodicTimer`

```php
public Mrloop::addPeriodicTimer(float $interval, callable $callback): void
```

Executes a specified action in perpetuity with each successive execution occurring after a specified time interval.

**Parameter(s)**

- **interval** (float) - The interval (in seconds) between successive executions of a specified action.
- **callback** (callable) - The function in which the specified action due for periodical execution is defined.
  > A return value of `0` will cancel the timer.

**Return value(s)**

The function does not return anything.

```php
use ringphp\Mrloop;

$loop = Mrloop::init();
$tick = 0;

$loop->addPeriodicTimer(
  2.0,
  function () use ($loop, &$tick) {
    echo \sprintf("Tick: %d\n", ++$tick);

    if ($tick === 5) {
      $loop->stop(); // return 0;
    }
  },
);

$loop->run();
```

The example above will produce output similar to that in the snippet to follow.

```
Tick: 1
Tick: 2
Tick: 3
Tick: 4
Tick: 5
```

### `Mrloop::addSignal`

```php
public Mrloop::addSignal(int $signal, callable $callback): void
```

Performs a specified action in the event that a specified signal is detected.

**Parameter(s)**

- **signal** (int) - The signal to listen for.
  > Only the signals `SIGINT`, `SIGTERM`, and `SIGHUP` are workable.
- **callback** (callable) - The function in which the specified action due for execution upon detection of a specified signal is defined.

**Return value(s)**

The function does not return anything.

```php
use ringphp\Mrloop;

$loop = Mrloop::init();

$loop->addReadStream(
  $fd = \fopen('/path/to/file', 'r'),
  null,
  function (...$args) use ($fd) {
    [$contents] = $args;

    echo \sprintf("%s\n", $contents);

    \fclose($fd);
  },
);

// CTRL + C to trigger
$loop->addSignal(
  SIGINT,
  function () use ($loop) {
    echo "Loop terminated with signal SIGINT\n";
  },
);

$loop->run();
```

The example above will produce output similar to that in the snippet to follow.

```
File contents...
Loop terminated with signal SIGINT
```

### `Mrloop::run`

```php
public Mrloop::run(): void
```

Runs the event loop.

- All code situated between the initialization of the loop and the run directive is funneled into the mrloop io_uring interface that is abstracted into the project.
- Invoking `run()` is mandatory.

> Please remember to minimize the use of expensive blocking calls in your code.

**Parameter(s)**

None.

**Return value(s)**

The function does not return anything.

### `Mrloop::stop`

```php
public Mrloop::stop(): void
```

Stops the event loop.

**Parameter(s)**

None.

**Return value(s)**

The function does not return anything.

```php
use ringphp\Mrloop;

$loop = Mrloop::init();

$loop->addReadStream(
  $fd = \fopen('/path/to/file', 'r'),
  'File contents...',
  function ($contents, $res) use ($fd, $loop) {
    echo \sprintf("%s\n", $contents);

    \fclose($fd);

    $loop->stop();
  },
);

$loop->run();
```

The example above will produce output similar to that in the snippet to follow.

```
File contents...
```
