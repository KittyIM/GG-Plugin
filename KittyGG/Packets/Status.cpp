#include "Status.h"

#include "KittyGG/DataStream.h"

namespace KittyGG
{

Status::Status(const quint32 &uin, const quint32 &status, const QString &description):
	m_uin(uin),
	m_status(status),
	m_description(description),
	m_imageSize(0xff)
{
}

Status Status::fromData(const QByteArray &data)
{
	quint32 uin;
	quint32 status;
	quint32 features;
	quint32 remote_ip;
	quint16 remote_port;
	quint8 image_size;
	quint8 unknown;
	quint32 flags;
	quint32 description_size;

	QDataStream str(data);
	str.setByteOrder(QDataStream::LittleEndian);

	str >> uin;
	str >> status;
	str >> features;
	str >> remote_ip;
	str >> remote_port;
	str >> image_size;
	str >> unknown;
	str >> flags;
	str >> description_size;

	char *description = 0;
	if(description_size > 0) {
		description = new char[description_size];

		str.readRawData(description, description_size);
	}

	Status packet(uin, status, QString::fromAscii(description, description_size));
	packet.setFeatures(features);
	packet.setRemoteIp(remote_ip);
	packet.setRemotePort(remote_port);
	packet.setImageSize(image_size);
	packet.setFlags(flags);

	delete [] description;

	return packet;
}

QByteArray Status::toData() const
{
	QByteArray data;
	DataStream str(&data);

	QByteArray description = m_description.toAscii();

	str << m_uin;
	str << m_status;
	str << m_features;
	str << m_remoteIp;
	str << m_remotePort;
	str << m_imageSize;
	str << (quint8)0x64;
	str << m_flags;
	str << (quint32)description.size();
	str << description;

	return data;
}

} // namespace KittyGG
