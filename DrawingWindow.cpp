#include "DrawingWindow.h"
#include <QPainter>

#include <iostream>

DrawingWindow::DrawingWindow(const DrawingArea &a)
    : QWidget()
    , drawingArea(a)
{
    initialize();
}

DrawingWindow::DrawingWindow(QWidget *parent, const DrawingArea &a)
    : QWidget(parent)
    , drawingArea(a)
{
    initialize();
}

DrawingWindow::DrawingWindow(QWidget *parent, Qt::WindowFlags flags,
                             const DrawingArea &a)
    : QWidget(parent, flags)
    , drawingArea(a)
{
    initialize();
}

void DrawingWindow::paintEvent(QPaintEvent *)
{
    std::cerr << "paint!\n";
    QPainter painter(this);
    painter.drawImage(0, 0, drawingArea.getImage());
}

void DrawingWindow::initialize()
{
    setFocusPolicy(Qt::StrongFocus);
    setFixedSize(drawingArea.getImage().size());
    setAttribute(Qt::WA_OpaquePaintEvent);
    connect(&drawingArea, SIGNAL(update()), this, SLOT(update()));

}
