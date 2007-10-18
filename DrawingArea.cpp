#include <DrawingArea.h>

DrawingArea::DrawingArea(int width, int height)
{
    image = new QImage(width, height, QImage::Format_RGB32);
    image->fill(QColor(Qt::white).rgb());
    painter = new QPainter(image);
}

DrawingArea::~DrawingArea()
{
    delete painter;
    delete image;
}

int DrawingArea::width() const
{
    return image->width();
}

int DrawingArea::height() const
{
    return image->height();
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
    painter->drawPoint(x, y);
    emit update();
}

void DrawingArea::drawLine(int x1, int y1, int x2, int y2)
{
    painter->drawLine(x1, y1, x2, y2);
    emit update();
}

const QImage &DrawingArea::getImage() const
{
    return *image;
}

