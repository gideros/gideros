<?php

include("pusher.php");

$push = new Pusher();

$push->set_ios('Password', 'path/to/cert.pem');
$push->set_ios_devices('deviceToken');

$push->set_android("apitoken");
$push->set_android_devices("deviceToken");

$push->set_message("Notification with custom data");
$push->set_title("Custom data");
$push->set_number(1);
$push->set_sound("default");
$push->set_custom('{"x":157,"y":100}');

$push->push();

print_r($push->get_errors());

?>