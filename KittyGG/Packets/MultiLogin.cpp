#include "MultiLogin.h"

#include <QtCore/QtEndian>
#include <QtNetwork/QHostAddress>

namespace KittyGG
{

MultiLogin::~MultiLogin()
{
	qDeleteAll(m_items);
}

MultiLogin MultiLogin::fromData(const QByteArray &data)
{
	quint32 count;
	quint32 ip;
	quint32 time;
	quint32 unknown;
	quint32 client_length;
	char *client = 0;

	QDataStream str(data);
	str.setByteOrder(QDataStream::LittleEndian);

	str >> count;

	MultiLogin ml;
	for(quint32 i = 0; i < count; ++i) {
		MultiLoginItem *item = new MultiLoginItem;

		str >> ip;
		str >> item->flags;
		str >> item->features;
		str >> time;
		str >> item->conn_id;
		str >> unknown;
		str >> client_length;

		if(client_length > 0) {
			client = new char[client_length];

			str.readRawData(client, client_length);
			item->client = QString::fromAscii(client, client_length);

			delete client;
		}

		item->ip = QHostAddress(qFromBigEndian(ip)).toString();
		item->time = QDateTime::fromTime_t(time);

		ml.addItem(item);
	}

	return ml;
}


}
