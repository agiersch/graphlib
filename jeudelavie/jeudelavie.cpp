#include <DrawingWindow.h>
#include <QApplication>
#include <iostream>

#define LARGEUR 1200
#define HAUTEUR 900

#define VIVANT   0x000000ffU    // bleu
#define MORT     0x00ffffffU    // blanc

struct cell {
    int neigh;
    bool now;
    bool next;
};
cell cells[LARGEUR][HAUTEUR];

void majVoisins(int i, int j, int delta)
{
    for (int k = i - 1 ; k <= i + 1 ; ++k)
        for (int l = j - 1 ; l <= j + 1 ; ++l)
            if (k != i || l != j) {
                int vi = (k + LARGEUR) % LARGEUR;
                int vj = (l + HAUTEUR) % HAUTEUR;
                cells[vi][vj].neigh += delta;
            }
}

void dessine(DrawingWindow& w, int i, int j, unsigned couleur)
{
    w.setColor(couleur);
    w.drawPoint(i, j);
}

void init(DrawingWindow& w)
{
    srand(time(NULL));
    for (int i = 0 ; i < LARGEUR ; ++i)
        for (int j = 0 ; j < HAUTEUR ; ++j)
            cells[i][j].neigh = 0;
    for (int i = 0 ; i < LARGEUR ; ++i)
        for (int j = 0 ; j < HAUTEUR ; ++j) {
            cells[i][j].now = cells[i][j].next = (rand() < RAND_MAX / 2);
            if (cells[i][j].now) {
                majVoisins(i, j, 1);
                dessine(w, i, j, VIVANT);
            }
        }
}

void update0(DrawingWindow& w __attribute__((unused)))
{
    for (int i = 0 ; i < LARGEUR ; ++i)
        for (int j = 0 ; j < HAUTEUR ; ++j) {
            switch (cells[i][j].neigh) {
            case 2:
                // cells[i][j].next = cells[i][j].now; // useless
                break;
            case 3:
                cells[i][j].next = true;
                break;
            default:
                cells[i][j].next = false;
                break;
            }
        }
}

void update1(DrawingWindow& w)
{
    for (int i = 0 ; i < LARGEUR ; ++i)
        for (int j = 0 ; j < HAUTEUR ; ++j) {
            if (cells[i][j].now) {
                if (!cells[i][j].next) {
                    cells[i][j].now = false;
                    majVoisins(i, j, -1);
                    dessine(w, i, j, MORT);
                }
            } else {
                if (cells[i][j].next) {
                    cells[i][j].now = true;
                    majVoisins(i, j, 1);
                    dessine(w, i, j, VIVANT);
                }
            }
        }
}

void jeudelavie(DrawingWindow& w)
{
    w.setBgColor(MORT);
    w.clearGraph();
    init(w);
    w.sync();
    for (int gen = 0 ; ; ++gen) {
        if (gen % 10 == 0)
            std::cerr << "generation " << gen << std::endl;
        update0(w);
        update1(w);
        w.sync();
    }
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    DrawingWindow win(jeudelavie, LARGEUR, HAUTEUR);
    win.show();
    return app.exec();
}
