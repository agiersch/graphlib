#ifndef DRAWING_WINDOW_H
#define DRAWING_WINDOW_H

#include <QWidget>
#include <Qt>

class DrawingWindowPrivate;

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

    void setColor(float red, float green, float blue);
    void setBgColor(float red, float green, float blue);

    void clearGraph();

    void drawPoint(int x, int y);
    void drawLine(int x1, int y1, int x2, int y2);
    void drawRect(int x1, int y1, int x2, int y2);

    bool sync(unsigned long time = ULONG_MAX);

    void sleep(unsigned long secs);
    void msleep(unsigned long msecs);
    void usleep(unsigned long usecs);

protected:
    void closeEvent(QCloseEvent *ev);
    void paintEvent(QPaintEvent *ev);
    void showEvent(QShowEvent *ev);
    void timerEvent(QTimerEvent *ev);

private:
    DrawingWindowPrivate * const d;

    friend class DrawingWindowPrivate;
};

#endif // !DRAWING_WINDOW_H
