#ifndef GGEDITWINDOW_H
#define GGEDITWINDOW_H

#include <QtGui/QWidget>

namespace KittySDK
{
  class Account;
  class Protocol;

  namespace Ui
  {
    class GGEditWindow;
  }

  class GGEditWindow: public QWidget
  {
    Q_OBJECT

    public:
      GGEditWindow(KittySDK::Protocol *proto, QWidget *parent = 0);
      ~GGEditWindow();

      void reset();
      void setup(KittySDK::Account *account);

    protected:
      void showEvent(QShowEvent *event);

    private slots:
      void on_buttonBox_accepted();

    private:
      Ui::GGEditWindow *m_ui;
      KittySDK::Protocol *m_protocol;
      KittySDK::Account *m_account;
  };
}

#endif // GGEDITWINDOW_H
