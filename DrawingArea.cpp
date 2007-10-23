#include <DrawingArea.h>

DrawingArea::DrawingArea(int width, int height)
{
    image = new QImage(width, height, QImage::Format_RGB32);
    image->fill(QColor(Qt::white).rgb());
    painter = new QPainter(image);
    setDirty();
}

DrawingArea::~DrawingArea()
{
    delete painter;
    delete image;
}

void DrawingArea::setColor(const QColor &color)
{
    QPen pen(painter->pen());
    pen.setColor(color);
    painter->setPen(pen);
}

void DrawingArea::setColor(float red, float green, float blue)
{
    QColor color;
    color.setRgbF(red, green, blue);
    this->setColor(color);
}

void DrawingArea::drawPoint(int x, int y)
{
    lock();
    painter->drawPoint(x, y);
    setDirty(QRect(x, y, 1, 1));
    unlock();
}

void DrawingArea::drawLine(int x1, int y1, int x2, int y2)
{
    lock();
    painter->drawLine(x1, y1, x2, y2);
    if (x1 > x2)
        std::swap(x1, x2);
    if (y1 > y2)
        std::swap(y1, y2);
    setDirty(QRect(x1, y1, x2 - x1 + 1, y2 - y1 + 1));
    unlock();
}

void DrawingArea::setDirty()
{
    setDirty(QRect(0, 0, width(), height()));
}

void DrawingArea::setDirty(const QRect &rect)
{
    if (dirtyFlag)
        dirtyRect |= rect;
    else
        dirtyRect = rect;
    dirtyFlag = true;
}

void DrawingArea::setClean()
{
    dirtyFlag = false;
}

