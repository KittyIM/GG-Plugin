#ifndef KITTYGG_MESSAGESENDPACKET_H
#define KITTYGG_MESSAGESENDPACKET_H

#include "MessageRecv.h"

namespace KittyGG
{
	class MessageSend: public MessageRecv
	{
		public:
			enum { Type = 0x2d };
			quint32 packetType() const { return Type; }

			QByteArray toData() const;
	};
}

#endif // KITTYGG_MESSAGESENDPACKET_H
