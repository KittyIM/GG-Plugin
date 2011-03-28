TARGET   = GG-plugin
TEMPLATE = lib

SOURCES += GGProtocol.cpp \
           GGAccount.cpp \
           GGEditWindow.cpp

HEADERS += GGProtocol.h \
           GGAccount.h \
           GGEditWindow.h \
           SDK/SettingPage.h \
           SDK/Protocol.h \
           SDK/PluginCore.h \
           SDK/Plugin.h \
           SDK/constants.h \
           SDK/Account.h

FORMS   += GGEditWindow.ui
