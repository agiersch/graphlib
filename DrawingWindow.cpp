#include "DrawingWindow.h"
#include <QApplication>
#include <QPaintEvent>
#include <QPainter>
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
};

//--- DrawingWindow ----------------------------------------------------

DrawingWindow::DrawingWindow(ThreadFunction f, int w, int h)
    : QWidget()
    , width(w)
    , height(h)
{
    initialize(f);
}

DrawingWindow::DrawingWindow(QWidget *parent,
                             ThreadFunction f, int w, int h)
    : QWidget(parent)
    , width(w)
    , height(h)
{
    initialize(f);
}

DrawingWindow::DrawingWindow(QWidget *parent, Qt::WindowFlags flags,
                             ThreadFunction f, int w, int h)
    : QWidget(parent, flags)
    , width(w)
    , height(h)
{
    initialize(f);
}

DrawingWindow::~DrawingWindow()
{
    delete thread;
    delete painter;
    delete image;
}

void DrawingWindow::setColor(float red, float green, float blue)
{
    fgColor.setRgbF(red, green, blue);
    applyColor();
}

void DrawingWindow::setColor(const char *name)
{
    fgColor.setNamedColor(name);
    applyColor();
}

void DrawingWindow::setBgColor(float red, float green, float blue)
{
    bgColor.setRgbF(red, green, blue);
}

void DrawingWindow::setBgColor(const char *name)
{
    bgColor.setNamedColor(name);
}

void DrawingWindow::clearGraph()
{
    safeLock(imageMutex);
    painter->fillRect(image->rect(), bgColor);    
    dirty();
    safeUnlock(imageMutex);
}

void DrawingWindow::drawPoint(int x, int y)
{
    safeLock(imageMutex);
    painter->drawPoint(x, y);
    dirty(x, y);
    safeUnlock(imageMutex);
}

void DrawingWindow::drawLine(int x1, int y1, int x2, int y2)
{
    safeLock(imageMutex);
    painter->drawLine(x1, y1, x2, y2);
    dirty(x1, y1, x2, y2);
    safeUnlock(imageMutex);
}

void DrawingWindow::drawRect(int x1, int y1, int x2, int y2)
{
    QRect r;
    r.setCoords(x1, y1, x2 - 1, y2 - 1);
    r = r.normalized();
    safeLock(imageMutex);
    painter->drawRect(r);
    r.adjust(0, 0, 1, 1);
    dirty(r);
    safeUnlock(imageMutex);
}

void DrawingWindow::fillRect(int x1, int y1, int x2, int y2)
{
    painter->setBrush(fgColor);
    drawRect(x1, y1, x2, y2);
    painter->setBrush(Qt::NoBrush);
}

void DrawingWindow::drawCircle(int x, int y, int r)
{
    QRect rect;
    rect.setCoords(x - r, y - r, x + r - 1, y + r - 1);
    safeLock(imageMutex);
    painter->drawEllipse(rect);
    rect.adjust(0, 0, 1, 1);
    dirty(rect);
    safeUnlock(imageMutex);
}

void DrawingWindow::fillCircle(int x, int y, int r)
{
    painter->setBrush(fgColor);
    drawCircle(x, y, r);
    painter->setBrush(Qt::NoBrush);
}

void DrawingWindow::drawText(int x, int y, const char *text)
{
    QRect r(image->rect());
    r.moveTo(x, y);
    safeLock(imageMutex);
    painter->drawText(r, 0, text, &r);
    dirty(r);
    safeUnlock(imageMutex);
}

bool DrawingWindow::sync(unsigned long time)
{
    bool synced;
    safeLock(syncMutex);
    if (terminateThread) {
        synced = false;
    } else {
        qApp->postEvent(this, new QEvent(QEvent::User));
        synced = syncCondition.wait(&syncMutex, time);
    }
    safeUnlock(syncMutex);
    return synced;
}

