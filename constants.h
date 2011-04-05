#ifndef CONSTANTS_H
#define CONSTANTS_H

#define qDebug() qDebug() << "[KittyGG] "

namespace KittyGG
{
  namespace Icons
  {
    const char * const I_AVAILABLE    = "KittyGG.Available";
    const char * const I_FFC          = "KittyGG.FFC";
    const char * const I_AWAY         = "KittyGG.Away";
    const char * const I_DND          = "KittyGG.DND";
    const char * const I_INVISIBLE    = "KittyGG.Invisible";
    const char * const I_UNAVAILABLE  = "KittyGG.Unavailable";
  }

  namespace Packets
  {
    enum
    {
      P_WELCOME        = 0x01,
      P_MSG_SEND_ACK   = 0x05,
      P_PONG           = 0x07,
      P_PING           = 0x08,
      P_DISCONNECTING  = 0x0b,
      P_DISCONNECT_ACK = 0x0d,
      P_NOTIFY_FIRST   = 0x0f,
      P_NOTIFY_LAST    = 0x10,
      P_LIST_EMPTY     = 0x12,
      P_MSG_SEND       = 0x2d,
      P_MSG_RECV       = 0x2e,
      P_LOGIN          = 0x31,
      P_LOGIN_OK       = 0x35,
      P_STATUS         = 0x36,
      P_NOTIFY_REPLY   = 0x37,
      P_NEW_STATUS     = 0x38,
      P_LOGIN_FAILED   = 0x43,
      P_USER_DATA      = 0x44,
      P_TYPING_NOTIFY  = 0x59
    };
  }

  namespace HashMethods
  {
    enum
    {
      H_GG   = 0x01,
      H_SHA1 = 0x02
    };
  }

  namespace Statuses
  {
    enum
    {
      S_UNAVAILABLE	= 0x0001,
      S_FFC	        = 0x0017,
      S_AVAILABLE	  = 0x0002,
      S_BUSY        = 0x0003,
      S_DND         = 0x0021,
      S_INVISIBLE   = 0x0014
    };
  }

  namespace Flags
  {
    enum
    {
      F_UNKNOWN  = 0x00000001,
      F_VIDEO    = 0x00000002,
      F_MOBILE   = 0x00100000,
      F_SPAM     = 0x00800000
    };
  }

  namespace Features
  {
    enum
    {
      F_STATUS80            = 0x0001 | 0x0004,
      F_MSG80               = 0x0002,
      F_DND_FFC             = 0x0010,
      F_IMAGE_DESCR         = 0x0020,
      F_NEW_LOGIN           = 0x0040,
      F_UNKNOWN_100         = 0x0100,
      F_USER_DATA           = 0x0200,
      F_MSG_ACK             = 0x0400,
      F_TYPING_NOTIFICATION = 0x2000,
      F_MULTILOGON          = 0x4000
    };
  }

  namespace Fonts
  {
    enum
    {
      F_BOLD      = 0x01,
      F_ITALIC    = 0x02,
      F_UNDERLINE = 0x04,
      F_COLOR     = 0x08,
      F_IMAGE     = 0x80
    };
  }
}

#endif // CONSTANTS_H