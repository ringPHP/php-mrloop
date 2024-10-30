--TEST--
init() creates instance of Mrloop
--FILE--
<?php

use ringphp\Mrloop;

$loop = Mrloop::init();

var_dump($loop instanceof Mrloop);

?>
--EXPECT--
bool(true)
