#include "GGAccount.h"

#include "KittyGG/DataStream.h"
#include "KittyGG/HUBLookup.h"
#include "KittyGG/KittyGG.h"
#include "KittyGG/Parser.h"
#include "GGContact.h"
#include "constants.h"

#include <SoundsConstants.h>
#include <SDKConstants.h>
#include <GGConstants.h>
#include <IMessage.h>
#include <IChat.h>

#include <QtCore/QSignalMapper>
#include <QtCore/QThreadPool>
#include <QtCore/QDebug>
#include <QtGui/QTextDocument>
#include <QtGui/QInputDialog>
#include <QtGui/QFileDialog>
#include <QtGui/QMenu>
#include <QtNetwork/QNetworkProxy>
#include <QtXml/QDomDocument>

#define qDebug() qDebug() << "[GGAccount]"
#define qWarning() qWarning() << "[GGAccount]"

namespace GG
{

Account::Account(const QString &uid, Protocol *parent): KittySDK::IAccount(uid, parent)
{
	m_socket = new QSslSocket(this);
	m_socket->setProxy(QNetworkProxy::applicationProxy());

	connect(m_socket, SIGNAL(readyRead()), SLOT(readSocket()));
	connect(m_socket, SIGNAL(disconnected()), SLOT(disconnected()));
	connect(m_socket, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(error(QAbstractSocket::SocketError)));
	connect(m_socket, SIGNAL(proxyAuthenticationRequired(QNetworkProxy,QAuthenticator*)), SLOT(proxyAuthenticationRequired(QNetworkProxy,QAuthenticator*)));
	connect(m_socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), SLOT(stateChanged(QAbstractSocket::SocketState)));

	connect(&m_pingTimer, SIGNAL(timeout()), SLOT(sendPingPacket()));
	connect(&m_ackTimer, SIGNAL(timeout()), SLOT(checkMsgAcks()));

	m_status = KittyGG::Status::Unavailable;

	m_pingTimer.setInterval(3 * 60 * 1000);
	m_ackTimer.setInterval(1000);
	m_ackTimer.start();

	m_parser = new KittyGG::Parser();
	connect(m_parser, SIGNAL(packetReceived(quint32,quint32,QByteArray)), SLOT(processPacket(quint32,quint32,QByteArray)));

	setMe(new Contact(uid, this));
	me()->setDisplay(protocol()->core()->profileName());

	connect(&m_blinkTimer, SIGNAL(timeout()), SLOT(toggleConnectingStatus()));

	m_statusMenu = new QMenu();


	QMenu *contactsMenu = m_statusMenu->addMenu(tr("Contacts"));
	QMenu *importMenu = contactsMenu->addMenu(tr("Import"));

	importMenu->addAction(tr("From server"), this, SLOT(importFromServer()));
	importMenu->addAction(tr("From file"), this, SLOT(importFromFile()));

	m_avatarAction = new QAction(tr("Update avatars"), this);
	connect(m_avatarAction, SIGNAL(triggered()), SLOT(updateAvatars()));
	m_statusMenu->addAction(m_avatarAction);

	m_statusMapper = new QSignalMapper(this);
	connect(m_statusMapper, SIGNAL(mapped(int)), SLOT(setStatus(int)));

	m_statusMenu->addSeparator();
	m_descriptionAction = new QAction(tr("Description..."), this);
	connect(m_descriptionAction, SIGNAL(triggered()), SLOT(showDescriptionInput()));
	m_statusMenu->addAction(m_descriptionAction);

	m_statusMenu->addSeparator();
	m_availableAction = new QAction(protocol()->core()->icon(KittySDK::Icons::I_GG_AVAILABLE), tr("Available"), this);
	m_statusMapper->setMapping(m_availableAction, KittySDK::IProtocol::Online);
	connect(m_availableAction, SIGNAL(triggered()), m_statusMapper, SLOT(map()));
	m_statusMenu->addAction(m_availableAction);

	m_awayAction = new QAction(protocol()->core()->icon(KittySDK::Icons::I_GG_AWAY), tr("Be right back"), this);
	m_statusMapper->setMapping(m_awayAction, KittySDK::IProtocol::Away);
	connect(m_awayAction, SIGNAL(triggered()), m_statusMapper, SLOT(map()));
	m_statusMenu->addAction(m_awayAction);

