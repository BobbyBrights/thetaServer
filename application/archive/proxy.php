<?

$address = "http://localhost:8090/?action=stream";

$cli = php_sapi_name() === 'cli';
$shm_key_base = ftok(__FILE__, 't');

$maxConnections = 50;
$boundary = "--boundarydonotcross\r\n";

if ($cli)
	@set_time_limit(0);
else
	set_time_limit(0);

$shms = array(
	"counter" => array(	"key" => $shm_key_base+1,
				"size" => 50,
				"id" => null
			),
	"header"  => array(	"key" => $shm_key_base+2,
				"size" => 500,
				"id" => null
			),
	"image"   => array(	"key" => $shm_key_base+4,
				"size" => 1000000,
				"id" => null
			)
);

foreach($shms as $name=>&$s)
{
	if ( ($s["sem"] = sem_get( $s["key"], $maxConnections )) === FALSE )
		die("Unable to get semaphore for '$name' @ {$s["key"]}\n");
	else
	{
		if ( $s["id"] = shm_attach( $s["key"], $s["size"], 0777) )
		{
			//echo "Attached semaphore for '$name' of size {$s["size"]}\n";
		}
		else
			die("Unable to attach '$name' of size {$s["size"]}\n");
	}
}

//print_r($shms);
//die("Created all shared memory chunks.");

function semwr($key,$value)
{
	global $shms;

	/*
	if (!$value)
	{	echo "Cannot write null value!\n";
		return;
	}
	*/

	if (sem_acquire( $shms[$key]["sem"] ))
	{
		//echo "Got write for $key\n";
		if ( !shm_put_var( $shms[$key]["id"], $shms[$key]["key"], $value ) )
			die("Could not put value in $key.\n");
		else
			sem_release( $shms[$key]["sem"] );
	}
	else
		die("Could not acquire resource for $key");
}

function semrd($key)
{
	global $shms;

	$value = null;

	if (sem_acquire( $shms[$key]["sem"] ))
	{
		if ( ($value = shm_get_var( $shms[$key]["id"], $shms[$key]["key"])) === FALSE )
		{	//@sem_release( $shms[$key]["sem"] );
			//print_r($shms);
			//die("Could not acquire lock for $key");
			return NULL;
		}

		sem_release( $shms[$key]["sem"] );
	}

	return $value;
}


//print_r($shms);
//die();


if ($cli)
{
	$opts = array('http' =>
	    array(
		'method' => 'GET',
		'max_redirects' => '0',
		'ignore_errors' => '1'
	    )
	);

	$context = stream_context_create($opts);
	$stream = fopen($address, 'r', false, $context);

	// header information as well as meta data
	// about the stream
	//var_dump(stream_get_meta_data($stream));
	$metadata = stream_get_meta_data($stream);

	// actual data at $url
	//var_dump(stream_get_contents($stream));
	//fclose($stream);

	//$header = implode("\n",$metadata["wrapper_data"]);
	$header = $metadata["wrapper_data"];

	//echo $header;
	semwr("header",$header);

	//print_r($shms);
	//die();

	$c = 0;
	$fps = 0;
	$time = time();

	while(1)
	{
		$line = stream_get_line($stream,500000,$boundary);
		//echo substr($line, 0, 200);

		if (false)
		{
			$idata = @explode("\r\n",$line,5)[4];
			if ($idata)
			{
				$image = imagecreatefromstring ( $idata );
				
				ob_start();
				imagejpeg($image,null,50);
				$image2 = ob_get_contents(); // read from buffer
				ob_end_clean(); // delete buffer
				//file_put_contents("/var/www/html/test.jpg",$image2);
				imagedestroy($image);
				$l = strlen($image2);
				$t = time();
				$image2 = "Content-Type: image/jpeg\nContent-Length: {$l}\n\X-Timestamp: {$t}\n\n".$image2;
			}
			else
			{	echo "Bad frame: $c\n";
				continue;
			}

			semwr("image",$image2);
		}
		else
			semwr("image",$line);

		semwr("counter",$c);

		$c++;
		$fps++;

		$t = time();
		if ($t > $time)
		{
			$time = $t;
			echo "FPS: $fps $time $c\n";
			$fps = 0;
		}
	}
}
else
{
	$header = semrd("header");

	if ($header === null)
	{	//header("Location: /application/mjpg");
		die("Can't get header!");
	}

	//$headers = explode("\n",$header);
	header_remove("Vary");
	header_remove("Content-Length");
	header_remove("Date");
	foreach($header as $h)
		header($h,true);		//replace the headers...
	//echo $header."\n";

	//$c = semrd("counter");
	$first = true;
	$c = 0;
	//echo "Frame: $c\n";
	//die("Frame: $c\n");

	//ob_end_flush();
	//$image = semrd("image");

	while(1)
	{
		$f = semrd("counter");

		if ($f !== null)
		if ($f != $c)
		{
			//echo "Frame: $f\n";
			$c = $f;

			$image = semrd("image");

			if ($image !== null)
			{
				echo $boundary;
				echo $image;
			}
		}

		usleep(1000/15);

		//die();
	}
}

?>
