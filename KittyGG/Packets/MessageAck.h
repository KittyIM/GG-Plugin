#ifndef KITTYGG_MESSAGEACKPACKET_H
#define KITTYGG_MESSAGEACKPACKET_H

#include "KittyGG/Packet.h"

namespace KittyGG
{
	class MessageAck: public Packet
	{
		public:
			enum Status
			{
				Blocked		= 0x01,
				Delivered	= 0x02,
				Queued		= 0x03,
				MBoxFull	= 0x04
			};

		public:
			MessageAck(const quint32 &status, const quint32 &recipient, const quint32 &seq);

			enum { Type = 0x05 };
			quint32 packetType() const { return Type; }

			void setStatus(const quint32 &status) { m_status = status; }
			quint32 status() const { return m_status; }

			void setRecipient(const quint32 &recipient) { m_recipient = recipient; }
			quint32 recipient() const { return m_recipient; }

			void setSeq(const quint32 &seq) { m_seq = seq; }
			quint32 seq() const { return m_seq; }

			static MessageAck fromData(const QByteArray &data);
			QByteArray toData() const;

		private:
			quint32 m_status;
			quint32 m_recipient;
			quint32 m_seq;
	};
}

#endif // KITTYGG_MESSAGEACKPACKET_H
