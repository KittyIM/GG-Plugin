#include "MessageAck.h"

#include "KittyGG/DataStream.h"

namespace KittyGG
{

MessageAck::MessageAck(const quint32 &status, const quint32 &recipient, const quint32 &seq):
	m_status(status),
	m_recipient(recipient),
	m_seq(seq)
{
}

MessageAck MessageAck::fromData(const QByteArray &data)
{
	quint32 status;
	quint32 recipient;
	quint32 seq;

	QDataStream str(data);
	str.setByteOrder(QDataStream::LittleEndian);

	str >> status;
	str >> recipient;
	str >> seq;

	MessageAck ack(status, recipient, seq);
	return ack;
}

QByteArray MessageAck::toData() const
{
	QByteArray data;
	DataStream str(&data);

	str << m_status;
	str << m_recipient;
	str << m_seq;

	return data;
}

}
