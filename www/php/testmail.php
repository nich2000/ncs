<?php
ini_set('display_errors', 1);
error_reporting( E_ALL );

require_once "Mail.php";

$from = "nich2000@yandex.ru";
$to = "nich2000@yandex.ru"; //CHANGE THIS TO YOUR GMAIL ADDRESS WELL
$subject = "Hi!";
$body = "Hi,\n\nHow are you?";

$host = "ssl://smtp.yandex.ru";
$port = "465";
$username = "nich2000@yandex.ru"; //CHANGE THIS TO YOUR GMAIL ADDRESS WELL
$password = "Ak-74$055400"; //CHANGE THIS TO YOUR GMAIL PASSWORD

$headers = array ('From' => $from,
'To' => $to,
'Subject' => $subject);
$smtp = Mail::factory('smtp',
array ('host' => $host,
'port' => $port,
'auth' => true,
'username' => $username,
'password' => $password));

$mail = $smtp->send($to, $headers, $body);

if (PEAR::isError($mail)) {
echo("<p>" . $mail->getMessage() . "</p>");
} else {
echo("<p>Message successfully sent!</p>");
}
?>
