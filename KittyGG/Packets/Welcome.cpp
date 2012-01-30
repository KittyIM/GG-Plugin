#include "Welcome.h"

#include "KittyGG/DataStream.h"

namespace KittyGG
{

Welcome::Welcome(const quint32 &seed):
	m_seed(seed)
{
}

Welcome Welcome::fromData(const QByteArray &data)
{
	quint32 seed;

	QDataStream str(data);
	str.setByteOrder(QDataStream::LittleEndian);

	str >> seed;

	Welcome welcome(seed);
	return welcome;
}

QByteArray Welcome::toData() const
{
	QByteArray data;
	DataStream str(&data);

	str << m_seed;

	return data;
}

}
