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
      Protocol::Status status() const;
      Contact *newContact(const QString &uid);
      Contact *newContact(const quint32 &uin);

      Contact *contactByUin(const quint32 &uin);
      void insertContact(const QString &uid, Contact *contact);

    public slots:
      void loadSettings(const QMap<QString, QVariant> &settings);
      QMap<QString, QVariant> saveSettings();
      void changeStatus(const KittySDK::Protocol::Status &status, const QString &description);
      QMenu *statusMenu();
      void sendMessage(const Message &msg);

    private slots:
      void changeContactStatus(const quint32 &uin, const quint32 &status, const QString &description);
      void processUserData(const quint32 &uin, const QString &name, const QString &data);
      void processMessage(const quint32 &sender, const QDateTime &time, const QString &plain);
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