	m_ffcAction = new QAction(protocol()->core()->icon(KittySDK::Icons::I_GG_FFC), tr("Free for chat"), this);
	m_statusMapper->setMapping(m_ffcAction, KittySDK::IProtocol::FFC);
	connect(m_ffcAction, SIGNAL(triggered()), m_statusMapper, SLOT(map()));
	m_statusMenu->addAction(m_ffcAction);

	m_dndAction = new QAction(protocol()->core()->icon(KittySDK::Icons::I_GG_DND), tr("Do not disturb"), this);
	m_statusMapper->setMapping(m_dndAction, KittySDK::IProtocol::DND);
	connect(m_dndAction, SIGNAL(triggered()), m_statusMapper, SLOT(map()));
	m_statusMenu->addAction(m_dndAction);

	m_invisibleAction = new QAction(protocol()->core()->icon(KittySDK::Icons::I_GG_INVISIBLE), tr("Invisible"), this);
	m_statusMapper->setMapping(m_invisibleAction, KittySDK::IProtocol::Invisible);
	connect(m_invisibleAction, SIGNAL(triggered()), m_statusMapper, SLOT(map()));
	m_statusMenu->addAction(m_invisibleAction);

	m_unavailableAction = new QAction(protocol()->core()->icon(KittySDK::Icons::I_GG_UNAVAILABLE), tr("Unavailable"), this);
	m_statusMapper->setMapping(m_unavailableAction, KittySDK::IProtocol::Offline);
	connect(m_unavailableAction, SIGNAL(triggered()), m_statusMapper, SLOT(map()));
	m_statusMenu->addAction(m_unavailableAction);
}

Account::~Account()
{
	QThreadPool::globalInstance()->waitForDone();

	m_pingTimer.stop();
	//FIXME: Maybe this could be handled better
	disconnect(m_socket, SIGNAL(disconnected()), this, SLOT(disconnected()));

	delete m_socket;
	delete m_statusMenu;
}

quint32 Account::uin() const
{
	return m_uid.toUInt();
}

KittySDK::IProtocol::Status Account::status() const
{
	return dynamic_cast<Protocol*>(protocol())->convertStatus(m_status);
}

QString Account::description() const
{
	return m_description;
}

KittySDK::IContact *Account::newContact(const QString &uid)
{
	foreach(KittySDK::IContact *cnt, contacts()) {
		if(cnt->uid() == uid) {
			return cnt;
		}
	}

	Contact *cnt = new Contact(uid, this);

	return cnt;
}

KittySDK::IContact *Account::newContact(const quint32 &uin)
{
	return newContact(QString::number(uin));
}

KittySDK::IContact *Account::contactByUin(const quint32 &uin, bool temp)
{
	foreach(KittySDK::IContact *cnt, m_contacts) {
		if(cnt->uid() == QString::number(uin)) {
			return cnt;
		}
	}

	KittySDK::IContact *cnt = newContact(uin);
	cnt->setDisplay(QString::number(uin));
	cnt->setData(KittySDK::ContactInfos::I_TEMPORARY, temp);
	insertContact(cnt->uid(), cnt);
	emit contactAdded(cnt);

	return cnt;
}

void Account::insertContact(const QString &uid, KittySDK::IContact *contact)
{
	KittySDK::IAccount::insertContact(uid, contact);

	if(isConnected()) {
		KittyGG::NotifyAdd packet(uid.toUInt());
		sendPacket(packet);
	}
}

bool Account::isConnected() const
{
	return (m_socket->state() == QAbstractSocket::ConnectedState);
}

