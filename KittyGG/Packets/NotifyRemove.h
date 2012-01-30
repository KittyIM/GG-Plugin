#ifndef KITTYGG_NOTIFYREMOVEPACKET_H
#define KITTYGG_NOTIFYREMOVEPACKET_H

#include "NotifyAdd.h"

namespace KittyGG
{
	class NotifyRemove : public NotifyAdd
	{
		public:
			NotifyRemove(const quint32 &uin);

			enum { Type = 0x0d };
			quint32 packetType() const { return Type; }
	};
}

#endif // KITTYGG_NOTIFYREMOVEPACKET_H
