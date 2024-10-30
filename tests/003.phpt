--TEST--
addWriteStream() funnels file descriptor in writable stream into event loop and thence executes a non-blocking write operation
--FILE--
<?php

use ringphp\Mrloop;

$loop = Mrloop::init();

$file = \sprintf("%s/file.txt", __DIR__);

$loop->addWriteStream(
  \fopen($file, 'w'),
  'Lorem ipsum dolor sit amet, consectetur adipiscing elit.',
  function (int $nbytes) use ($file, $loop) {
    var_dump($nbytes);

    \unlink($file);

    $loop->stop();
  },
);

$loop->run();

?>
--EXPECT--
int(56)
