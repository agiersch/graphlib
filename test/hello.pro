TEMPLATE = app
TARGET = hello

CONFIG += qt debug

QMAKE_CFLAGS += -O2
QMAKE_CXXFLAGS += -O2

#QMAKE_CFLAGS += -pg
#QMAKE_CXXFLAGS += -pg
#QMAKE_LFLAGS += -pg

HEADERS += ../DrawingWindow.h

SOURCES += ../DrawingWindow.cpp \
           hello.cc
