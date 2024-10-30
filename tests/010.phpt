--TEST--
addPeriodicTimer() executes a specified action in perpetuity with each successive execution occurring after a specified time interval
--FILE--
<?php

use ringphp\Mrloop;

$loop = Mrloop::init();

$tick = 0;

$loop->addPeriodicTimer(
  1.2,
  function () use ($loop, &$tick) {
    var_dump(++$tick);

    if ($tick === 5) {
      $loop->stop();
    }
  },
);

$loop->run();

?>
--EXPECT--
int(1)
int(2)
int(3)
int(4)
int(5)
