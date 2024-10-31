--TEST--
addSignal() performs a specified action in the event that a specified signal is detected
--FILE--
<?php

use ringphp\Mrloop;

$loop = Mrloop::init();

$loop->addTimer(
  1.2,
  function () {
    var_dump('Tick');

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
string(4) "Tick"
Terminated with SIGINT
