#ifndef DRAWING_WINDOW_H
#define DRAWING_WINDOW_H

#define USE_PIXMAP_CACHE

#include <QBasicTimer>
#include <QColor>
#include <QImage>
#include <QMutex>
#include <QPainter>
#ifdef USE_PIXMAP_CACHE
#  include <QPixmap>
#endif
#include <QRect>
#include <QWidget>
#include <Qt>

class DrawingWindow: public QWidget {
/*     Q_OBJECT */

public:
    typedef int (*ThreadFunction)(DrawingWindow &);

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
    void paintEvent(QPaintEvent *ev);
    void showEvent(QShowEvent *ev);
    void timerEvent(QTimerEvent *ev);

private:
    class DrawingThread;

    static const int paintInterval = 33;

    QBasicTimer timer;

    QImage *image;
    QPainter *painter;
#ifdef USE_PIXMAP_CACHE
    QPixmap *pixmap;
#endif

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
    mutex.lock();
}

inline
void DrawingWindow::unlock()
{
    mutex.unlock();
}

#endif // !DRAWING_WINDOW_H
