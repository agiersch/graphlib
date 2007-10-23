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
    int h = a.height();
    int w = a.width();
    int count = 10;
    while (1) {
//         std::cerr << "loooooooooooooooooooooop "
//                   << y << " (" << c << ")\n";
        a.setColor(c, c, c);
        for (int yy = y; yy < y + 10; yy++)
            for (int x = 0; x < w; x++)
                a.drawPoint(x, yy);
        if ((y += 10) >= h) {
            y = 0;
            c = !c;
            if (!--count) break;
            std::cerr << "loooooooooooooooooooooop "
                      << y << " (" << c << ")\n";
        }
    }
    return 0;
}

int main(int argc, char *argv[])
{
    QApplication application(argc, argv);
    DrawingArea drawingArea(800, 600);
    DrawingWindow drawingWindow(drawingArea);
    DrawingThread drawingThread(drawingArea, main_drawing_thread);

    drawingWindow.show();
    drawingThread.start();

    return application.exec();
}
