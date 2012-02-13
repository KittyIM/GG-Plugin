#ifndef GGEDITWINDOW_H
#define GGEDITWINDOW_H

#include <QtGui/QDialog>

namespace KittySDK
{
	class GGAccount;
	class Protocol;

	namespace Ui
	{
		class GGEditDialog;
	}

	class GGEditDialog: public QDialog
	{
		Q_OBJECT

		public:
			GGEditDialog(Protocol *proto, QWidget *parent = 0);
			~GGEditDialog();

			void reset();
			void setup(GGAccount *account);

		private slots:
			void on_buttonBox_accepted();

		private:
			Ui::GGEditDialog *m_ui;
			Protocol *m_protocol;
			GGAccount *m_account;
	};
}

#endif // GGEDITWINDOW_H
