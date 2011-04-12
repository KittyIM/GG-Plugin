#include "GGContact.h"

#include <QtGui/QMenu>

KittySDK::GGContact::GGContact(const QString &uid, KittySDK::GGAccount *account): KittySDK::Contact(uid, account)
{
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

void KittySDK::GGContact::prepareContextMenu(QMenu *menu)
{
  menu->addAction("I'm from the plugin.");
}
