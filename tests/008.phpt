--TEST--
addSignal() performs a specified action in the event that a specified signal is detected
--FILE--
<?php

use ringphp\Mrloop;

$loop = Mrloop::init();

$loop->addReadStream(
  \popen('echo "Readable"', 'r'),
  null,
  function (...$args) use ($loop) {
    [$data] = $args;

    echo \rtrim($data) . PHP_EOL;

    \posix_kill(\posix_getpid(), SIGINT);
  },
);

$loop->addSignal(
  SIGINT,
  function () {
    echo "Terminated with SIGINT\n";
  },
);

$loop->run();

?>
--EXPECT--
Readable
Terminated with SIGINT
