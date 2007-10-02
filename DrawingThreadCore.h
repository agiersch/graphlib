#ifndef DRAWING_THREAD_CORE_H
#define DRAWING_THREAD_CORE_H

#include <QThread>

class DrawingThreadCore: public QThread {
private:
    virtual int runForReal() = 0;

public:
    void run();

};

#endif // !DRAWING_THREAD_CORE_H
