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

void Contact::changeStatus(const quint32 &status, const QString &description, bool silent)
{
	KittySDK::IProtocol::Status status_conv = dynamic_cast<Protocol*>(m_account->protocol())->convertStatus(status);

	bool status_changed = (m_status != status_conv);
	bool desc_changed = (m_description != description);

	m_status = status_conv;
	m_description = description;

	if(!silent) {
		if((m_status == KittySDK::IProtocol::Offline) && (status_conv < KittySDK::IProtocol::Offline)) {
			QMap<QString, QVariant> args;
			args.insert("id", Sounds::Sounds::S_CONTACT_AVAIL);
			protocol()->core()->execPluginAction("sounds", "playSound", args);
		}

		QString notifyText = "<span class=\"notifyText\"><b>" + Qt::escape(display()) + "</b> ";
		if(desc_changed) {
			QString desc = Qt::escape(description);
			const int maxDescLength = 40;

			notifyText += tr("changed description");
			if(desc.length() > 0) {
				notifyText += "<br>\"";
				if(desc.length() <= maxDescLength) {
					notifyText += desc;
				} else {
					notifyText += desc.left(maxDescLength) + "...";
				}
				notifyText += "\"";
			}
		} else if(status_changed) {
			switch(status_conv) {
				case KittySDK::IProtocol::Online:
					notifyText += tr("is online");
				break;

				case KittySDK::IProtocol::Away:
					notifyText += tr("is away");
				break;

				case KittySDK::IProtocol::FFC:
					notifyText += tr("is free for chat");
				break;

				case KittySDK::IProtocol::DND:
					notifyText += tr("shouldn't be disturbed");
				break;

				case KittySDK::IProtocol::Offline:
					notifyText += tr("is offline");
				break;

				default:
					notifyText += tr("is wtf");
				break;
			}
		}
		notifyText += "</span>";

		if(status_changed || desc_changed) {
			QMap<QString, QVariant> args;
			args.insert("icon", protocol()->core()->icon(m_account->protocol()->statusIcon(status_conv)));
			args.insert("text", notifyText);
			protocol()->core()->execPluginAction("notify", "addNotify", args);
		}
	}

	emit statusChanged(m_status, m_description);
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
