#ifndef GGPROTOCOL_H
#define GGPROTOCOL_H

#include "SDK/Protocol.h"

namespace KittySDK
{
  class Account;
  class GGEditWindow;

  class GGProtocol: public Protocol
  {
    public:
      GGProtocol(PluginCore *core);
      ~GGProtocol();

      Account *newAccount(const QString &uid);
      QWidget *editWindow(Account *account = 0);

    private:
      GGEditWindow *m_editWindow;
  };
}
#endif // GGPROTOCOL_H
