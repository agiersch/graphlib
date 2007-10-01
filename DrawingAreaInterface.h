#ifndef DRAWING_AREA_INTERFACE_H
#define DRAWING_AREA_INTERFACE_H

class DrawingAreaInterface {
public:
    static const int DEFAULT_WIDTH = 640;
    static const int DEFAULT_HEIGHT = 480;

    virtual ~DrawingAreaInterface() { }

    virtual void setColor(float red, float green, float blue) = 0;
    virtual void drawPoint(int x, int y) = 0;
    virtual void drawLine(int x1, int y1, int x2, int y2) = 0;
    
    virtual void wait() = 0;
    virtual void waitAll() = 0;
};

#endif // !DRAWING_AREA_INTERFACE_H
