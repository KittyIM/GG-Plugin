#ifndef KITTYGG_PONGPACKET_H
#define KITTYGG_PONGPACKET_H

#include "KittyGG/Packet.h"

namespace KittyGG
{
	class Pong: public Packet
	{
		public:
			enum { Type = 0x07 };
			quint32 packetType() const { return Type; }

			QByteArray toData() const { return QByteArray(); }
	};
}

#endif // KITTYGG_PONGPACKET_H
