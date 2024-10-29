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
  public freadv(
    string $file,
    ?int $offset,
    ?int $nbytes,
    callable $callback,
  ): void
  public fwritev(
    string $file,
    string $contents,
    int $flags,
    callable $callback,
  ): void
  public preadv(
    string $command,
    ?int $nbytes,
    callable $callback,
  ): void
  public pwritev(
    string $command,
    string $contents,
    callable $callback,
  ): void
  public tcpServer(int $port, callable $callback): void
  public static parseHttp(string $request, int $headerlimit = 100): iterable
  public addTimer(float $interval, callable $callback): void
  public addPeriodicTimer(float $interval, callable $callback): void
  public addSignal(int $signal, callable $callback): void
  public run(): void
  public stop(): void
}
```

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

### `Mrloop::freadv`

```php
public Mrloop::freadv(
  string $file,
  ?int $offset,
  ?int $nbytes,
  callable $callback,
): void
```

Performs a non-blocking file read operation.

**Parameter(s)**

- **file** (string) - The file to read.
- **offset** (int|null) - The cursor position at which to start the read operation.
  > Specifying `null` will condition the start of the read operation at the beginning of the file (offset `0`).
- **nbytes** (int|null) - The number of bytes to read.
  > Specifying `null` will condition the termination of a read operation at the end of a file.
- **callback** (callable) - The unary function through which the file's contents are propagated.

**Return value(s)**

The function does not return anything.

```php
use ringphp\MrLoop;

$loop = Mrloop::init();

$loop->freadv(
  '/path/to/file',
  null, // will default to 0
  null, // will default to the file size
  function (string $contents) {
    echo \sprintf("%s\n", $contents);
  },
);

$loop->run();
```

The example above will produce output similar to that in the snippet to follow.

```
File contents...

```

### `Mrloop::fwritev`

```php
public fwritev(
  string $file,
  string $contents,
  int $flags,
  callable $callback,
): void
```

Performs a non-blocking file write operation.

**Parameter(s)**

- **file** (string) - The file to write to.
- **contents** (string) - The contents to write.
- **flags** (int) - Relevant flags that condition write operations.
  > All flags that apply to the [`file_put_contents`](https://www.php.net/manual/en/function.file-put-contents.php) function apply here.
- **callback** (callable) - The unary function through which the number of written bytes is propagated.

**Return value(s)**

The function does not return anything.

```php
use ringphp\Mrloop;

$loop = MrLoop::init();

$file = '/path/to/file';
$loop->fwritev(
  $file,
  "file contents...\n",
  LOCK_EX,
  function (int $nbytes) use ($file) {
    echo \sprintf("Wrote %d bytes to %s\n", $nbytes, $file);
  },
);

$loop->run();
```

The example above will produce output similar to that in the snippet to follow.

```
Wrote 18 bytes to /path/to/file

```

### `Mrloop::preadv`

```php
public Mrloop::preadv(
  string $command,
  ?int $nbytes,
  callable $callback,
): void
```

Opens a process, executes it in a non-blocking fashion, and relays the shell output stream.

**Parameter(s)**

- **command** (string) - The process to execute.
- **nbytes** (int|null) - The number of bytes of the output stream to read.
  > Setting the value to `null` conditions the internal use of a buffer that is `8192` bytes long.
- **callback** (callable) - The function through which the contents of the shell output stream are propagated.

**Return value(s)**

The function does not return anything.

```php
use ringphp\Mrloop;

$loop = Mrloop::init();

$loop->preadv(
  'ls -a',
  null,
  function (string $contents) {
    echo \sprintf("%s\n", $contents);
  },
);

$loop->run();
```

The example above will produce output similar to that in the snippet to follow.

```
.
..
dir-1
dir-2
file-1
file-2

```

### `Mrloop::pwritev`

```php
public Mrloop::pwritev(
  string $command,
  string $contents,
  callable $callback,
): void
```

Opens a process, funnels arbitrary input into its writable stream, and thence executes it in a non-blocking fashion.

**Parameter(s)**

- **command** (string) - The process to execute.
- **contents** (int|null) - The data to funnel into the process' writable stream.
- **callback** (callable) - The function through which the contents of the shell output stream are propagated.

**Return value(s)**

The function does not return anything.

```php
use ringphp\Mrloop;

$loop = Mrloop::init();

$loop->pwritev(
  'cat >> /path/to/file',
  'File contents...',
  function (int $nbytes) {
    var_dump($nbytes);
  },
);

$loop->run();
```

The example above will produce output similar to that in the snippet to follow.

```
int(16)

```

### `Mrloop::tcpServer`

```php
public tcpServer(int $port, callable $callback): void
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

    // echo server
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

### `Mrloop::parseHttp`

```php
public static Mrloop::parseHttp(string $request, int $headerlimit = 100): iterable
```

Parses an HTTP request.

> This is an opt-in function that utilizes the picohttpparser's robust API.

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
      $request = Mrloop::parseHttp($message);

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

### `Mrloop::addTimer`

```php
public addTimer(float $interval, callable $callback): void
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
public addPeriodicTimer(float $interval, callable $callback): void
```

Executes a specified action in perpetuity with each successive execution occurring after a specified time interval.

**Parameter(s)**

- **interval** (float) - The interval (in seconds) between successive executions of a specified action.
- **callback** (callable) - The function in which the specified action due for periodical execution is defined.

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
      $loop->stop();
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
public addSignal(int $signal, callable $callback): void
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

$loop->freadv(
  '/path/to/file',
  null,
  null,
  function (string $contents) {
    echo \sprintf("%s\n", $contents);
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

$loop->readv(
  '/path/to/file',
  null,
  null,
  function (string $contents) use ($loop) {
    echo \sprintf("%s\n", $contents);

    $loop->stop();
  },
);

$loop->run();
```

The example above will produce output similar to that in the snippet to follow.

```
File contents...
```
