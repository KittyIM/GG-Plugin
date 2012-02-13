#include "GGEditDialog.h"
#include "ui_GGEditDialog.h"

#include "SDK/GGConstants.h"
#include "SDK/constants.h"
#include "SDK/Protocol.h"
#include "GGAccount.h"

#include <QtGui/QMessageBox>

#define qDebug() qDebug() << "[GGEditDialog]"
#define qWarning() qWarning() << "[GGEditDialog]"

namespace KittySDK
{

GGEditDialog::GGEditDialog(Protocol *proto, QWidget *parent): QDialog(parent), m_ui(new Ui::GGEditDialog), m_protocol(proto)
{
	m_ui->setupUi(this);

	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
}

GGEditDialog::~GGEditDialog()
{
	delete m_ui;
}

void GGEditDialog::reset()
{
	m_ui->uinEdit->clear();
	m_ui->uinEdit->setEnabled(true);
	m_ui->passwordEdit->clear();
	m_ui->SSLCheckBox->setChecked(true);
	m_ui->friendsCheckBox->setChecked(false);
	m_ui->serversTextEdit->clear();
	m_ui->startupComboBox->setCurrentIndex(0);
	m_ui->startupComboBox->setItemIcon(0, m_protocol->core()->icon(KittySDK::Icons::I_REFRESH));
	m_ui->startupComboBox->setItemIcon(1, m_protocol->core()->icon(KittySDK::Icons::I_GG_AVAILABLE));
	m_ui->startupComboBox->setItemIcon(2, m_protocol->core()->icon(KittySDK::Icons::I_GG_AWAY));
	m_ui->startupComboBox->setItemIcon(3, m_protocol->core()->icon(KittySDK::Icons::I_GG_DND));
	m_ui->startupComboBox->setItemIcon(4, m_protocol->core()->icon(KittySDK::Icons::I_GG_FFC));
	m_ui->startupComboBox->setItemIcon(5, m_protocol->core()->icon(KittySDK::Icons::I_GG_INVISIBLE));
}

void GGEditDialog::setup(GGAccount *account)
{
	reset();

	if(!account) {
		m_account = new GGAccount("", dynamic_cast<GGProtocol*>(m_protocol));
	} else {
		m_account = account;

		m_ui->uinEdit->setText(m_account->uid());
		m_ui->uinEdit->setEnabled(false);
		m_ui->passwordEdit->setText(m_account->password());
		m_ui->SSLCheckBox->setChecked(m_account->useSSL());
		m_ui->friendsCheckBox->setChecked(m_account->friendsOnly());
		m_ui->serversTextEdit->setPlainText(m_account->serverList().join("\n"));

		switch(m_account->initialStatus()) {
			case 0:
				m_ui->startupComboBox->setCurrentIndex(0);
			break;

			case Protocol::Online:
				m_ui->startupComboBox->setCurrentIndex(1);
			break;

			case Protocol::Away:
				m_ui->startupComboBox->setCurrentIndex(2);
			break;

			case Protocol::DND:
				m_ui->startupComboBox->setCurrentIndex(3);
			break;

			case Protocol::FFC:
				m_ui->startupComboBox->setCurrentIndex(4);
			break;

			case Protocol::Invisible:
				m_ui->startupComboBox->setCurrentIndex(5);
			break;
		}
	}
}

void GGEditDialog::on_buttonBox_accepted()
{
	if(m_ui->uinEdit->text().toUInt() == 0) {
		QMessageBox::information(this, tr("Missing info"), tr("You have to enter UID"));
		return;
	}

	if(m_ui->passwordEdit->text().isEmpty()) {
		QMessageBox::information(this, tr("Missing info"), tr("You have to enter password"));
		return;
	}

	m_account->setPassword(m_ui->passwordEdit->text());
	m_account->setUseSSL(m_ui->SSLCheckBox->isChecked());
	m_account->setFriendsOnly(m_ui->friendsCheckBox->isChecked());
	m_account->setServerList(m_ui->serversTextEdit->toPlainText().split('\n'));

	switch(m_ui->startupComboBox->currentIndex()) {
		case 0:
			m_account->setInitialStatus(0);
		break;

		case 1:
			m_account->setInitialStatus(Protocol::Online);
		break;

		case 2:
			m_account->setInitialStatus(Protocol::Away);
		break;

		case 3:
			m_account->setInitialStatus(Protocol::DND);
		break;

		case 4:
			m_account->setInitialStatus(Protocol::FFC);
		break;

		case 5:
			m_account->setInitialStatus(Protocol::Invisible);
		break;
	}

	if(m_ui->uinEdit->isEnabled()) {
		m_account->setUid(m_ui->uinEdit->text());
		m_protocol->core()->addAccount(m_account);
	}

	close();
}

}
