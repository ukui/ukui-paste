QT       += core gui sql dbus

unix:!macx {
        QT += KWindowSystem
        HEADERS += shortcut_x11.h
        SOURCES += shortcut_x11.cpp
        CONFIG += link_pkgconfig
        PKGCONFIG += gio-2.0 glib-2.0 gio-unix-2.0 gsettings-qt
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

TARGET = pastes
TEMPLATE = app

unix:!macx {
        target.path = /usr/bin
        QM_FILES_INSTALL_PATH = /usr/share/$${TARGET}/translations/
        DEFINES += QM_FILES_INSTALL_PATH='\\"$${QM_FILES_INSTALL_PATH}\\"'
}

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    database.cpp \
    main.cpp \
    mainframe.cpp \
    mainwindow.cpp \
    pasteitem.cpp \
    pasteitembarnner.cpp \
    pasteitemcontext.cpp \
    pastestyle.cpp \
    searchbar.cpp \
    shortcut.cpp

HEADERS += \
    database.h \
    mainframe.h \
    mainwindow.h \
    pasteitem.h \
    pasteitembarnner.h \
    pasteitemcontext.h \
    pastestyle.h \
    searchbar.h \
    shortcut.h

TRANSLATIONS += \
    Pastes_zh_CN.ts

unix:!macx {
        # CONFIG += lrelase not work for qt5.6, add those from lrelease.prf for compatibility
        qtPrepareTool(QMAKE_LRELEASE, lrelease)
        lrelease.name = lrelease
        lrelease.input = TRANSLATIONS
        lrelease.output = ${QMAKE_FILE_IN_BASE}.qm
        lrelease.commands = $$QMAKE_LRELEASE ${QMAKE_FILE_IN} -qm ${QMAKE_FILE_OUT}
        lrelease.CONFIG = no_link
        QMAKE_EXTRA_COMPILERS += lrelease
        PRE_TARGETDEPS += compiler_lrelease_make_all

        for (translation, TRANSLATIONS) {
            translation = $$basename(translation)
            QM_FILES += $$OUT_PWD/$$replace(translation, \\..*$, .qm)
        }
        qm_files.files = $$QM_FILES
        qm_files.path = $$QM_FILES_INSTALL_PATH
        qm_files.CONFIG = no_check_exist
        INSTALLS += qm_files
}

RESOURCES += \
    sources.qrc

DISTFILES += \
    stylesheet.qss

unix:!macx {
        desktop_file.files = $${TARGET}.desktop
        desktop_file.path = /etc/xdg/autostart
        INSTALLS += desktop_file
}

INSTALLS += target
