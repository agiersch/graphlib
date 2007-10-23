#ifndef DRAWING_AREA_H
#define DRAWING_AREA_H

#include <QColor>
#include <QImage>
#include <QObject>
#include <QPainter>
#include <QRect>
#include <QSize>
#include <QMutex>

class DrawingArea: public QObject {
/*     Q_OBJECT */

public:
    static const int DEFAULT_WIDTH = 640;
    static const int DEFAULT_HEIGHT = 480;

    DrawingArea(int width = DEFAULT_WIDTH, int height = DEFAULT_HEIGHT);
    ~DrawingArea();

    int width() const;
    int height() const;
    const QSize size() const;

    void setColor(const QColor &color);
    void setColor(float red, float green, float blue);

    void drawPoint(int x, int y);
    void drawLine(int x1, int y1, int x2, int y2);

    QImage &getImage();

    bool isDirty() const;
    void setDirty();
    void setDirty(const QRect &rect);
    void setClean();

    QRect getDirtyRect() const;

    void lock();
    void unlock();

private:
    QImage *image;
    QPainter *painter;
    bool dirtyFlag;
    QRect dirtyRect;
    QMutex mutex;
};

inline
int DrawingArea::width() const
{
    return image->width();
}

inline
int DrawingArea::height() const
{
    return image->height();
}

inline
const QSize DrawingArea::size() const
{
    return image->size();
}

inline
QImage &DrawingArea::getImage()
{
    return *image;
}

inline
bool DrawingArea::isDirty() const
{
    return dirtyFlag;
}

inline
QRect DrawingArea::getDirtyRect() const
{
    return dirtyRect;
}

inline
void DrawingArea::lock()
{
    mutex.lock();
}

inline
void DrawingArea::unlock()
{
    mutex.unlock();
}

#endif // !DRAWING_AREA_H
