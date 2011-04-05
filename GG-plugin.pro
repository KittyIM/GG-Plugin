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
             GGContact.h \
             SDK/Contact.h \
             GGClient.h

FORMS     += GGEditWindow.ui

RESOURCES += res.qrc
