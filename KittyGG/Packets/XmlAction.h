#ifndef KITTYGG_XMLACTIONPACKET_H
#define KITTYGG_XMLACTIONPACKET_H

#include "KittyGG/Packet.h"

namespace KittyGG
{
	class XmlAction: public Packet
	{
		public:
			enum { Type = 0x2c };
			quint32 packetType() const { return Type; }

			QString action() const { return m_action; }
			void setAction(const QString &action) { m_action = action; }

			static XmlAction fromData(const QByteArray &data);
			QByteArray toData() const { return QByteArray(); }

		private:
			QString m_action;
	};
}

#endif // KITTYGG_XMLACTIONPACKET_H
