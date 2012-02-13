#include "GGAccount.h"

#include "SDK/SoundsConstants.h"
#include "SDK/GGConstants.h"
#include "SDK/constants.h"
#include "SDK/Message.h"
#include "GGContact.h"
#include "constants.h"
#include "GGClient.h"

#include <QtCore/QDebug>
#include <QtGui/QTextDocument>
#include <QtGui/QInputDialog>
#include <QtGui/QFileDialog>
#include <QtGui/QMenu>

#define qDebug() qDebug() << "[GGAccount]"
#define qWarning() qWarning() << "[GGAccount]"

using namespace KittySDK;

KittySDK::GGAccount::GGAccount(const QString &uid, GGProtocol *parent): Account(uid, parent)
{
	m_client = new GGClient(this);
	connect(m_client, SIGNAL(statusChanged(quint32,quint32,QString)), this, SLOT(changeContactStatus(quint32,quint32,QString)));
	connect(m_client, SIGNAL(userDataReceived(quint32,QString,QString)), this, SLOT(processUserData(quint32,QString,QString)));
	connect(m_client, SIGNAL(messageReceived(QList<quint32>,QDateTime,QString)), this, SLOT(processMessage(QList<quint32>,QDateTime,QString)));
	connect(m_client, SIGNAL(imageReceived(quint32,QString,quint32,QByteArray)), this, SLOT(processImage(quint32,QString,quint32,QByteArray)));
	connect(m_client, SIGNAL(contactImported(quint32,QMap<QString,QString>)), this, SLOT(importContact(quint32,QMap<QString,QString>)));
	connect(m_client, SIGNAL(typingNotifyReceived(quint32,int)), this, SLOT(processTypingNotify(quint32,int)));

	setMe(new GGContact(uid, this));
	me()->setDisplay(protocol()->core()->profileName());

	connect(&m_blinkTimer, SIGNAL(timeout()), SLOT(toggleConnectingStatus()));

	m_statusMenu = new QMenu();

	QMenu *contactsMenu = m_statusMenu->addMenu(tr("Contacts"));
	QMenu *importMenu = contactsMenu->addMenu(tr("Import"));

	importMenu->addAction(tr("From server"), this, SLOT(importFromServer()));
	importMenu->addAction(tr("From file"), this, SLOT(importFromFile()));

	m_statusMenu->addSeparator();
	m_descriptionAction = new QAction(tr("Description..."), this);
	connect(m_descriptionAction, SIGNAL(triggered()), this, SLOT(showDescriptionInput()));
	m_statusMenu->addAction(m_descriptionAction);

	m_statusMenu->addSeparator();

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

QString KittySDK::GGAccount::description() const
{
	return m_client->description();
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

	m_useSSL = settings.value("useSSL", true).toBool();
	m_friendsOnly = settings.value("statusFriendsOnly", false).toBool();
	m_initialStatus = settings.value("initialStatus", 0).toUInt();
	m_serverList = settings.value("serverList").toStringList();

	int count = settings.value("descriptionCount").toInt();
	for(int i = 0; i < count; i++) {
		m_descriptionHistory.append(settings.value(QString("description%1").arg(i)).toString());
	}
}

QMap<QString, QVariant> GGAccount::saveSettings()
{
	QMap<QString, QVariant> settings;

	settings.insert("userSSL", m_useSSL);
	settings.insert("statusFriendsOnly", m_friendsOnly);
	settings.insert("initialStatus", m_initialStatus);
	settings.insert("serverList", m_serverList);

	settings.insert("descriptionCount", m_descriptionHistory.count());
	for(int i = 0; i < m_descriptionHistory.count(); i++) {
		settings.insert(QString("description%1").arg(i), m_descriptionHistory.at(i));
	}

	return settings;
}

void KittySDK::GGAccount::changeStatus(const KittySDK::Protocol::Status &status, const QString &descr)
{
	quint32 stat = KittyGG::Status::Available;
	switch(status) {
		case Protocol::Away:
			stat = KittyGG::Status::Busy;
		break;

		case Protocol::FFC:
			stat = KittyGG::Status::FreeForChat;
		break;

		case Protocol::DND:
			stat = KittyGG::Status::DoNotDisturb;
		break;

		case Protocol::Invisible:
			stat = KittyGG::Status::Invisible;
		break;

		case Protocol::Offline:
			stat = KittyGG::Status::Unavailable;
		break;

		default:
			stat = KittyGG::Status::Available;
		break;
	}

	if((stat != m_client->status()) || (descr != this->description())) {
		if(!m_client->isConnected()) {
			m_blinkTimer.start(1000);
		}

		m_client->changeStatus(stat, descr);
	}
}

QMenu *KittySDK::GGAccount::statusMenu()
{
	return m_statusMenu;
}

void KittySDK::GGAccount::sendMessage(const Message &msg)
{
	if(m_client->isConnected()) {
		if(GGContact *cnt = dynamic_cast<GGContact*>(msg.to().first())) {
			m_client->sendMessage(cnt->uin(), msg.body());
		}
	} else {
		//TODO: Create a queue and send when connected
		Message err(msg.to().first(), me());
		err.setBody(tr("Not connected!"));
		err.setDirection(Message::System);
		emit messageReceived(err);
	}
}

void GGAccount::sendTypingNotify(Contact *contact, bool typing, const int &length)
{
	m_client->sendTypingNotify(contact->uid().toUInt(), typing ? length : 0);
}

void KittySDK::GGAccount::changeContactStatus(const quint32 &uin, const quint32 &status, const QString &description)
{
	QString uid = QString::number(uin);

	if(uid == this->uid()) {
		if(GGProtocol *ggproto = dynamic_cast<KittySDK::GGProtocol*>(protocol())) {
			m_blinkTimer.stop();
			emit statusChanged(ggproto->convertStatus(status), description);
		}
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

void KittySDK::GGAccount::processMessage(QList<quint32> senders, const QDateTime &time, const QString &plain)
{
	QList<Contact*> contacts;
	for(int i = 1; i < senders.count(); i++) {
		contacts.append(contactByUin(senders[i]));
	}
	contacts.prepend(me());

	Message msg(contactByUin(senders.first()), contacts);
	msg.setDirection(Message::Incoming);
	msg.setBody(plain);
	msg.setTimeStamp(time);

	QMap<QString, QVariant> args;
	args.insert("id", Sounds::S_MSG_RECV);
	protocol()->core()->execPluginAction("Sounds", "playSound", args);

	emit messageReceived(msg);
}

void KittySDK::GGAccount::processImage(const quint32 &sender, const QString &imgName, const quint32 &crc32, const QByteArray &data)
{
	QDir imgDir(protocol()->core()->profilesDir() + protocol()->core()->profileName() + "/imgcache/");

	//create dir if it doesn't exist
	imgDir.mkpath(".");

	QFile file(imgDir.absolutePath() + "/" + imgName);
	if(file.open(QFile::WriteOnly)) {
		file.write(data);
		file.close();

		Message msg(contactByUin(sender), me());
		msg.setDirection(Message::Incoming);
		msg.setBody(QString("<img src=\"%1%2\" alt=\"%2\" title=\"%2\">").arg(imgDir.absolutePath() + "/").arg(Qt::escape(imgName)));

		QMap<QString, QVariant> args;
		args.insert("id", Sounds::S_MSG_RECV);
		protocol()->core()->execPluginAction("Sounds", "playSound", args);

		emit messageReceived(msg);
	}
}

void KittySDK::GGAccount::importContact(const quint32 &uin, const QMap<QString, QString> &data)
{
	Contact *cnt = contactByUin(uin);

	if(cnt->display() == cnt->uid()) {
		cnt->setDisplay(data.value("ShowName"));
	}

	if(cnt->group().isEmpty()) {
		cnt->setGroup(data.value("Group"));
	}

	if(cnt->data(ContactInfos::I_HOMEPAGE).toString().isEmpty()) {
		cnt->setData(ContactInfos::I_HOMEPAGE, data.value("WwwAddress"));
	}

	if(cnt->data(ContactInfos::I_FIRSTNAME).toString().isEmpty()) {
		cnt->setData(ContactInfos::I_FIRSTNAME, data.value("FirstName"));
	}

	if(cnt->data(ContactInfos::I_LASTNAME).toString().isEmpty()) {
		cnt->setData(ContactInfos::I_LASTNAME, data.value("LastName"));
	}

	if(cnt->data(ContactInfos::I_HOME_STATE).toString().isEmpty()) {
		cnt->setData(ContactInfos::I_HOME_STATE, data.value("Province"));
	}

	/*
  data.insert("MobilePhone", phone.firstChild().nodeValue());
  data.insert("Email", email.firstChild().nodeValue());
  data.insert("Gender", sex.firstChild().nodeValue());
  data.insert("Birth", birthday.firstChild().nodeValue());
  data.insert("City", city.firstChild().nodeValue());
*/

	emit contactAdded(cnt);
}

void GGAccount::processTypingNotify(const quint32 &sender, const int &type)
{
	Contact *cnt = contactByUin(sender);
	if(cnt) {
		emit typingNotifyReceived(cnt, type > 0, type);
	} else {
		qWarning() << "Unknown uin" << sender << "is sending us typing notify";
	}
}

void GGAccount::toggleConnectingStatus()
{
	if(m_blinkTimer.property("online").toBool()) {
		emit statusChanged((Protocol::Status)-1, "");
		m_blinkTimer.setProperty("online", false);
	} else {
		emit statusChanged(Protocol::Online, "");
		m_blinkTimer.setProperty("online", true);
	}
}

void KittySDK::GGAccount::showDescriptionInput()
{
	QInputDialog dialog;
	dialog.setLabelText(tr("New description:"));
	dialog.setComboBoxEditable(true);
	dialog.setComboBoxItems(m_descriptionHistory);

	if(dialog.exec() == QDialog::Accepted) {
		QString description = dialog.textValue();

		if(!description.isEmpty()) {
			m_descriptionHistory.removeAll(description);
			m_descriptionHistory.prepend(description);
		}

		m_client->setDescription(description);
	}
}

void KittySDK::GGAccount::setStatusAvailable()
{
	changeStatus(Protocol::Online, description());
}

void KittySDK::GGAccount::setStatusAway()
{
	changeStatus(Protocol::Away, description());
}

void KittySDK::GGAccount::setStatusFFC()
{
	changeStatus(Protocol::FFC, description());
}

void KittySDK::GGAccount::setStatusDND()
{
	changeStatus(Protocol::DND, description());
}

void KittySDK::GGAccount::setStatusInvisible()
{
	changeStatus(Protocol::Invisible, description());
}

void KittySDK::GGAccount::setStatusUnavailable()
{
	changeStatus(Protocol::Offline, description());
}

void KittySDK::GGAccount::importFromServer()
{
	m_client->requestRoster();
}

void KittySDK::GGAccount::importFromFile()
{
	QString fileName = QFileDialog::getOpenFileName(0, tr("Choose file"), "", tr("XML files") + " (GG 8+) (*.xml);;" + tr("Text files") + " (GG 6-7) (*.txt)");
	if(!fileName.isEmpty()) {
		QFile file(fileName);
		if(file.open(QFile::ReadOnly)) {
			m_client->parseXMLRoster(file.readAll());
			file.close();
		}
	}
}
