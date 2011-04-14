#include "GGContact.h"

#include <QtCore/QCryptographicHash>
#include <QtCore/QFile>
#include <QtGui/QMenu>
#include <QtNetwork/QNetworkReply>

KittySDK::GGContact::GGContact(const QString &uid, KittySDK::GGAccount *account): KittySDK::Contact(uid, account)
{
  connect(&m_netManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(processReply(QNetworkReply*)));
}

KittySDK::GGContact::~GGContact()
{

}

void KittySDK::GGContact::changeStatus(const quint32 &status, const QString &description)
{
  m_status = dynamic_cast<KittySDK::GGProtocol*>(m_account->protocol())->convertStatus(status);
  m_description = description;

  emit statusChanged(m_status, m_description);
}

void KittySDK::GGContact::setData(const QString &name, const QString &data)
{
  //qDebug() << "data" << uid() << name << data;
  if(name == "avatar") {
    m_settings.insert(name, data);
    //updateAvatar();
  }
}

void KittySDK::GGContact::prepareContextMenu(QMenu *menu)
{
  menu->addAction(tr("Update avatar"), this, SLOT(updateAvatar()));
}

void KittySDK::GGContact::loadSettings(const QMap<QString, QVariant> &settings)
{
  m_settings.unite(settings);
}

QMap<QString, QVariant> KittySDK::GGContact::saveSettings()
{
  return m_settings;
}

void KittySDK::GGContact::updateAvatar()
{
  m_netManager.get(QNetworkRequest(QUrl(QString("http://avatars.gg.pl/%1").arg(uid()))));
}

void KittySDK::GGContact::processReply(QNetworkReply *reply)
{
  qDebug() << "request finished";

  if(reply->error() == QNetworkReply::NoError) {
    QString redirectUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();
    if(!redirectUrl.isEmpty() && redirectUrl != reply->url().toString()) {
      qDebug() << "redirect to" << redirectUrl;
      m_netManager.get(QNetworkRequest(QUrl(redirectUrl)));
    } else {
      QPixmap avat;
      avat.loadFromData(reply->readAll());
      avat.save(QCryptographicHash::hash(QString("%1@%2").arg(account()->uid()).arg(uid()).toLocal8Bit(), QCryptographicHash::Md5).toHex());
    }
  } else {
    qDebug() << reply->errorString();
  }

  reply->deleteLater();
}

