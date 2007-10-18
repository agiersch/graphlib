TEMPLATE = app
TARGET = hello

CONFIG += qt debug

HEADERS += DrawingArea.h \
           DrawingThread.h \
           DrawingWindow.h

SOURCES += DrawingArea.cpp \
           DrawingThread.cpp \
           DrawingWindow.cpp \
           hello.cc
