TEMPLATE = app
TARGET = chateaux

CONFIG += qt
CONFIG += debug
#CONFIG += profile

profile {
	QMAKE_CFLAGS += -pg
	QMAKE_CXXFLAGS += -pg
	QMAKE_LFLAGS += -pg
}

INCLUDEPATH += ../
DEPENDPATH += ../

HEADERS += ../DrawingWindow.h
SOURCES += ../DrawingWindow.cpp \
           chateaux.cpp
