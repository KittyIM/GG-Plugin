#ifndef KITTYGG_PACKET_H
#define KITTYGG_PACKET_H

#include <QtCore/QByteArray>
#include <QtCore/QDebug>

namespace KittyGG
{
	class Packet
	{
		public:
			enum { Type = -1 };
			virtual quint32 packetType() const { return Type; }

			virtual QByteArray toData() const = 0;
			virtual quint32 size() const { return toData().size(); }
	};
}

#endif // KITTYGG_PACKET_H
