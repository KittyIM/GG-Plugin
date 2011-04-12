#ifndef GGCONTACT_H
#define GGCONTACT_H

#include "SDK/Contact.h"
#include "GGAccount.h"


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
  };
}

#endif // GGCONTACT_H
