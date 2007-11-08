TEMPLATE = app
TARGET = hello

CONFIG += qt
CONFIG += debug
#CONFIG += profile

profile {
	QMAKE_CFLAGS += -pg
	QMAKE_CXXFLAGS += -pg
	QMAKE_LFLAGS += -pg
}

INCLUDEPATH += ../
HEADERS += ../DrawingWindow.h
SOURCES += ../DrawingWindow.cpp \
           hello.cpp
