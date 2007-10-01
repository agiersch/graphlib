#include <MainDrawingThread.h>

#include <QApplication>

MainDrawingThread::MainDrawingThread(int argc, char **argv)
    : DrawingThreadCore(argc, argv)
{
}

int main(int argc, char *argv[])
{
    QApplication application(argc, argv);
    MainDrawingThread mainDrawingThread(argc, argv);

    mainDrawingThread.start();

    return application.exec();
}
