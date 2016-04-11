
#ifndef PROXY_H
#define PROXY_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
//#include <QSystemSemaphore>
#include <QSharedMemory>
#include <QFutureWatcher>

//#define MAX_SEM 10

#define BOUNDARY "--boundarydonotcross\r\n"

class proxy : public QObject
{
	Q_OBJECT

	public:
		proxy(QString address);
		~proxy();

	protected:
		QString			_address;
		QNetworkAccessManager	_manager;
		QNetworkReply		*_reply;

		int			_count;
		int			_fps;
		uint			_time;
		QByteArray		_boundary;
		QByteArray		_buffer;

		bool			_headerSet;
		//QSystemSemaphore	_counter_sem;
		//QSystemSemaphore	_header_sem;
		//QSystemSemaphore	_image_sem;
		QSharedMemory		_counter_shm;
		QSharedMemory		_header_shm;
		QSharedMemory		_image_shm;

		void recompressFrame( QByteArray jpg, int fnum );

	private:
		void getRequest();
		void setHeader(QByteArray header);
		void setCounter(int c);
		void setImage(QByteArray image);

	protected slots:
		void replyFinished(QNetworkReply*);
		void replyReady();
		void replyError(QNetworkReply::NetworkError);
		void frameRecompressed(QByteArray, int);
};

class client : public QObject
{
	Q_OBJECT

	public:
		client();
		~client();

	protected:
		bool			_headerSent;

		QSharedMemory		_counter_shm;
		QSharedMemory		_header_shm;
		QSharedMemory		_image_shm;

	private:
		void stream();
		void timerEvent(QTimerEvent *e);
};

#endif //PROXY_H

