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

      quint32 ggUid() const;
  };
}

#endif // GGACCOUNT_H
