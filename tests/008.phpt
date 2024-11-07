--TEST--
addSignal() performs a specified action in the event that a specified signal is detected
--SKIPIF--
<?php

if (
  !(
    \extension_loaded('posix') &&
    \extension_loaded('pcntl')
  )
) {
  echo 'skip';
}

?>
--FILE--
<?php

use ringphp\Mrloop;

$loop = Mrloop::init();

$loop->addTimer(
  1.2,
  function () {
    var_dump('Tick');

    \posix_kill(
      \posix_getpid(),
      (\defined('SIGINT') ? SIGINT : 2),
    );
  },
);

$loop->addSignal(
  (\defined('SIGINT') ? SIGINT : 2),
  function () {
    echo "Terminated with SIGINT\n";
  },
);

$loop->run();

?>
--EXPECT--
string(4) "Tick"
Terminated with SIGINT
