#ifndef KITTYGG_NOTIFYADDPACKET_H
#define KITTYGG_NOTIFYADDPACKET_H

#include "NotifyFirst.h"

namespace KittyGG
{
	class NotifyAdd: public Packet
	{
		public:
			NotifyAdd(const quint32 uin);

			enum { Type = 0x0d };
			quint32 packetType() const { return Type; }

			quint32 uin() const { return m_entry.uin; }
			void setUin(const quint32 &uin) { m_entry.uin = uin; }

			quint8 type() const { return m_entry.type; }
			void setType(const quint8 &type) { m_entry.type = type; }

			QByteArray toData() const;

		protected:
			NotifyEntry m_entry;
	};
}

#endif // KITTYGG_NOTIFYADDPACKET_H
