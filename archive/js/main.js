
$(window).load(init);
$(window).ready(ready);

var _fps = 30;
var delay = 1000/_fps;
//var url = 'application/generate.php?m=png&w=500';
var url = 'application/cache/image.jpg';

var _fcount=0;

var running = false;

var buffer;
var ppButton;
var fps;

function init()
{
	//console.log("ready");
	ppButton = $("#playPause");
	ppButton.bind({
		'click': playPause
	});

	fps = $("#fps");
}

function ready()
{
	//console.log("init");

	buffer = $("#buffer");
	buffer.bind({
		'load': imageLoaded,
		'error': imageLoadError
	});

	refresh();
	startFpsCounter();
}

function imageLoaded(evt)
{
	//console.log("loaded!");

	if (running)
		setTimeout( refresh, delay );
}

function imageLoadError(evt)
{
	console.log("error!");

	if (running)
		setTimeout( refresh, delay );
}

function refresh()
{
	var rand = Math.random();
	//console.log(rand);

	buffer.attr("src",url+"?r="+rand);
	_fcount++;
}

function playPause()
{
	running = !running;

	if (running)
	{	ppButton.val("Pause");
		refresh();
	}
	else
	{	ppButton.val("Play");
	}
}

function startFpsCounter()
{
	setInterval( function() {
		fps.text(_fcount+" fps");
		_fcount=0;
	}, 1000);
}


