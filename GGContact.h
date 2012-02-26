#ifndef GGCONTACT_H
#define GGCONTACT_H

#include "GGAccount.h"

#include <IContact.h>

#include <QtNetwork/QNetworkAccessManager>

namespace GG
{
	class Contact: public KittySDK::IContact
	{
		Q_OBJECT

		public:
			Contact(const QString &uid, Account *account);
			~Contact();

			quint32 uin() const;

			void prepareContextMenu(QMenu *menu);
			void changeStatus(const quint32 &status, const QString &description, bool silent = false);
			void setData(const QString &name, const QVariant &data);

		public slots:
			void loadSettings(const QMap<QString, QVariant> &settings);
			QMap<QString, QVariant> saveSettings();

		private slots:
			void updateAvatar();
			void processReply(QNetworkReply *reply);

		private:
			QNetworkAccessManager m_netManager;
	};
}

#endif // GGCONTACT_H
