# ext-mrloop

A PHP port of an event loop designed to harness the powers of `io_uring`.

## Rationale

PHP has, in recent years, seen an emergence of eventware built atop potent multiplexing functions like `epoll()`, `poll()`, and `select()`. In event loops like `libev`, `libuv`, and `libevent` are powerful abstractions of the aforelisted functions for interleaving I/O inside a single process. Eponymous PHP extensions have, in the years since the popularization of the non-blocking I/O paradigm, been created to avail users of the language of the many potencies of the aforestated libraries. `io_uring` is yet another async system call API, and mrloop is an event loop built atop its interface. The former, which the latter is written atop—presents an enhancive proposition: unlike the commonplace non-blocking I/O APIs, it is a unified evented I/O interface with an emphasis on efficient userspace-kernel communications. `io_uring` is, internally, a set of [ring buffers](https://en.wikipedia.org/wiki/Circular_buffer)—structures the operations on which are of an **O(1)** complexity—whose communication mechanism is such that a userspace-resident application places a file descriptor on a submission queue and subsequently retrieves matching output from a kernel-controlled completion queue. It confers performance benefits that stem from a small system call footprint and is thus useful to any PHP developer keen on evented I/O.

## Requirements

- PHP 8.1 or newer
- Linux Kernel 5.4.1 or newer
- [mrloop](https://github.com/markreedz/mrloop)
- [liburing](https://github.com/axboe/liburing)

## Installation

It is important to have all the aforelisted requirements at the ready before attempting to install `ext-mrloop`. The directives in the snippet to follow should allow you to build the extension's shared object file (`mrloop.so`).

```sh
$ git clone https://github.com/ace411/mrloop.git <mrloop-dir>
$ git clone https://github.com/ringphp/php-mrloop.git <dir>
$ cd <dir>
$ phpize && ./configure --with-mrloop=<mrloop-dir>
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
    ?int $vcount,
    ?int $offset,
    callable $callback,
  ): void
  public addWriteStream(
    resource $stream,
    string $contents,
    ?int $vcount,
    callable $callback,
  ): void
  public tcpServer(
    int $port,
    ?int $connections,
    ?int $nbytes,
    callable $callback,
  ): void
  public writev(int|resource $fd, string $message): void
  public addTimer(float $interval, callable $callback): void
  public addPeriodicTimer(float $interval, callable $callback): void
  public futureTick(callable $callback): void
  public addSignal(int $signal, callable $callback): void
  public run(): void
  public stop(): void
}
```

- [`Mrloop::init`](#mrloopinit)
- [`Mrloop::addReadStream`](#mrloopaddreadstream)
- [`Mrloop::addWriteStream`](#mrloopaddwritestream)
- [`Mrloop::tcpServer`](#mrlooptcpserver)
- [`Mrloop::writev`](#mrloopwritev)
- [`Mrloop::addTimer`](#mrloopaddtimer)
- [`Mrloop::addPeriodicTimer`](#mrloopaddperiodictimer)
- [`Mrloop::futureTick`](#mrloopfuturetick)
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
  ?int $vcount,
  ?int $offset,
  callable $callback,
): void
```

Funnels file descriptor in readable stream into event loop and thence executes a non-blocking read operation.

**Parameter(s)**

- **stream** (resource) - A userspace-defined readable stream.
  > The file descriptor in the stream is internally given a non-blocking disposition.
- **nbytes** (int|null) - The number of bytes to read.
  > Specifying `null` will condition the use of a 1KB buffer.
- **vcount** (int|null) - The number of read vectors to use.
  > Specifying `null` will condition the use of `1` vector.
  > Any value north of `8` will likely result in an inefficient read.
- **offset** (int|null) - The point at which to start the read operation.
  > Specifying `null` will condition the use of an offset of `0`.
- **callback** (callable) - The binary function through which the file's contents and read result code are propagated.

**Return value(s)**

The function does not return anything.

```php
use ringphp\MrLoop;

$loop = Mrloop::init();

$loop->addReadStream(
  $fd = \fopen('/path/to/file', 'r'),
  null,
  null,
  null,
  function (string $contents, int $res) use ($fd) {
    if ($res === 0) {
      echo \sprintf("%s\n", $contents);
    }

    \fclose($fd);
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
  ?int $vcount,
  callable $callback,
): void
```

Funnels file descriptor in writable stream into event loop and thence executes a non-blocking write operation.

**Parameter(s)**

- **stream** (resource) - A userspace-defined writable stream.
  > The file descriptor in the stream is internally given a non-blocking disposition.
- **contents** (string) - The contents to write to the file descriptor.
- **vcount** (int|null) - The number of write vectors to use.
  > Specifying `null` will condition the use of `1` vector.
  > Any value north of `8` will likely result in an inefficient write.
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
  null,
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
public Mrloop::tcpServer(
  int $port,
  ?int $connections,
  ?int $nbytes,
  callable $callback,
): void
```

Instantiates a simple TCP server.

**Parameter(s)**

- **port** (int) - The port on which to listen for incoming connections.
- **connections** (int|null) - The maximum number of connections to accept.
  > This parameter does not have any effect when a version of mrloop in which the `mr_tcp_server` function lacks the `max_conn` parameter is included in the compilation process.
  > Specifying `null` will condition the use of a `1024` connection threshold.
- **nbytes** (int|null) - The maximum number of readable bytes for each connection.
  > This setting is akin to the `client_max_body_size` option in NGINX.
  > Specifying null will condition the use of an `8192` byte threshold.
- **callback** (callable) - The binary function with which to define a response to a client.
  > Refer to the segment to follow for more information on the callback.
  - **Callback parameters**
    - **message** (string) - The message sent via client socket to the server.
    - **client** (iterable) - An array containing client socket information.
      - **client_addr** (string) - The client IP address.
      - **client_port** (integer) - The client socket port.
      - **client_fd** (integer) - The client socket file descriptor.

**Return value(s)**

The function does not return anything.

```php
use ringphp\Mrloop;

$loop = Mrloop::init();

$loop->tcpServer(
  8080,
  null,
  null,
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

### `Mrloop::writev`

```php
public Mrloop::writev(int|resource $fd, string $contents): void
```

Performs vectorized non-blocking write operation on a specified file descriptor.

**Parameter(s)**

- **fd** (integer|resource) - The file descriptor to write to.
- **contents** (string) - The arbitrary contents to write.

**Return value(s)**

The parser will throw an exception in the event that an invalid file descriptor is encountered and will not return anything otherwise.

```php
use ringphp\Mrloop;

$loop = Mrloop::init();

$loop->tcpServer(
  8080,
  null,
  null,
  function (string $message, iterable $client) use ($loop) {
    [
      'client_addr' => $addr,
      'client_port' => $port,
      'client_fd'   => $fd,
    ] = $client;

    $loop->writev(
      $fd,
      \sprintf(
        "Hello, %s:%d\r\n",
        $addr,
        $port,
      ),
    );
  },
);

echo "Listening on port 8080\n";

$loop->run();
```

The example above will produce output similar to that in the snippet to follow.

```
Listening on port 8080

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

### `Mrloop::futureTick`

```php
public Mrloop::futureTick(callable $callback): void
```

Schedules the execution of a specified action for the next event loop tick.

**Parameter(s)**

- **callback** (callable) - The function in which the action to be scheduled is defined.

**Return value(s)**

The function does not return anything.

```php
use ringphp\Mrloop;

$loop = Mrloop::init();
$tick = 0;

$loop->futureTick(
  function () use (&$tick) {
    echo \sprintf("Tick: %d\n", ++$tick);
  },
);

$loop->futureTick(
  function () use (&$tick) {
    echo \sprintf("Tick: %d\n", ++$tick);
  },
);

$loop->run();
```

The example above will produce output similar to that in the snippet to follow.

```
Tick: 1
Tick: 2
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
  null,
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
  null,
  null,
  null,
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
