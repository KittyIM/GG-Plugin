#include "GGAccount.h"

using namespace KittySDK;

KittySDK::GGAccount::GGAccount(const QString &uid, Protocol *parent): KittySDK::Account(uid, parent)
{

}

quint32 KittySDK::GGAccount::uin() const
{
  return m_uid.toUInt();
}

