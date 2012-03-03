#include "GGContact.h"

#include <SoundsConstants.h>
#include <SDKConstants.h>

#include <QtCore/QCryptographicHash>
#include <QtCore/QDate>
#include <QtCore/QFile>
#include <QtGui/QTextDocument>
#include <QtGui/QMenu>
#include <QtNetwork/QNetworkReply>

#define qDebug() qDebug() << "[GGContact]"
#define qWarning() qWarning() << "[GGContact]"

namespace GG
{

Contact::Contact(const QString &uid, Account *account): KittySDK::IContact(uid, account)
{
	connect(&m_netManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(processReply(QNetworkReply*)));
}

Contact::~Contact()
{

}

void Contact::changeStatus(const quint32 &status, const QString &description)
{
	KittySDK::IProtocol::Status status_conv = dynamic_cast<Protocol*>(m_account->protocol())->convertStatus(status);

	emit statusChanged(status_conv, description);

	m_status = status_conv;
	m_description = description;
}

void Contact::setData(const QString &key, const QVariant &value)
{
	//qDebug() << "data" << uid() << key << value;

	if(key == "birthday_data") {
		m_data.insert(KittySDK::ContactInfos::I_BIRTHDAY, QDate::fromString(value.toString(), "yyyyMMdd0"));
	} else {
		m_data.insert(key, value);
	}

	emit dataChanged();
}

quint32 Contact::uin() const
{
	return uid().toUInt();
}

void Contact::prepareContextMenu(QMenu *menu)
{
	menu->addAction(tr("Update avatar"), this, SLOT(updateAvatar()));
}

void Contact::loadSettings(const QMap<QString, QVariant> &settings)
{
	m_data.unite(settings);
}

QMap<QString, QVariant> Contact::saveSettings()
{
	return m_data;
}

void Contact::updateAvatar()
{
	m_netManager.get(QNetworkRequest(QUrl(QString("http://avatars.gg.pl/%1").arg(uid()))));
}

void Contact::processReply(QNetworkReply *reply)
{
	if(reply->error() == QNetworkReply::NoError) {
		QString redirectUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();
		if(!redirectUrl.isEmpty() && redirectUrl != reply->url().toString()) {
			m_netManager.get(QNetworkRequest(QUrl(redirectUrl)));
		} else {
			QPixmap avat;
			avat.loadFromData(reply->readAll());
			avat.save(protocol()->core()->profilesDir() + protocol()->core()->profileName() + "/avatars/" + QCryptographicHash::hash(QString("avatar_" + protocol()->protoInfo()->protoName() + "_" + uid()).toAscii(), QCryptographicHash::Md5).toHex() + ".png");
		}
	} else {
		qDebug() << reply->errorString();
	}

	reply->deleteLater();
}

}
