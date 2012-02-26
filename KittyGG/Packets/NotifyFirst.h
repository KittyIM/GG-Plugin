#ifndef KITTYGG_NOTIFYFIRSTPACKET_H
#define KITTYGG_NOTIFYFIRSTPACKET_H

#include "KittyGG/Packet.h"

namespace KittyGG
{
	struct NotifyEntry
	{
		NotifyEntry(const quint32 &uin, const quint8 &type = 0x03):
			uin(uin),
			type(type)
		{ }

		quint32 uin;
		quint8 type;
	};

	class NotifyFirst:  public Packet
	{
		public:
			enum { Type = 0x0f };
			quint32 packetType() const { return Type; }

			void setContacts(const QList<NotifyEntry> &contacts) { m_contacts = contacts; }
			QList<NotifyEntry> contacts() const { return m_contacts; }

			QByteArray toData() const;

		protected:
			QList<NotifyEntry> m_contacts;
	};
}


#endif // KITTYGG_NOTIFYFIRSTPACKET_H
