TARGET     = GG
TEMPLATE   = lib
QT        += network xml

SOURCES   += GGProtocol.cpp \
             GGAccount.cpp \
             GGEditWindow.cpp \
             GGContact.cpp \
             GGClient.cpp \
             KittyGG/Managers.cpp \
    KittyGG/DataStream.cpp \
    KittyGG/Packets/XmlAction.cpp \
    KittyGG/Packets/Welcome.cpp \
    KittyGG/Packets/UserData.cpp \
    KittyGG/Packets/TypingNotify.cpp \
    KittyGG/Packets/Status.cpp \
    KittyGG/Packets/NotifyRemove.cpp \
    KittyGG/Packets/NotifyFirst.cpp \
    KittyGG/Packets/NotifyAdd.cpp \
    KittyGG/Packets/NewStatus.cpp \
    KittyGG/Packets/MessageSend.cpp \
    KittyGG/Packets/MessageRecv.cpp \
    KittyGG/Packets/MessageAck.cpp \
    KittyGG/Packets/Login.cpp \
    KittyGG/Packets/ListRequest.cpp \
    KittyGG/Packets/ListReply.cpp \
    KittyGG/Message.cpp \
    KittyGG/Parser.cpp

HEADERS   += GGProtocol.h \
             GGAccount.h \
             GGEditWindow.h \
             SDK/SettingPage.h \
             SDK/Protocol.h \
             SDK/PluginCore.h \
             SDK/Plugin.h \
             SDK/constants.h \
             SDK/Account.h \
             constants.h \
             SDK/Contact.h \
             GGClient.h \
             SDK/Message.h \
             SDK/Chat.h \
             SDK/GGConstants.h \
             GGContact.h \
             SDK/SoundsConstants.h \
             zlib/zlib.h \
             zlib/zconf.h \
             KittyGG/Packet.h \
             KittyGG/Managers.h \
    KittyGG/DataStream.h \
    KittyGG/Packets/XmlAction.h \
    KittyGG/Packets/Welcome.h \
    KittyGG/Packets/UserData.h \
    KittyGG/Packets/TypingNotify.h \
    KittyGG/Packets/Status.h \
    KittyGG/Packets/Pong.h \
    KittyGG/Packets/Ping.h \
    KittyGG/Packets/NotifyReply.h \
    KittyGG/Packets/NotifyRemove.h \
    KittyGG/Packets/NotifyLast.h \
    KittyGG/Packets/NotifyFirst.h \
    KittyGG/Packets/NotifyAdd.h \
    KittyGG/Packets/NewStatus.h \
    KittyGG/Packets/MessageSend.h \
    KittyGG/Packets/MessageRecv.h \
    KittyGG/Packets/MessageAck.h \
    KittyGG/Packets/LoginOk.h \
    KittyGG/Packets/LoginFailed.h \
    KittyGG/Packets/Login.h \
    KittyGG/Packets/ListRequest.h \
    KittyGG/Packets/ListReply.h \
    KittyGG/Packets/ListEmpty.h \
    KittyGG/Packets/Disconnecting.h \
    KittyGG/Packets/DisconnectAck.h \
    KittyGG/KittyGG.h \
    KittyGG/Message.h \
    KittyGG/Parser.h

FORMS     += GGEditWindow.ui

RESOURCES += res.qrc


