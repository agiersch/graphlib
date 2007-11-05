#include "DrawingWindow.h"
#include <QPaintEvent>
#include <QThread>
#include <QTimerEvent>

class DrawingWindow::DrawingThread: public QThread {
public:
    DrawingThread(DrawingWindow &w, ThreadFunction f)
        : drawingWindow(w)
        , threadFunction(f)
    {
    }

    void run()
    {
        exit(threadFunction(drawingWindow));
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

#ifdef USE_PIXMAP_CACHE
    pixmap = new QPixmap(image->size());
    QPainter pixmapPainter(pixmap);
    pixmapPainter.drawImage(0, 0, *image);
#endif

    dirtyFlag = false;

    setFocusPolicy(Qt::StrongFocus);
    setFixedSize(image->size());
    setAttribute(Qt::WA_OpaquePaintEvent);
    setFocus();
    timer.start(paintInterval, this);

    thread = new DrawingThread(*this, fun);
    thread_started = false;
}

DrawingWindow::~DrawingWindow()
{
    delete thread;
#ifdef USE_PIXMAP_CACHE
    delete pixmap;
#endif
    delete painter;
    delete image;
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
#ifdef USE_PIXMAP_CACHE
    widgetPainter.drawPixmap(rect, *pixmap, rect);
#else
    lock();
    widgetPainter.drawImage(rect, *image, rect);
    unlock();
#endif
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
#ifdef USE_PIXMAP_CACHE
            QPainter pixmapPainter(pixmap);
            pixmapPainter.drawImage(dirtyRect, *image, dirtyRect);
#endif
            dirtyFlag = false;
            update(dirtyRect);
            timer.start(paintInterval, this);
        }
        unlock();
    } else {
        QWidget::timerEvent(ev);
    }
}

void DrawingWindow::setDirtyRect()
{
    setDirtyRect(QRect(0, 0, width(), height()));
}

void DrawingWindow::setDirtyRect(int x, int y)
{
    setDirtyRect(QRect(x, y, 1, 1));
}

void DrawingWindow::setDirtyRect(int x1, int y1, int x2, int y2)
{
    QRect r;
    r.setCoords(x1, y1, x2, y2);
    setDirtyRect(r.normalized());
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
