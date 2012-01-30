#ifndef KITTYGG_LOGINPACKET_H
#define KITTYGG_LOGINPACKET_H

#include "KittyGG/Packet.h"

#include <QtCore/QString>

namespace KittyGG
{
	class Login: public Packet
	{
		public:
			enum HashMethod
			{
				GaduGadu	= 0x01,
				Sha1		= 0x02
			};

			enum ConnectionFlag
			{
				UnknownFlag		= 0x000001,
				Video			= 0x000002,
				Mobile			= 0x100000,
				StrangerLinks	= 0x800000
			};

			enum ConnectionFeature
			{
				Status80			= 0x0001 | 0x0004,
				Message80			= 0x0002,
				NewStatuses			= 0x0010,
				ImageDescription	= 0x0020,
				NewLogin			= 0x0040,
				UnknownFeature		= 0x0100,
				UserData			= 0x0200,
				MessageAck			= 0x0400,
				TypingNotify		= 0x2000,
				MultiLogin			= 0x4000
			};

		public:
			Login(const quint32 &uin, const QString &password, const quint32 &seed);

			enum { Type = 0x31 };
			quint32 packetType() const { return Type; }

			void setUin(const quint32 &uin) { m_uin = uin; }
			quint32 uin() const { return m_uin; }

			void setPassword(const QString &password) { m_password = password; }
			QString password() const { return m_password; }

			void setSeed(const quint32 &seed) { m_seed = seed; }
			quint32 seed() const { return m_seed; }

			void setInitialStatus(const quint32 &initialStatus) { m_initialStatus = initialStatus; }
			quint32 initialStatus() const { return m_initialStatus; }

			void setInitialDescription(const QString &initialDescription) { m_initialDescription = initialDescription; }
			QString initialDescription() const { return m_initialDescription; }

			void setHashType(const quint8 &hashType) { m_hashType = hashType; }
			quint8 hashType() const { return m_hashType; }

			void setLanguage(const QString &language) { m_language = language; }
			QString language() const { return m_language; }

			void setFlags(const quint32 &flags) { m_flags = flags; }
			quint32 flags() const { return m_flags; }

			void setFeatures(const quint32 &features) { m_features = features; }
			quint32 features() const { return m_features; }

			QByteArray toData() const;

		private:
			quint32 m_uin;
			QString m_password;
			quint32 m_seed;
			quint32 m_initialStatus;
			QString m_initialDescription;
			quint8 m_hashType;
			QString m_language;
			quint32 m_flags;
			quint32 m_features;
	};
}

#endif // KITTYGG_LOGINPACKET_H
