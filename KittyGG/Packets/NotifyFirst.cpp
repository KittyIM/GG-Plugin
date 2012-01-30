#include "NotifyFirst.h"

#include "KittyGG/DataStream.h"

namespace KittyGG
{
QByteArray NotifyFirst::toData() const
{
	QByteArray data;
	DataStream str(&data);

	foreach(const NotifyEntry &contact, m_contacts) {
		str << contact.uin;
		str << contact.type;
	}

	return data;
}

}
