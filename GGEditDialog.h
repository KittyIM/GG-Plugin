#ifndef GGEDITWINDOW_H
#define GGEDITWINDOW_H

#include <QtGui/QDialog>

namespace KittySDK
{
	class IProtocol;
}

namespace GG
{
	namespace Ui
	{
		class EditDialog;
	}

	class Account;

	class EditDialog: public QDialog
	{
		Q_OBJECT

		public:
			EditDialog(KittySDK::IProtocol *proto, QWidget *parent = 0);
			~EditDialog();

			void reset();
			void setup(Account *account);

		private slots:
			void on_buttonBox_accepted();

		protected:
			void changeEvent(QEvent *event);

		private:
			Ui::EditDialog *m_ui;
			KittySDK::IProtocol *m_protocol;
			Account *m_account;
	};
}

#endif // GGEDITWINDOW_H
