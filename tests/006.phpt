--TEST--
parseHttpResponse() parses HTTP response
--FILE--
<?php

use ringphp\Mrloop;

var_dump(
  Mrloop::parseHttpResponse(
    "HTTP/1.1 200 OK\r\nserver: mrloop\r\ncontent-type: text/plain\r\n\r\nHello, user",
  ),
);

?>
--EXPECT--
array(4) {
  ["reason"]=>
  string(2) "OK"
  ["status"]=>
  int(200)
  ["body"]=>
  string(11) "Hello, user"
  ["headers"]=>
  array(2) {
    ["server"]=>
    string(6) "mrloop"
    ["content-type"]=>
    string(10) "text/plain"
  }
}
