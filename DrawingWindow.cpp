#include "DrawingWindow.h"
#include <QBasicTimer>
#include <QColor>
#include <QImage>
#include <QMutex>
#include <QPaintEvent>
#include <QPainter>
#include <QRect>
#include <QThread>
#include <QTimerEvent>

class DrawingThread: public QThread {
public:
    DrawingThread(DrawingWindow &w, DrawingWindow::ThreadFunction f);
    void start_once(Priority priority = InheritPriority);

protected:
    void run();

private:
    DrawingWindow &drawingWindow;
    DrawingWindow::ThreadFunction threadFunction;
    bool started_once;

    friend class DrawingWindow;
    friend class DrawingWindowPrivate;
};

class DrawingWindowPrivate {
public:
    static const int paintInterval = 33;

    DrawingWindow * const q;

    QBasicTimer timer;
    QMutex mutex;

    QImage *image;
    QPainter *painter;

    QColor fgColor;
    QColor bgColor;

    bool dirtyFlag;
    QRect dirtyRect;

    DrawingThread *thread;

    DrawingWindowPrivate(DrawingWindow *w,
                         DrawingWindow::ThreadFunction f);
    ~DrawingWindowPrivate();

    void initialize();

    void lock();
    void unlock();

    void setDirtyRect();
    void setDirtyRect(int x, int y);
    void setDirtyRect(int x1, int y1, int x2, int y2);
    void setDirtyRect(const QRect &rect);

};

//--- DrawingWindow ----------------------------------------------------

DrawingWindow::DrawingWindow(ThreadFunction f, int w, int h)
    : QWidget()
    , width(w)
    , height(h)
    , d(new DrawingWindowPrivate(this, f))
{
    d->initialize();
}

DrawingWindow::DrawingWindow(QWidget *parent,
                             ThreadFunction f, int w, int h)
    : QWidget(parent)
    , width(w)
    , height(h)
    , d(new DrawingWindowPrivate(this, f))
{
    d->initialize();
}

DrawingWindow::DrawingWindow(QWidget *parent, Qt::WindowFlags flags,
                             ThreadFunction f, int w, int h)
    : QWidget(parent, flags)
    , width(w)
    , height(h)
    , d(new DrawingWindowPrivate(this, f))
{
    d->initialize();
}

DrawingWindow::~DrawingWindow()
{
    delete d;
}

void DrawingWindow::setColor(float red, float green, float blue)
{
    d->fgColor.setRgbF(red, green, blue);
    QPen pen(d->painter->pen());
    pen.setColor(d->fgColor);
    d->painter->setPen(pen);
}

void DrawingWindow::setBgColor(float red, float green, float blue)
{
    d->bgColor.setRgbF(red, green, blue);
}

void DrawingWindow::clearGraph()
{
    d->lock();
    d->painter->fillRect(d->image->rect(), d->bgColor);    
    d->setDirtyRect();
    d->unlock();
}

void DrawingWindow::drawPoint(int x, int y)
{
    d->lock();
    d->painter->drawPoint(x, y);
    d->setDirtyRect(x, y);
    d->unlock();
}

void DrawingWindow::drawLine(int x1, int y1, int x2, int y2)
{
    d->lock();
    d->painter->drawLine(x1, y1, x2, y2);
    d->setDirtyRect(x1, y1, x2, y2);
    d->unlock();
}

void DrawingWindow::drawRect(int x1, int y1, int x2, int y2)
{
    QRect r;
    r.setCoords(x1, y1, x2, y2);
    r = r.normalized();
    d->lock();
    d->painter->drawRect(r);
    r.adjust(0, 0, 1, 1);
    d->setDirtyRect(r);
    d->unlock();
}

void DrawingWindow::sleep(unsigned long secs)
{
    DrawingThread::sleep(secs);
}

void DrawingWindow::msleep(unsigned long msecs)
{
    DrawingThread::msleep(msecs);
}

void DrawingWindow::usleep(unsigned long usecs)
{
    DrawingThread::usleep(usecs);
}

void DrawingWindow::closeEvent(QCloseEvent *ev)
{
    d->timer.stop();
    d->thread->terminate();
    QWidget::closeEvent(ev);
    d->thread->wait();
}

void DrawingWindow::paintEvent(QPaintEvent *ev)
{
    QPainter widgetPainter(this);
    QRect rect = ev->rect();
    d->mutex.lock();
    QImage imageCopy(*d->image);
    d->mutex.unlock();
    widgetPainter.drawImage(rect, imageCopy, rect);
}

void DrawingWindow::showEvent(QShowEvent *ev)
{
    d->timer.start(d->paintInterval, this);
    d->thread->start_once(QThread::IdlePriority);
    QWidget::showEvent(ev);
}

void DrawingWindow::timerEvent(QTimerEvent *ev)
{
    if (ev->timerId() == d->timer.timerId()) {
        d->mutex.lock();
        if (d->dirtyFlag) {
            update(d->dirtyRect);
            d->dirtyFlag = false;
        }
        d->mutex.unlock();
        d->timer.start(d->paintInterval, this);
    } else {
        QWidget::timerEvent(ev);
    }
}

//--- DrawingWindowPrivate ---------------------------------------------

DrawingWindowPrivate::DrawingWindowPrivate(DrawingWindow *w,
                                           DrawingWindow::ThreadFunction f)
    : q(w)
    , image(new QImage(q->width, q->height, QImage::Format_RGB32))
    , painter(new QPainter(image))
    , thread(new DrawingThread(*q, f))
{
}

void DrawingWindowPrivate::initialize()
{
    q->setFocusPolicy(Qt::StrongFocus);
    q->setFixedSize(image->size());
    q->setAttribute(Qt::WA_OpaquePaintEvent);
    q->setFocus();

    q->setColor(0.0, 0.0, 0.0);
    q->setBgColor(1.0, 1.0, 1.0);
    q->clearGraph();

    dirtyFlag = false;
}

DrawingWindowPrivate::~DrawingWindowPrivate()
{
    delete thread;
    delete painter;
    delete image;
}

inline
void DrawingWindowPrivate::lock()
{
    thread->setTerminationEnabled(false);
    mutex.lock();
}

inline
void DrawingWindowPrivate::unlock()
{
    mutex.unlock();
    thread->setTerminationEnabled(true);
}

inline
void DrawingWindowPrivate::setDirtyRect()
{
    dirtyFlag = true;
    dirtyRect = image->rect();
}

inline
void DrawingWindowPrivate::setDirtyRect(int x, int y)
{
    setDirtyRect(QRect(x, y, 1, 1));
}

inline
void DrawingWindowPrivate::setDirtyRect(int x1, int y1, int x2, int y2)
{
    QRect r;
    r.setCoords(x1, y1, x2, y2);
    setDirtyRect(r.normalized());
}

void DrawingWindowPrivate::setDirtyRect(const QRect &rect)
{
    if (dirtyFlag) {
        dirtyRect |= rect;
    } else {
        dirtyFlag = true;
        dirtyRect = rect;
    }
}

//--- DrawingThread ----------------------------------------------------

DrawingThread::DrawingThread(DrawingWindow &w, DrawingWindow::ThreadFunction f)
    : drawingWindow(w)
    , threadFunction(f)
    , started_once(false)
{
}

void DrawingThread::start_once(Priority priority)
{
    if (!started_once) {
        started_once = true;
        start(priority);
    }
}

void DrawingThread::run()
{
    threadFunction(drawingWindow);
}
