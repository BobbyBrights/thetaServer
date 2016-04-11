<?php

$colorsMade = false;
$color;
$background_color;
$checker_color;
$text_color;

function generate($im,$width,$height,$mode)
{
	global $colorsMade,$color,$background_color,$checker_color,$text_color;

	$checker = 50;

	if (!$colorsMade)
	{
		$color = sin(microtime(true))*50+50;
		$background_color = imagecolorallocate($im, $color, 50, 0);
		$checker_color = imagecolorallocate($im, 100, $color, 0);
		$text_color = imagecolorallocate($im, 233, 233, 233);
		$colorsMade = true;
	}

	imagefilledrectangle($im, 0, 0, $width, $height, $background_color);

	$p = false;
	$off = microtime(true)*10 % $checker*2;
	for ($x=-$off;$x<$width;$x+=$checker,$p=!$p)
		for ($y=-$off;$y<$height;$y+=$checker,$p=!$p)
			if ($p)
				imagefilledrectangle($im, $x, $y, $x+$checker, $y+$checker, $checker_color);

	imagestring($im, 5, 10, $height-25,  $mode." ".microtime(true), $text_color);
}

?>
