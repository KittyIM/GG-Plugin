#ifndef KITTYGG_LOGINOKPACKET_H
#define KITTYGG_LOGINOKPACKET_H

#include "KittyGG/Packet.h"

namespace KittyGG
{
	class LoginOk: public Packet
	{
		public:
			enum { Type = 0x35 };
			quint32 packetType() const { return Type; }
	};
}


#endif // KITTYGG_LOGINOKPACKET_H
