#include "GGProtocol.h"

#include "GGEditWindow.h"
#include "GGAccount.h"

using namespace KittySDK;

KittySDK::GGProtocol::GGProtocol(PluginCore *core): Protocol(core)
{
  m_info = new PluginInfo("GGProtocol", "0.0.1", "arturo182", "arturo182@tlen.pl", "http://www.arturpacholec.pl/");
  m_editWindow = 0;
}

KittySDK::GGProtocol::~GGProtocol()
{
  if(m_editWindow) {
    delete m_editWindow;
  }
}

KittySDK::Account *KittySDK::GGProtocol::newAccount(const QString &uid)
{
  return new KittySDK::GGAccount(uid, this);
}

QWidget *KittySDK::GGProtocol::editWindow(KittySDK::Account *account)
{
  if(!m_editWindow) {
    m_editWindow = new KittySDK::GGEditWindow();
  }

  return m_editWindow;
}

KITTY_PLUGIN(GGProtocol)
