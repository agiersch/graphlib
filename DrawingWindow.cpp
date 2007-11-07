#include "DrawingWindow.h"
#include <QApplication>
#include <QBasicTimer>
#include <QColor>
#include <QImage>
#include <QMutex>
#include <QPaintEvent>
#include <QPainter>
#include <QRect>
#include <QThread>
#include <QTimerEvent>
#include <QWaitCondition>

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
    QMutex imageMutex;
    QMutex syncMutex;
    QWaitCondition syncCondition;
    bool terminateThread;
    int lockCount;

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

    void safeLock(QMutex &mutex);
    void safeUnlock(QMutex &mutex);

    void dirty();
    void dirty(int x, int y);
    void dirty(int x1, int y1, int x2, int y2);
    void dirty(const QRect &rect);

    void update();
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
    d->safeLock(d->imageMutex);
    d->painter->fillRect(d->image->rect(), d->bgColor);    
    d->dirty();
    d->safeUnlock(d->imageMutex);
}

void DrawingWindow::drawPoint(int x, int y)
{
    d->safeLock(d->imageMutex);
    d->painter->drawPoint(x, y);
    d->dirty(x, y);
    d->safeUnlock(d->imageMutex);
}

void DrawingWindow::drawLine(int x1, int y1, int x2, int y2)
{
    d->safeLock(d->imageMutex);
    d->painter->drawLine(x1, y1, x2, y2);
    d->dirty(x1, y1, x2, y2);
    d->safeUnlock(d->imageMutex);
}

void DrawingWindow::drawRect(int x1, int y1, int x2, int y2)
{
    QRect r;
    r.setCoords(x1, y1, x2, y2);
    r = r.normalized();
    d->safeLock(d->imageMutex);
    d->painter->drawRect(r);
    r.adjust(0, 0, 1, 1);
    d->dirty(r);
    d->safeUnlock(d->imageMutex);
}

bool DrawingWindow::sync(unsigned long time)
{
    bool synced;
    d->safeLock(d->syncMutex);
    if (d->terminateThread) {
        synced = false;
    } else {
        qApp->postEvent(this, new QEvent(QEvent::User));
        synced = d->syncCondition.wait(&d->syncMutex, time);
    }
    d->safeUnlock(d->syncMutex);
    return synced;
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
    d->syncMutex.lock();
    d->terminateThread = true;  // this flag is needed for the case
                                // where the following wakeAll() call
                                // occurs between the
                                // setTerminationEnable(false) and the
                                // mutex lock in safeLock() called
                                // from sync()
    d->syncCondition.wakeAll();
    d->syncMutex.unlock();
    QWidget::closeEvent(ev);
    d->thread->wait();
}

void DrawingWindow::customEvent(QEvent *)
{
    d->update();
    qApp->sendPostedEvents(this, QEvent::UpdateLater);
    qApp->sendPostedEvents(this, QEvent::UpdateRequest);
    qApp->sendPostedEvents(this, QEvent::Paint);
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents |
                        QEventLoop::ExcludeSocketNotifiers |
                        QEventLoop::DeferredDeletion |
                        QEventLoop::X11ExcludeTimers);
    qApp->flush();
    qApp->syncX();
    d->syncMutex.lock();
    d->syncCondition.wakeAll();
    d->syncMutex.unlock();
}

void DrawingWindow::keyPressEvent(QKeyEvent *ev)
{
    bool accept = true;
    switch (ev->key()) {
    case Qt::Key_Escape:
        close();
        break;
    default:
        accept = false;
        break;
    }
    if (accept)
        ev->accept();
}

void DrawingWindow::paintEvent(QPaintEvent *ev)
{
    QPainter widgetPainter(this);
    d->imageMutex.lock();
    QImage imageCopy(*d->image);
    d->imageMutex.unlock();
    QRect rect = ev->rect();
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
        d->update();
        d->timer.start(d->paintInterval, this);
    } else {
        QWidget::timerEvent(ev);
    }
}

//--- DrawingWindowPrivate ---------------------------------------------

DrawingWindowPrivate::DrawingWindowPrivate(DrawingWindow *w,
                                           DrawingWindow::ThreadFunction f)
    : q(w)
    , terminateThread(false)
    , lockCount(0)
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

    q->setColor(0.0, 0.0, 0.0); // black
    q->setBgColor(1.0, 1.0, 1.0); // white
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
void DrawingWindowPrivate::safeLock(QMutex &mutex)
{
    if (lockCount++ == 0)
        thread->setTerminationEnabled(false);
    mutex.lock();
}

inline
void DrawingWindowPrivate::safeUnlock(QMutex &mutex)
{
    mutex.unlock();
    if (--lockCount == 0)
        thread->setTerminationEnabled(true);
}

inline
void DrawingWindowPrivate::dirty()
{
    dirtyFlag = true;
    dirtyRect = image->rect();
}

inline
void DrawingWindowPrivate::dirty(int x, int y)
{
    dirty(QRect(x, y, 1, 1));
}

inline
void DrawingWindowPrivate::dirty(int x1, int y1, int x2, int y2)
{
    QRect r;
    r.setCoords(x1, y1, x2, y2);
    dirty(r.normalized());
}

void DrawingWindowPrivate::dirty(const QRect &rect)
{
    if (dirtyFlag) {
        dirtyRect |= rect;
    } else {
        dirtyFlag = true;
        dirtyRect = rect;
    }
}

void DrawingWindowPrivate::update()
{
    imageMutex.lock();
    bool dirty = dirtyFlag;
    QRect rect = dirtyRect;
    dirtyFlag = false;
    imageMutex.unlock();
    if (dirty)
        q->update(rect);
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
