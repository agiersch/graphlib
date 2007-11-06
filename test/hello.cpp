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
#include <DrawingWindow.h>

#include <iostream>

void flip(DrawingWindow &w)
{
    std::cout << "[ " << w.width << " x " << w.height << " ]\n";

    int c = 0;
    int y = 0;
//     int h = w.height;
//     int w = w.width;
    int count = 50;//1 << 31;
    while (1) {
//         std::cerr << "loooooooooooooooooooooop "
//                   << y << " (" << c << ")\n";
        w.setColor(c, c, c);
        for (int yy = y; yy < y + 10; yy++) {
            for (int x = 0; x < w.width; x++)
                w.drawPoint(x, yy);
            w.sync();
        }
        if ((y += 10) >= w.height) {
            y = 0;
            c = !c;
            if (!--count) break;
//             std::cerr << "loooooooooooooooooooooop "
//                       << y << " (" << c << ")\n";
        }
    }
}

void mandel(DrawingWindow &w)
{
    /* paramètres par défaut */
    int larg = w.width;
    int haut = w.height;
    float Rmin = -2.05;
    float Rmax = 0.55;
    float Imin = -1.3;
    float Imax = 1.3;

    int maxiter = 100;

    int x, y;                   /* le pixel considéré */
    float cr, ci;               /* le complexe correspondant */
    float zr, zi;               /* pour calculer la suite */
    float zr2, zi2;
    float pr, pi;               /* taille d'un pixel */
    float rouge, vert, bleu;
    int i;

    pr = (Rmax - Rmin) / larg;
    pi = (Imax - Imin) / haut;

    cr = Rmin;
    for (x = 0; x < larg; x++) {
        ci = Imin;
        for (y = 0; y < haut; y++) {
            /* z_1 = c */
            zr = cr;
            zi = ci;
            for (i = 1; i <= maxiter; i++) {
                zr2 = zr * zr;
                zi2 = zi * zi;
                if (zr2 + zi2 >= 4) {
                    /* |z| >= 2 : on sort de la boucle */
                    break;
                }
                /* on calcule le z suivant */
                zi = 2*zr*zi + ci;
                zr = zr2 - zi2 + cr;
            }
                /* on est sorti trop tôt du for(...):
                   on affiche le pixel d'un couleur en fonction 
                   de i */
                 if (i <= maxiter / 2) {
                    /* entre rouge et vert */
                    vert = (2.0 * i) / maxiter;
                    rouge = 1.0 - vert;
                    bleu = 0.0;
                } else if (i <= maxiter) {
                    /* entre vert et bleu */
                    rouge = 0.0;
                    bleu = (2.0 * i) / maxiter - 1.0;
                    vert = 1.0 - bleu;
                } else /* (i > maxiter) */
                    rouge = vert = bleu = 0.0;
                 w.setColor(rouge, vert, bleu);
                 w.drawPoint(x, y);

            ci += pi;
        }
        cr += pr;
//         w.sync();
    }
}

void lines(DrawingWindow &w)
{
    int n = 100000;
    int xmax = w.width;
    int ymax = w.height;
    while (n-- > 0) {
        double r = rand() / (float )RAND_MAX;
        double g = rand() / (float )RAND_MAX;
        double b = rand() / (float )RAND_MAX;
        int x1 = rand() % xmax;
        int y1 = rand() % ymax;
        int x2 = rand() % xmax;
        int y2 = rand() % ymax;
        w.setColor(r, g, b);
        w.drawLine(x1, y1, x2, y2);
        w.sync();
    }
}

void rectangles(DrawingWindow &w)
{
    int d = 5;
    int z = (w.width > w.height ? w.height : w.width) / 2;
    z = d * (z / d);
    while (z > 0) {
        w.drawRect(z, z, w.width - 1 - z, w.height - 1 - z);
        z -= d;
    }
}

int main(int argc, char *argv[])
{
    const int w = 700;
    const int h = 700;
    QApplication application(argc, argv);

    const int nf = 1;
    const int nm = 1;
    DrawingWindow *dw[nf + nm];

    for (int i = 0; i < nf; ++i)
        dw[i] = new DrawingWindow(flip, w, h);
    for (int i = nf; i < nf + nm; ++i)
        dw[i] = new DrawingWindow(mandel, w, h);

    for (int i = 0; i < nf + nm; ++i)
        dw[i]->show();

    DrawingWindow dr(rectangles, w, h);
    dr.show();

    DrawingWindow dl(lines, w, h);
    dl.show();

    return application.exec();
}
