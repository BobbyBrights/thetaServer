
#include <stdio.h>
#include <iostream>
#include <time.h>
#include <unistd.h>

#include <QCoreApplication>
#include <QtConcurrentRun>
#include <QBuffer>
#include <QImageReader>
#include <QThread>

#include "proxy.h"

proxy::proxy(QString address)
: QObject()
, _address(address)
, _manager(this)
, _reply(0)
, _count(0)
, _fps(0)
, _time(0)
, _boundary(BOUNDARY)
, _headerSet(false)
//, _counter_sem("counter", MAX_SEM, QSystemSemaphore::Create)
//, _header_sem("header",   MAX_SEM, QSystemSemaphore::Create)
//, _image_sem("image",     MAX_SEM, QSystemSemaphore::Create)
, _counter_shm("counter", this)
, _header_shm("header", this)
, _image_shm("image", this)
{
	connect(&_manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));
	//connect(&_futureWatcher, SIGNAL(finished()), this, SLOT(frameRecompressed()));
	//connect(&_futureWatcher, SIGNAL(resultReadyAt(int)), this, SLOT(frameRecompressed(int)));
	//_futureWatcher.setFuture( _future );

	/*
	if (	_counter_sem.error() != QSystemSemaphore::NoError	||	
		_header_sem.error() != QSystemSemaphore::NoError	||
		_image_sem.error() != QSystemSemaphore::NoError		)
	{
		printf("Unable to create semaphores...\n");
		printf("counter: %s\nheader: %s\nimage: %s\n",qPrintable(_counter_sem.errorString()),qPrintable(_header_sem.errorString()),qPrintable(_image_sem.errorString()));
		QCoreApplication::exit(1);
	}
	*/

//	_counter_shm.setNativeKey("counter");
	_counter_shm.attach()	|| _counter_shm.create(50);
//	_header_shm.setNativeKey("header");
	_header_shm.attach()	|| _header_shm.create(500);
//	_image_shm.setNativeKey("image");
	_image_shm.attach()	|| _image_shm.create(1000000);

	if (	_counter_shm.error() != QSharedMemory::NoError	||	
		_header_shm.error() != QSharedMemory::NoError	||
		_image_shm.error() != QSharedMemory::NoError		)
	{
		printf("Unable to create shared memory segments...\n");
		printf("counter: %s\nheader: %s\nimage: %s\n",qPrintable(_counter_shm.errorString()),qPrintable(_header_shm.errorString()),qPrintable(_image_shm.errorString()));
		exit(1);
	}

	setCounter(0);

	getRequest();
}

proxy::~proxy()
{
}

void proxy::getRequest()
{
	printf("Connecting to %s...\n",qPrintable(_address));

	if (_reply) _reply->deleteLater();

	_headerSet = false;
	_buffer.clear();

	_reply = _manager.get(QNetworkRequest(QUrl(_address)));

	connect(_reply, SIGNAL(readyRead()), this, SLOT(replyReady()));
	connect(_reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(replyError(QNetworkReply::NetworkError)));
}

void proxy::replyFinished(QNetworkReply *)
{
	printf("Connection ended... reconnecting...\n");
	getRequest();
}

void proxy::replyError(QNetworkReply::NetworkError e)
{
	printf("Network error: %d\n%s\n",e,qPrintable(_reply->errorString()));
	//printf("FIXME: do we need to reconnect here?\n");

	QThread::sleep(3);
}

void proxy::replyReady()
{
	//printf("DATA READY!\n");

	if (!_headerSet)
	{
		QList<QByteArray> headers = _reply->rawHeaderList();

		QByteArray header;
		foreach( QByteArray h, headers )
		{	header += h+": "+_reply->rawHeader(h)+"\n";
		}

		//printf("%s\n",qPrintable(header));
		setHeader(header);
	}

	_buffer += _reply->readAll();

	int i = _buffer.indexOf( _boundary );
	if ( i > -1 )
	{
		QByteArray image = _buffer.left(i);
		_buffer.remove( 0, i+_boundary.size() );

		int n=0;
		int c=0;
		while (c++<4)	//get past the image header
		{	n = image.indexOf("\n",n+1);
			if (n==-1)
			{	//printf("Could not find image data... skipping.\n");
				return;
			}
		}

		//printf("I: %d N: %d\n%s\n",i,n,qPrintable(image.mid(0,n)));
		QByteArray ba = image.mid(n+1);
	//	QBuffer bu( &ba );
	//	QImageReader ir( &bu, "jpeg" );
	//	QImage f = ir.read();
		//printf("%d x %d\n",f.width(),f.height());

		_count++;

		QtConcurrent::run( this, &proxy::recompressFrame, ba, _count );

	/*	
		setImage( image );

		//printf(".\n");
		setCounter( _count );

		_count++;
		_fps++;

		uint t = QDateTime::currentDateTimeUtc().toTime_t();

		if (t > _time)
		{
			_time = t;
			printf("FPS: %d %d\n",_fps,_count);
			_fps = 0;
		}
	*/
	}
}

