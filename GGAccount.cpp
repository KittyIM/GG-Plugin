#include "GGAccount.h"

#include "constants.h"

#include <QtCore/QDebug>
#include <QtGui/QMenu>

using namespace KittySDK;

KittySDK::GGAccount::GGAccount(const QString &uid, Protocol *parent): KittySDK::Account(uid, parent)
{
  m_statusMenu = new QMenu();

  m_availableAction = new QAction(protocol()->core()->icon(KittyGG::Icons::I_AVAILABLE), tr("Available"), this);
  m_statusMenu->addAction(m_availableAction);

  m_awayAction = new QAction(protocol()->core()->icon(KittyGG::Icons::I_AWAY), tr("Be right back"), this);
  m_statusMenu->addAction(m_awayAction);

  m_ffcAction = new QAction(protocol()->core()->icon(KittyGG::Icons::I_FFC), tr("Free for chat"), this);
  m_statusMenu->addAction(m_ffcAction);

  m_dndAction = new QAction(protocol()->core()->icon(KittyGG::Icons::I_DND), tr("Do not disturb"), this);
  m_statusMenu->addAction(m_dndAction);

  m_invisibleAction = new QAction(protocol()->core()->icon(KittyGG::Icons::I_INVISIBLE), tr("Invisible"), this);
  m_statusMenu->addAction(m_invisibleAction);

  m_unavailableAction = new QAction(protocol()->core()->icon(KittyGG::Icons::I_UNAVAILABLE), tr("Unavailable"), this);
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

void KittySDK::GGAccount::loadSettings(const QMap<QString, QVariant> &settings)
{
  qDebug() << settings;
}

QMap<QString, QVariant> KittySDK::GGAccount::saveSettings()
{
  QMap<QString, QVariant> settings;

  settings.insert("setting", "value");

  return settings;
}

QMenu *KittySDK::GGAccount::statusMenu()
{
  return m_statusMenu;
}

