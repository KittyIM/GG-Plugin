TARGET     = GG-plugin
TEMPLATE   = lib
QT        += network

SOURCES   += GGProtocol.cpp \
             GGAccount.cpp \
             GGEditWindow.cpp \
             GGContact.cpp \
             GGClient.cpp

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
             SDK/GGContact.h \
             SDK/Contact.h \
             GGClient.h \
    SDK/Message.h \
    SDK/Chat.h \
    GGConstants.h \
    SDK/GGConstants.h \
    GGContact.h \
    SDK/SoundsConstants.h

FORMS     += GGEditWindow.ui

RESOURCES += res.qrc
