#ifndef GGACCOUNT_H
#define GGACCOUNT_H

#include "KittyGG/Packets/NotifyFirst.h"
#include "GGProtocol.h"

#include <IAccount.h>

#include <QtCore/QStringList>
#include <QtCore/QTimer>
#include <QtNetwork/QSslSocket>

class QSignalMapper;

namespace KittyGG
{
	struct ImageUpload;
	class Parser;
}

namespace GG
{
	class Account: public KittySDK::IAccount
	{
		Q_OBJECT

		public:
			Account(const QString &uid, Protocol *parent);
			~Account();

			quint32 uin() const;
			KittySDK::IProtocol::Status status() const;
			QString description() const;

			KittySDK::IContact *newContact(const QString &uid);
			KittySDK::IContact *newContact(const quint32 &uin);

			KittySDK::IContact *contactByUin(const quint32 &uin, bool temp = false);
			void insertContact(const QString &uid, KittySDK::IContact *contact);

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
			void changeStatus(const KittySDK::IProtocol::Status &status, const QString &description);
			QMenu *statusMenu();
			void sendMessage(const KittySDK::IMessage &msg);
			void sendTypingNotify(KittySDK::IContact *contact, bool typing, const int &length);
			void retranslate();

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
			void updateAvatars();

		private:
			quint32 m_status;
			QString m_description;
			QSslSocket *m_socket;
			KittyGG::Parser *m_parser;
			bool m_useSSL;
			bool m_friendsOnly;
			quint32 m_initialStatus;
			QStringList m_serverList;
			QStringList m_descriptionHistory;
			QTimer m_pingTimer;
			QTimer m_blinkTimer;
			QSignalMapper *m_statusMapper;
			QMenu *m_statusMenu;
			QAction *m_availableAction;
			QAction *m_awayAction;
			QAction *m_ffcAction;
			QAction *m_dndAction;
			QAction *m_invisibleAction;
			QAction *m_unavailableAction;
			QAction *m_descriptionAction;
			QAction *m_avatarAction;
	};
}

#endif // GGACCOUNT_H