void Account::loadSettings(const QMap<QString, QVariant> &settings)
{
	QStringList defaultServers;
	for(int i = 10; i <= 30; ++i) {
		defaultServers << "ggproxy-" + QString::number(i) + ".gadu-gadu.pl";
	}

	m_useSSL = settings.value("useSSL", true).toBool();
	m_friendsOnly = settings.value("statusFriendsOnly", false).toBool();
	m_initialStatus = settings.value("initialStatus", 0).toUInt();
	m_serverList = settings.value("serverList", defaultServers).toStringList();

	int count = settings.value("descriptionCount").toInt();
	for(int i = 0; i < count; ++i) {
		m_descriptionHistory.append(settings.value(QString("description%1").arg(i)).toString());
	}

	if(m_initialStatus) {
		if(m_initialStatus != KittyGG::Status::Unavailable) {
			if(Protocol *ggproto = dynamic_cast<Protocol*>(m_protocol)) {
				changeStatus(ggproto->convertStatus(m_initialStatus), settings.value("previousDescription").toString());
			}
		}
	} else {
		if(Protocol *ggproto = dynamic_cast<Protocol*>(m_protocol)) {
			quint32 status = settings.value("previousStatus").toUInt();
			changeStatus(ggproto->convertStatus(status), settings.value("previousDescription").toString());
		}
	}
}

QMap<QString, QVariant> Account::saveSettings()
{
	QMap<QString, QVariant> settings;

	settings.insert("userSSL", m_useSSL);
	settings.insert("statusFriendsOnly", m_friendsOnly);
	settings.insert("initialStatus", m_initialStatus);
	settings.insert("previousStatus", m_status);
	settings.insert("previousDescription", m_description);
	settings.insert("serverList", m_serverList);

	settings.insert("descriptionCount", m_descriptionHistory.count());
	for(int i = 0; i < m_descriptionHistory.count(); ++i) {
		settings.insert(QString("description%1").arg(i), m_descriptionHistory.at(i));
	}

	return settings;
}

void Account::changeStatus(const KittySDK::IProtocol::Status &stat, const QString &description)
{

	quint32 status = KittyGG::Status::Available;
	switch(stat) {
		case KittySDK::IProtocol::Away:
			if(description.isEmpty()) {
				status = KittyGG::Status::Busy;
			} else {
				status = KittyGG::Status::BusyDescr;
			}
		break;

		case KittySDK::IProtocol::FFC:
			if(description.isEmpty()) {
				status = KittyGG::Status::FreeForChat;
			} else {
				status = KittyGG::Status::FreeForChatDescr;
			}
		break;

		case KittySDK::IProtocol::DND:
			if(description.isEmpty()) {
				status = KittyGG::Status::DoNotDisturb;
			} else {
				status = KittyGG::Status::DoNotDisturbDescr;
			}
		break;

		case KittySDK::IProtocol::Invisible:
			if(description.isEmpty()) {
				status = KittyGG::Status::Invisible;
			} else {
				status = KittyGG::Status::InvisibleDescr;
			}
		break;

		case KittySDK::IProtocol::Offline:
			if(description.isEmpty()) {
				status = KittyGG::Status::Unavailable;
			} else {
				status = KittyGG::Status::UnavailableDescr;
			}
		break;

		default:
			if(description.isEmpty()) {
				status = KittyGG::Status::Available;
			} else {
				status = KittyGG::Status::AvailableDescr;
			}
		break;
	}

	if(((status | 0x4000) != (m_status | 0x4000)) || (description != m_description)) {
		m_status = status;
		m_description = description;

		if(isConnected()) {
			sendChangeStatusPacket();
		} else {
			startHubLookup();
		}
	}
}

QMenu *Account::statusMenu()
{
	return m_statusMenu;
}

void Account::sendMessage(const KittySDK::IMessage &msg)
{
	if(isConnected()) {
		QList<quint32> uins;

		foreach(KittySDK::IContact *cnt, msg.to()) {
			uins << cnt->uid().toUInt();
		}

		foreach(quint32 uin, uins) {
			QList<quint32> customUins = uins;
			customUins.removeAll(uin);
			customUins.prepend(uin);

			KittyGG::MessageSend packet;
			packet.setHtmlBody(msg.body());
			packet.setUins(customUins);

			if(msg.chat()) {
				QString body = msg.body();
				body.remove(QRegExp("<[^>]*>"));

				AckItem *ack = new AckItem();
				ack->timer = 0;
				ack->msg = body;
				ack->contact = msg.to().first();
				ack->seq = packet.seq();
				m_ackList << ack;
			}

			sendPacket(packet);
		}
	} else {
		protocol()->core()->enqueueMsg(msg);
	}
}

