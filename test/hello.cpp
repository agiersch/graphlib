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
#include <QImage>
#include <QLabel>
#include <QPaintEvent>
#include <QPainter>
#include <QPixmap>
#include <QThread>
#include <Qt>

#include <cmath>
#include <iostream>
#include <string>

//============================================================
// DrawingAreaInterface

class DrawingAreaInterface {
public:
    static const int DEFAULT_WIDTH = 640;
    static const int DEFAULT_HEIGHT = 480;

    virtual ~DrawingAreaInterface() { }

    virtual void setColor(float red, float green, float blue) = 0;
    virtual void drawPoint(int x, int y) = 0;
    virtual void drawLine(int x1, int y1, int x2, int y2) = 0;
    
    virtual void wait() = 0;
    virtual void waitAll() = 0;
};

//============================================================
// WindowCore

class WindowCore {
private:
    static int instanceCount;
    static int DEFAULT_ARGC;
    static char *DEFAULT_ARGV[];

protected:
    WindowCore(int &argc = DEFAULT_ARGC, char *argv[] = DEFAULT_ARGV);
    ~WindowCore();
};

int WindowCore::instanceCount = 0;
int WindowCore::DEFAULT_ARGC = 1;
char *WindowCore::DEFAULT_ARGV[] = { "class WindowCore", NULL };

WindowCore::WindowCore(int &argc, char *argv[])
{
    if (!qApp)
        new QApplication(argc, argv);
    else
        WindowCore::instanceCount++;
}

WindowCore::~WindowCore()
{
    if (WindowCore::instanceCount == 0)
        delete qApp;
    else
        WindowCore::instanceCount--;
}

//============================================================
// QtDrawingArea

class QtDrawingArea: public DrawingAreaInterface,
                     public WindowCore, public QWidget {
private:
    static int visibleCount;

    QImage *image;
    QPainter *painter;

private:
    void init(int width, int height, const char *title);

protected:
    void paintEvent(QPaintEvent *)
    {
        QPainter painter(this);
        painter.drawImage(0, 0, *image);
    }

    void closeEvent(QCloseEvent *event)
    {
        QtDrawingArea::visibleCount--;
        event->accept();
    }

    void keyPressEvent(QKeyEvent *event)
    {
        if (event->key() == Qt::Key_Escape) {
            event->accept();
            close();
        } else
            event->ignore();
    }

public:
    QtDrawingArea(int argc, char *argv[],
                  int width = DEFAULT_WIDTH, int height = DEFAULT_HEIGHT,
                  const char *title = NULL);

    QtDrawingArea(QWidget *parent, Qt::WindowFlags flags,
                  int width = DEFAULT_WIDTH, int height = DEFAULT_HEIGHT,
                  const char *title = NULL);

    QtDrawingArea(int width = DEFAULT_WIDTH, int height = DEFAULT_HEIGHT,
                  const char *title = NULL);

    ~QtDrawingArea();

    int width() const
    {
        return image->width();
    }

    int height() const
    {
        return image->height();
    }

    void setColor(const QColor &color)
    {
        QPen pen(painter->pen());
        pen.setColor(color);
        painter->setPen(pen);
    }
    void setColor(float red, float green, float blue)
    {
        QColor color;
        color.setRgbF(red, green, blue);
        this->setColor(color);
    }

    void drawPoint(int x, int y)
    {
        painter->drawPoint(x, y);
        this->update();
    }

    void drawLine(int x1, int y1, int x2, int y2)
    {
        painter->drawLine(x1, y1, x2, y2);
        this->update();
    }

    void flush()
    {
        qApp->sendPostedEvents(this, 0);
        qApp->processEvents();
        qApp->flush();
    }

    void wait()
    {
        if (QtDrawingArea::visibleCount > 1)
            while (this->isVisible())
                qApp->processEvents(QEventLoop::WaitForMoreEvents);
        else if (QtDrawingArea::visibleCount > 0)
            qApp->exec();
    }

    void waitAll()
    {
        if (QtDrawingArea::visibleCount > 0)
            qApp->exec();
    }
};

