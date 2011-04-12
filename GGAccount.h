#ifndef GGACCOUNT_H
#define GGACCOUNT_H

#include "SDK/Account.h"
#include "GGProtocol.h"

namespace KittySDK
{
  class Protocol;
  class GGClient;
  class Contact;

  class GGAccount: public Account
  {
    Q_OBJECT

    public:
      GGAccount(const QString &uid, GGProtocol *parent);
      ~GGAccount();

      quint32 uin() const;
      KittySDK::Protocol::Status status() const;
      KittySDK::Contact *newContact(const QString &uid);

      void insertContact(const QString &uid, KittySDK::Contact *contact);

    public slots:
      void loadSettings(const QMap<QString, QVariant> &settings);
      QMap<QString, QVariant> saveSettings();
      QMenu *statusMenu();

    private slots:
      void changeContactStatus(const quint32 &uin, const quint32 &status, const QString &description);
      void setStatusAvailable();
      void setStatusAway();
      void setStatusFFC();
      void setStatusDND();
      void setStatusInvisible();
      void setStatusUnavailable();

    private:
      GGClient *m_client;
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
