/*
 * Pour compiler
 * =============
 *
 * 1. Créer le fichier hello.pro :
 *      +------------------------------------------------------------+
 *      |TARGET = hello                                              |
 *      |CONFIG += qt debug                                          |
 *      |SOURCES += hello.cc                                         |
 *      +------------------------------------------------------------+
 *
 * 2. Créer le fichier Makefile avec la commande :
 *      $ qmake -makefile hello.pro
 *    ou tout simplement :
 *      $ qmake -makefile
 *
 * 3. Compiler avec la commande :
 *      $ make hello
 *    ou tou simplement :
 *      $ make
 */


#include <QApplication>

#include <DrawingArea.h>
#include <DrawingThread.h>
#include <DrawingWindow.h>

#include <iostream>

int main_drawing_thread(DrawingArea &a)
{
    std::cout << "[ " << a.width() << " x " << a.height() << " ]\n";

    int c = 0;
    int y = 0;
    while (1) {
        std::cerr << "loooooooooooooooooooooop "
                  << y << " (" << c << ")\n";
        a.setColor(c, c, c);
        for (int yy = y; yy < y + 10; yy++)
            for (int x = 0; x < a.width(); x++)
                a.drawPoint(x, yy);
        if ((y += 10) >= a.height()) {
            y = 0;
            c = !c;
        }
        sleep(1);
    }
    return 0;
}

int main(int argc, char *argv[])
{
    QApplication application(argc, argv);
    DrawingArea drawingArea(800, 600);
    DrawingWindow drawingWindow(drawingArea);
    DrawingThread drawingThread(drawingArea, main_drawing_thread);

    QObject::connect(&drawingArea, SIGNAL(update()),
                     &drawingWindow, SLOT(update()));

    drawingWindow.show();
    drawingThread.start();

    return application.exec();
}
