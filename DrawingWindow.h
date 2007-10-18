#ifndef DRAWING_WINDOW_H
#define DRAWING_WINDOW_H

#include "DrawingArea.h"
#include <QWidget>

class DrawingWindow: public QWidget {
public:
    DrawingWindow(const DrawingArea &a);
    DrawingWindow(QWidget *parent, const DrawingArea &a);
    DrawingWindow(QWidget *parent, Qt::WindowFlags flags,
                  const DrawingArea &a);

protected:
    void paintEvent(QPaintEvent *e);

private:
    const DrawingArea &drawingArea;

    void initialize();
};

#endif // !DRAWING_WINDOW_H
