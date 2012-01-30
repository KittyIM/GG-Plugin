#include "Login.h"

#include "KittyGG/DataStream.h"
#include "constants.h"

#include <QtCore/QCryptographicHash>

namespace KittyGG
{

Login::Login(const quint32 &uin, const QString &password, const quint32 &seed):
	m_uin(uin),
	m_password(password),
	m_seed(seed),
	m_hashType(Sha1),
	m_language("pl"),
	m_flags(UnknownFlag | StrangerLinks),
	m_features(Status80 | Message80 | NewLogin | NewStatuses | ImageDescription |  TypingNotify | UserData)
{
}

QByteArray Login::toData() const
{
	QByteArray data;
	DataStream str(&data);

	QCryptographicHash hash(QCryptographicHash::Sha1);
	hash.addData(m_password.toLatin1(), m_password.length());
	hash.addData((char*)&m_seed, sizeof(m_seed));

	QByteArray version("KittyIM");

	str << m_uin;
	str << m_language.toLatin1();
	str << m_hashType;
	str << QByteArray(hash.result().data(), 64);
	str << m_initialStatus;
	str << m_flags;
	str << m_features;
	str << QByteArray(12, 0x00);			// deprecated (local_ip, local_port, external_ip, external_port)
	str << (quint8)0xff;					// image_size
	str << (quint8)0x64;					// unknown
	str << (quint32)version.size();
	str << version;
	str << (quint32)m_initialDescription.size();
	str << m_initialDescription.toLocal8Bit().data();

	return data;
}

}