void DrawingWindow::closeGraph()
{
    qApp->postEvent(this, new QCloseEvent());
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
    qDebug(">>>CLOSING<<<\n");
    char x = 'A';
    qDebug(">>> %c <<<\n", x++);
    timer.stop();
    qDebug(">>> %c <<<\n", x++);
    thread->terminate();
    qDebug(">>> %c <<<\n", x++);
    syncMutex.lock();
    qDebug(">>> %c <<<\n", x++);
    terminateThread = true;     // this flag is needed for the case
                                // where the following wakeAll() call
                                // occurs between the
                                // setTerminationEnable(false) and the
                                // mutex lock in safeLock() called
                                // from sync()
    qDebug(">>> %c <<<\n", x++);
    syncCondition.wakeAll();
    qDebug(">>> %c <<<\n", x++);
    syncMutex.unlock();
    qDebug(">>> %c <<<\n", x++);
    QWidget::closeEvent(ev);
    qDebug(">>> %c <<<\n", x++);
    thread->wait();
    qDebug(">>> %c <<<\n", x++);
    qDebug(">>>CLOSED<<<\n");
}

void DrawingWindow::customEvent(QEvent *)
{
    mayUpdate();
    qApp->sendPostedEvents(this, QEvent::UpdateLater);
    qApp->sendPostedEvents(this, QEvent::UpdateRequest);
    qApp->sendPostedEvents(this, QEvent::Paint);
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents |
                        QEventLoop::ExcludeSocketNotifiers |
                        QEventLoop::DeferredDeletion |
                        QEventLoop::X11ExcludeTimers);
    qApp->flush();
    qApp->syncX();
    syncMutex.lock();
    syncCondition.wakeAll();
    syncMutex.unlock();
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
    imageMutex.lock();
    QImage imageCopy(*image);
    imageMutex.unlock();
    QRect rect = ev->rect();
    widgetPainter.drawImage(rect, imageCopy, rect);
}

void DrawingWindow::showEvent(QShowEvent *ev)
{
    timer.start(paintInterval, this);
    thread->start_once(QThread::IdlePriority);
    QWidget::showEvent(ev);
}

void DrawingWindow::timerEvent(QTimerEvent *ev)
{
    if (ev->timerId() == timer.timerId()) {
        mayUpdate();
        timer.start(paintInterval, this);
    } else {
        QWidget::timerEvent(ev);
    }
}

//--- DrawingWindow (private methods) ----------------------------------

void DrawingWindow::initialize(DrawingWindow::ThreadFunction f)
{
    terminateThread = false;
    lockCount = 0;
    image = new QImage(width, height, QImage::Format_RGB32);
    painter = new QPainter(image);
    thread = new DrawingThread(*this, f);

    setFocusPolicy(Qt::StrongFocus);
    setFixedSize(image->size());
    setAttribute(Qt::WA_OpaquePaintEvent);
    setFocus();

    setColor("black");
    setBgColor("white");
    clearGraph();

    dirtyFlag = false;
}

inline
void DrawingWindow::applyColor()
{
    QPen pen(painter->pen());
    pen.setColor(fgColor);
    painter->setPen(pen);
}

inline
void DrawingWindow::safeLock(QMutex &mutex)
{
    if (lockCount++ == 0)
        thread->setTerminationEnabled(false);
    mutex.lock();
}

inline
void DrawingWindow::safeUnlock(QMutex &mutex)
{
    mutex.unlock();
    if (--lockCount == 0)
        thread->setTerminationEnabled(true);
}

inline
void DrawingWindow::dirty()
{
    dirtyFlag = true;
    dirtyRect = image->rect();
}

inline
void DrawingWindow::dirty(int x, int y)
{
    dirty(QRect(x, y, 1, 1));
}

inline
void DrawingWindow::dirty(int x1, int y1, int x2, int y2)
{
    QRect r;
    r.setCoords(x1, y1, x2, y2);
    dirty(r.normalized());
}

void DrawingWindow::dirty(const QRect &rect)
{
    if (dirtyFlag) {
        dirtyRect |= rect;
    } else {
        dirtyFlag = true;
        dirtyRect = rect;
    }
}

void DrawingWindow::mayUpdate()
{
    imageMutex.lock();
    bool dirty = dirtyFlag;
    QRect rect = dirtyRect;
    dirtyFlag = false;
    imageMutex.unlock();
    if (dirty)
        update(rect);
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
