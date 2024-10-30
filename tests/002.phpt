--TEST--
addReadStream() funnels file descriptor in readable stream into event loop and thence executes a non-blocking read operation
--FILE--
<?php

use ringphp\Mrloop;

$loop = Mrloop::init();

$loop->addReadStream(
  \popen('php -m | grep "mrloop"', 'r'),
  null,
  function (...$args) use ($loop) {
    [$data] = $args;

    var_dump(\rtrim($data));

    $loop->stop();
  },
);

$loop->run();

?>
--EXPECT--
string(6) "mrloop"
