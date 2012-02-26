#ifndef KITTYGG_NOTIFYLASTPACKET_H
#define KITTYGG_NOTIFYLASTPACKET_H

#include "NotifyFirst.h"

namespace KittyGG
{
	class NotifyLast: public NotifyFirst
	{
		public:
			enum { Type = 0x10 };
			quint32 packetType() const { return Type; }
	};
}

#endif // KITTYGG_NOTIFYLASTPACKET_H
