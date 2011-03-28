#include "GGEditWindow.h"
#include "ui_GGEditWindow.h"

KittySDK::GGEditWindow::GGEditWindow(QWidget *parent): QWidget(parent), m_ui(new Ui::GGEditWindow)
{
  m_ui->setupUi(this);
}

KittySDK::GGEditWindow::~GGEditWindow()
{
  delete m_ui;
}

