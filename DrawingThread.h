#ifndef DRAWING_THREAD_H
#define DRAWING_THREAD_H

#include "DrawingArea.h"
#include <QThread>

class DrawingThread: public QThread {
public:
    typedef int (*ThreadFunction)(DrawingArea &);

    DrawingThread(DrawingArea &a, ThreadFunction f);

    void run();

private:
    DrawingArea &drawingArea;
    ThreadFunction threadFunction;

};

#endif // !DRAWING_THREAD_H
