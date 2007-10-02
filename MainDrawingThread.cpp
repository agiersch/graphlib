#include <MainDrawingThread.h>

#include <QApplication>

MainDrawingThread::MainDrawingThread(int argc_, char **argv_)
    : DrawingThreadCore()
    , argc(argc_)
    , argv(argv_)
{
}

int MainDrawingThread::runForReal()
{
    return MainDrawingThread::main(argc, argv);
}

int main(int argc, char *argv[])
{
    QApplication application(argc, argv);
    MainDrawingThread mainDrawingThread(argc, argv);

    mainDrawingThread.start();

    return application.exec();
}
