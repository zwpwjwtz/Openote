#-------------------------------------------------
#
# Project created by QtCreator 2021-04-26T14:43:16
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Openote
TEMPLATE = app

VERSION = 1.0.0

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS APP_VERSION=\\\"$$VERSION\\\"

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    OpenTable/ontablecolumn.cpp \
    OpenTable/ontableintcolumn.cpp \
    OpenTable/ontabledoublecolumn.cpp \
    OpenTable/ontablestringcolumn.cpp \
    OpenTable/ontable.cpp \
    OpenTable/utils/filesystem.cpp \
    OpenTable/onbook.cpp \
    widgets/bookview.cpp \
    dialogs/dialogabout.cpp \
    dialogs/dialogcolumnadd.cpp \
    OpenTable/ontableintlistcolumn.cpp \
    models/tablemodel.cpp \
    models/bookmodel.cpp \
    widgets/columnreferencedelegate.cpp \
    widgets/columnreferenceselector.cpp \
    widgets/bookcontextmenu.cpp \
    widgets/bookactiondispatcher.cpp \
    dialogs/dialogpreference.cpp \
    global.cpp \
    widgets/tableview.cpp \
    widgets/columnheaderview.cpp \
    models/clipboardmodel.cpp \
    dialogs/dialogfind.cpp

HEADERS += \
        mainwindow.h \
    OpenTable/ontablecolumn.h \
    OpenTable/ontablecolumn_p.h \
    OpenTable/ontableintcolumn.h \
    OpenTable/ontabledoublecolumn.h \
    OpenTable/ontablestringcolumn.h \
    OpenTable/ontable.h \
    OpenTable/ontable_p.h \
    OpenTable/utils/filesystem.h \
    OpenTable/onbook.h \
    OpenTable/onbook_p.h \
    widgets/bookview.h \
    dialogs/dialogabout.h \
    dialogs/dialogcolumnadd.h \
    OpenTable/ontableintlistcolumn.h \
    models/tablemodel.h \
    models/bookmodel.h \
    models/bookmodel_p.h \
    models/tablemodel_p.h \
    widgets/columnreferencedelegate.h \
    widgets/columnreferenceselector.h \
    widgets/bookcontextmenu.h \
    widgets/bookactiondispatcher.h \
    dialogs/dialogpreference.h \
    global.h \
    widgets/tableview.h \
    widgets/columnheaderview.h \
    models/clipboardmodel.h \
    dialogs/dialogfind.h \
    widgets/bookview_p.h

FORMS += \
    mainwindow.ui \
    dialogs/dialogabout.ui \
    dialogs/dialogcolumnadd.ui \
    widgets/columnreferenceselector.ui \
    dialogs/dialogpreference.ui \
    dialogs/dialogfind.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    reources.qrc \
    translations.qrc

TRANSLATIONS += \
    translations/Openote_zh_CN.ts

include(translation.pri)
