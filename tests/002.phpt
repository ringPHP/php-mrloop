--TEST--
addReadStream() funnels file descriptor in readable stream into event loop and thence executes a non-blocking read operation
--FILE--
<?php

use ringphp\Mrloop;

$loop = Mrloop::init();

$loop->addReadStream(
  \popen('pwd', 'r'),
  null,
  function (...$args) use ($loop) {
    [$message] = $args;

    var_dump(
      (bool) \preg_match(
        \sprintf(
          '/%s/ix',
          \preg_quote($message, '/'),
        ),
        __DIR__,
      ),
    );

    $loop->stop();
  },
);

$loop->run();

?>
--EXPECT--
bool(true)
