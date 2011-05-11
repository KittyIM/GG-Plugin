#include "GGProtocol.h"

#include "SDK/GGConstants.h"
#include "SDK/constants.h"
#include "GGEditWindow.h"
#include "GGAccount.h"
#include "constants.h"

#define qDebug() qDebug() << "[GGProtocol]"
#define qWarning() qWarning() << "[GGProtocol]"

using namespace KittySDK;

KittySDK::GGProtocol::GGProtocol(PluginCore *core): Protocol(core)
{
  m_info = new ProtocolInfo("Gadu-Gadu Protocol", "0.0.1", "arturo182", "arturo182@tlen.pl", "http://www.arturpacholec.pl/", "Gadu-Gadu", Icons::I_GG_AVAILABLE);
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
  qDebug() << "Init";
  core()->addIcon(Icons::I_GG_AVAILABLE, QPixmap(":/glyphs/available.png"));
  core()->addIcon(Icons::I_GG_AWAY, QPixmap(":/glyphs/away.png"));
  core()->addIcon(Icons::I_GG_DND, QPixmap(":/glyphs/dnd.png"));
  core()->addIcon(Icons::I_GG_FFC, QPixmap(":/glyphs/ffc.png"));
  core()->addIcon(Icons::I_GG_INVISIBLE, QPixmap(":/glyphs/invisible.png"));
  core()->addIcon(Icons::I_GG_UNAVAILABLE, QPixmap(":/glyphs/unavailable.png"));
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
      return Icons::I_GG_AVAILABLE;
    break;

    case Away:
      return Icons::I_GG_AWAY;
    break;

    case FFC:
      return Icons::I_GG_FFC;
    break;

    case DND:
      return Icons::I_GG_DND;
    break;

    case Invisible:
      return Icons::I_GG_INVISIBLE;
    break;

    case Offline:
    default:
      return Icons::I_GG_UNAVAILABLE;
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

void KittySDK::GGProtocol::execAction(const QString &name, const QMap<QString, QVariant> &args)
{
  qDebug() << name << args;
}

KITTY_PLUGIN(GGProtocol)
