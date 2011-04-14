#ifndef GGCONTACT_H
#define GGCONTACT_H

#include "SDK/Contact.h"
#include "GGAccount.h"

#include <QtNetwork/QNetworkAccessManager>

namespace KittySDK
{
  class GGContact: public Contact
  {
    Q_OBJECT

    public:
      GGContact(const QString &uid, GGAccount *account);
      ~GGContact();

      void prepareContextMenu(QMenu *menu);
      void changeStatus(const quint32 &status, const QString &description);
      void setData(const QString &name, const QString &data);

    public slots:
      void loadSettings(const QMap<QString, QVariant> &settings);
      QMap<QString, QVariant> saveSettings();

    private slots:
      void updateAvatar();
      void processReply(QNetworkReply *reply);

    private:
      QMap<QString, QVariant> m_settings;
      QNetworkAccessManager m_netManager;
  };
}

#endif // GGCONTACT_H
