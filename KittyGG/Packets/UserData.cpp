#include "UserData.h"

namespace KittyGG
{

UserData::UserData(const quint32 &type):
	m_type(type)
{
}

UserData UserData::fromData(const QByteArray &data)
{
	quint32 type;
	quint32 num;
	quint32 uin;
	quint32 num_attr;
	quint32 name_size;
	quint32 attr_type;
	quint32 value_size;
	QMap<quint32, QList<UserDataAttribute> > attr_data;

	QDataStream str(data);
	str.setByteOrder(QDataStream::LittleEndian);

	str >> type;
	str >> num;

	while(num > 0) {
		str >> uin;
		str >> num_attr;

		QList<UserDataAttribute> attributes;

		while(num_attr > 0) {
			str >> name_size;

			char *name = new char[name_size];
			if(name_size > 0) {
				str.readRawData(name, name_size);
			}

			str >> attr_type;
			str >> value_size;

			char *value = new char[value_size];
			if(value_size > 0) {
				str.readRawData(value, value_size);
			}

			attributes << UserDataAttribute(QString::fromAscii(name, name_size), attr_type, QString::fromAscii(value, value_size));

			delete [] name;
			delete [] value;

			num_attr--;
		}

		attr_data.insert(uin, attributes);

		num--;
	}

	UserData packet(type);
	packet.setData(attr_data);
	return packet;
}

QByteArray UserData::toData() const
{
	return QByteArray();
}

}