void Account::sendTypingNotify(KittySDK::IContact *contact, bool typing, const int &length)
{
	if(isConnected()) {
		KittyGG::TypingNotify notify(typing ? length : 0, contact->uid().toUInt());
		sendPacket(notify);
	}
}

void Account::retranslate()
{
	if(m_statusMenu->actions().count()) {
		if(QAction *contactsAction = m_statusMenu->actions().first()) {
			contactsAction->setText(tr("Contacts"));

			QMenu *contactsMenu = contactsAction->menu();
			if(contactsMenu->actions().count()) {
				if(QAction *importAction = contactsMenu->actions().first()) {
					importAction->setText(tr("Import"));

					QMenu *importMenu = importAction->menu();
					QList<QAction*> importActions = importMenu->actions();
					if(importActions.count()) {
						importActions[0]->setText(tr("From server"));
						importActions[1]->setText(tr("From file"));
					}
				}
			}
		}
	}

	m_descriptionAction->setText(tr("Description..."));
	m_availableAction->setText(tr("Available"));
	m_awayAction->setText(tr("Be right back"));
	m_ffcAction->setText(tr("Free for chat"));
	m_dndAction->setText(tr("Do not disturb"));
	m_invisibleAction->setText(tr("Invisible"));
	m_unavailableAction->setText(tr("Unavailable"));
}

void Account::changeContactStatus(const quint32 &uin, const quint32 &status, const QString &description)
{
	QString uid = QString::number(uin);

	if(uid == m_uid) {
		if(Protocol *ggproto = dynamic_cast<Protocol*>(m_protocol)) {
			m_blinkTimer.stop();
			emit statusChanged(ggproto->convertStatus(status), description);
		}
	}

	if(m_contacts.contains(uid)) {
		dynamic_cast<Contact*>(m_contacts.value(uid))->changeStatus(status, description);
	}
}

void Account::toggleConnectingStatus()
{
	if(m_blinkTimer.property("online").toBool()) {
		emit statusChanged((KittySDK::IProtocol::Status)-1, "");
		m_blinkTimer.setProperty("online", false);
	} else {
		emit statusChanged(KittySDK::IProtocol::Online, "");
		m_blinkTimer.setProperty("online", true);
	}
}

void Account::showDescriptionInput()
{
	QInputDialog dialog;
	dialog.setLabelText(tr("New description:"));
	dialog.setComboBoxEditable(true);
	dialog.setComboBoxItems(m_descriptionHistory);

	if(dialog.exec() == QDialog::Accepted) {
		QString description = dialog.textValue();

		if(!description.isEmpty()) {
			m_descriptionHistory.removeOne(description);
			m_descriptionHistory.prepend(description);
		}

		m_description = description;
		if(isConnected()) {
			sendChangeStatusPacket();
		}
	}
}

void Account::setStatus(int status)
{
	changeStatus(static_cast<KittySDK::IProtocol::Status>(status), description());
}

void Account::importFromServer()
{
	KittyGG::ListRequest packet(KittyGG::ListRequest::Get, 0);
	sendPacket(packet);
}

void Account::importFromFile()
{
	QString fileName = QFileDialog::getOpenFileName(0, tr("Choose file"), "", tr("XML files") + " (GG 8+) (*.xml);;" + tr("Text files") + " (GG 6-7) (*.txt)");
	if(!fileName.isEmpty()) {
		QFile file(fileName);
		if(file.open(QFile::ReadOnly)) {
			parseXMLRoster(file.readAll());
			file.close();
		}
	}
}

