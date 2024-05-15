#-------------------------------------------------
#
# Project created by QtCreator
#
#-------------------------------------------------

QT       += core gui
QT       += sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Flashcard
TEMPLATE = app


SOURCES += main.cpp\
    addeditquestion.cpp \
    keyword.cpp \
        mainwindow.cpp \
    question.cpp \
    properties.cpp \
    latex.cpp

HEADERS  += mainwindow.h \
    addeditquestion.h \
    keyword.h \
    question.h \
    properties.h \
    latex.h

