#include <QApplication>
#include <DrawingWindow.h>
#include <cmath>
#include <iostream>
#include <ctime>
#include <cstdlib>
#include <sstream>

/* Note : les coordonnées réelles vont de -100 à +100 en abscisse, et
 *  de -10 à +140 en ordonnée
 */

const float PI = 4.0 * atan(1.0);

const float rXMin = -100.0;
const float rXMax = 100.0;
const float rYMin = -10.0;
const float rYMax = 140.0;

const float hauteurMin = 10;
const float hauteurMax = 130;
const float largeurMin = 40;
const float largeurMax = 150;

const float largeurChateau = 8.5;
const float hauteurChateau = 7;

const float positionChateau1 = -85.0;
const float positionChateau2 = 85.0;

const float g = 9.81;
const float k = 0.005;
const float dt = 0.05;

int nbJoueurs = 2;

float largeurMont;
float hauteurMont;
float wnd;

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

float deg2rad(float deg)
{
    return deg * PI / 180.0;
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

float hauteurMontagne(float largeur, float hauteur, float x)
{
    float rx = 2.0 * x / largeur;
    return hauteur * (1.0 - rx * rx);
}

void dessineTerrain(DrawingWindow& w, float largeur, float hauteur)
{
    int y0 = rtowY(w, 0) + 1;
    int xmin = rtowX(w, -largeur / 2.0) - 1;
    int xmax = rtowX(w, largeur / 2.0) + 1;
    w.setColor("forestgreen");
    for (int x = xmin; x <= xmax; x++) {
        float rx = wtorX(w, x);
        float ry = hauteurMontagne(largeur, hauteur, rx);
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

void dessineVent(DrawingWindow &w, float vitesse)
{
    int lg = rtowX(w, vitesse) - rtowX(w, 0);
    int dir = lg > 0 ? 1 : -1;
    int y = 20;
    w.setColor("black");
    if (lg == 0) {
        w.drawCircle(w.width / 2, y, 4);
    } else {
        int x1 = (w.width - lg) / 2;
        int x2 = (w.width + lg) / 2;
        w.drawLine(x1 - dir, y - 1, x2 - dir, y - 1);
        w.drawLine(x1, y, x2, y);
        w.drawLine(x1 - dir, y + 1, x2 - dir, y + 1);
        for (int i = 0; i < 3; i++) {
            w.drawLine(x2 - i * dir, y, x2 - (6 + i) * dir, y - 4);
            w.drawLine(x2 - i * dir, y, x2 - (6 + i) * dir, y + 4);
        }
    }
}

void dessineExplosion(DrawingWindow& w, float rx, float ry)
{
    const int maxray = rtowX(w, 2.5) - rtowX(w, 0);
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
        w.msleep(10);
    }
    w.fillCircle(x, y, maxray - 1);
}

void dessineFlammes(DrawingWindow& w, float x0, float y0)
{
    for (int i = 0; i < 70; i++) {
        float dt = 0.05;
        float vx = frand(-2.5, 2.5);
        float vy = frand(5, 17);
        float x = x0;
        float y = y0;
        float red = frand(0.5, 1);
        float green = frand(0, red);
        float blue = 0;
        w.setColor(red, green, blue);
        while (y >= 0.0) {
            w.drawPoint(rtowX(w, x), rtowY(w, y));
            x += vx * dt;
            y += vy * dt;
            vy -= 9.81 * dt;
        }
        w.msleep(30);
    }
}

void initialise(DrawingWindow& w)
{
    largeurMont = frand(largeurMin, largeurMax);
    hauteurMont = frand(hauteurMin, hauteurMax);
    wnd = frand(-30, 30);
    w.setBgColor("white");
    w.clearGraph();
    dessineTerrain(w, largeurMont, hauteurMont);
    dessineChateau(w, positionChateau1);
    dessineChateau(w, positionChateau2);
    w.setColor("wheat");
    w.drawText(rtowX(w, positionChateau1), rtowY(w, 0) + 8, "Joueur 1",
               Qt::AlignHCenter);
    w.drawText(rtowX(w, positionChateau2), rtowY(w, 0) + 8, "Joueur 2",
               Qt::AlignHCenter);
    dessineVent(w, wnd);
}

/* Retour : numéro du perdant, 0 sinon
   x et y contiennent les coordonnées de la collision
*/
int tir(DrawingWindow& w,
        float x0, float y0, float v0, float alpha, float& x, float &y)
{
    float vx = v0 * cos(alpha);
    float vy = v0 * sin(alpha);
    x = x0;
    y = y0;
    int collision = 0;
    do {
        int wx = rtowX(w, x);
        int wy = rtowY(w, y);
        w.setColor("black");
        w.fillCircle(wx, wy, 2);

        float vxr = vx - wnd;
        float kvr = -k * sqrt(vxr * vxr + vy * vy);
        float ax = kvr * vxr;
        float ay = kvr * vy - g;
        x += vx * dt;
        y += vy * dt;
        vx += ax * dt;
        vy += ay * dt;

        w.msleep(10);
        w.setColor("white");
        w.fillCircle(wx, wy, 2);
//         w.setColor("black");
//         w.drawPoint(wx, wy);

        if (y <= 0) {
            collision = 3;
        } else if (y < hauteurChateau) {
            if (positionChateau1 - largeurChateau <= x
                && positionChateau1 + largeurChateau >= x)
                collision = 1;
            else if (positionChateau2 - largeurChateau <= x
                       && positionChateau2 + largeurChateau >= x)
                collision = 2;
        }
        if (!collision) {
            float h = hauteurMontagne(largeurMont, hauteurMont, x);
            if (h > 0 && y < h)
                collision = 3;
        }
    } while (!collision);
    return collision == 3 ? 0 : collision;
}

int jeu1(DrawingWindow& w)
{
    initialise(w);
    int joueur = 2;
    float x, y;
    int perdant;
    do {
        joueur = 3 - joueur;

        std::cout << "-=| Joueur " << joueur << " |=-";

        float alpha;
        float v0;
        if (joueur <= nbJoueurs) {
            std::cout << "\nangle ? ";
            std::cin >> alpha;
            std::cout << "vitesse initiale ? ";
            std::cin >> v0;
        } else {
            alpha = frand(10, 90);
            v0 = frand(10, 100);
            std::cout << " [ " << alpha << " ; " << v0 << " ]\n";
        }

        alpha = deg2rad(alpha);
        float x0;
        if (joueur == 1) {
            x0 = positionChateau1 + 8;
        } else {
            x0 = positionChateau2 - 8;
            alpha = PI - alpha;
        }
        float y0 = 8;
        perdant = tir(w, x0, y0, v0, alpha, x, y);
        dessineExplosion(w, x, y);
        dessineVent(w, wnd);
    } while (!perdant);
    dessineFlammes(w, x, y);
    std::stringstream msg;
    msg << "Joueur " << perdant;
    if (perdant == joueur)
        msg << " s'est suicidé !";
    else
        msg << " a perdu !";
    w.setColor("darkred");
    w.setBgColor("white");
    w.drawTextBg(w.width / 2, w.height / 3, msg.str().c_str(),
                 Qt::AlignCenter);
    std::cout << msg.str() << "\n";
    return perdant;
}

void jeu(DrawingWindow& w)
{
    int score1 = 0;
    int score2 = 0;
    bool rejouer = true;
    do {
        int perdant = jeu1(w);
        if (perdant == 1)
            score2++;
        else if (perdant == 2)
            score1++;
        std::cout << "### SCORE : " << score1 << " / " << score2 << " ###\n";
        if (nbJoueurs == 0)
            w.sleep(2);
        else {
            char r;
            do {
                std::cout << "Recommencer (o/n) ? ";
                std::cin >> r;
            } while (std::cin.good() && r != 'o' && r != 'n');
            rejouer = r == 'o';
        }
    } while (rejouer);
    w.closeGraph();
}

int main(int argc, char *argv[])
{
    QApplication application(argc, argv);

    if (argc > 1)
        nbJoueurs = atoi(argv[1]);

    DrawingWindow window(jeu, 640, 480);
    window.show();
    return application.exec();
}