void Account::parseXMLRoster(const QString &xml)
{
	//qDebug() << QString(xml).replace("<", "&lt;");

	QDomDocument doc;
	doc.setContent(xml);

	QDomElement root = doc.documentElement();
	if(root.nodeName() == "ContactBook") {
		QMap <QString, QString> groups;

		QDomElement groupsElement = root.namedItem("Groups").toElement();
		QDomNodeList groupList = groupsElement.elementsByTagName("Group");
		for(int i = 0; i < groupList.count(); ++i) {
			QDomElement group = groupList.at(i).toElement();

			QDomElement id = group.namedItem("Id").toElement();
			QDomElement name = group.namedItem("Name").toElement();

			groups.insert(id.firstChild().nodeValue(), name.firstChild().nodeValue());
		}

		QDomElement contactsElement = root.namedItem("Contacts").toElement();
		QDomNodeList contactList = contactsElement.elementsByTagName("Contact");
		for(int i = 0; i < contactList.count(); ++i) {
			QDomElement contact = contactList.at(i).toElement();

			QDomElement number = contact.namedItem("GGNumber").toElement();
			QDomElement display = contact.namedItem("ShowName").toElement();
			QDomElement phone = contact.namedItem("MobilePhone").toElement();
			QDomElement email = contact.namedItem("Email").toElement();
			QDomElement homepage = contact.namedItem("WwwAddress").toElement();
			QDomElement firstname = contact.namedItem("FirstName").toElement();
			QDomElement lastname = contact.namedItem("LastName").toElement();
			QDomElement sex = contact.namedItem("Gender").toElement();
			QDomElement birthday = contact.namedItem("Birth").toElement();
			QDomElement city = contact.namedItem("City").toElement();
			QDomElement state = contact.namedItem("Province").toElement();
			QDomElement group = contact.namedItem("Groups").toElement();

			QMap<QString, QString> data;
			data.insert("ShowName", display.firstChild().nodeValue());
			data.insert("MobilePhone", phone.firstChild().nodeValue());
			data.insert("Email", email.firstChild().nodeValue());
			data.insert("WwwAddress", homepage.firstChild().nodeValue());
			data.insert("FirstName", firstname.firstChild().nodeValue());
			data.insert("LastName", lastname.firstChild().nodeValue());
			data.insert("Gender", sex.firstChild().nodeValue());
			data.insert("Birth", birthday.firstChild().nodeValue());
			data.insert("City", city.firstChild().nodeValue());
			data.insert("Province", state.firstChild().nodeValue());

			QString groupId = group.namedItem("GroupId").firstChild().nodeValue();
			if((groupId == "00000000-0000-0000-0000-000000000000") && (group.childNodes().count() > 1)) {
				groupId = group.elementsByTagName("GroupId").at(1).firstChild().nodeValue();
			}

			data.insert("Group", groups.value(groupId));

			quint32 uin = number.firstChild().nodeValue().toUInt();
			KittySDK::IContact *cnt = contactByUin(uin);

			if(cnt->display() == cnt->uid()) {
				cnt->setDisplay(data.value("ShowName"));
			}

			if(cnt->group().isEmpty()) {
				cnt->setGroup(data.value("Group"));
			}

			if(cnt->data(KittySDK::ContactInfos::I_HOMEPAGE).toString().isEmpty()) {
				cnt->setData(KittySDK::ContactInfos::I_HOMEPAGE, data.value("WwwAddress"));
			}

			if(cnt->data(KittySDK::ContactInfos::I_FIRSTNAME).toString().isEmpty()) {
				cnt->setData(KittySDK::ContactInfos::I_FIRSTNAME, data.value("FirstName"));
			}

			if(cnt->data(KittySDK::ContactInfos::I_LASTNAME).toString().isEmpty()) {
				cnt->setData(KittySDK::ContactInfos::I_LASTNAME, data.value("LastName"));
			}

			if(cnt->data(KittySDK::ContactInfos::I_HOME_STATE).toString().isEmpty()) {
				cnt->setData(KittySDK::ContactInfos::I_HOME_STATE, data.value("Province"));
			}

		/*
		  data.insert("MobilePhone", phone.firstChild().nodeValue());
		  data.insert("Email", email.firstChild().nodeValue());
		  data.insert("Gender", sex.firstChild().nodeValue());
		  data.insert("Birth", birthday.firstChild().nodeValue());
		  data.insert("City", city.firstChild().nodeValue());
		*/
		}
	} else {
		qWarning() << "Wrong format!";
	}
}

