#ifndef KITTYGG_LISTREQUESTPACKET_H
#define KITTYGG_LISTREQUESTPACKET_H

#include "KittyGG/Packet.h"

namespace KittyGG
{
	class ListRequest: public Packet
	{
		public:
			enum RequestType
			{
				Put = 0x00,
				Get = 0x02
			};

		public:
			ListRequest(const quint8 &type, const quint32 &version);

			enum { Type = 0x40 };
			virtual quint32 packetType() const { return Type; }

			quint8 type() const { return m_type; }
			void setType(const quint8 &type) { m_type = type; }

			quint32 version() const { return m_version; }
			void setVersion(const quint32 &version) { m_version = version; }

			quint8 formatType() const { return m_formatType; }
			void setFormatType(const quint8 &formatType) { m_formatType = formatType; }

			QByteArray toData() const;

		protected:
			quint8 m_type;
			quint32 m_version;
			quint8 m_formatType;
	};
}

#endif // KITTYGG_LISTREQUESTPACKET_H
