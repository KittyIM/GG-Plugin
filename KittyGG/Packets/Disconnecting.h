#ifndef KITTYGG_DISCONNECTINGPACKET_H
#define KITTYGG_DISCONNECTINGPACKET_H

#include "KittyGG/Packet.h"

namespace KittyGG
{
	class Disconnecting: public Packet
	{
		public:
			enum { Type = 0x0b };
			quint32 packetType() const { return Type; }

			QByteArray toData() const { return QByteArray(); }
	};
}

#endif // KITTYGG_DISCONNECTINGPACKET_H
