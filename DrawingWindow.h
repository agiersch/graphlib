#ifndef DRAWING_WINDOW_H
#define DRAWING_WINDOW_H

#include "DrawingArea.h"
#include <QPixmap>
#include <QTimer>
#include <QWidget>

class DrawingWindow: public QWidget {
/*     Q_OBJECT */

public:
    DrawingWindow(DrawingArea &a);
    DrawingWindow(QWidget *parent, DrawingArea &a);
    DrawingWindow(QWidget *parent, Qt::WindowFlags flags, DrawingArea &a);

protected:
    void paintEvent(QPaintEvent *ev);
    void timerEvent(QTimerEvent *ev);

private:
    DrawingArea &drawingArea;
    QPixmap pixmap;
    QBasicTimer timer;

    void initialize();
};

#endif // !DRAWING_WINDOW_H
