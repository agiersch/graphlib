#ifndef DRAWING_AREA_H
#define DRAWING_AREA_H

#include <QColor>
#include <QImage>
#include <QObject>
#include <QPainter>

class DrawingArea: public QObject {
    Q_OBJECT

public:
    static const int DEFAULT_WIDTH = 640;
    static const int DEFAULT_HEIGHT = 480;

    DrawingArea(int width = DEFAULT_WIDTH, int height = DEFAULT_HEIGHT);
    ~DrawingArea();

    int width() const;
    int height() const;

    void setColor(const QColor &color);
    void setColor(float red, float green, float blue);

    void drawPoint(int x, int y);
    void drawLine(int x1, int y1, int x2, int y2);

    const QImage &getImage() const;

signals:
    void update();

private:
    QImage *image;
    QPainter *painter;

};

#endif // !DRAWING_AREA_H



