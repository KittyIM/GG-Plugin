#ifndef KITTYGG_PINGPACKET_H
#define KITTYGG_PINGPACKET_H

#include "KittyGG/Packet.h"

namespace KittyGG
{
	class Ping: public Packet
	{
		public:
			enum { Type = 0x08 };
			quint32 packetType() const { return Type; }

			QByteArray toData() const { return QByteArray(); }
	};
}

#endif // KITTYGG_PINGPACKET_H
