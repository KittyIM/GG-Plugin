#ifndef KITTYGG_USERDATAPACKET_H
#define KITTYGG_USERDATAPACKET_H

#include "KittyGG/Packet.h"

namespace KittyGG
{
	struct UserDataAttribute
	{
		UserDataAttribute(const QString &name, const int &type, const QString &value):
			name(name),
			type(type),
			value(value)
		{ }

		QString name;
		int type;
		QString value;
	};

	class UserData: public Packet
	{
		public:
			UserData(const quint32 &type);

			enum { Type = 0x44 };
			quint32 packetType() const { return Type; }

			quint32 type() const { return m_type; }
			void setType(const quint32 &type) { m_type = type; }

			QMap<quint32, QList<UserDataAttribute> > data() const { return m_data; }
			void setData(const QMap<quint32, QList<UserDataAttribute> > &data) { m_data = data; }

			static UserData fromData(const QByteArray &data);
			QByteArray toData() const;

		private:
			quint32 m_type;
			QMap<quint32, QList<UserDataAttribute> > m_data;
	};
}

#endif // KITTYGG_USERDATAPACKET_H
