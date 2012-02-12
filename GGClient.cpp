#include "GGClient.h"

#include "KittyGG/DataStream.h"
#include "KittyGG/Managers.h"
#include "KittyGG/KittyGG.h"
#include "KittyGG/Parser.h"
#include "constants.h"
#include "zlib/zlib.h"

#include <QtCore/QThreadPool>
#include <QtCore/QDebug>
#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtGui/QTextDocument>
#include <QtGui/QTextBlock>
#include <QtNetwork/QNetworkProxy>
#include <QtXml/QDomDocument>
#include <QtXml/QDomElement>

#define qDebug() qDebug() << "[GGClient]"
#define qWarning() qWarning() << "[GGClient]"

using namespace KittySDK;

KittySDK::GGClient::GGClient(QObject *parent): QObject(parent)
{
	m_socket = new QSslSocket(this);
	m_socket->setProxy(QNetworkProxy::applicationProxy());

	connect(m_socket, SIGNAL(readyRead()), this, SLOT(readSocket()));
	connect(m_socket, SIGNAL(connected()), this, SLOT(connected()));
	connect(m_socket, SIGNAL(disconnected()), this, SLOT(disconnected()));
	connect(m_socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(error(QAbstractSocket::SocketError)));
	connect(m_socket, SIGNAL(hostFound()), this, SLOT(hostFound()));
	connect(m_socket, SIGNAL(proxyAuthenticationRequired(QNetworkProxy,QAuthenticator*)), this, SLOT(proxyAuthenticationRequired(QNetworkProxy,QAuthenticator*)));
	connect(m_socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(stateChanged(QAbstractSocket::SocketState)));

	connect(&m_pingTimer, SIGNAL(timeout()), this, SLOT(sendPingPacket()));

	m_status = KittyGG::Status::Unavailable;

	m_initialStatus = KittyGG::Status::Available;

	m_pingTimer.setInterval(3 * 60 * 1000);
	m_parser = new KittyGG::Parser();
	connect(m_parser, SIGNAL(packetReceived(quint32,quint32,QByteArray)), SLOT(processPacket(quint32,quint32,QByteArray)));
}

KittySDK::GGClient::~GGClient()
{
	delete m_socket;
}

void KittySDK::GGClient::setStatus(const quint32 &status)
{
	if(!isConnected()) {
		m_initialStatus = status;

		connectToHost("ggproxy-26.gadu-gadu.pl", 443);
	} else {
		m_status = status;
		sendChangeStatusPacket();
	}
}

void KittySDK::GGClient::setDescription(const QString &description)
{
	if(!isConnected()) {
		m_initialDescription = description;
	} else {
		m_description = description;
		sendChangeStatusPacket();
	}
}

void KittySDK::GGClient::setAccount(const quint32 &uin, const QString &passsword)
{
	setUin(uin);
	setPassword(passsword);
}

bool KittySDK::GGClient::isConnected()
{
	return (m_socket->state() == QAbstractSocket::ConnectedState);
}

void KittySDK::GGClient::addContact(const quint32 &uin)
{
	bool contains = false;
	foreach(const KittyGG::NotifyEntry &entry, m_roster) {
		if(entry.uin == uin) {
			contains = true;
			break;
		}
	}

	if(!contains) {
		m_roster << KittyGG::NotifyEntry(uin);
	}

	if(isConnected()) {
		KittyGG::NotifyAdd packet(uin);
		sendPacket(packet);
	}
}

void KittySDK::GGClient::removeContact(const quint32 &uin)
{
	for(int i = m_roster.size(); i >= 0; --i) {
		if(m_roster[i].uin == uin) {
			m_roster.removeAt(i);
		}
	}

	if(isConnected()) {
		KittyGG::NotifyRemove packet(uin);
		sendPacket(packet);
	}
}

void KittySDK::GGClient::connectToHost(const QString &host, const int &port)
{
	m_host = host;
	m_port = port;

	if(port == 443) {
		m_socket->connectToHostEncrypted(m_host, m_port);
	} else {
		m_socket->connectToHost(m_host, m_port);
	}
}

void KittySDK::GGClient::sendMessage(const quint32 &recipient, const QString &text)
{
	KittyGG::MessageSend msg;
	msg.setHtmlBody(text);
	msg.setUin(recipient);
	sendPacket(msg);
}

void GGClient::sendTypingNotify(const quint32 &recipient, const quint16 &length)
{
	KittyGG::TypingNotify notify(length, recipient);
	sendPacket(notify);
}

void GGClient::sendImage(const quint32 &recipient, KittyGG::ImageUpload *image)
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

		qDebug() << "image sent";

		file.close();
	}
}

void KittySDK::GGClient::changeStatus(const quint32 &status, const QString &description)
{
	if(isConnected()) {
		m_status = status;
		m_description = description;

		sendChangeStatusPacket();
	} else {
		setDescription(description);
		setStatus(status);
	}
}

void KittySDK::GGClient::requestRoster()
{
	KittyGG::ListRequest packet(KittyGG::ListRequest::Get, 0);
	sendPacket(packet);
}

void KittySDK::GGClient::readSocket()
{
	m_parser->append(m_socket->readAll());
	QThreadPool::globalInstance()->start(m_parser);
}

void KittySDK::GGClient::connected()
{
	qDebug() << "Socket::Connected";
}

void KittySDK::GGClient::disconnected()
{
	qDebug() << "Socket::Disconnected";

	m_status = KittyGG::Status::Unavailable;
	emit statusChanged(uin(), m_status, m_description);

	m_pingTimer.stop();
}

