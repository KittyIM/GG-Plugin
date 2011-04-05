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
  };
}

#endif // GGCONTACT_H
