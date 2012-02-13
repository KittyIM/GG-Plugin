#include "HUBLookup.h"

#include <QtCore/QStringList>
#include <QtCore/QEventLoop>
#include <QtCore/QDebug>
#include <QtCore/QUrl>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>

namespace KittyGG
{

void HUBLookup::run()
{
	QNetworkAccessManager nam;
	QNetworkReply *reply = nam.get(QNetworkRequest(QUrl("http://appmsg.gadu-gadu.pl/appsvc/appmsg_ver8.asp?fmnumber=1")));

	QEventLoop loop;
	connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));

	//block until request is finished
	loop.exec();

	QString hostname;
	QString data = reply->readAll();
	if(data != "notoperating") {
		QStringList parts = data.split(' ');
		if((parts.count() == 4) && (parts[0] == "0")) {
			hostname = parts[3].trimmed();
		}
	}

	emit serverFound(hostname);
}

}
