#ifndef DRAWING_AREA_H
#define DRAWING_AREA_H

#include <DrawingAreaInterface.h>

#include <QColor>
#include <QImage>
#include <QPainter>

class DrawingArea: public DrawingAreaInterface {
private:
    QImage *image;
    QPainter *painter;

public:
    DrawingArea(int width = DEFAULT_WIDTH, int height = DEFAULT_HEIGHT);
    ~DrawingArea();

    int width() const;
    int height() const;

    void setColor(const QColor &color);
    void setColor(float red, float green, float blue);

    void drawPoint(int x, int y);
    void drawLine(int x1, int y1, int x2, int y2);

};

#endif // !DRAWING_AREA_H



