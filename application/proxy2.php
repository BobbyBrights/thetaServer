<?
	set_time_limit(0);	

	$base = dirname(__FILE__);
	$boundary = "--boundarydonotcross\r\n";

	foreach (headers_list() as $header)
		header_remove($header);

	header_remove("Vary");
	header_remove("Content-Length");
	header_remove("Date");

	header("Access-Control-Allow-Origin: *", true);
	header("Connection: close", true);
	header("Server: MJPG-Streamer/0.2", true);
	header("Cache-Control: no-store, no-cache, must-revalidate, pre-check=0, post-check=0, max-age=0",true);
	header("Pragma: no-cache", true);
	header("Expires: Mon, 3 Jan 2000 12:34:56 GMT", true);
	header("Content-Type: multipart/x-mixed-replace;boundary=boundarydonotcross", true);

	passthru("$base/proxy.qt/proxy client");

	die();


	/*
	$fp = popen("$base/proxy.qt/proxy client", "r");
	while(!feof($fp)) 
	{
		if (connection_aborted())
			exit();

		// send the current file part to the browser 
		print fread($fp, 1024);

		// flush the content to the browser 
		flush(); 
	} 
	fclose($fp);
	*/


	ignore_user_abort(true);

	
	$descriptorspec = array(
	   0 => array("pipe", "r"),  // stdin is a pipe that the child will read from
	   1 => array("pipe", "w"),  // stdout is a pipe that the child will write to
//	   2 => array("file", "/tmp/error-output.txt", "a") // stderr is a file to write to
	);

	$cwd = '/tmp';
	//$env = array('some_option' => 'aeiou');
	$env = array();

	$process = proc_open("$base/proxy.qt/proxy client", $descriptorspec, $pipes, $cwd, $env);

	stream_set_timeout($pipes[1], 1);

	if (is_resource($process)) {
	    // $pipes now looks like this:
	    // 0 => writeable handle connected to child stdin
	    // 1 => readable handle connected to child stdout
	    // Any error output will be appended to /tmp/error-output.txt

	//    fwrite($pipes[0], '</?php print_r($_ENV); ?/>');
	//    fclose($pipes[0]);

		while(1)
		{
			echo stream_get_contents($pipes[1],1024);
			flush();

			$m = stream_get_meta_data($pipes[1]);
			if ($m["timed_out"])
				break;

			if (connection_aborted())
				break;
		}
	    fclose($pipes[1]);

	    // It is important that you close any pipes before calling
	    // proc_close in order to avoid a deadlock
	    $return_value = proc_close($process);

	//    echo "command returned $return_value\n";
	}

?>
