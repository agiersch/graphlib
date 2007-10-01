#include <DrawingThreadCore.h>

DrawingThreadCore::DrawingThreadCore(int argc_, char **argv_)
    : QThread()
    , argc(argc_)
    , argv(argv_)
{
}

void DrawingThreadCore::run()
{
    exit(this->runForReal(argc, argv));
}

int DrawingThreadCore::runForReal()
{
    throw MissingImplementationException();
}

int DrawingThreadCore::runForReal(int /* argc */, char ** /* argv_ */)
{
    return this->runForReal();
}

