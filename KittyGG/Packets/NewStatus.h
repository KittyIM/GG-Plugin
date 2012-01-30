#ifndef KITTYGG_NEWSTATUSPACKET_H
#define KITTYGG_NEWSTATUSPACKET_H

#include "KittyGG/Packet.h"

namespace KittyGG
{
	class NewStatus: public Packet
	{
		public:
			NewStatus(const quint32 &status, const QString &description);

			enum { Type = 0x38 };
			quint32 packetType() const { return Type; }

			quint32 status() const { return m_status; }
			void setStatus(const quint32 &status) { m_status = status; }

			QString description() const { return m_description; }
			void setDescription(const QString &description) { m_description = description; }

			quint32 flags() const { return m_flags; }
			void setFlags(const quint32 &flags) { m_flags = flags; }

			QByteArray toData() const;

		private:
			quint32 m_status;
			QString m_description;
			quint32 m_flags;
	};
}

#endif // KITTYGG_NEWSTATUSPACKET_H
