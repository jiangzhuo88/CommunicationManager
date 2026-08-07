#-------------------------------------------------
#
# Project created by QtCreator 2026-04-28T15:47:54
#
#-------------------------------------------------

QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = CommIdentifyManagerClient
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
DESTDIR = $$PWD/../bin/
# 中间产物输出到build目录
OBJECTS_DIR = build
MOC_DIR = build
UI_DIR = build
RCC_DIR = build
SOURCES += \
    LoginDialog.cpp \
    TableEditDelegate.cpp \
    UserManagementDialog.cpp \
    UserStore.cpp \
    dmdatabase.cpp \
    ipc_notify.cpp \
    localdatabase.cpp \
        main.cpp \
    mainwindow.cpp \
    ZToolButton.cpp \
    CenterWidget.cpp \
    CFramelessWindowBase.cpp \
    CDialogBase.cpp \
    ImportDataDialog.cpp \
    qt_ipcnotify.cpp

RESOURCES += \
    resource.qrc

FORMS += \
    mainwindow.ui \
    ZToolButton.ui \
    CenterWidget.ui \
    ImportDataDialog.ui

HEADERS += \
    LoginDialog.h \
    TableEditDelegate.h \
    UserManagementDialog.h \
    UserStore.h \
    define.h \
    dmdatabase.h \
    ipc_notify.h \
    localdatabase.h \
    mainwindow.h \
    ZToolButton.h \
    CenterWidget.h \
    CFramelessWindowBase.h \
    CDialogBase.h \
    ImportDataDialog.h \
    qt_ipcnotify.h
