QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    event.cpp \
    gpt.cpp \
    main.cpp \
    databasemanager.cpp \
    mapper.cpp \
    serverthread.cpp \
    time_convert.cpp \
    uuid_gen.cpp

HEADERS += \
    connect.h \
    databasemanager.h \
    event.h \
    gpt.h \
    mapper.h \
    serverthread.h \
    time_convert.h \
    uuid_gen.h

FORMS += \
    databasemanager.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

QT +=widgets gui
QT += core gui sql
QT += network

INCLUDEPATH += C:/dev/vcpkg/installed/x86-windows/include


LIBS += -lmswsock
LIBS += -lws2_32

CONFIG += resources_big