int QtDrawingArea::visibleCount = 0;

QtDrawingArea::QtDrawingArea(int argc, char *argv[],
                             int width, int height, const char *title)
    : WindowCore(argc, argv)
    , QWidget()
{
    init(width, height, title);
}

QtDrawingArea::QtDrawingArea(QWidget *parent, Qt::WindowFlags flags,
                             int width, int height, const char *title)
    : WindowCore()
    , QWidget(parent, flags)
{
    init(width, height, title);
}

QtDrawingArea::QtDrawingArea(int width, int height, const char *title)
    : WindowCore()
    , QWidget()
{
    init(width, height, title);
}

QtDrawingArea::~QtDrawingArea()
{
    delete painter;
    delete image;
}

void QtDrawingArea::init(int width, int height, const char *title)
{
    if (title)
        this->setWindowTitle(title);
    this->setFocusPolicy(Qt::StrongFocus);
    image = new QImage(width, height, QImage::Format_RGB32);
    image->fill(QColor(Qt::white).rgb());
    this->setFixedSize(image->size());
    QtDrawingArea::visibleCount++;
    this->show();
    painter = new QPainter(image);
}


//============================================================

/* paramètres par défaut */
int larg = 600;
int haut = 600;
float Rmin = -2.05;
float Rmax = 0.55;
float Imin = -1.3;
float Imax = 1.3;

int maxiter = 100;

int main(int argc, char *argv[])
{
    int x, y;                   /* le pixel considéré */
    float cr, ci;               /* le complexe correspondant */
    float zr, zi;               /* pour calculer la suite */
    float zr2, zi2;
    float pr, pi;               /* taille d'un pixel */
    float rouge, vert, bleu;
    int i;

    QtDrawingArea fen(argc, argv, larg, haut);

    pr = (Rmax - Rmin) / larg;
    pi = (Imax - Imin) / larg;

    cr = Rmin;
    for (x = 0; x < larg; x++) {
        ci = Imin;
        for (y = 0; y < larg; y++) {
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
                /* on est sorti trop t-bôt du for(...):-A
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
                fen.setColor(rouge, vert, bleu);
                fen.drawPoint(x, y);

            ci += pi;
        }
        cr += pr;
        fen.flush();
    }
    fen.wait();
}
#if 0
    int err = 0;
    std::cerr << err++;
    for (int x = 0; x < fen.width(); x++) 
        for (int y = 0; y < fen.height(); y++)
            float dx = x;
            float dy = y;
            float r = sqrt(dx * dx + dy * dy) / fen.width();
            dx = fen.width() - 1 - x;
            float g = sqrt(dx * dx + dy * dy) / fen.width();
            dx = fen.width() / 2 - x;
            dy = fen.height() - y;
            float b = sqrt(dx * dx + dy * dy) / fen.height();
            if (r > 1.0)  r = 1.0;
            if (g > 1.0)  g = 1.0;
            if (b > 1.0)  b = 1.0;
            fen.setColor(r, g, b);
            fen.drawPoint(x, y);
        }
    std::cerr << err++;
    fen.flush();
    std::cerr << err++;
    fen.setColor(0.0, 0.0, 1.0);
    for (int x = 0; x < 10; x++)
        for (int y = 0; y < 10; y++)
            fen.drawPoint(x, y);
    std::cerr << err++;
    fen.flush();
    std::cerr << err++;
    fen.setColor(0.0, 0.0, 0.0);
    for (int z = 100; z <= 300; z += 4) {
        fen.drawLine(150, 150, z, 100);
        fen.drawLine(150, 150, z, 300);
        fen.drawLine(150, 150, 100, z);
        fen.drawLine(150, 150, 300, z);
    }
    std::cerr << "OOOO\n";
    QtDrawingArea fen2;
    fen.wait();
    std::cerr << "qwewet\n";

    fen.waitAll();
}
#endif
