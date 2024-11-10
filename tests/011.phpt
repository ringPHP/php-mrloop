--TEST--
writev() performs vectorized non-blocking write operation on file descriptor
--FILE--
<?php

use ringphp\Mrloop;

$loop = Mrloop::init();

$loop->writev(1, "Hello, user");
$loop->stop();

$loop->run();

?>
--EXPECT--
Hello, user
