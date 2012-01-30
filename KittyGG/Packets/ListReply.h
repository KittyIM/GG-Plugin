#ifndef KITTYGG_LISTREPLYPACKET_H
#define KITTYGG_LISTREPLYPACKET_H

#include "ListRequest.h"

namespace KittyGG
{
	class ListReply: public ListRequest
	{
		public:
			ListReply(const quint8 &type, const quint32 &version);

			enum { Type = 0x41 };
			virtual quint32 packetType() const { return Type; }

			QByteArray reply() const { return m_reply; }
			void setReply(const QByteArray &reply) { m_reply = reply; }

			static ListReply fromData(const QByteArray &data);
		private:
			QByteArray m_reply;
	};
}

#endif // KITTYGG_LISTREPLYPACKET_H
