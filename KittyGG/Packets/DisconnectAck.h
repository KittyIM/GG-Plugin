#ifndef KITTYGG_DISCONNECTACKPACKET_H
#define KITTYGG_DISCONNECTACKPACKET_H

#include "KittyGG/Packet.h"

namespace KittyGG
{
	class DisconnectAck: public Packet
	{
		public:
			enum { Type = 0x0d };
			virtual quint32 packetType() const { return Type; }

			QByteArray toData() const { return QByteArray(); }
	};
}

#endif // KITTYGG_DISCONNECTACKPACKET_H
