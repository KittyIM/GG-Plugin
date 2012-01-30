#include "DataStream.h"

namespace KittyGG
{

DataStream::DataStream(QByteArray *data):
	m_data(data)
{
}

DataStream &DataStream::operator <<(const quint8 &num)
{
	m_data->append((char*)&num, sizeof(quint8));
	return *this;
}

DataStream &DataStream::operator <<(const quint16 &num)
{
	m_data->append((char*)&num, sizeof(quint16));
	return *this;
}

DataStream &DataStream::operator <<(const quint32 &num)
{
	m_data->append((char*)&num, sizeof(quint32));
	return *this;
}

DataStream &DataStream::operator <<(const QByteArray &data)
{
	m_data->append(data, data.size());
	return *this;
}

}
