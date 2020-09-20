QT       += core gui sql

unix:!macx {
        QT += KWindowSystem
        HEADERS += shortcut_x11.h
        SOURCES += shortcut_x11.cpp
        CONFIG += link_pkgconfig
        PKGCONFIG += gio-2.0 glib-2.0 gio-unix-2.0
        LIBS +=-lgio-2.0 -lglib-2.0 -lX11 -lXtst
}

win32: {
        QT += winextras
        HEADERS += shortcut_win.h
        SOURCES += shortcut_win.cpp
        LIBS += -lpsapi
}

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# 3rd library
include(3rd/SingleApplication/singleapplication.pri)

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += QAPPLICATION_CLASS=QApplication

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    database.cpp \
    main.cpp \
    mainwindow.cpp \
    pasteitem.cpp \
    pasteitembarnner.cpp \
    pasteitemcontext.cpp \
    searchbar.cpp \
    shortcut.cpp

HEADERS += \
    database.h \
    mainwindow.h \
    pasteitem.h \
    pasteitembarnner.h \
    pasteitemcontext.h \
    searchbar.h \
    shortcut.h

TRANSLATIONS += \
    Pastes_zh_CN.ts

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    sources.qrc

DISTFILES += \
    stylesheet.qss
