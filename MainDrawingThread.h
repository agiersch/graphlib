#ifndef MAIN_DRAWING_THREAD_H
#define MAIN_DRAWING_THREAD_H

#include <DrawingThreadCore.h>

class MainDrawingThread: public DrawingThreadCore {
private:
    int argc;
    char **argv;

    template<int (*f)()>
    static int mainWrapper(int, char **)
    {
        return f();
    }

    template<int (*f)(int, char **)>
    static int mainWrapper(int argc, char **argv)
    {
        return f(argc, argv);
    }

    static int (*main)(int, char **);

    int runForReal();

public:
    MainDrawingThread(int argc_, char **argv_);
        
};

#define main_thread(...)                                \
    /* int */ main_thread(__VA_ARGS__);                 \
    int (*MainDrawingThread::main)(int, char **) =      \
        MainDrawingThread::mainWrapper<main_thread>;    \
    int main_thread(__VA_ARGS__)

#endif // !MAIN_DRAWING_THREAD_H
