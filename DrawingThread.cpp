#include "DrawingThread.h"
#include <QThread>

DrawingThread::DrawingThread(DrawingArea &a, ThreadFunction f)
    : drawingArea(a)
    , threadFunction(f)
{
}

void DrawingThread::run()
{
    exit(threadFunction(drawingArea));
}
