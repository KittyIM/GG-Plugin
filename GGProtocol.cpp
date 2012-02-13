#include "GGProtocol.h"

#include "SDK/GGConstants.h"
#include "SDK/constants.h"
#include "GGEditDialog.h"
#include "GGAccount.h"
#include "constants.h"

#define qDebug() qDebug() << "[GGProtocol]"
#define qWarning() qWarning() << "[GGProtocol]"

namespace KittySDK
{
GGProtocol::GGProtocol(PluginCore *core): Protocol(core)
{
	m_info = new ProtocolInfo("Gadu-Gadu Protocol", "0.0.1", "arturo182", "arturo182@tlen.pl", "http://www.arturpacholec.pl/", "Gadu-Gadu", Icons::I_GG_AVAILABLE);
	m_editWindow = 0;

	setAbilities(TextStandard | TextColor | SendImages | SendFiles | ChangeStatus | BlockContacts | TypingNotification);
}

GGProtocol::~GGProtocol()
{
	if(m_editWindow) {
		delete m_editWindow;
	}
}

void GGProtocol::init()
{
	qDebug() << "Init";
	core()->addIcon(Icons::I_GG_AVAILABLE, QPixmap(":/glyphs/available.png"));
	core()->addIcon(Icons::I_GG_AWAY, QPixmap(":/glyphs/away.png"));
	core()->addIcon(Icons::I_GG_DND, QPixmap(":/glyphs/dnd.png"));
	core()->addIcon(Icons::I_GG_FFC, QPixmap(":/glyphs/ffc.png"));
	core()->addIcon(Icons::I_GG_INVISIBLE, QPixmap(":/glyphs/invisible.png"));
	core()->addIcon(Icons::I_GG_UNAVAILABLE, QPixmap(":/glyphs/unavailable.png"));
}

void GGProtocol::load()
{

}

void GGProtocol::unload()
{

}

QString GGProtocol::statusIcon(Protocol::Status status)
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
			return Icons::I_GG_UNAVAILABLE;
		break;

		default:
			return Icons::I_BLANK;
		break;
	}
}

Account *GGProtocol::newAccount(const QString &uid)
{
	return new GGAccount(uid, this);
}

QDialog *GGProtocol::editDialog(Account *account)
{
	if(!m_editWindow) {
		m_editWindow = new GGEditDialog(this);
	}

	m_editWindow->setup(dynamic_cast<GGAccount*>(account));

	return m_editWindow;
}

Protocol::Status GGProtocol::convertStatus(const quint32 &status) const
{
	switch(status & ~0x4000) {
		case KittyGG::Status::Available:
		case KittyGG::Status::AvailableDescr:
			return Protocol::Online;
		break;

		case KittyGG::Status::Busy:
		case KittyGG::Status::BusyDescr:
			return Protocol::Away;
		break;

		case KittyGG::Status::DoNotDisturb:
		case KittyGG::Status::DoNotDisturbDescr:
			return Protocol::DND;
		break;

		case KittyGG::Status::FreeForChat:
		case KittyGG::Status::FreeForChatDescr:
			return Protocol::FFC;
		break;

		case KittyGG::Status::Invisible:
		case KittyGG::Status::InvisibleDescr:
			return Protocol::Invisible;
		break;
	}

	return Protocol::Offline;
}

void GGProtocol::execAction(const QString &name, const QMap<QString, QVariant> &args)
{
	qDebug() << name << args;
}

KITTY_PLUGIN(GGProtocol)
}