void Account::processPacket(const quint32 &type, const quint32 &length, QByteArray packet)
{
	//qDebug() << "PACKET" << type << length;

	switch(type) {
		case KittyGG::Welcome::Type:
		{
			KittyGG::Welcome welcome = KittyGG::Welcome::fromData(packet);

			KittyGG::Login login(uin(), m_password, welcome.seed());
			login.setInitialStatus(m_status | 0x4000);
			login.setInitialDescription(m_description);
			sendPacket(login);
		}
		break;

		case KittyGG::LoginOk::Type:
		{
			m_pingTimer.start();

			//send roster
			if(m_contacts.empty()) {
				KittyGG::ListEmpty packet;
				sendPacket(packet);
			} else {
				QList<KittyGG::NotifyEntry> roster;
				foreach(const KittySDK::IContact *cnt, m_contacts) {
					roster << KittyGG::NotifyEntry(cnt->uid().toUInt());
				}

				int count = roster.size();
				while(count > 0) {
					int part_count;

					KittyGG::NotifyFirst *packet;

					if(count > 400) {
						part_count = 400;
						packet = new KittyGG::NotifyFirst();
					} else {
						part_count = count;
						packet = new KittyGG::NotifyLast();
					}

					packet->setContacts(roster.mid(roster.size() - count, part_count));

					sendPacket(*packet);

					delete packet;

					count -= part_count;
				}
			}
		}
		break;

		case KittyGG::LoginFailed::Type:
		{
			m_socket->disconnectFromHost();
		}
		break;

		case KittyGG::NotifyReply::Type:
		case KittyGG::Status::Type:
		{
			int left = length;
			while(left > 0) {
				KittyGG::Status status = KittyGG::Status::fromData(packet);

				if(status.uin() == uin()) {
					m_status = status.status();
					m_description = status.description();
				}

				packet = packet.mid(status.size());
				left -= status.size();

				changeContactStatus(status.uin(), status.status(), status.description());
			}

			if(left > 0) {
				qDebug() << "left" << left;
			}
		}
		break;

		case KittyGG::XmlAction::Type:
		{
			KittyGG::XmlAction data = KittyGG::XmlAction::fromData(packet);
			//TODO: emit xmlActionReceived(data.action());
		}
		break;

		case KittyGG::MessageRecv::Type:
		{
			KittyGG::MessageRecv msg = KittyGG::MessageRecv::fromData(packet);

			if(msg.htmlBody().size() > 0) {
				QList<KittySDK::IContact*> contacts;

				contacts << me();
				for(int i = 1; i < msg.uins().count(); ++i) {
					contacts.append(contactByUin(msg.uins().at(i), true));
				}

				//remove bots
				QString htmlBody = msg.htmlBody();
				htmlBody.remove(QRegExp("<bot[^>]*/>"));

				KittySDK::IMessage message(contactByUin(msg.uins().first(), true), contacts);
				message.setDirection(KittySDK::IMessage::Incoming);
				message.setBody(htmlBody);
				message.setTimeStamp(msg.timeStamp());
				emit messageReceived(message);
			}

			//process image download
			KittyGG::ImageDownloadInfo *imgDown = msg.imageDownload();
			if(imgDown) {
				if(imgDown->type == 0x05) {
					KittyGG::DownloadMgr::append(new KittyGG::ImageDownload(imgDown->size, imgDown->crc32, imgDown->fileName));
				}

				KittyGG::ImageDownload *img = KittyGG::DownloadMgr::byCrc32(imgDown->crc32);
				if(img) {
					img->data.append(imgDown->data);
					if((quint32)img->data.size() == img->size) {
						QDir imgDir(protocol()->core()->profilesDir() + protocol()->core()->profileName() + "/imgcache/");

						//create dir if it doesn't exist
						imgDir.mkpath(".");

						QFile file(imgDir.absolutePath() + "/" + img->fileName);
						if(file.open(QFile::WriteOnly)) {
							file.write(img->data);
							file.close();

							KittySDK::IMessage message(contactByUin(msg.uin()), me());
							message.setDirection(KittySDK::IMessage::Incoming);
							message.setBody(QString("<img src=\"file:/%1%2\" alt=\"%2\" title=\"%2\">").arg(imgDir.absolutePath() + "/").arg(Qt::escape(img->fileName)));
							emit messageReceived(message);
						}

						KittyGG::DownloadMgr::remove(img);
						delete img;
					}
				} else {
					qWarning() << "Image not in download list!" << imgDown->crc32;
				}
			}

			//respond to image request
			KittyGG::ImageDetails *imgUpl = msg.imageUpload();
			if(imgUpl) {
				KittyGG::ImageUpload *img = KittyGG::UploadMgr::byCrc32(imgUpl->crc32);
				if(img) {
					sendImage(msg.uin(), img);
				} else {
					qWarning() << "Image not in upload list!" << imgUpl->crc32;
				}
			}

			//request all the images!
			foreach(KittyGG::ImageDetails *imgReq, msg.imageRequests()) {
				KittyGG::MessageSend req;
				req.setUin(msg.uin());
				req.addImageRequest(new KittyGG::ImageDetails(imgReq->size, imgReq->crc32));
				sendPacket(req);
			}
		}
		break;

		case KittyGG::MessageAck::Type:
		{
			KittyGG::MessageAck ack = KittyGG::MessageAck::fromData(packet);
			//qDebug() << "ACK" << ack.recipient() << ack.seq() << ack.status();

			if(ack.status() == KittyGG::MessageAck::Delivered) {
				foreach(AckItem *item, m_ackList) {
					if(item->seq == ack.seq()) {
						m_ackList.removeOne(item);
						delete item;
						break;
					}
				}
			} else {
				qDebug() << "Message sent to" << ack.recipient() << "with seq" << ack.seq() << "has status" << ack.status();
			}
		}
		break;

		case KittyGG::TypingNotify::Type:
		{
			KittyGG::TypingNotify notify = KittyGG::TypingNotify::fromData(packet);

			if(protocol()->core()->contact(protocol()->protoInfo()->protoName(), uid(), QString::number(notify.uin()))) {
				emit typingNotifyReceived(contactByUin(notify.uin()), notify.type() > 0, notify.type());
			}
		}
		break;

		case KittyGG::UserData::Type:
		{
			KittyGG::UserData data = KittyGG::UserData::fromData(packet);

			QMapIterator<quint32, QList<KittyGG::UserDataAttribute> > it(data.data());
			while(it.hasNext()) {
				it.next();

				foreach(const KittyGG::UserDataAttribute &attr, it.value()) {
					QString uid = QString::number(it.key());

					if(m_contacts.contains(uid)) {
						//qDebug() << uid << attr.name << attr.value;
						dynamic_cast<Contact*>(m_contacts.value(uid))->setData(attr.name, attr.value);
					}
				}
			}
		}
		break;

		case KittyGG::Disconnecting::Type:
		{
			//qDebug() << "It's Disconnecting";
		}
		break;

		case KittyGG::DisconnectAck::Type:
		{
			//qDebug() << "It's DisconnectAck";
			m_socket->disconnectFromHost();
		}
		break;

		case KittyGG::ListReply::Type:
		{
			KittyGG::ListReply reply = KittyGG::ListReply::fromData(packet);
			parseXMLRoster(reply.reply());
		}
		break;

		case KittyGG::MultiLogin::Type:
		{
			KittyGG::MultiLogin logon = KittyGG::MultiLogin::fromData(packet);
			foreach(KittyGG::MultiLoginItem *item, logon.items()) {
				QString notifyText = "<b>" + tr("Multilogin") + "</b><br>";
				notifyText += tr("IP") + ": " + item->ip + "<br>";
				notifyText += tr("Login time") + ": " + item->time.toString() + "<br>";
				notifyText += tr("Client") + ": " + item->client;

				QMap<QString, QVariant> notifyArgs;
				notifyArgs.insert("icon", protocol()->core()->icon(KittySDK::Icons::I_CONNECT));
				notifyArgs.insert("text", notifyText);
				protocol()->core()->execPluginAction("notify", "addNotify", notifyArgs);
			}
		}
		break;

		default:
			qDebug() << "Unknown type" << type << "length" << length;
		break;
	}
}

