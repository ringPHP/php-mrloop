--TEST--
parseHttpResponse() throws exception when invalid HTTP response syntax is encountered
--FILE--
<?php

use ringphp\Mrloop;

try {
  var_dump(
    Mrloop::parseHttpResponse("foo\r\nbar\r\n"),
  );
} catch (Throwable $err) {
  var_dump(
    $err->getMessage(),
  );
}

?>
--EXPECT--
string(45) "There is an error in the HTTP response syntax"
