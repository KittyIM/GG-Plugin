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

  setAbilities(TextStandard | TextColor | SendImages | SendFiles | ChangeStatus | BlockContacts);
}

KittySDK::GGProtocol::~GGProtocol()
{
  if(m_editWindow) {
    delete m_editWindow;
  }
}

void KittySDK::GGProtocol::init()
{
  qDebug() << "GGProtocol::init()";
  core()->addIcon(KittyGG::Icons::I_AVAILABLE, QPixmap(":/glyphs/available.png"));
  core()->addIcon(KittyGG::Icons::I_AWAY, QPixmap(":/glyphs/away.png"));
  core()->addIcon(KittyGG::Icons::I_DND, QPixmap(":/glyphs/dnd.png"));
  core()->addIcon(KittyGG::Icons::I_FFC, QPixmap(":/glyphs/ffc.png"));
  core()->addIcon(KittyGG::Icons::I_INVISIBLE, QPixmap(":/glyphs/invisible.png"));
  core()->addIcon(KittyGG::Icons::I_UNAVAILABLE, QPixmap(":/glyphs/unavailable.png"));
}

void KittySDK::GGProtocol::load()
{

}

void KittySDK::GGProtocol::unload()
{

}

QString KittySDK::GGProtocol::statusIcon(KittySDK::Protocol::Status status)
{
  switch(status) {
    case Online:
      return KittyGG::Icons::I_AVAILABLE;
    break;

    case Away:
      return KittyGG::Icons::I_AWAY;
    break;

    case FFC:
      return KittyGG::Icons::I_FFC;
    break;

    case DND:
      return KittyGG::Icons::I_DND;
    break;

    case Invisible:
      return KittyGG::Icons::I_INVISIBLE;
    break;

    case Offline:
    default:
      return KittyGG::Icons::I_UNAVAILABLE;
    break;
  }
}

KittySDK::Account *KittySDK::GGProtocol::newAccount(const QString &uid)
{
  return new KittySDK::GGAccount(uid, this);
}

QWidget *KittySDK::GGProtocol::editWindow(KittySDK::Account *account)
{
  if(!m_editWindow) {
    m_editWindow = new KittySDK::GGEditWindow(this);
  }

  m_editWindow->setup(account);

  return m_editWindow;
}

KittySDK::Protocol::Status KittySDK::GGProtocol::convertStatus(const quint32 &status) const
{
  switch(status & ~0x4000) {
    case KittyGG::Statuses::S_AVAILABLE:
    case KittyGG::Statuses::S_AVAILABLE_D:
      return KittySDK::Protocol::Online;
    break;

    case KittyGG::Statuses::S_BUSY:
    case KittyGG::Statuses::S_BUSY_D:
      return KittySDK::Protocol::Away;
    break;

    case KittyGG::Statuses::S_DND:
    case KittyGG::Statuses::S_DND_D:
      return KittySDK::Protocol::DND;
    break;

    case KittyGG::Statuses::S_FFC:
    case KittyGG::Statuses::S_FFC_D:
      return KittySDK::Protocol::FFC;
    break;

    case KittyGG::Statuses::S_INVISIBLE:
    case KittyGG::Statuses::S_INVISIBLE_D:
      return KittySDK::Protocol::Invisible;
    break;
  }

  return KittySDK::Protocol::Offline;
}

KITTY_PLUGIN(GGProtocol)
