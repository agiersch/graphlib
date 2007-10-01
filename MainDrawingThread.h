#ifndef MAIN_DRAWING_THREAD_H
#define MAIN_DRAWING_THREAD_H

#include <DrawingThreadCore.h>

class MainDrawingThread: public DrawingThreadCore {
public:
    MainDrawingThread(int argc, char **argv);
    ~MainDrawingThread();

    int runForReal();
    int runForReal(int argc, char **argv);
};

#define main_thread MainDrawingThread::runForReal

#endif // !MAIN_DRAWING_THREAD_H
