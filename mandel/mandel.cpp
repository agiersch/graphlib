#include <DrawingWindow.h>
#include <QApplication>
#include <iostream>

void do_mandel(DrawingWindow &w,
               double Rmin, double Rmax, double Imin, double Imax,
               int maxiter)
{
    int x, y;                   // le pixel consid�r�
    double cr, ci;              // le complexe correspondant
    double zr, zi;              // pour calculer la suite
    double zr2, zi2;
    double pr, pi;              // taille d'un pixel
    double rouge, vert, bleu;
    int i;

    pr = (Rmax - Rmin) / (w.width - 1);
    pi = (Imax - Imin) / (w.height - 1);
    
    cr = Rmin;
    for (x = 0; x < w.width; x++) {
        ci = Imax;
        for (y = 0; y < w.height; y++) {
            // z_1 = c
            zr = cr;
            zi = ci;
            for (i = 1; i <= maxiter; i++) {
                zr2 = zr * zr;
                zi2 = zi * zi;
                if (zr2 + zi2 >= 4) {
                    // |z| >= 2 : on sort de la boucle
                    break;
                }
                // on calcule le z suivant
                zi = 2 * zr * zi + ci;
                zr = zr2 - zi2 + cr;
            }
            if (i > maxiter) {
                rouge = vert = bleu = 0.0;
            } else {
                // on est sorti trop t�t du for(...): on affiche le
                // pixel d'un couleur en fonction de i
                int ii = (maxiter - i) % 96;
                if (ii < 32) {
                    // vert -> bleu
                    bleu = ii / 32.0;
                    vert = 1.0 - bleu;
                    rouge = 0.0;
                } else if (ii < 64) {
                    // bleu -> rouge
                    rouge = (ii - 32) / 32.0;
                    bleu = 1.0 - rouge;
                    vert = 0.0;
                } else {
                    // rouge -> vert
                    vert = (ii - 64) / 32.0;
                    rouge = 1.0 - vert;
                    bleu = 0.0;
                }
            }
            w.setColor(rouge, vert, bleu);
            w.drawPoint(x, y);

            ci -= pi;
        }
        cr += pr;
    }
}

void mandel(DrawingWindow &w)
{
    // nombre max d'it�rations
    int maxiter = 500;
    // zone d'int�r�t par d�faut
    double Rmin = -2.05;
    double Rmax = 0.55;
    double Imin = -1.3;
    double Imax = 1.3;
    // cible du zoom
    double Tr;
    double Ti;

    while (1) {
        do_mandel(w, Rmin, Rmax, Imin, Imax, maxiter);

        w.setColor("white");
        w.drawText(5, 5, "Cliquer sur l'image pour zoomer");

        int x, y;
        int button;
        w.waitMousePress(x, y, button);

        // calcul des coordonn�es du point cliqu�
        Tr = Rmin + x * (Rmax - Rmin) / (w.width - 1);
        Ti = Imax - y * (Imax - Imin) / (w.height - 1);

        // calcul de la nouvelle zone d'int�r�t :
        // zoom �2 en direction du point cliqu�
        const int zoom = 2;
        double Rmin2 = Rmin / zoom;
        double Rmax2 = Rmax / zoom;
        double Imin2 = Imin / zoom;
        double Imax2 = Imax / zoom;
        double Rshift = Tr - (Rmin2 + Rmax2) / 2;
        double Ishift = Ti - (Imin2 + Imax2) / 2;
        Rmin2 += Rshift;
        Rmax2 += Rshift;
        Imin2 += Ishift;
        Imax2 += Ishift;

        // affichage d'un rectangle autour de la nouvelle zone d'int�r�t
        double x_factor = (w.width - 1) / (Rmax - Rmin);
        double y_factor = (w.height - 1) / (Imax - Imin);
        w.setColor("white");
        w.drawRect((Rmin2 - Rmin) * x_factor, (Imax - Imin2) * y_factor,
                   (Rmax2 - Rmin) * x_factor, (Imax - Imax2) * y_factor);

        Rmin = Rmin2;
        Rmax = Rmax2;
        Imin = Imin2;
        Imax = Imax2;
    }
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    DrawingWindow win(mandel, 800, 800);
    win.show();
    return app.exec();
}
