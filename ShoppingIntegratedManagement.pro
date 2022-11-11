QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets network

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    chatclientform.cpp \
    chatserverform.cpp \
    clientmanagerform.cpp \
    logthread.cpp \
    main.cpp \
    mainwindow.cpp \
    productmanagerform.cpp \
    shoppingmanagerform.cpp

HEADERS += \
    chatclientform.h \
    chatserverform.h \
    clientmanagerform.h \
    logthread.h \
    mainwindow.h \
    productmanagerform.h \
    shoppingmanagerform.h

FORMS += \
    chatclientform.ui \
    chatserverform.ui \
    clientmanagerform.ui \
    mainwindow.ui \
    productmanagerform.ui \
    shoppingmanagerform.ui

TRANSLATIONS += \
    ShoppingIntegratedManagement_ko_KR.ts
CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resource.qrc
