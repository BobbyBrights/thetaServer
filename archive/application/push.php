<?
	//http://www.howtocreate.co.uk/php/serverpushdemo.php

	$UUID = md5(uniqid());		//to make a long random-ish boundary

	header("Content-Type: multipart/x-mixed-replace;boundary=$UUID");

	



?>
