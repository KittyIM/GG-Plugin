#include "ListRequest.h"

#include "KittyGG/DataStream.h"

namespace KittyGG
{

ListRequest::ListRequest(const quint8 &type, const quint32 &version):
	m_type(type),
	m_version(version),
	m_formatType(0x02)
{
}

QByteArray ListRequest::toData() const
{
	QByteArray data;
	DataStream str(&data);

	str << m_type;
	str << m_version;
	str << m_formatType;
	str << (quint8)0x01;		//unknown

	return data;
}

}