void Account::sendPacket(const KittyGG::Packet &packet)
{
	QByteArray data;
	KittyGG::DataStream str(&data);
	str << packet.packetType();
	str << packet.size();
	str << packet.toData();

	qint64 res = m_socket->write(data);
	if(res != data.size()) {
		qDebug() << "Didn't send whole packet!";
	}
}

void Account::sendImage(const quint32 &recipient, KittyGG::ImageUpload *image)
{
	//qDebug() << "gonna send" << image->filePath + "/" + image->fileName << "to" << recipient << image->size << image->crc32;

	QFile file(image->filePath + "/" + image->fileName);
	if(file.open(QFile::ReadOnly)) {
		QByteArray buffer = file.readAll();
		QByteArray fileName = image->fileName.toLocal8Bit();
		quint8 flag = 0x05;

		while(buffer.size()) {
			KittyGG::MessageSend msg;
			msg.setUin(recipient);
			msg.setImageDownload(new KittyGG::ImageDownloadInfo(flag, image->size, image->crc32, buffer.mid(0, 1905), fileName));
			sendPacket(msg);

			flag = 0x06;
			buffer = buffer.mid(1905);
		}

		//qDebug() << "image sent";

		file.close();
	}
}

void Account::sendChangeStatusPacket()
{
	KittyGG::NewStatus packet(m_status | 0x4000, m_description);
	sendPacket(packet);
}

