/*
 * Pour compiler et exécuter
 * =========================
 *
 * 1. Créer le fichier exemple.pro :
 *      +------------------------------------------------------------+
 *      |TEMPLATE = app                                              |
 *      |TARGET = exemple                                              |
 *      |CONFIG += qt                                                |
 *      |CONFIG += debug                                             |
 *      |HEADERS += DrawingWindow.h                                  |
 *      |SOURCES += DrawingWindow.cpp                                |
 *      |SOURCES += exemple.cpp                                      |
 *      +------------------------------------------------------------+
 *
 * 2. Créer le fichier Makefile avec la commande :
 *      $ qmake-qt4 exemple.pro
 *    ou tout simplement :
 *      $ qmake-qt4
 *
 * 3. Compiler avec la commande :
 *      $ make exemple
 *    ou tou simplement :
 *      $ make
 *
 * 4. Exécuter le programme avec la commande :
 *      $ ./exemple
 */

#include <QApplication>
#include <DrawingWindow.h>
#include <algorithm>
#include <cstdlib>

float frand()
{
    return rand() / (float )RAND_MAX;
}

void exemple1(DrawingWindow &w)
{
    const int cx = w.width / 2;
    const int cy = w.height / 2;
    const int delta = 5;
    for (int x = 0; x < w.width; x += delta) {
        w.drawLine(cx, cy, x, 0);
        w.drawLine(cx, cy, w.width - 1 - x, w.height - 1);
    }
    for (int y = 0; y < w.height; y += delta) {
        w.drawLine(cx, cy, 0, w.height - 1 - y);
        w.drawLine(cx, cy, w.width - 1, y);
    }
}

void exemple2(DrawingWindow &w)
{
    int width = std::min(w.width, w.height) / 2;
    for (int z = 0; z <= width; z++) {
        float r, g, b;
        float s = 3.0 * z / width;
        if (z <= width / 3.0) {
            r = 1.0 - s;
            g = s;
            b = 0.0;
        } else if (z <= 2.0 * width / 3.0) {
            s -= 1.0;
            r = 0.0;
            g = 1.0 - s;
            b = s;
        } else {
            s -= 2.0;
            r = s;
            g = 0.0;
            b = 1.0 - s;
        }
        w.setColor(r, g, b);
        w.drawRect(z, z, w.width - 1 - z, w.height - 1 - z);
    }
}

void exemple3(DrawingWindow &w)
{
    while (1) {
        int x1 = rand() % w.width;
        int y1 = rand() % w.height;
        int x2 = rand() % w.width;
        int y2 = rand() % w.height;
        w.setColor(frand(), frand(), frand());
        w.drawLine(x1, y1, x2, y2);
        w.sync();
    }
}

int main(int argc, char *argv[])
{
    QApplication application(argc, argv);
    DrawingWindow window1(exemple1, 640, 480);
    DrawingWindow window2(exemple2, 640, 480);
    DrawingWindow window3(exemple3, 640, 480);

    window1.setWindowTitle("Exemple 1");
    window2.setWindowTitle("Exemple 2");
    window3.setWindowTitle("Exemple 3");

    window1.show();
    window2.show();
    window3.show();

    return application.exec();
}
