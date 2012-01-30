#ifndef KITTYGG_TYPINGNOTIFYPACKET_H
#define KITTYGG_TYPINGNOTIFYPACKET_H

#include "KittyGG/Packet.h"

namespace KittyGG
{

	class TypingNotify: public Packet
	{
		public:
			TypingNotify(const quint16 &type, const quint32 &uin);

			enum { Type = 0x59 };
			quint32 packetType() const { return Type; }

			void setType(const quint16 &type) { m_type = type; }
			quint16 type() const { return m_type; }

			void setUin(const quint32 &uin) { m_uin = uin; }
			quint32 uin() const { return m_uin; }

			static TypingNotify fromData(const QByteArray &data);
			QByteArray toData() const;

		private:
			quint16 m_type;
			quint32 m_uin;
	};

}

#endif // KITTYGG_TYPINGNOTIFYPACKET_H
