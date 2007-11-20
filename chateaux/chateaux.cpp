#include <QApplication>
#include <DrawingWindow.h>
#include <cmath>
#include <iostream>
#include <ctime>
#include <cstdlib>

/* Note : les coordonnées réelles vont de -100 à +100 en abscisse, et
 *  de -10 à +140 en ordonnée
 */

const float rXMin = -100.0;
const float rXMax = 100.0;
const float rYMin = -10.0;
const float rYMax = 140.0;

const float hauteurMin = 10;
const float hauteurMax = 130;
const float largeurMin = 40;
const float largeurMax = 150;

const float positionChateau1 = -85.0;
const float positionChateau2 = 85.0;

/* Retourne un nombre pseudo-aléatoire compris entre 0 et le paramètre
 * 'max' (exclus)
 */
float frand(float min, float max)
{
    static bool first = true;
    if (first) {
        srand(time(NULL));
        first = false;
    }
    return min + (max - min)* (rand() / (RAND_MAX + 1.0));
}

// conversion coordonnées réelles -> coordonnées fenêtre
int rtowX(const DrawingWindow& w, float rx)
{
    return (int )roundf(w.width * (rx - rXMin) / (rXMax - rXMin + 1.0));
}

int rtowY(const DrawingWindow& w, float ry)
{
    return (int )roundf(w.height * (rYMax - ry) / (rYMax - rYMin + 1.0));
}

// conversion coordonnées réelles -> coordonnées fenêtre
float wtorX(const DrawingWindow& w, int wx)
{
    return (rXMax - rXMin + 1.0) * wx / w.width + rXMin;
}

float wtorY(const DrawingWindow& w, int wy)
{
    return -(rYMax - rYMin + 1.0) * wy / w.height - rYMax;
}

void dessineTerrain(DrawingWindow& w, float largeur, float hauteur)
{
    float l = largeur / 2.0;
    float h = hauteur;
    int y0 = rtowY(w, 0) + 1;
    int xmin = rtowX(w, -l) - 1;
    int xmax = rtowX(w, l) + 1;
    w.setColor("forestgreen");
    for (int x = xmin; x <= xmax; x++) {
        float rx = wtorX(w, x) / l;
        float ry = h * (1.0 - rx * rx);
        int y = rtowY(w, ry);
        if (y <= y0)
            w.drawLine(x, y0, x, y);
    }
    w.setColor("maroon");
    w.fillRect(0, y0 + 1, w.width - 1, w.height - 1);
}

void dessineChateau(DrawingWindow& w, float position)
{
    w.setColor("black");
    w.setColor("darkslategray");
    int y1 = rtowY(w, 0);
    int h0 = rtowY(w, 3);
    int h1 = rtowY(w, 4);
    for (int i = 0; i < 7; i++) {
        int h = i % 2 ? h0 : h1;
        int x1 = rtowX(w, position + i - 3.5);
        int x2 = rtowX(w, position + i - 2.5) - 1;
        w.fillRect(x1, y1, x2, h);
    }
    w.setColor("dimgray");
    h0 = rtowY(w, 6);
    h1 = rtowY(w, 7);
    for (int i = 0; i < 5; i++) {
        int h = i % 2 ? h0 : h1;
        int x1 = rtowX(w, position + i - 8.5);
        int x2 = rtowX(w, position + i - 7.5) - 1;        
        w.fillRect(x1, y1, x2, h);
        x1 = rtowX(w, position + i + 3.5);
        x2 = rtowX(w, position + i + 4.5) - 1;        
        w.fillRect(x1, y1, x2, h);
    }
}

void dessineExplosion(DrawingWindow& w, float rx, float ry)
{
    const int maxray = rtowX(w, 5) - rtowX(w, 0);
    // 1/2 rouge -> rouge -> jaune
    const int x = rtowX(w, rx);
    const int y = rtowY(w, ry);
    int i;
    for (i = 0; i <= maxray / 3; i++) {
        w.setColor(0.5 + 3.0 * i / (2.0 * maxray), 0.0, 0.0);
        w.drawCircle(x, y, i);
        w.msleep(20);
    }
    for (/* i */; i < maxray; i++) {
        w.setColor(1.0, 1.5 * i / maxray - 0.5, 0.0);
        w.drawCircle(x, y, i);
        w.msleep(20);
    }
    w.setColor("white");
    for (i = 0; i < maxray; i++) {
        w.drawCircle(x, y, i);
        w.msleep(20);
    }
    w.fillCircle(x, y, maxray - 1);
}

void jeu(DrawingWindow& w)
{
    dessineTerrain(w, frand(largeurMin, largeurMax),
                   frand(hauteurMin, hauteurMax));
    dessineChateau(w, positionChateau1);
    dessineChateau(w, positionChateau2);

    while (1)
        dessineExplosion(w, frand(rXMin, rXMax), frand(rYMin, rYMax));
    
}

int main(int argc, char *argv[])
{
    QApplication application(argc, argv);
    DrawingWindow window(jeu, 640, 480);
    window.show();
    return application.exec();
}

