#ifndef GGCLIENT_H
#define GGCLIENT_H

#include "KittyGG/Packets/NotifyFirst.h"

#include <QtCore/QtGlobal>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QThread>
#include <QtCore/QTimer>
#include <QtCore/QMutex>
#include <QtNetwork/QSslSocket>

namespace KittyGG
{
	struct ImageUpload;
	class Parser;
}

namespace KittySDK
{
	class GGClient: public QObject
	{
		Q_OBJECT
		Q_PROPERTY(quint32 uin READ uin WRITE setUin)
		Q_PROPERTY(QString password READ password WRITE setPassword)
		Q_PROPERTY(quint32 status READ status WRITE setStatus)
		Q_PROPERTY(QString description READ description WRITE setDescription)

		public:
			explicit GGClient(QObject *parent = 0);
			~GGClient();

			quint32 uin() const { return m_uin; }
			void setUin(const quint32 &uin) { m_uin = uin; }

			QString password() const { return m_password; }
			void setPassword(const QString &password) { m_password = password; }

			quint32 status() const { return m_status; }
			void setStatus(const quint32 &status);

			QString description() const { return m_description; }
			void setDescription(const QString &description);

			void setAccount(const quint32 &uin, const QString &passsword);
			bool isConnected();

			void addContact(const quint32 &uin);
			void removeContact(const quint32 &uin);

		public slots:
			void sendMessage(const quint32 &recipient, const QString &text);
			void sendTypingNotify(const quint32 &recipient, const quint16 &length);
			void sendImage(const quint32 &recipient, KittyGG::ImageUpload *image);
			void changeStatus(const quint32 &status, const QString &description);
			void requestRoster();
			void parseXMLRoster(const QString &xml);

		signals:
			void statusChanged(const quint32 &uin, const quint32 &status, const QString &description);
			void messageReceived(QList<quint32> senders, const QDateTime &time, const QString &plain);
			void userDataReceived(const quint32 &uin, const QString &name, const QString &data);
			void xmlActionReceived(const QString &xmlAction);
			void contactImported(const quint32 &uin, const QMap<QString, QString> &data);
			void imageReceived(const quint32 &sender, const QString &fileName, const quint32 &crc32, const QByteArray &data);
			void typingNotifyReceived(const quint32 &sender, const int &length);

		private slots:
			void readSocket();
			void disconnected();
			void error(QAbstractSocket::SocketError socketError);
			void proxyAuthenticationRequired(const QNetworkProxy &proxy, QAuthenticator *authenticator);
			void stateChanged(QAbstractSocket::SocketState socketState);
			void connectToHost(const QString &hostname);
			void processPacket(const quint32 &type, const quint32 &length, QByteArray packet);
			void sendChangeStatusPacket();
			void sendPingPacket();
			void sendPacket(const KittyGG::Packet &packet);

		private:
			quint32 m_uin;
			QString m_password;
			quint32 m_status;
			QString m_description;
			quint32 m_initialStatus;
			QString m_initialDescription;
			QTimer m_pingTimer;
			QSslSocket *m_socket;
			QList<KittyGG::NotifyEntry> m_roster;
			KittyGG::Parser *m_parser;
	};
}

#endif // GGCLIENT_H
