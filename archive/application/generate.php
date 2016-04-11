<?php

$cli = php_sapi_name() === 'cli';

if ($cli)
foreach ($argv as $arg)
{
	$e=explode("=",$arg);
	if(count($e)==2)
		$_REQUEST[$e[0]]=$e[1];
	else
		$_REQUEST[$e[0]]="true";
}

require_once("generators/counter.php");

$width	= @$_REQUEST["w"] ? $_REQUEST["w"] : 210;
$height	= @$_REQUEST["h"] ? $_REQUEST["h"] : 300;
$mode	= @$_REQUEST["m"] ? $_REQUEST["m"] : "png";

//$run	= @$_REQUEST["run"] ? $_REQUEST["run"]=="true" : false;
$run	= $cli;
$filePath = $run ? __DIR__."/cache/image.".$mode : NULL;
$filePathTemp = $run ? $filePath.".tmp" : NULL;

$fps = 60;
$fpsDelay = 1000000/$fps;
$fpsA = 0;
$fpsC = 0;

if ($run) echo "Generating at $fps fps\n";
$time = time();

$im = @imagecreate($width, $height) or die("Cannot Initialize new GD image stream");

function output()
{
	global $im,$mode,$filePath,$filePathTemp,$run;

	switch($mode)
	{
		case "jpg":	if (!$run) header("Content-type: image/jpeg");
				imagejpeg($im,$filePathTemp,10);
				break;

		case "png":
		default:
				if (!$run) header("Content-Type: image/png");
				imagepng($im,$filePathTemp);
				break;
	}

	if ($run)
		rename($filePathTemp,$filePath);
}

do
{
	generate($im,$width,$height,$mode);
	output();

	$fpsC++;

	if ($run)
	{	echo "  $fpsA fps / wrote: $filePath\r";
		usleep($fpsDelay);

		if (time() > $time)
		{
			$fpsA = $fpsC;
			$fpsC = 0;
			$time = time();
		}
	}

} while ($run);

imagedestroy($im);

?>
