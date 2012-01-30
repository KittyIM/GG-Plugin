#ifndef KITTYGG_NOTIFYREPLYPACKET_H
#define KITTYGG_NOTIFYREPLYPACKET_H

#include "Status.h"

namespace KittyGG
{
	class NotifyReply: public Status
	{
		public:
			enum { Type = 0x37 };
			quint32 packetType() const { return Type; }
	};
}

#endif // KITTYGG_NOTIFYREPLYPACKET_H
