#include "DrawingWindow.h"
#include <QPaintEvent>
#include <QThread>
#include <QTimerEvent>

#include <iostream>

class DrawingWindow::DrawingThread: public QThread {
public:
    DrawingThread(DrawingWindow &w, ThreadFunction f)
        : drawingWindow(w)
        , threadFunction(f)
    {
    }

    void run()
    {
        threadFunction(drawingWindow);
    }

private:
    DrawingWindow &drawingWindow;
    ThreadFunction threadFunction;

};

DrawingWindow::DrawingWindow(ThreadFunction fun, int width, int height)
    : QWidget()
{
    initialize(fun, width, height);
}

DrawingWindow::DrawingWindow(QWidget *parent,
                             ThreadFunction fun, int width, int height)
    : QWidget(parent)
{
    initialize(fun, width, height);
}

DrawingWindow::DrawingWindow(QWidget *parent, Qt::WindowFlags flags,
                             ThreadFunction fun, int width, int height)
    : QWidget(parent, flags)
{
    initialize(fun, width, height);
}

void DrawingWindow::initialize(ThreadFunction fun, int width, int height)
{
    image = new QImage(width, height, QImage::Format_RGB32);
    image->fill(QColor(Qt::white).rgb());

    painter = new QPainter(image);

    dirtyFlag = false;

    setFocusPolicy(Qt::StrongFocus);
    setFixedSize(image->size());
    setAttribute(Qt::WA_OpaquePaintEvent);
    setFocus();
    timer.start(paintInterval, this);

    thread = new DrawingThread(*this, fun);
    thread_started = false;

    mutex_enabled = true;
}

DrawingWindow::~DrawingWindow()
{
    mutex.lock();
    mutex_enabled = false;
    mutex.unlock();
    std::cerr << "A\n";
    thread->terminate();    
    std::cerr << "B\n";
    thread->wait();
    std::cerr << "C\n";
    delete thread;
    delete painter;
    delete image;
    std::cerr << "D\n";
}

void DrawingWindow::setColor(const QColor &color)
{
    QPen pen(painter->pen());
    pen.setColor(color);
    painter->setPen(pen);
}

void DrawingWindow::setColor(float red, float green, float blue)
{
    QColor color;
    color.setRgbF(red, green, blue);
    this->setColor(color);
}

void DrawingWindow::drawPoint(int x, int y)
{
    lock();
    painter->drawPoint(x, y);
    setDirtyRect(x, y);
    unlock();
}

void DrawingWindow::drawLine(int x1, int y1, int x2, int y2)
{
    lock();
    painter->drawLine(x1, y1, x2, y2);
    setDirtyRect(x1, y1, x2, y2);
    unlock();
}

void DrawingWindow::paintEvent(QPaintEvent *ev)
{
    QPainter widgetPainter(this);
    QRect rect = ev->rect();
    lock();
    QImage imageCopy(*image);
    unlock();
    widgetPainter.drawImage(rect, imageCopy, rect);
}

void DrawingWindow::showEvent(QShowEvent *ev)
{
    if (!thread_started) {
        thread->start();
        thread_started = true;
    }
    QWidget::showEvent(ev);
}

void DrawingWindow::timerEvent(QTimerEvent *ev)
{
    if (ev->timerId() == timer.timerId()) {
        lock();
        if (dirtyFlag) {
            update(dirtyRect);
            dirtyFlag = false;
        }
        unlock();
        timer.start(paintInterval, this);
    } else {
        QWidget::timerEvent(ev);
    }
}

void DrawingWindow::setDirtyRect(const QRect &rect)
{
    if (dirtyFlag) {
        dirtyRect |= rect;
    } else {
        dirtyFlag = true;
        dirtyRect = rect;
    }
}
