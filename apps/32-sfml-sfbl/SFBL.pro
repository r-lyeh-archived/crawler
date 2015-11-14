#-------------------------------------------------
#
# Project created by QtCreator 2015-01-22T15:48:23
#
#-------------------------------------------------

QT       -= core gui

TARGET = SFBL
TEMPLATE = lib
CONFIG += staticlib

SOURCES += sfbl.cpp

HEADERS += sfbl.h
unix {
    target.path = /usr/lib
    INSTALLS += target
}

INCLUDEPATH += "pathtoSFML\include"
DEPENDPATH += "pathtoSFML\include"
LIBS += -L"pathtoSFML\lib"

CONFIG(release, debug|release): LIBS += -lsfml-audio -lsfml-graphics -lsfml-network -lsfml-window -lsfml-system -lBox2D
CONFIG(debug, debug|release): LIBS += -lsfml-audio-d -lsfml-graphics-d -lsfml-main-d -lsfml-network-d -lsfml-window-d -lsfml-system-d -lBox2D

DEFINES += SFML_DYNAMIC

OTHER_FILES += \
    Light.frag
CONFIG += c++11
