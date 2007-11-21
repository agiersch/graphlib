#ifndef DRAWING_WINDOW_H
#define DRAWING_WINDOW_H

#include <QBasicTimer>
#include <QColor>
#include <QImage>
#include <QMutex>
#include <QPainter>
#include <QPen>
#include <QRect>
#include <QWaitCondition>
#include <QWidget>
#include <Qt>

class DrawingThread;

class DrawingWindow: public QWidget {
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

    const int width;
    const int height;

    // http://www.w3.org/TR/SVG/types.html#ColorKeywords
    void setColor(unsigned int color);
    void setColor(const char *name);
    void setColor(float red, float green, float blue);

    void setBgColor(unsigned int color);
    void setBgColor(const char *name);
    void setBgColor(float red, float green, float blue);

    void clearGraph();

    void drawPoint(int x, int y);
    void drawLine(int x1, int y1, int x2, int y2);
    void drawRect(int x1, int y1, int x2, int y2);
    void fillRect(int x1, int y1, int x2, int y2);
    void drawCircle(int x, int y, int r);
    void fillCircle(int x, int y, int r);

    void drawText(int x, int y, const char *text, int flags = 0);
    void drawTextBg(int x, int y, const char *text, int flags = 0);

    unsigned int getPointColor(int x, int y);

    bool sync(unsigned long time = ULONG_MAX);

    void closeGraph();

    void sleep(unsigned long secs);
    void msleep(unsigned long msecs);
    void usleep(unsigned long usecs);

protected:
    void closeEvent(QCloseEvent *ev);
    void customEvent(QEvent *ev);
    void keyPressEvent(QKeyEvent *ev);
    void paintEvent(QPaintEvent *ev);
    void showEvent(QShowEvent *ev);
    void timerEvent(QTimerEvent *ev);

private:
    static const int paintInterval = 33;

    QBasicTimer timer;
    QMutex imageMutex;
    QMutex syncMutex;
    QWaitCondition syncCondition;
    bool terminateThread;
    int lockCount;

    QImage *image;
    QPainter *painter;

    bool dirtyFlag;
    QRect dirtyRect;

    DrawingThread *thread;

    void initialize(ThreadFunction f);

    void setColor(const QColor& color);
    void setBgColor(const QColor& color);
    QColor getColor();
    QColor getBgColor();

    void safeLock(QMutex &mutex);
    void safeUnlock(QMutex &mutex);

    void dirty();
    void dirty(int x, int y);
    void dirty(int x1, int y1, int x2, int y2);
    void dirty(const QRect &rect);

    void mayUpdate();
};

#endif // !DRAWING_WINDOW_H
