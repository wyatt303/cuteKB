#-------------------------------------------------
#
# Project created by QtCreator 2023-01-02T07:55:40
#
#-------------------------------------------------

QT       += core gui sql network widgets webenginewidgets
TARGET = cuteKB
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
        settingswindow.cpp \
        editorwindow.cpp \
        aboutwindow.cpp \
        library/hoedown/html.c \
        library/hoedown/autolink.c \
        library/hoedown/buffer.c \
        library/hoedown/document.c \
        library/hoedown/escape.c \
        library/hoedown/html_blocks.c \
        library/hoedown/stack.c \
        library/hoedown/version.c

HEADERS  += mainwindow.h \
        settingswindow.h \
        editorwindow.h \
        aboutwindow.h

FORMS    += mainwindow.ui \
        settingswindow.ui \
        editorwindow.ui \
        aboutwindow.ui

RESOURCES += \
        resources.qrc

include(library/qmarkdowntextedit/qmarkdowntextedit.pri)
