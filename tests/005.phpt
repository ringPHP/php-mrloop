--TEST--
parseHttpRequest() throws exception when invalid HTTP request syntax is encountered
--FILE--
<?php

use ringphp\Mrloop;

try {
  var_dump(
    Mrloop::parseHttpRequest("foo\nbar\n"),
  );
} catch (Throwable $err) {
  var_dump(
    $err->getMessage(),
  );
}

?>
--EXPECT--
string(44) "There is an error in the HTTP request syntax"
