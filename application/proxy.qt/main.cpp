

#include  <signal.h>

#include <QCoreApplication>

#include "proxy.h"

void PipeHandler(int);

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	//QCoreApplication::setOrganizationName(ORGANIZATION_NAME);
	//QCoreApplication::setOrganizationDomain(ORGANIZATION_DOMAIN);
	//QCoreApplication::setApplicationName(APPLICATION_NAME);
	//QCoreApplication::setApplicationVersion(APPLICATION_VERSION);

	signal(SIGPIPE, PipeHandler);

	QStringList args = a.arguments();
	bool clientMode = (args.length() > 1 && args[1] == "client");

	if (clientMode)
		new client();
	else
		new proxy("http://localhost:8090/?action=stream");

	return a.exec();
}

//this will kill the app if php/apache disconnect
void PipeHandler(int)
{
	exit(2);
}

