#include "GGProtocol.h"

#include "SDK/constants.h"
#include "GGEditWindow.h"
#include "GGAccount.h"
#include "constants.h"

using namespace KittySDK;

KittySDK::GGProtocol::GGProtocol(PluginCore *core): Protocol(core)
{
  m_info = new ProtocolInfo("Gadu-Gadu Protocol", "0.0.1", "arturo182", "arturo182@tlen.pl", "http://www.arturpacholec.pl/", "Gadu-Gadu", KittyGG::Icons::I_AVAILABLE);
  m_editWindow = 0;

  core->addIcon(KittyGG::Icons::I_AVAILABLE, QPixmap(":/glyphs/available.png"));
  core->addIcon(KittyGG::Icons::I_AWAY, QPixmap(":/glyphs/away.png"));
  core->addIcon(KittyGG::Icons::I_DND, QPixmap(":/glyphs/dnd.png"));
  core->addIcon(KittyGG::Icons::I_FFC, QPixmap(":/glyphs/ffc.png"));
  core->addIcon(KittyGG::Icons::I_INVISIBLE, QPixmap(":/glyphs/invisible.png"));
  core->addIcon(KittyGG::Icons::I_UNAVAILABLE, QPixmap(":/glyphs/unavailable.png"));

  setAbilities(TextBold | TextItalics | TextUnderline | TextStriketrough | TextColor | BackgroundColor | SendImages | SendFiles | ChangeStatus | BlockContacts);
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
    m_editWindow = new KittySDK::GGEditWindow(account, this);
  }

  return m_editWindow;
}

KITTY_PLUGIN(GGProtocol)
