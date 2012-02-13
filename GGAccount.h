#ifndef GGACCOUNT_H
#define GGACCOUNT_H

#include "SDK/Account.h"
#include "GGProtocol.h"

#include <QtCore/QStringList>

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
			void processUserData(const quint32 &uin, const QString &name, const QString &data);
			void processMessage(QList<quint32> senders, const QDateTime &time, const QString &plain);
			void processImage(const quint32 &sender, const QString &imgName, const quint32 &crc32, const QByteArray &data);
			void importContact(const quint32 &uin, const QMap<QString, QString> &data);
			void processTypingNotify(const quint32 &sender, const int &type);
			void showDescriptionInput();
			void setStatusAvailable();
			void setStatusAway();
			void setStatusFFC();
			void setStatusDND();
			void setStatusInvisible();
			void setStatusUnavailable();
			void importFromServer();
			void importFromFile();

		private:
			GGClient *m_client;
			QMenu *m_statusMenu;
			QStringList m_descriptionHistory;
			bool m_useSSL;
			bool m_friendsOnly;
			quint32 m_initialStatus;
			QStringList m_serverList;
			QAction *m_availableAction;
			QAction *m_awayAction;
			QAction *m_ffcAction;
			QAction *m_dndAction;
			QAction *m_invisibleAction;
			QAction *m_unavailableAction;
			QAction *m_descriptionAction;
	};
}

#endif // GGACCOUNT_H
