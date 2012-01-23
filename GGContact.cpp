#include "GGContact.h"

#include "SDK/SoundsConstants.h"
#include "SDK/constants.h"

#include <QtCore/QCryptographicHash>
#include <QtCore/QDate>
#include <QtCore/QFile>
#include <QtGui/QMenu>
#include <QtNetwork/QNetworkReply>

#define qDebug() qDebug() << "[GGContact]"
#define qWarning() qWarning() << "[GGContact]"

using namespace KittySDK;

KittySDK::GGContact::GGContact(const QString &uid, KittySDK::GGAccount *account): KittySDK::Contact(uid, account)
{
	connect(&m_netManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(processReply(QNetworkReply*)));
}

KittySDK::GGContact::~GGContact()
{

}

void KittySDK::GGContact::changeStatus(const quint32 &status, const QString &description)
{
	KittySDK::Protocol::Status status_conv = dynamic_cast<KittySDK::GGProtocol*>(m_account->protocol())->convertStatus(status);

	if((m_status == KittySDK::Protocol::Offline) && (status_conv < KittySDK::Protocol::Offline)) {
		QMap<QString, QVariant> args;
		args.insert("id", Sounds::S_CONTACT_AVAIL);
		protocol()->core()->execPluginAction("Sounds", "playSound", args);
	}

	m_status = status_conv;
	m_description = description;

	emit statusChanged(m_status, m_description);
}

void KittySDK::GGContact::setData(const QString &key, const QVariant &value)
{
	//qDebug() << "data" << uid() << key << value;

	if(key == "birthday_data") {
		m_data.insert(ContactInfos::I_BIRTHDAY, QDate::fromString(value.toString(), "yyyyMMdd0"));
	} else {
		m_data.insert(key, value);
	}

	emit dataChanged();
}

quint32 KittySDK::GGContact::uin() const
{
	return uid().toUInt();
}

void KittySDK::GGContact::prepareContextMenu(QMenu *menu)
{
	menu->addAction(tr("Update avatar"), this, SLOT(updateAvatar()));
}

void KittySDK::GGContact::loadSettings(const QMap<QString, QVariant> &settings)
{
	m_data.unite(settings);
}

QMap<QString, QVariant> KittySDK::GGContact::saveSettings()
{
	return m_data;
}

void KittySDK::GGContact::updateAvatar()
{
	m_netManager.get(QNetworkRequest(QUrl(QString("http://avatars.gg.pl/%1").arg(uid()))));
}

void KittySDK::GGContact::processReply(QNetworkReply *reply)
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

