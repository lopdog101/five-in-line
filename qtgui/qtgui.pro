#-------------------------------------------------
#
# Project created by QtCreator 2013-12-03T19:28:25
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = qtgui
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    fieldwidget.cpp \
    ../algo/wsplayer_treat.cpp \
    ../algo/wsplayer_node.cpp \
    ../algo/wsplayer_common.inl \
    ../algo/wsplayer_common.cpp \
    ../algo/wsplayer.cpp \
    ../algo/gomoku_exceptions.cpp \
    ../algo/game.cpp \
    ../algo/field.cpp \
    ../algo/check_player.cpp \
    ../extern/object_progress.cpp

HEADERS  += mainwindow.h \
    fieldwidget.h \
    ../algo/wsplayer_treat.h \
    ../algo/wsplayer_node.h \
    ../algo/wsplayer_common.h \
    ../algo/wsplayer.h \
    ../algo/step.h \
    ../algo/primitives.h \
    ../algo/gomoku_exceptions.h \
    ../algo/game_xml.h \
    ../algo/game.h \
    ../algo/field_xml.h \
    ../algo/field.h \
    ../algo/env_variables.h \
    ../algo/check_player.h \
    ../algo/algo_utils.h \
    ../extern/polimvar_intf.h \
    ../extern/pair_comparator.h \
    ../extern/object_progress.hpp \
    ../extern/exception_catch.h \
    ../extern/binary_find.h

FORMS    += mainwindow.ui

RESOURCES += \
    qtgui_res.qrc

INCLUDEPATH += "D:\\src\\boost"

CONFIG += no_keywords

DEFINES += _CRT_SECURE_NO_WARNINGS
QMAKE_MOC = $$QMAKE_MOC -DBOOST_TT_HAS_OPERATOR_HPP_INCLUDED

LIBS += -L"D:\\src\boost\\stage\\lib"

win32{
    QMAKE_CXXFLAGS_DEBUG += -wd4100 -wd4189 -wd4996 -wd4018
    QMAKE_CXXFLAGS_RELEASE += -wd4100 -wd4189 -wd4996 -wd4018
}
