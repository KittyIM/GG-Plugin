#ifndef GGACCOUNT_H
#define GGACCOUNT_H

#include "SDK/Account.h"

namespace KittySDK
{
  class Protocol;

  class GGAccount: public Account
  {
    Q_OBJECT

    public:
      GGAccount(const QString &uid, Protocol *parent);
      ~GGAccount();

      quint32 uin() const;

    public slots:
      void loadSettings(const QMap<QString, QVariant> &settings);
      QMap<QString, QVariant> saveSettings();
      QMenu *statusMenu();

    private:
      QMenu *m_statusMenu;
      QAction *m_availableAction;
      QAction *m_awayAction;
      QAction *m_ffcAction;
      QAction *m_dndAction;
      QAction *m_invisibleAction;
      QAction *m_unavailableAction;
  };
}

#endif // GGACCOUNT_H
