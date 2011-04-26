#include "GGAccount.h"

#include "SDK/Message.h"
#include "GGContact.h"
#include "constants.h"
#include "GGClient.h"

#include <QtCore/QDebug>
#include <QtGui/QMenu>

using namespace KittySDK;

KittySDK::GGAccount::GGAccount(const QString &uid, GGProtocol *parent): KittySDK::Account(uid, parent)
{
  m_client = new GGClient(this);
  connect(m_client, SIGNAL(statusChanged(quint32,quint32,QString)), this, SLOT(changeContactStatus(quint32,quint32,QString)));
  connect(m_client, SIGNAL(userDataReceived(quint32,QString,QString)), this, SLOT(processUserData(quint32,QString,QString)));

  m_statusMenu = new QMenu();

  m_availableAction = new QAction(protocol()->core()->icon(KittyGG::Icons::I_AVAILABLE), tr("Available"), this);
  connect(m_availableAction, SIGNAL(triggered()), this, SLOT(setStatusAvailable()));
  m_statusMenu->addAction(m_availableAction);

  m_awayAction = new QAction(protocol()->core()->icon(KittyGG::Icons::I_AWAY), tr("Be right back"), this);
  connect(m_awayAction, SIGNAL(triggered()), this, SLOT(setStatusAway()));
  m_statusMenu->addAction(m_awayAction);

  m_ffcAction = new QAction(protocol()->core()->icon(KittyGG::Icons::I_FFC), tr("Free for chat"), this);
  connect(m_ffcAction, SIGNAL(triggered()), this, SLOT(setStatusFFC()));
  m_statusMenu->addAction(m_ffcAction);

  m_dndAction = new QAction(protocol()->core()->icon(KittyGG::Icons::I_DND), tr("Do not disturb"), this);
  connect(m_dndAction, SIGNAL(triggered()), this, SLOT(setStatusDND()));
  m_statusMenu->addAction(m_dndAction);

  m_invisibleAction = new QAction(protocol()->core()->icon(KittyGG::Icons::I_INVISIBLE), tr("Invisible"), this);
  connect(m_invisibleAction, SIGNAL(triggered()), this, SLOT(setStatusInvisible()));
  m_statusMenu->addAction(m_invisibleAction);

  m_unavailableAction = new QAction(protocol()->core()->icon(KittyGG::Icons::I_UNAVAILABLE), tr("Unavailable"), this);
  connect(m_unavailableAction, SIGNAL(triggered()), this, SLOT(setStatusUnavailable()));
  m_statusMenu->addAction(m_unavailableAction);
}

KittySDK::GGAccount::~GGAccount()
{
  delete m_statusMenu;
}

quint32 KittySDK::GGAccount::uin() const
{
  return m_uid.toUInt();
}

KittySDK::Protocol::Status KittySDK::GGAccount::status() const
{
  return dynamic_cast<KittySDK::GGProtocol*>(protocol())->convertStatus(m_client->status());
}

KittySDK::Contact *KittySDK::GGAccount::newContact(const QString &uid)
{
  KittySDK::GGContact *cnt = new KittySDK::GGContact(uid, this);

  return cnt;
}

void KittySDK::GGAccount::insertContact(const QString &uid, KittySDK::Contact *contact)
{
  Account::insertContact(uid, contact);

  m_client->addContact(uid.toUInt());
}

void KittySDK::GGAccount::loadSettings(const QMap<QString, QVariant> &settings)
{
  m_client->setAccount(uin(), password());
}

QMap<QString, QVariant> KittySDK::GGAccount::saveSettings()
{
  QMap<QString, QVariant> settings;

  //settings.insert("setting", "value");

  return settings;
}

QMenu *KittySDK::GGAccount::statusMenu()
{
  return m_statusMenu;
}

void KittySDK::GGAccount::sendMessage(const KittySDK::Message &msg)
{
  GGContact *cnt = dynamic_cast<GGContact*>(msg.to().first());
  m_client->sendMessage(cnt->uin(), msg.body());
}

void KittySDK::GGAccount::changeContactStatus(const quint32 &uin, const quint32 &status, const QString &description)
{
  QString uid = QString::number(uin);

  if(uid == this->uid()) {
    emit statusChanged();
  }

  if(contacts().contains(uid)) {
    dynamic_cast<KittySDK::GGContact*>(contacts().value(uid))->changeStatus(status, description);
  } else {
    qWarning() << "Contact not on list" << uid;
  }
}

void KittySDK::GGAccount::processUserData(const quint32 &uin, const QString &name, const QString &data)
{
  QString uid = QString::number(uin);

  if(contacts().contains(uid)) {
    dynamic_cast<KittySDK::GGContact*>(contacts().value(uid))->setData(name, data);
  }
}

void KittySDK::GGAccount::setStatusAvailable()
{
  m_client->setStatus(KittyGG::Statuses::S_AVAILABLE);
}

void KittySDK::GGAccount::setStatusAway()
{
  m_client->setStatus(KittyGG::Statuses::S_BUSY);
}

void KittySDK::GGAccount::setStatusFFC()
{
  m_client->setStatus(KittyGG::Statuses::S_FFC);
}

void KittySDK::GGAccount::setStatusDND()
{
  m_client->setStatus(KittyGG::Statuses::S_DND);
}

void KittySDK::GGAccount::setStatusInvisible()
{
  m_client->setStatus(KittyGG::Statuses::S_INVISIBLE);
}

void KittySDK::GGAccount::setStatusUnavailable()
{
  m_client->setStatus(KittyGG::Statuses::S_UNAVAILABLE);
}

