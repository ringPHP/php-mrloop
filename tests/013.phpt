--TEST--
futureTick() schedules the execution of a specified action for the next event loop tick
--FILE--
<?php

use ringphp\Mrloop;

$loop = Mrloop::init();

$tick = 0;

$loop->futureTick(
  function () use (&$tick) {
    echo \sprintf("Tick: %d\n", ++$tick);
  },
);

$loop->futureTick(
  function () use ($loop, &$tick) {
    echo \sprintf("Tick: %d\n", ++$tick);
    $loop->stop();
  },
);

$loop->run();

?>
--EXPECT--
Tick: 1
Tick: 2
