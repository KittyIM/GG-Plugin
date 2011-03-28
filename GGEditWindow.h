#ifndef GGEDITWINDOW_H
#define GGEDITWINDOW_H

#include <QtGui/QWidget>

namespace KittySDK
{
  namespace Ui
  {
    class GGEditWindow;
  }

  class GGEditWindow: public QWidget
  {
    Q_OBJECT

    public:
      explicit GGEditWindow(QWidget *parent = 0);
      ~GGEditWindow();

    private:
      Ui::GGEditWindow *m_ui;
  };
}

#endif // GGEDITWINDOW_H
