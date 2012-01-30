#include "NotifyAdd.h"

#include "KittyGG/DataStream.h"

namespace KittyGG
{

NotifyAdd::NotifyAdd(const quint32 uin):
	m_entry(uin)
{
}

QByteArray NotifyAdd::toData() const
{
	QByteArray data;
	DataStream str(&data);

	str << m_entry.uin;
	str << m_entry.type;

	return data;
}

}
