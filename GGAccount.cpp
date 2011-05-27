#include "GGAccount.h"

#include "SDK/SoundsConstants.h"
#include "SDK/GGConstants.h"
#include "SDK/Message.h"
#include "GGContact.h"
#include "constants.h"
#include "GGClient.h"

#include <QtCore/QDebug>
#include <QtGui/QMenu>

#define qDebug() qDebug() << "[GGAccount]"
#define qWarning() qWarning() << "[GGAccount]"

using namespace KittySDK;

KittySDK::GGAccount::GGAccount(const QString &uid, GGProtocol *parent): Account(uid, parent)
{
  m_client = new GGClient(this);
  connect(m_client, SIGNAL(statusChanged(quint32,quint32,QString)), this, SLOT(changeContactStatus(quint32,quint32,QString)));
  connect(m_client, SIGNAL(userDataReceived(quint32,QString,QString)), this, SLOT(processUserData(quint32,QString,QString)));
  connect(m_client, SIGNAL(messageReceived(quint32,QDateTime,QString)), this, SLOT(processMessage(quint32,QDateTime,QString)));

  setMe(new GGContact(uid, this));
  me()->setDisplay(protocol()->core()->profileName());

  m_statusMenu = new QMenu();

  m_availableAction = new QAction(protocol()->core()->icon(Icons::I_GG_AVAILABLE), tr("Available"), this);
  connect(m_availableAction, SIGNAL(triggered()), this, SLOT(setStatusAvailable()));
  m_statusMenu->addAction(m_availableAction);

  m_awayAction = new QAction(protocol()->core()->icon(Icons::I_GG_AWAY), tr("Be right back"), this);
  connect(m_awayAction, SIGNAL(triggered()), this, SLOT(setStatusAway()));
  m_statusMenu->addAction(m_awayAction);

  m_ffcAction = new QAction(protocol()->core()->icon(Icons::I_GG_FFC), tr("Free for chat"), this);
  connect(m_ffcAction, SIGNAL(triggered()), this, SLOT(setStatusFFC()));
  m_statusMenu->addAction(m_ffcAction);

  m_dndAction = new QAction(protocol()->core()->icon(Icons::I_GG_DND), tr("Do not disturb"), this);
  connect(m_dndAction, SIGNAL(triggered()), this, SLOT(setStatusDND()));
  m_statusMenu->addAction(m_dndAction);

  m_invisibleAction = new QAction(protocol()->core()->icon(Icons::I_GG_INVISIBLE), tr("Invisible"), this);
  connect(m_invisibleAction, SIGNAL(triggered()), this, SLOT(setStatusInvisible()));
  m_statusMenu->addAction(m_invisibleAction);

  m_unavailableAction = new QAction(protocol()->core()->icon(Icons::I_GG_UNAVAILABLE), tr("Unavailable"), this);
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

Protocol::Status KittySDK::GGAccount::status() const
{
  return dynamic_cast<KittySDK::GGProtocol*>(protocol())->convertStatus(m_client->status());
}

Contact *KittySDK::GGAccount::newContact(const QString &uid)
{
  foreach(Contact *cnt, contacts()) {
    if(cnt->uid() == uid) {
      return cnt;
    }
  }

  GGContact *cnt = new GGContact(uid, this);

  return cnt;
}

Contact *KittySDK::GGAccount::newContact(const quint32 &uin)
{
  return newContact(QString::number(uin));
}

Contact *KittySDK::GGAccount::contactByUin(const quint32 &uin)
{
  foreach(Contact *cnt, contacts()) {
    if(cnt->uid() == QString::number(uin)) {
      return cnt;
    }
  }

  Contact *cnt = newContact(uin);
  cnt->setDisplay(QString::number(uin));
  insertContact(cnt->uid(), cnt);

  return cnt;
}

void KittySDK::GGAccount::insertContact(const QString &uid, Contact *contact)
{
  Account::insertContact(uid, contact);

  m_client->addContact(uid.toUInt());
}

void KittySDK::GGAccount::loadSettings(const QMap<QString, QVariant> &settings)
{
  m_client->setAccount(uin(), password());
}

QMap<QString, QVariant> GGAccount::saveSettings()
{
  QMap<QString, QVariant> settings;

  return settings;
}

void KittySDK::GGAccount::changeStatus(const KittySDK::Protocol::Status &status, const QString &description)
{
  quint32 stat = KittyGG::Statuses::S_AVAILABLE;
  switch(status) {
    case Protocol::Away:
      stat = KittyGG::Statuses::S_BUSY;
    break;

    case Protocol::FFC:
      stat = KittyGG::Statuses::S_FFC;
    break;

    case Protocol::DND:
      stat = KittyGG::Statuses::S_DND;
    break;

    case Protocol::Invisible:
      stat = KittyGG::Statuses::S_INVISIBLE;
    break;

    case Protocol::Offline:
      stat = KittyGG::Statuses::S_UNAVAILABLE;
    break;

    default:
      stat = KittyGG::Statuses::S_AVAILABLE;
    break;
  }

  m_client->changeStatus(stat, description);
}

QMenu *KittySDK::GGAccount::statusMenu()
{
  return m_statusMenu;
}

void KittySDK::GGAccount::sendMessage(const Message &msg)
{
  if(m_client->isConnected()) {
    GGContact *cnt = dynamic_cast<GGContact*>(msg.to().first());
    m_client->sendMessage(cnt->uin(), msg.body());
  } else {
    Message err(msg.to().first(), me());
    err.setBody(tr("Not connected!"));
    err.setDirection(Message::System);
    emit messageReceived(err);
  }
}

void KittySDK::GGAccount::changeContactStatus(const quint32 &uin, const quint32 &status, const QString &description)
{
  QString uid = QString::number(uin);

  if(uid == this->uid()) {
    GGProtocol *ggproto = dynamic_cast<KittySDK::GGProtocol*>(protocol());
    emit statusChanged(ggproto->convertStatus(status), description);
  }

  if(contacts().contains(uid)) {
    dynamic_cast<GGContact*>(contacts().value(uid))->changeStatus(status, description);
  } else {
    qWarning() << "Contact not on list" << uid;
  }
}

void KittySDK::GGAccount::processUserData(const quint32 &uin, const QString &name, const QString &data)
{
  QString uid = QString::number(uin);

  if(contacts().contains(uid)) {
    dynamic_cast<GGContact*>(contacts().value(uid))->setData(name, data);
  }
}

void KittySDK::GGAccount::processMessage(const quint32 &sender, const QDateTime &time, const QString &plain)
{
  Message msg(contactByUin(sender), me());
  msg.setDirection(Message::Incoming);
  msg.setBody(plain);
  msg.setTimeStamp(time);

  QMap<QString, QVariant> args;
  args.insert("id", Sounds::S_MSG_RECV);
  protocol()->core()->execPluginAction("Sounds", "playSound", args);

  emit messageReceived(msg);
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
