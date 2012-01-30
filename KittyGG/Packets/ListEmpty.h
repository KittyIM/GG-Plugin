#ifndef KITTYGG_LISTEMPTYPACKET_H
#define KITTYGG_LISTEMPTYPACKET_H

#include "KittyGG/Packet.h"

namespace KittyGG
{
	class ListEmpty: public Packet
	{
		public:
			enum { Type = 0x12 };
			virtual quint32 packetType() const { return Type; }

			QByteArray toData() const { return QByteArray(); }
	};
}

#endif // KITTYGG_LISTEMPTYPACKET_H
