#ifndef GGPROTOCOL_H
#define GGPROTOCOL_H

#include <IProtocol.h>

namespace GG
{
	class EditDialog;
	class Account;

	class Protocol: public KittySDK::IProtocol
	{
		Q_OBJECT

		public:
			Protocol(KittySDK::IPluginCore *core);
			~Protocol();

			void init();
			void load();
			void unload();

			QString statusIcon(KittySDK::IProtocol::Status status);
			KittySDK::IAccount *newAccount(const QString &uid);
			QDialog *editDialog(KittySDK::IAccount *account = 0);

			Status convertStatus(const quint32 &status) const;

		public slots:
			void execAction(const QString &name, const QMap<QString, QVariant> &args);

		private:
			EditDialog *m_editWindow;
			QList<Account*> m_accounts;
	};
}

#endif // GGPROTOCOL_H
