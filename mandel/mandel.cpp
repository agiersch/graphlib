#include <DrawingWindow.h>
#include <QApplication>
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

struct parameters {
    // nombre max d'itérations
    int maxiter;
    // zone d'intérêt par défaut
    double Rmin;
    double Rmax;
    double Imin;
    double Imax;
    // facteur d'échelle
    double Rscale;
    double Iscale;
};

const parameters initial_parameters = {
    1000,                       // maxiter
    -2.05,                      // Rmin
    0.55,                       // Rmax
    -1.3,                       // Imin
    1.3,                        // Imax
    0.0,                        // Rscale
    0.0,                        // Iscale
};

static inline double sqr(double x)
{
    return x * x;
}

static int check_point(parameters& p, double cr, double ci)
{
    double zr2, zi2;
    zi2 = sqr(ci);
    if (sqr(cr + 1) + zi2 < 1.0 / 16.0)
        return p.maxiter;
    double x4 = cr - 1.0 / 4.0;
    double q = sqr(x4) + zi2;
    if (q * (q + x4) < zi2 / 4.0)
        return p.maxiter;
    zr2 = sqr(cr);
    double zr = cr;
    double zi = ci;
    int i;
    for (i = 0 ; i < p.maxiter && zr2 + zi2 < 4 ; i++) {
        zi = 2 * zr * zi + ci;
        zr = zr2 - zi2 + cr;
        zr2 = sqr(zr);
        zi2 = sqr(zi);
    }
    return i;
}

// int check_point(parameters& p, int x, int y)
// {
//     double cr = p.Rmin + x * p.Rscale;
//     double ci = p.Imax - y * p.Iscale;
//     return check_point(p, cr, ci);
// }

static void set_color(DrawingWindow& w, parameters& p, int i)
{
    double rouge, vert, bleu;
    if (i >= p.maxiter) {
        rouge = vert = bleu = 0.0;
    } else {
        int ii = (p.maxiter - 1 - i) % 96;
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
}

// Fonction de dessin de l'ensemble de Madelbrot, dans la zone
// spécifiée, et avec la précision souhgaitée.
static void do_mandel(DrawingWindow& w, parameters& p)
{
    int x, y;                   // le pixel considéré
    double cr, ci;              // le complexe correspondant

    int kmax = 4;
    for (int k = kmax ; k != 0 ; k /= 2) {
        int kk = 2 * k;
        for (y = 0 ; y < w.height ; y += k) {
            ci = p.Imax - y * p.Iscale;
            cr = p.Rmin;
            int x0 = 0;
            int i0 = check_point(p, cr, ci);
            for (x = 1 ; x < w.width ; x += k) {
                cr = p.Rmin + x * p.Rscale;
                if (x % kk != 0 || y % kk != 0 || k == kmax) {
                    int i = check_point(p, cr, ci);
                    if (i != i0) {
                        set_color(w, p, i0);
                        if (k == 1)
                            w.drawLine(x0, y, x - 1, y);
                        else
                            w.fillRect(x0, y, x - 1, y + k - 1);
                        i0 = i;
                        x0 = x;
                    }
                }
            }
            set_color(w, p, i0);
            w.fillRect(x0, y, w.width - 1, y + k - 1);
        }
    }
}
    
// Fonction de dessin principale, calcule la zone d'intérêt, appelle
// do_mandel(), pour dessiner l'ensemle, et permet le zoom.
static void mandel(DrawingWindow &w)
{
    parameters p = initial_parameters;
    while (1) {
        p.Rscale = (p.Rmax - p.Rmin) / (w.width - 1);
        p.Iscale = (p.Imax - p.Imin) / (w.height - 1);
        do_mandel(w, p);

        w.setColor("white");
        w.drawText(5, 5, "Cliquer sur l'image pour zoomer");

        int x, y;
        int button;
        w.waitMousePress(x, y, button);

        // calcul des coordonnées du point cliqué
        double Tr = p.Rmin + x * p.Rscale;
        double Ti = p.Imax - y * p.Rscale;

        // calcul de la nouvelle zone d'intérêt :
        // zoom ×2 en direction du point cliqué
        const int zoom = 2;
        double Rmin2 = p.Rmin / zoom;
        double Rmax2 = p.Rmax / zoom;
        double Imin2 = p.Imin / zoom;
        double Imax2 = p.Imax / zoom;
        double Rshift = Tr - (Rmin2 + Rmax2) / 2;
        double Ishift = Ti - (Imin2 + Imax2) / 2;
        Rmin2 += Rshift;
        Rmax2 += Rshift;
        Imin2 += Ishift;
        Imax2 += Ishift;

        // affichage d'un rectangle autour de la nouvelle zone d'intérêt
        w.setColor("white");
        w.drawRect((Rmin2 - p.Rmin) / p.Rscale, (p.Imax - Imin2) / p.Iscale,
                   (Rmax2 - p.Rmin) / p.Rscale, (p.Imax - Imax2) / p.Iscale);

        p.Rmin = Rmin2;
        p.Rmax = Rmax2;
        p.Imin = Imin2;
        p.Imax = Imax2;
    }
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    DrawingWindow win(mandel, 800, 800);
    win.show();
    return app.exec();
}
