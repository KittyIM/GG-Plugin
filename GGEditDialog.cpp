#include "GGEditDialog.h"
#include "ui_GGEditDialog.h"

#include "KittyGG/Packets/Status.h"
#include "GGAccount.h"

#include <SDKConstants.h>
#include <GGConstants.h>
#include <IProtocol.h>

#include <QtGui/QMessageBox>

#define qDebug() qDebug() << "[GGEditDialog]"
#define qWarning() qWarning() << "[GGEditDialog]"

namespace GG
{

EditDialog::EditDialog(KittySDK::IProtocol *proto, QWidget *parent):
	QDialog(parent),
	m_ui(new Ui::EditDialog),
	m_protocol(proto)
{
	m_ui->setupUi(this);

	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
}

EditDialog::~EditDialog()
{
	delete m_ui;
}

void EditDialog::reset()
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
	m_ui->startupComboBox->setItemIcon(6, m_protocol->core()->icon(KittySDK::Icons::I_GG_UNAVAILABLE));
}

void EditDialog::setup(Account *account)
{
	reset();

	if(!account) {
		m_account = new Account("", dynamic_cast<Protocol*>(m_protocol));
		setWindowTitle(tr("Add account"));
	} else {
		m_account = account;
		setWindowTitle(tr("Edit account"));

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

			case KittyGG::Status::Available:
				m_ui->startupComboBox->setCurrentIndex(1);
			break;

			case KittyGG::Status::Busy:
				m_ui->startupComboBox->setCurrentIndex(2);
			break;

			case KittyGG::Status::DoNotDisturb:
				m_ui->startupComboBox->setCurrentIndex(3);
			break;

			case KittyGG::Status::FreeForChat:
				m_ui->startupComboBox->setCurrentIndex(4);
			break;

			case KittyGG::Status::Invisible:
				m_ui->startupComboBox->setCurrentIndex(5);
			break;

			case KittyGG::Status::Unavailable:
				m_ui->startupComboBox->setCurrentIndex(6);
			break;
		}
	}
}

void EditDialog::on_buttonBox_accepted()
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
			m_account->setInitialStatus(KittyGG::Status::Available);
		break;

		case 2:
			m_account->setInitialStatus(KittyGG::Status::Busy);
		break;

		case 3:
			m_account->setInitialStatus(KittyGG::Status::DoNotDisturb);
		break;

		case 4:
			m_account->setInitialStatus(KittyGG::Status::FreeForChat);
		break;

		case 5:
			m_account->setInitialStatus(KittyGG::Status::Invisible);
		break;

		case 6:
			m_account->setInitialStatus(KittyGG::Status::Unavailable);
		break;
	}

	if(m_ui->uinEdit->isEnabled()) {
		m_account->setUid(m_ui->uinEdit->text());
		m_protocol->core()->addAccount(m_account);
	}

	close();
}

void EditDialog::changeEvent(QEvent *event)
{
	if(event->type() == QEvent::LanguageChange) {
		m_ui->retranslateUi(this);
	}

	QDialog::changeEvent(event);
}

}
