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

      QString statusIcon(KittySDK::Protocol::Status status);
      Account *newAccount(const QString &uid);
      QWidget *editWindow(Account *account = 0);

      Status convertStatus(const quint32 &status) const;

    private:
      GGEditWindow *m_editWindow;
  };
}
#endif // GGPROTOCOL_H
