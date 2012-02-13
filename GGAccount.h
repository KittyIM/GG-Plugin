#ifndef GGACCOUNT_H
#define GGACCOUNT_H

#include "KittyGG/Packets/NotifyFirst.h"
#include "SDK/Account.h"
#include "GGProtocol.h"

#include <QtCore/QStringList>
#include <QtCore/QTimer>
#include <QtNetwork/QSslSocket>

class QSignalMapper;

namespace KittyGG
{
	struct ImageUpload;
	class Parser;
}

namespace KittySDK
{
	class Protocol;
	class GGClient;
	class Contact;

	class GGAccount: public Account
	{
		Q_OBJECT

		public:
			GGAccount(const QString &uid, GGProtocol *parent);
			~GGAccount();

			quint32 uin() const;
			Protocol::Status status() const;
			QString description() const;

			Contact *newContact(const QString &uid);
			Contact *newContact(const quint32 &uin);

			Contact *contactByUin(const quint32 &uin);
			void insertContact(const QString &uid, Contact *contact);

			bool isConnected() const;

			void setUseSSL(const bool &useSSL) { m_useSSL = useSSL; }
			bool useSSL() const { return m_useSSL; }

			void setFriendsOnly(const bool &friendsOnly) { m_friendsOnly = friendsOnly; }
			bool friendsOnly() const { return m_friendsOnly; }

			void setInitialStatus(const quint32 &initialStatus) { m_initialStatus = initialStatus; }
			quint32 initialStatus() const { return m_initialStatus; }

			void setServerList(const QStringList &serverList) { m_serverList = serverList; }
			QStringList serverList() const { return m_serverList; }

		public slots:
			void loadSettings(const QMap<QString, QVariant> &settings);
			QMap<QString, QVariant> saveSettings();
			void changeStatus(const KittySDK::Protocol::Status &status, const QString &description);
			QMenu *statusMenu();
			void sendMessage(const Message &msg);
			void sendTypingNotify(KittySDK::Contact *contact, bool typing, const int &length);

		private slots:
			void changeContactStatus(const quint32 &uin, const quint32 &status, const QString &description);
			void toggleConnectingStatus();
			void showDescriptionInput();
			void setStatus(int status);
			void importFromServer();
			void importFromFile();
			void parseXMLRoster(const QString &xml);
			void processPacket(const quint32 &type, const quint32 &length, QByteArray packet);
			void sendPacket(const KittyGG::Packet &packet);
			void sendImage(const quint32 &recipient, KittyGG::ImageUpload *image);
			void sendChangeStatusPacket();
			void sendPingPacket();
			void readSocket();
			void disconnected();
			void error(QAbstractSocket::SocketError socketError);
			void proxyAuthenticationRequired(const QNetworkProxy &proxy, QAuthenticator *authenticator);
			void stateChanged(QAbstractSocket::SocketState socketState);
			void connectToHost(const QString &hostname);

		private:
			QMenu *m_statusMenu;
			QStringList m_descriptionHistory;
			bool m_useSSL;
			bool m_friendsOnly;
			quint32 m_initialStatus;
			QStringList m_serverList;
			QTimer m_blinkTimer;
			QSignalMapper *m_statusMapper;
			QAction *m_availableAction;
			QAction *m_awayAction;
			QAction *m_ffcAction;
			QAction *m_dndAction;
			QAction *m_invisibleAction;
			QAction *m_unavailableAction;
			QAction *m_descriptionAction;

			quint32 m_status;
			QString m_description;
			QString m_initialDescription;
			QTimer m_pingTimer;
			QSslSocket *m_socket;
			KittyGG::Parser *m_parser;
	};
}

#endif // GGACCOUNT_H
