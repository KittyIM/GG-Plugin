#ifndef KITTYGG_WELCOMEPACKET_H
#define KITTYGG_WELCOMEPACKET_H

#include "KittyGG/Packet.h"

namespace KittyGG
{
	class Welcome : public Packet
	{
		public:
			Welcome(const quint32 &seed);

			enum { Type = 0x01 };
			quint32 packetType() const { return Type; }

			void setSeed(const quint32 &seed) { m_seed = seed; }
			quint32 seed() const { return m_seed; }

			static Welcome fromData(const QByteArray &data);
			QByteArray toData() const;

	private:
			quint32 m_seed;
	};
}

#endif // KITTYGG_WELCOMEPACKET_H
