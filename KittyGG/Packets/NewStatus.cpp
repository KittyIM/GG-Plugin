#include "NewStatus.h"

#include "KittyGG/DataStream.h"

namespace KittyGG
{

NewStatus::NewStatus(const quint32 &status, const QString &description):
	m_status(status),
	m_description(description)
{
}

QByteArray NewStatus::toData() const
{
	QByteArray data;
	DataStream str(&data);

	QByteArray description = m_description.toLocal8Bit();

	str << m_status;
	str << m_flags;
	str << (quint32)description.size();
	str << description;

	return data;
}

}
