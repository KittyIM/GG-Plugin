#ifndef KITTYGG_LOGINFAILEDPACKET_H
#define KITTYGG_LOGINFAILEDPACKET_H

#include "KittyGG/Packet.h"

namespace KittyGG
{
	class LoginFailed: public Packet
	{
		public:
			enum { Type = 0x43 };
			quint32 packetType() const { return Type; }
	};
}


#endif // KITTYGG_LOGINFAILEDPACKET_H
