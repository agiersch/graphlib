#ifndef DRAWING_THREAD_CORE_H
#define DRAWING_THREAD_CORE_H

#include <QThread>

class DrawingThreadCore: public QThread {
private:
    int argc;
    char **argv;

public:
    class MissingImplementationException { };

protected:
    virtual int runForReal();
    virtual int runForReal(int argc, char **argv);

public:
    DrawingThreadCore(int argc, char **argv);
    virtual ~DrawingThreadCore();
    void run();

};

#endif // !DRAWING_THREAD_CORE_H
