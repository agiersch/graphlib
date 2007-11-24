#include <QApplication>
#include <DrawingWindow.h>

void dessine(DrawingWindow& w)
{
    w.drawText(w.width / 2, w.height / 2, "Hello world!");
}

int main(int argc, char *argv[])
{
    QApplication application(argc, argv);
    DrawingWindow fenetre(dessine, 640, 480);
    fenetre.show();
    return application.exec();
}
