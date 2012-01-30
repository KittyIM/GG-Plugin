#ifndef KITTYGG_STATUSPACKET_H
#define KITTYGG_STATUSPACKET_H

#include "KittyGG/Packet.h"

namespace KittyGG
{
	class Status: public Packet
	{
		public:
			enum
			{
				Unavailable			= 0x0001,
				UnavailableDescr	= 0x0015,
				FreeForChat			= 0x0017,
				FreeForChatDescr	= 0x0018,
				Available			= 0x0002,
				AvailableDescr		= 0x0004,
				Busy				= 0x0003,
				BusyDescr			= 0x0005,
				DoNotDisturb		= 0x0021,
				DoNotDisturbDescr	= 0x0022,
				Invisible			= 0x0014,
				InvisibleDescr		= 0x0016
			};

		public:
			Status(const quint32 &uin, const quint32 &status, const QString &description);

			enum { Type = 0x36 };
			quint32 packetType() const { return Type; }

			quint32 uin() const { return m_uin; }
			void setUin(const quint32 &uin) { m_uin = uin; }

			quint32 status() const { return m_status; }
			void setStatus(const quint32 &status) { m_status = status; }

			QString description() const { return m_description; }
			void setDescription(const QString &description) { m_description = description; }

			quint32 features() const { return m_features; }
			void setFeatures(const quint32 &features) { m_features = features; }

			quint32 flags() const { return m_flags; }
			void setFlags(const quint32 &flags) { m_flags = flags; }

			quint32 remoteIp() const { return m_remoteIp; }
			void setRemoteIp(const quint32 &remoteIp) { m_remoteIp = remoteIp; }

			quint16 remotePort() const { return m_remotePort; }
			void setRemotePort(const quint16 &remotePort) { m_remotePort = remotePort; }

			quint8 imageSize() const { return m_imageSize; }
			void setImageSize(const quint8 &imageSize) { m_imageSize = imageSize; }

			static Status fromData(const QByteArray &data);
			QByteArray toData() const;
		private:
			quint32 m_uin;
			quint32 m_status;
			QString m_description;
			quint32 m_features;
			quint32 m_flags;
			quint32 m_remoteIp;
			quint16 m_remotePort;
			quint8 m_imageSize;
	};
}

#endif // KITTYGG_STATUSPACKET_H
