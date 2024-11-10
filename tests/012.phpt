--TEST--
writev() throws exception on detection of invalid file descriptor
--FILE--
<?php

use ringphp\Mrloop;

$loop = Mrloop::init();

try {
  $loop->writev(987874, "Hello, user");
} catch (\Throwable $err) {
  $loop->writev(1, $err->getMessage());
}
$loop->stop();

$loop->run();

?>
--EXPECT--
Detected invalid file descriptor
