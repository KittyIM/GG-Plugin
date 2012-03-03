#include "GGProtocol.h"

#include <SDKConstants.h>
#include <GGConstants.h>

#include "GGEditDialog.h"
#include "GGAccount.h"
#include "constants.h"

#define qDebug() qDebug() << "[GGProtocol]"
#define qWarning() qWarning() << "[GGProtocol]"

namespace GG
{

Protocol::Protocol(KittySDK::IPluginCore *core): KittySDK::IProtocol(core)
{
	m_info = new KittySDK::IProtocolInfo();
	m_info->setName(tr("Gadu-Gadu Protocol"));
	m_info->setId("ggproto");
	m_info->setVersion("0.0.1");
	m_info->setAuthor("arturo182");
	m_info->setEmail("arturo182@tlen.pl");
	m_info->setURL("http://www.arturpacholec.pl/");
	static_cast<KittySDK::IProtocolInfo*>(m_info)->setProtoName("Gadu-Gadu");
	static_cast<KittySDK::IProtocolInfo*>(m_info)->setProtoIcon(KittySDK::Icons::I_GG_AVAILABLE);

	m_editWindow = 0;

	setAbilities(TextStandard | TextColor | SendImages | SendFiles | ChangeStatus | BlockContacts | TypingNotification);
}

Protocol::~Protocol()
{
	if(m_editWindow) {
		delete m_editWindow;
	}
}

void Protocol::init()
{
	core()->addIcon(KittySDK::Icons::I_GG_AVAILABLE, QPixmap(":/glyphs/available.png"));
	core()->addIcon(KittySDK::Icons::I_GG_AWAY, QPixmap(":/glyphs/away.png"));
	core()->addIcon(KittySDK::Icons::I_GG_DND, QPixmap(":/glyphs/dnd.png"));
	core()->addIcon(KittySDK::Icons::I_GG_FFC, QPixmap(":/glyphs/ffc.png"));
	core()->addIcon(KittySDK::Icons::I_GG_INVISIBLE, QPixmap(":/glyphs/invisible.png"));
	core()->addIcon(KittySDK::Icons::I_GG_UNAVAILABLE, QPixmap(":/glyphs/unavailable.png"));
}

void Protocol::load()
{

}

void Protocol::unload()
{

}

QString Protocol::statusIcon(KittySDK::IProtocol::Status status)
{
	switch(status) {
		case Online:
			return KittySDK::Icons::I_GG_AVAILABLE;
		break;

		case Away:
			return KittySDK::Icons::I_GG_AWAY;
		break;

		case FFC:
			return KittySDK::Icons::I_GG_FFC;
		break;

		case DND:
			return KittySDK::Icons::I_GG_DND;
		break;

		case Invisible:
			return KittySDK::Icons::I_GG_INVISIBLE;
		break;

		case Offline:
			return KittySDK::Icons::I_GG_UNAVAILABLE;
		break;

		default:
			return KittySDK::Icons::I_BLANK;
		break;
	}
}

KittySDK::IAccount *Protocol::newAccount(const QString &uid)
{
	Account *acc = new Account(uid, this);
	m_accounts << acc;
	return acc;
}

QDialog *Protocol::editDialog(KittySDK::IAccount *account)
{
	if(!m_editWindow) {
		m_editWindow = new EditDialog(this);
	}

	m_editWindow->setup(dynamic_cast<Account*>(account));

	return m_editWindow;
}

KittySDK::IProtocol::Status Protocol::convertStatus(const quint32 &status) const
{
	switch(status & ~0x4000) {
		case KittyGG::Status::Available:
		case KittyGG::Status::AvailableDescr:
			return KittySDK::IProtocol::Online;
		break;

		case KittyGG::Status::Busy:
		case KittyGG::Status::BusyDescr:
			return KittySDK::IProtocol::Away;
		break;

		case KittyGG::Status::DoNotDisturb:
		case KittyGG::Status::DoNotDisturbDescr:
			return KittySDK::IProtocol::DND;
		break;

		case KittyGG::Status::FreeForChat:
		case KittyGG::Status::FreeForChatDescr:
			return KittySDK::IProtocol::FFC;
		break;

		case KittyGG::Status::Invisible:
		case KittyGG::Status::InvisibleDescr:
			return KittySDK::IProtocol::Invisible;
		break;
	}

	return KittySDK::IProtocol::Offline;
}

void Protocol::execAction(const QString &name, const QMap<QString, QVariant> &args)
{
	if(name == "retranslate") {
		m_info->setName(tr("Gadu-Gadu Protocol"));

		foreach(Account *acc, m_accounts) {
			acc->retranslate();
		}
	} else {
		qDebug() << name << args;
	}
}

KITTY_PLUGIN(Protocol)
}