void KittySDK::GGClient::error(QAbstractSocket::SocketError socketError)
{
	qDebug() << "Socket::error(" << socketError << ")" << m_socket->errorString();
}

void KittySDK::GGClient::hostFound()
{
	qDebug() << "Socket::hostNotFound";
	qDebug() << m_socket->errorString();
}

void KittySDK::GGClient::proxyAuthenticationRequired(const QNetworkProxy &proxy, QAuthenticator *authenticator)
{
	qDebug() << "Socket::proxyAuthReq";
}

void KittySDK::GGClient::stateChanged(QAbstractSocket::SocketState socketState)
{
	qDebug() << "Socket::stateChanged(" << socketState << ")";
}

void KittySDK::GGClient::processPacket(const quint32 &type, const quint32 &length, QByteArray packet)
{
	switch(type) {
		case KittyGG::Welcome::Type:
		{
			KittyGG::Welcome welcome = KittyGG::Welcome::fromData(packet);

			KittyGG::Login login(m_uin, m_password, welcome.seed());
			login.setInitialStatus(m_initialStatus | 0x4000);
			login.setInitialDescription(m_initialDescription);
			sendPacket(login);
		}
		break;

		case KittyGG::LoginOk::Type:
		{
			m_status = m_initialStatus;
			m_description = m_initialDescription;

			m_pingTimer.start();

			//send roster
			if(m_roster.empty()) {
				KittyGG::ListEmpty packet;
				sendPacket(packet);
			} else {
				int count = m_roster.size();
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

					packet->setContacts(m_roster.mid(m_roster.size() - count, part_count));

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

				if(status.uin() == m_uin) {
					m_status = status.status();
					m_description = status.description();
				}

				packet = packet.mid(status.size());
				left -= status.size();

				emit statusChanged(status.uin(), status.status(), status.description());
			}

			if(left > 0) {
				qDebug() << "left" << left;
			}
		}
		break;

		case KittyGG::XmlAction::Type:
		{
			KittyGG::XmlAction data = KittyGG::XmlAction::fromData(packet);
			emit xmlActionReceived(data.action());
		}
		break;

		case KittyGG::MessageRecv::Type:
		{
			KittyGG::MessageRecv msg = KittyGG::MessageRecv::fromData(packet);

			if(msg.htmlBody().size() > 0) {
				emit messageReceived(msg.uins(), msg.time(), msg.htmlBody());
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
						emit imageReceived(msg.uin(), img->fileName, img->crc32, img->data);

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

			if(ack.status() != KittyGG::MessageAck::Delivered) {
				qDebug() << "Message sent to" << ack.recipient() << "with seq" << ack.seq() << "has status" << ack.status();
			}
		}
		break;

		case KittyGG::TypingNotify::Type:
		{
			KittyGG::TypingNotify notify = KittyGG::TypingNotify::fromData(packet);

			emit typingNotifyReceived(notify.uin(), notify.type());
		}
		break;

		case KittyGG::UserData::Type:
		{
			KittyGG::UserData data = KittyGG::UserData::fromData(packet);

			QMapIterator<quint32, QList<KittyGG::UserDataAttribute> > it(data.data());
			while(it.hasNext()) {
				it.next();

				foreach(const KittyGG::UserDataAttribute &attr, it.value()) {
					emit userDataReceived(it.key(), attr.name, attr.value);
				}
			}
		}
		break;

		case KittyGG::Disconnecting::Type:
		{
			//qDebug() << "It's P_DISCONNECTING";
		}
		break;

		case KittyGG::DisconnectAck::Type:
		{
			m_socket->disconnectFromHost();
		}
		break;

		case KittyGG::ListReply::Type:
		{
			KittyGG::ListReply reply = KittyGG::ListReply::fromData(packet);
			parseXMLRoster(reply.reply());
		}
		break;

		default:
			qDebug() << "Unknown type" << type;
		break;
	}
}

void KittySDK::GGClient::sendChangeStatusPacket()
{
	KittyGG::NewStatus packet(m_status | 0x4000, m_description);
	sendPacket(packet);
}

void KittySDK::GGClient::sendPingPacket()
{
	if(isConnected()) {
		KittyGG::Ping ping;
		sendPacket(ping);
	}
}

void GGClient::sendPacket(const KittyGG::Packet &packet)
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

void KittySDK::GGClient::parseXMLRoster(const QString &xml)
{
	QDomDocument doc;
	doc.setContent(xml);

	QDomElement root = doc.documentElement();
	if(root.nodeName() == "ContactBook") {
		QMap <QString, QString> groups;

		QDomElement groupsElement = root.namedItem("Groups").toElement();
		QDomNodeList groupList = groupsElement.elementsByTagName("Group");
		for(int i = 0; i < groupList.count(); i++) {
			QDomElement group = groupList.at(i).toElement();

			QDomElement id = group.namedItem("Id").toElement();
			QDomElement name = group.namedItem("Name").toElement();

			groups.insert(id.firstChild().nodeValue(), name.firstChild().nodeValue());
		}

		QDomElement contactsElement = root.namedItem("Contacts").toElement();
		QDomNodeList contactList = contactsElement.elementsByTagName("Contact");
		for(int i = 0; i < contactList.count(); i++) {
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

			emit contactImported(uin, data);
		}
	} else {
		qWarning() << "Wrong format!";
	}
}


