--TEST--
addTimer() executes a specified action after a specified amount of time
--FILE--
<?php

use ringphp\Mrloop;

$loop = Mrloop::init();

$loop->addTimer(
  1.5,
  function () use ($loop) {
    var_dump('Tick');

    $loop->stop();
  },
);

$loop->run();

?>
--EXPECT--
string(4) "Tick"