void Account::sendPingPacket()
{
	if(isConnected()) {
		KittyGG::Ping ping;
		sendPacket(ping);
	}
}

void Account::startHubLookup()
{
	m_blinkTimer.start(protocol()->core()->setting(KittySDK::Settings::S_BLINKING_SPEED, 500).toInt());

	//let's look for a server
	KittyGG::HUBLookup *lookup = new KittyGG::HUBLookup();
	connect(lookup, SIGNAL(serverFound(QString)), SLOT(connectToHost(QString)));
	QThreadPool::globalInstance()->start(lookup);
}

void Account::readSocket()
{
	m_parser->append(m_socket->readAll());
	QThreadPool::globalInstance()->start(m_parser);
}

void Account::disconnected()
{
	qDebug() << "Socket::Disconnected";

	m_pingTimer.stop();

	if(m_socket->error() != QAbstractSocket::RemoteHostClosedError) {
		m_status = KittyGG::Status::Unavailable;
	}

	changeContactStatus(uin(), KittyGG::Status::Unavailable, m_description);

	//all contacts go bye bye
	foreach(KittySDK::IContact *cnt, m_contacts) {
		dynamic_cast<Contact*>(cnt)->changeStatus(KittyGG::Status::Unavailable, "");
	}

	if(m_socket->error() == QAbstractSocket::RemoteHostClosedError) {
		if(protocol()->core()->setting(KittySDK::Settings::S_RECONNECT, true).toBool()) {
			startHubLookup();
		}
	}
}

void Account::error(QAbstractSocket::SocketError socketError)
{
	qDebug() << "Socket::error(" << socketError << ")" << m_socket->errorString();

	m_status = KittyGG::Status::Unavailable;
	changeContactStatus(uin(), m_status, m_description);

	m_pingTimer.stop();
}

void Account::proxyAuthenticationRequired(const QNetworkProxy &proxy, QAuthenticator *authenticator)
{
	qDebug() << "Socket::proxyAuthReq";
}

void Account::stateChanged(QAbstractSocket::SocketState socketState)
{
	qDebug() << "Socket::stateChanged(" << socketState << ")";
}

void Account::connectToHost(const QString &hostname)
{
	//qDebug() << "CONNECT TO" << hostname;
	if(!hostname.isEmpty()) {
		if(m_useSSL) {
			m_socket->connectToHostEncrypted(hostname, 443);
		} else {
			m_socket->connectToHost(hostname, 8074);
		}
	} else {
		//TODO: iterate over server list
	}
}

void Account::updateAvatars()
{
	foreach(KittySDK::IContact *cnt, m_contacts) {
		if(Contact *contact = qobject_cast<Contact*>(cnt)) {
			contact->updateAvatar();
		}
	}
}

void Account::checkMsgAcks()
{
	foreach(AckItem *ack, m_ackList) {
		ack->timer++;

		if(ack->timer > 10) {
			QString body = ack->msg;
			if(body.length() > 15) {
				body = body.left(15) + "...";
			}

			KittySDK::IMessage msg(ack->contact, me());
			msg.setBody(tr("Message may not have been sent") + ":" + body);
			msg.setDirection(KittySDK::IMessage::System);

			emit messageReceived(msg);

			m_ackList.removeOne(ack);
			delete ack;
		}
	}
}

}
