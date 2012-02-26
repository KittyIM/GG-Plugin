#ifndef KITTYGG_MULTILOGON_H
#define KITTYGG_MULTILOGON_H

#include "KittyGG/Packet.h"

#include <QtCore/QDateTime>

namespace KittyGG
{
	struct MultiLoginItem
	{
		QString ip;
		QString client;
		QDateTime time;
		quint64 conn_id;
		quint32 flags;
		quint32 features;
	};

	class MultiLogin: public Packet
	{
		public:
			~MultiLogin();

			enum { Type = 0x5b };
			quint32 packetType() const { return Type; }

			const QList<MultiLoginItem*> &items() { return m_items; }
			void addItem(MultiLoginItem *item) { m_items.append(item); }

			static MultiLogin fromData(const QByteArray &data);
			QByteArray toData() const { return QByteArray(); }

		private:
			QList<MultiLoginItem*> m_items;
	};
}

#endif // KITTYGG_MULTILOGON_H