void proxy::frameRecompressed(QByteArray image, int fnum)
{
	//printf("int: %d\n",fnum);

	//setImage( image );
	//setCounter( fnum );

	_counter_shm.lock();
	_image_shm.lock();

	int cc;
	sscanf((char*) _counter_shm.data(),"%d",&cc);

	if (cc < fnum)
	{
		char *m = (char *) _counter_shm.data();
		sprintf(m,"%d",fnum);
		int size = image.size();
		char *m2 = (char *) _image_shm.data();
		memcpy( m2, &size, sizeof(size) );
		memcpy( m2+sizeof(size), image.data(), size );
	}
	//else
	//	printf("Skipping %d (%d)\n",fnum,cc);

	_image_shm.unlock();
	_counter_shm.unlock();

	_fps++;

	uint t = QDateTime::currentDateTimeUtc().toTime_t();

	if (t > _time)
	{
		_time = t;
		printf("FPS: %d %d\n",_fps,_count);
		_fps = 0;
	}
}

void proxy::setHeader(QByteArray header)
{
	_header_shm.lock();
	char *m = (char *) _header_shm.data();
	sprintf(m,"%s",qPrintable(header));
	_header_shm.unlock();

	_headerSet = true;
}

void proxy::setCounter(int c)
{
	_counter_shm.lock();
	char *m = (char *) _counter_shm.data();
	sprintf(m,"%d",c);
	_counter_shm.unlock();
}

void proxy::setImage(QByteArray image)
{
	int size = image.size();

	_image_shm.lock();
	char *m = (char *) _image_shm.data();
	memcpy( m, &size, sizeof(size) );
	memcpy( m+sizeof(size), image.data(), size );
	_image_shm.unlock();
}


//multithreading magic here...
void proxy::recompressFrame( QByteArray jpg, int fnum )
{
	QBuffer bu( &jpg );
	QImageReader ir( &bu, "jpeg" );
	QImage f = ir.read();
		
	//printf("Uncompressed frame %d: %d\n",fnum,jpg.size());

	//QImage f2 = f.scaledToWidth( f.width()/2 );
	QImage f2 = f;

	QBuffer bu2;
	f2.save(&bu2,"JPEG",50);

	//printf("Recompressed frame %d: %lld\n",fnum,bu2.size());
	QString h = QString( "Content-type: image/jpg\nContent-length: %1\nX-Timestamp: %2\n\n").arg(bu2.size()).arg(_time);
	QByteArray b = qPrintable(h) + bu2.data();

	frameRecompressed( b, fnum );
}


client::client()
: QObject()
, _headerSent(false)
, _counter_shm("counter", this)
, _header_shm("header", this)
, _image_shm("image", this)
{
//	_counter_shm.setNativeKey("counter");
	_counter_shm.attach();
//	_header_shm.setNativeKey("header");
	_header_shm.attach();
//	_image_shm.setNativeKey("image");
	_image_shm.attach();

	if (	_counter_shm.error() != QSharedMemory::NoError	||	
		_header_shm.error() != QSharedMemory::NoError	||
		_image_shm.error() != QSharedMemory::NoError		)
	{
		printf("Unable to create shared memory segments...\n");
		printf("counter: %s\nheader: %s\nimage: %s\n",qPrintable(_counter_shm.errorString()),qPrintable(_header_shm.errorString()),qPrintable(_image_shm.errorString()));
		exit(1);
	}

	//startStreaming();
	startTimer( 1000/15 );
}

void client::timerEvent(QTimerEvent *)
{
	stream();
}

void client::stream()
{
	int c=0;
	int counter;
	int size;

	_headerSent = true;		//php now sends the header
	if (!_headerSent)
	{
		_header_shm.lock();
		printf("%s",(char *)_header_shm.data());
		_header_shm.unlock();
		_headerSent = true;
	}

	//while (1)
	{
		//printf("c: %d %d\n", c, counter);

		_counter_shm.lock();
		sscanf((char*) _counter_shm.data(),"%d",&counter);
		_counter_shm.unlock();

		if (counter > c)
		{
			c = counter;

			_image_shm.lock();
			memcpy( &size, _image_shm.data(), sizeof(size) );
			QByteArray image( (char *) _image_shm.data() + sizeof(size), size );
			_image_shm.unlock();

			printf( "\n" );
			printf( BOUNDARY );
			//printf("Content-type: image/jpg\nContent-length:%d\n 
			fwrite( image.data(), image.size(), 1, stdout );
			//std::cout << image;
		}
		//else
			//usleep( 1000/15 );
		//	QThread::usleep( 1000/15 );
	}
}

client::~client()
{
}

