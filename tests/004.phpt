--TEST--
parseHttpRequest() parses HTTP request
--FILE--
<?php

use ringphp\Mrloop;

var_dump(
  Mrloop::parseHttpRequest(
    "GET / HTTP/1.1\r\nhost: localhost:8080\r\ncontent-type: text/plain\r\n\r\n",
  ),
);
?>
--EXPECT--
array(4) {
  ["path"]=>
  string(1) "/"
  ["method"]=>
  string(3) "GET"
  ["body"]=>
  string(0) ""
  ["headers"]=>
  array(2) {
    ["host"]=>
    string(14) "localhost:8080"
    ["content-type"]=>
    string(10) "text/plain"
  }
}
