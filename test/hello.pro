TEMPLATE = app
TARGET = hello

CONFIG += qt debug

QMAKE_CFLAGS += -O3
QMAKE_CXXFLAGS += -O3

QMAKE_CFLAGS += -pg
QMAKE_CXXFLAGS += -pg
QMAKE_LFLAGS += -pg

HEADERS += DrawingWindow.h

SOURCES += DrawingWindow.cpp \
           hello.cc
