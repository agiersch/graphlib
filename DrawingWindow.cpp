#include "DrawingWindow.h"
#include <QPainter>
#include <QPaintEvent>

#include <iostream>

DrawingWindow::DrawingWindow(DrawingArea &a)
    : QWidget()
    , drawingArea(a)
    , pixmap(a.size())
{
    initialize();
}

DrawingWindow::DrawingWindow(QWidget *parent, DrawingArea &a)
    : QWidget(parent)
    , drawingArea(a)
    , pixmap(a.size())
{
    initialize();
}

DrawingWindow::DrawingWindow(QWidget *parent, Qt::WindowFlags flags,
                             DrawingArea &a)
    : QWidget(parent, flags)
    , drawingArea(a)
    , pixmap(a.size())
{
    initialize();
}

void DrawingWindow::paintEvent(QPaintEvent *ev)
{
    QRect rect = ev->rect();
    drawingArea.lock();
    if (drawingArea.isDirty()) {
        QPainter pixmapPainter(&pixmap);
        pixmapPainter.drawImage(drawingArea.getDirtyRect(),
                                drawingArea.getImage(),
                                drawingArea.getDirtyRect());
        drawingArea.setClean();
        rect |= drawingArea.getDirtyRect();
    }
    drawingArea.unlock();
    QPainter painter(this);
    painter.drawPixmap(0, 0, pixmap);
}

void DrawingWindow::timerEvent(QTimerEvent *ev)
{
    if (ev->timerId() == timer.timerId()) {
        update();
    } else {
        QWidget::timerEvent(ev);
    }
}

void DrawingWindow::initialize()
{
    setFocusPolicy(Qt::StrongFocus);
    setFixedSize(drawingArea.getImage().size());
    setAttribute(Qt::WA_OpaquePaintEvent);
    timer.start(50, this);
}
