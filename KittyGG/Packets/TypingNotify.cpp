#include "TypingNotify.h"

#include "KittyGG/DataStream.h"

namespace KittyGG
{

TypingNotify::TypingNotify(const quint16 &type, const quint32 &uin):
	m_type(type),
	m_uin(uin)
{
}

TypingNotify TypingNotify::fromData(const QByteArray &data)
{
	quint16 type;
	quint32 uin;

	QDataStream str(data);
	str.setByteOrder(QDataStream::LittleEndian);

	str >> type;
	str >> uin;

	TypingNotify notify(type, uin);
	return notify;
}

QByteArray TypingNotify::toData() const
{
	QByteArray data;
	DataStream str(&data);

	str << m_type;
	str << m_uin;

	return data;
}

}
