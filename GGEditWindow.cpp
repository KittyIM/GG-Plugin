#include "GGEditWindow.h"
#include "ui_GGEditWindow.h"

#include "SDK/Protocol.h"
#include "GGAccount.h"

#include <QtGui/QDesktopWidget>
#include <QtGui/QMessageBox>

#define qDebug() qDebug() << "[GGEditWindow]"
#define qWarning() qWarning() << "[GGEditWindow]"

KittySDK::GGEditWindow::GGEditWindow(KittySDK::Protocol *proto, QWidget *parent): QWidget(parent), m_ui(new Ui::GGEditWindow), m_protocol(proto)
{
	m_ui->setupUi(this);

	setWindowFlags(windowFlags() & ~Qt::WindowMinMaxButtonsHint);
}

KittySDK::GGEditWindow::~GGEditWindow()
{
	delete m_ui;
}

void KittySDK::GGEditWindow::reset()
{
	m_ui->uinLineEdit->clear();
	m_ui->uinLineEdit->setEnabled(true);
	m_ui->passwordLineEdit->clear();
}

void KittySDK::GGEditWindow::setup(KittySDK::Account *account)
{
	reset();

	if(!account) {
		m_account = new KittySDK::GGAccount("", dynamic_cast<KittySDK::GGProtocol*>(m_protocol));
	} else {
		m_account = dynamic_cast<KittySDK::GGAccount*>(account);

		m_ui->uinLineEdit->setText(m_account->uid());
		m_ui->uinLineEdit->setEnabled(false);
		m_ui->passwordLineEdit->setText(m_account->password());
	}
}

void KittySDK::GGEditWindow::showEvent(QShowEvent *event)
{
	QWidget::showEvent(event);

	move(QApplication::desktop()->screen()->rect().center() - rect().center());
}

void KittySDK::GGEditWindow::on_buttonBox_accepted()
{
	if(m_ui->uinLineEdit->text().toUInt() == 0) {
		QMessageBox::information(this, tr("Missing info"), tr("You have to enter UID"));
		return;
	}

	if(m_ui->passwordLineEdit->text().isEmpty()) {
		QMessageBox::information(this, tr("Missing info"), tr("You have to enter password"));
		return;
	}

	m_account->setPassword(m_ui->passwordLineEdit->text());

	if(m_ui->uinLineEdit->isEnabled()) {
		m_account->setUid(m_ui->uinLineEdit->text());
		m_protocol->core()->addAccount(m_account);
	}

	close();
}
