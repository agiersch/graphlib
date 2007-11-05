#ifndef DRAWING_WINDOW_H
#define DRAWING_WINDOW_H

#include <QBasicTimer>
#include <QColor>
#include <QImage>
#include <QMutex>
#include <QPainter>
#include <QRect>
#include <QWidget>
#include <Qt>

#include <QThread>

class DrawingWindow: public QWidget {
/*     Q_OBJECT */

public:
    typedef void (*ThreadFunction)(DrawingWindow &);

    static const int DEFAULT_WIDTH = 640;
    static const int DEFAULT_HEIGHT = 480;

    DrawingWindow(ThreadFunction fun,
                  int width = DEFAULT_WIDTH, int height = DEFAULT_HEIGHT);
    DrawingWindow(QWidget *parent,
                  ThreadFunction fun,
                  int width = DEFAULT_WIDTH, int height = DEFAULT_HEIGHT);
    DrawingWindow(QWidget *parent, Qt::WindowFlags flags,
                  ThreadFunction fun,
                  int width = DEFAULT_WIDTH, int height = DEFAULT_HEIGHT);

    ~DrawingWindow();

    int width() const;
    int height() const;

    void setColor(float red, float green, float blue);
    void setColor(const QColor &color);

    void drawPoint(int x, int y);
    void drawLine(int x1, int y1, int x2, int y2);

protected:
    void closeEvent(QCloseEvent *ev);
    void paintEvent(QPaintEvent *ev);
    void showEvent(QShowEvent *ev);
    void timerEvent(QTimerEvent *ev);

private:
    class DrawingThread: public QThread {
    public:
        DrawingThread(DrawingWindow &w, ThreadFunction f);
        void run();

        void enableTerminate();
        void disableTerminate();

    private:
        DrawingWindow &drawingWindow;
        ThreadFunction threadFunction;
        
    };

    static const int paintInterval = 33;

    QBasicTimer timer;

    QImage *image;
    QPainter *painter;

    DrawingThread *thread;
    bool thread_started;

    bool dirtyFlag;
    QRect dirtyRect;

    QMutex mutex;

    void initialize(ThreadFunction fun, int width, int height);

    void lock();
    void unlock();

    void setDirtyRect();
    void setDirtyRect(int x, int y);
    void setDirtyRect(int x1, int y1, int x2, int y2);
    void setDirtyRect(const QRect &rect);
};

inline
int DrawingWindow::width() const
{
    return image->width();
}

inline
int DrawingWindow::height() const
{
    return image->height();
}

inline
void DrawingWindow::lock()
{
    thread->disableTerminate();
    mutex.lock();
}

inline
void DrawingWindow::unlock()
{
    mutex.unlock();
    thread->enableTerminate();
}

inline
void DrawingWindow::setDirtyRect()
{
    dirtyFlag = true;
    dirtyRect.setRect(0, 0, width(), height());
}

inline
void DrawingWindow::setDirtyRect(int x, int y)
{
    setDirtyRect(QRect(x, y, 1, 1));
}

inline
void DrawingWindow::setDirtyRect(int x1, int y1, int x2, int y2)
{
    QRect r;
    r.setCoords(x1, y1, x2, y2);
    setDirtyRect(r.normalized());
}

inline
void DrawingWindow::DrawingThread::enableTerminate()
{
    if (currentThread() == this)
        setTerminationEnabled(true);
}

inline
void DrawingWindow::DrawingThread::disableTerminate()
{
    if (currentThread() == this)
        setTerminationEnabled(false);
}

#endif // !DRAWING_WINDOW_H
