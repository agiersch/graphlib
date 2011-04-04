#include <DrawingWindow.h>
#include <QApplication>
#include <iostream>

#define LARGEUR 213
#define HAUTEUR 160

#define TCELL 5                 // taille d'une cellule

#define BORDURE  0x00000000U    // noir
#define NAISSANT 0x0000ff00U    // vert
#define VIVANT   0x000000ffU    // bleu
#define MOURANT  0x00ff0000U    // rouge
#define MORT     0x00ffffffU    // blanc

bool cells[LARGEUR][HAUTEUR][2];

void init()
{
    srand(time(NULL));
    for (int i = 0 ; i < LARGEUR ; ++i)
        for (int j = 0 ; j < HAUTEUR ; ++j)
            cells[i][j][0] = cells[i][j][1] = (rand() < RAND_MAX / 2);
}

void draw(DrawingWindow& w)
{
    for (int i = 0 ; i < LARGEUR ; ++i)
        for (int j = 0 ; j < HAUTEUR ; ++j) {
            if (cells[i][j][0] == cells[i][j][1])
                w.setColor(cells[i][j][1] ? VIVANT : MORT);
            else
                w.setColor(cells[i][j][1] ? NAISSANT : MOURANT);
            w.fillRect(TCELL * i + 1, TCELL * j + 1,
                       TCELL * (i + 1) - 1, TCELL * (j + 1) - 1);
            cells[i][j][0] = cells[i][j][1];
        }
}

int nbVoisins(int i, int j)
{
    int n = 0;
    for (int k = i - 1 ; k <= i + 1 ; ++k)
        for (int l = j - 1 ; l <= j + 1 ; ++l)
            if (k != i || l != j) {
                int vi = (k + LARGEUR) % LARGEUR;
                int vj = (l + HAUTEUR) % HAUTEUR;
                if (cells[vi][vj][0])
                    ++n;
            }
    return n;
}

void update()
{
    for (int i = 0 ; i < LARGEUR ; ++i)
        for (int j = 0 ; j < HAUTEUR ; ++j) {
            int v = nbVoisins(i, j);
            if (cells[i][j][0])
                cells[i][j][1] = (v == 2 || v == 3);
            else
                cells[i][j][1] = (v == 3);
        }
}

void jeudelavie(DrawingWindow& w)
{
    w.setBgColor(BORDURE);
    w.clearGraph();
    init();
    for (int gen = 0 ; ; ++gen) {
        if (gen % 10 == 0)
            std::cerr << "generation " << gen << std::endl;
        w.sync();
        draw(w);
        update();
    }
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    DrawingWindow win(jeudelavie, TCELL * LARGEUR + 1, TCELL * HAUTEUR + 1);
    win.show();
    return app.exec();
}
