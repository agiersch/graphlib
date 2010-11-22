/*
 * Copyright (c) 2007-2010, Arnaud Giersch <arnaud.giersch@iut-bm.univ-fcomte.fr>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "DrawingWindow.h"
#include <QApplication>
#include <QPaintEvent>
#include <QThread>
#include <QTimerEvent>

/*! \class DrawingWindow
 *  \brief Fenêtre de dessin.
 *
 * \author Arnaud Giersch <arnaud.giersch@iut-bm.univ-fcomte.fr>
 * \date 2007-2010
 *
 * Cette classe décrit un widget Qt permettant d'écrire des
 * applications graphiques simples.  Pour cela, il faut définir une
 * fonction de dessin.  Cette fonction ne retourne rien et prend comme
 * unique paramètre une référence vers un objet de class
 * DrawingWindow.
 *
 * La fonction devra ensuite être passée en paramètre pour les
 * constructeurs de la classe, ainsi que les dimension requises pour
 * la fenêtre graphique.  Le programme est ensuite compilé comme
 * n'importe quel programme Qt.
 *
 * Concrètement, la fonction sera exécutée dans un nouveau thread,
 * tandis que le thread principal s'occupera de la gestion des
 * évènements et du rendu dans la fenêtre.
 *
 * <b>NB.</b> Pour toutes les méthodes de dessin, le coin en haut à gauche
 * de la fenêtre a les coordonnées (0, 0).  Le coin en bas à droite de
 * la fenêtre a les coordonnées (largeur - 1, hauteur - 1), si la
 * fenêtre est de dimension largeur × hauteur.
 *
 * Un appui sur la touche &lt;Esc&gt; provoque la fermeture de la fenêtre.
 * Comme pour la plupart des applications, il est également possible
 * de fermer la fenêtre via le gestionnaire de fenêtres.
 *
 * Il est possible, dans une application, d'ouvrir plusieurs fenêtres,
 * avec des fonctions de dessin éventuellement différentes.
 * L'application se terminera normalement lorsque la dernière fenêtre
 * sera fermée.
 */

/*! \example hello.cpp
 *
 * Voir le code source à la fin de la page.  Pour compiler et exécuter
 * ce programme, il faut :
 *
 * <b>1. Créer le fichier \c hello.pro</b>
 *
 * Pour simplifier, ce fichier contient la liste des fichiers sources
 * composant le programme.
 *
 * \include hello.pro
 *
 * <b>2. Créer le fichier \c Makefile avec la commande :</b>
 *
 * \verbatim qmake-qt4 hello.pro \endverbatim
 * ou tout simplement :
 * \verbatim qmake-qt4 \endverbatim
 *
 * <b>3. Compiler le programme avec la commande :</b>
 *
 * \verbatim make hello \endverbatim
 * ou tout simplement :
 * \verbatim make \endverbatim
 *
 * <b>4. Exécuter le programme avec la commande :</b>
 *
 * \verbatim ./hello \endverbatim
 *
 * <b>Code source de l'exemple</b>
 */

/*! \example exemple.cpp
 *
 * Un exemple un peu plus sophistiqué.
 */

//! Classe de thread.
class DrawingThread: public QThread {
public:
    DrawingThread(DrawingWindow &w, DrawingWindow::ThreadFunction f);
    void start_once(Priority priority = InheritPriority);

protected:
    void run();

private:
    DrawingWindow &drawingWindow;
    DrawingWindow::ThreadFunction threadFunction;
    bool started_once;

    friend class DrawingWindow;
};

enum UserEvents {
    SyncRequest = QEvent::User, //!< Demande de synchronisation.
    CloseRequest,               //!< Demande de fermeture de la fenêtre.
    DrawTextRequest,            //!< Demande d'écriture de texte.
};

//! Demande de synchronisation.
class SyncRequestEvent: public QEvent {
public:
    SyncRequestEvent(): QEvent(static_cast<QEvent::Type>(SyncRequest))
    { }
};

//! Demande de fermeture de fenêtre.
class CloseRequestEvent: public QEvent {
public:
    CloseRequestEvent(): QEvent(static_cast<QEvent::Type>(CloseRequest))
    { }
};

//! Demande de tracé de texte. 
class DrawTextEvent: public QEvent {
public:
    const int x;
    const int y;
    const char *text;
    const int flags;
    DrawTextEvent(int x_, int y_, const char* text_, int flags_)
        : QEvent(static_cast<QEvent::Type>(DrawTextRequest))
        , x(x_), y(y_), text(text_), flags(flags_)
    { }
};

//--- DrawingWindow ----------------------------------------------------

/*! \file DrawingWindow.h
 *  \brief Classe DrawingWindow.
 */

/*! \typedef DrawingWindow::ThreadFunction
 *  \brief Type de la fonction de dessin, passée en paramètre de construction.
 */
/*! \var DrawingWindow::DEFAULT_WIDTH
 *  \brief Largeur par défaut de la fenêtre.
 */
/*! \var DrawingWindow::DEFAULT_HEIGHT
 *  \brief Hauteur par défaut de la fenêtre.
 */
/*! \var DrawingWindow::width
 *  \brief Largeur de la fenêtre.
 */
/*! \var DrawingWindow::height
 *  \brief Hauteur de la fenêtre.
 */
/*! \var DrawingWindow::paintInterval
 *  \brief Intervalle de temps entre deux rendus (ms).
 */

//! Constructeur.
/*!
 * Construit une nouvelle fenêtre de dessin avec les dimensions
 * passées en paramètres.  La fonction fun sera exécutée dans un
 * nouveau thread.
 *
 * \param fun           fonction de dessin
 * \param width_        largeur de la fenêtre
 * \param height_       hauteur de la fenêtre
 *
 * \see QWidget
 */
DrawingWindow::DrawingWindow(ThreadFunction fun, int width_, int height_)
    : QWidget()
    , width(width_)
    , height(height_)
{
    initialize(fun);
}

//! Constructeur.
/*!
 * Construit un nouveau widget de dessin avec les dimensions passées
 * en paramètres.  La fonction fun sera exécutée dans un nouveau
 * thread.
 *
 * \param parent        widget parent
 * \param fun           fonction de dessin
 * \param width_        largeur de la fenêtre
 * \param height_       hauteur de la fenêtre
 *
 * \see QWidget
 */
DrawingWindow::DrawingWindow(QWidget *parent,
                             ThreadFunction fun, int width_, int height_)
    : QWidget(parent)
    , width(width_)
    , height(height_)
{
    initialize(fun);
}

//! Constructeur.
/*!
 * Construit un nouveau widget de dessin avec les dimensions passées
 * en paramètres.  La fonction fun sera exécutée dans un nouveau
 * thread.
 *
 * \param parent        widget parent
 * \param flags         flags passés au constructeur de QWidget
 * \param fun           fonction de dessin
 * \param width_        largeur de la fenêtre
 * \param height_       hauteur de la fenêtre
 *
 * \see QWidget
 */
DrawingWindow::DrawingWindow(QWidget *parent, Qt::WindowFlags flags,
                             ThreadFunction fun, int width_, int height_)
    : QWidget(parent, flags)
    , width(width_)
    , height(height_)
{
    initialize(fun);
}

//! Destructeur.
DrawingWindow::~DrawingWindow()
{
    delete thread;
    delete painter;
    delete image;
}

//! Change la couleur de dessin.
/*!
 * La couleur est un entier, tel que retourné par getPointColor.
 * Normalement de la forme #00RRGGBB.
 *
 * \param color         couleur
 *
 * \see setColor(const char *), setColor(float, float, float),
 *      setBgColor(unsigned int),
 *      getPointColor
 */
void DrawingWindow::setColor(unsigned int color)
{
    setColor(QColor::fromRgb(color));
}

//! Change la couleur de dessin.
/*!
 * Le nom de couleur est de la forme "black", "white", "red", "blue", ...
 *
 * \param name          nom de couleur
 *
 * \see setColor(unsigned int), setColor(float, float, float),
 *      setBgColor(const char *)
 * \see http://www.w3.org/TR/SVG/types.html#ColorKeywords
 */
void DrawingWindow::setColor(const char *name)
{
    setColor(QColor(name));
}

//! Change la couleur de dessin.
/*!
 * Les composantes de rouge, vert et bleu de la couleur doivent être
 * compris entre 0 et 1.  Si le trois composantes sont à 0, on obtient
 * du noir; si les trois composantes sont à 1, on obtient du blanc.
 *
 * \param red           composante de rouge
 * \param green         composante de vert
 * \param blue          composante de bleu
 *
 * \see setColor(unsigned int), setColor(const char *),
 *      setBgColor(float, float, float)
 */
void DrawingWindow::setColor(float red, float green, float blue)
{
    setColor(QColor::fromRgbF(red, green, blue));
}

//! Change la couleur de fond.
/*!
 * \param color         couleur
 *
 * \see setBgColor(const char *), setBgColor(float, float, float),
 *      setColor(unsigned int),
 *      getPointColor,
 *      clearGraph
 */
void DrawingWindow::setBgColor(unsigned int color)
{
    setBgColor(QColor::fromRgb(color));
}

//! Change la couleur de fond.
/*!
 * \param name          nom de couleur
 *
 * \see setBgColor(unsigned int), setBgColor(float, float, float),
 *      setColor(const char *),
 *      clearGraph
 * \see http://www.w3.org/TR/SVG/types.html#ColorKeywords
 */
void DrawingWindow::setBgColor(const char *name)
{
    setBgColor(QColor(name));
}

//! Change la couleur de fond.
/*!
 * \param red           composante de rouge
 * \param green         composante de vert
 * \param blue          composante de bleu
 *
 * \see setBgColor(unsigned int), setBgColor(const char *),
 *      setColor(float, float, float),
 *      clearGraph
 */
void DrawingWindow::setBgColor(float red, float green, float blue)
{
    setBgColor(QColor::fromRgbF(red, green, blue));
}

//! Retourne la fonte courante utilisée pour dessiner du texte.
/*!
 * \see QFont, setFont
 */
const QFont& DrawingWindow::getFont() const
{
    return painter->font();
}

//! Applique une nouvelle font pour dessiner du texte.
/*!
 * \see QFont, getFont
 */
void DrawingWindow::setFont(const QFont& font)
{
    painter->setFont(font);
}

//! Efface la fenêtre.
/*!
 * La fenêtre est effacée avec la couleur de fond courante.
 *
 * \see setBgColor
 */
void DrawingWindow::clearGraph()
{
    safeLock(imageMutex);
    painter->fillRect(image->rect(), getBgColor());
    dirty();
    safeUnlock(imageMutex);
}

//! Dessine un point.
/*!
 * Dessine un point (pixel) aux coordonnées (x, y), avec la couleur de
 * dessin courante.
 *
 * \param x, y          coordonnées du point
 *
 * \see setColor
 */
void DrawingWindow::drawPoint(int x, int y)
{
    safeLock(imageMutex);
    painter->drawPoint(x, y);
    dirty(x, y);
    safeUnlock(imageMutex);
}

//! Dessine un segment.
/*!
 * Dessine un segement de droite entre les coordonnées (x1, y1) et
 * (x2, y2), avec la couleur de dessin courante.
 *
 * \param x1, y1        coordonnées d'une extrémité du segment
 * \param x2, y2        coordonnées de l'autre extrémité du segment
 *
 * \see setColor
 */
void DrawingWindow::drawLine(int x1, int y1, int x2, int y2)
{
    safeLock(imageMutex);
    painter->drawLine(x1, y1, x2, y2);
    dirty(x1, y1, x2, y2);
    safeUnlock(imageMutex);
}

//! Dessine un rectangle.
/*!
 * Dessine le rectangle parallèle aux axes et défini par les
 * coordonnées de deux sommets opposés (x1, y1) et (x2, y2).  Utilise
 * la couleur de dessin courante.
 *
 * \param x1, y1        coordonnées d'un sommet du rectangle
 * \param x2, y2        coordonnées du sommet opposé du rectangle
 *
 * \see fillRect, setColor
 */
void DrawingWindow::drawRect(int x1, int y1, int x2, int y2)
{
    QRect r;
    r.setCoords(x1, y1, x2 - 1, y2 - 1);
    r = r.normalized();
    safeLock(imageMutex);
    painter->drawRect(r);
    r.adjust(0, 0, 1, 1);
    dirty(r);
    safeUnlock(imageMutex);
}

//! Dessine un rectangle plein.
/*!
 * Dessine le rectangle plein parallèle aux axes et défini par les
 * coordonnées de deux sommets opposés (x1, y1) et (x2, y2).  Utilise
 * la couleur de dessin courante.
 *
 * \param x1, y1        coordonnées d'un sommet du rectangle
 * \param x2, y2        coordonnées du sommet opposé du rectangle
 *
 * \see drawRect, setColor
 */
void DrawingWindow::fillRect(int x1, int y1, int x2, int y2)
{
    painter->setBrush(getColor());
    drawRect(x1, y1, x2, y2);
    painter->setBrush(Qt::NoBrush);
}

//! Dessine un cercle.
/*!
 * Dessine un cercle de centre (x, y) et de rayon r.  Utilise la
 * couleur de dessin courante.
 *
 * \param x, y          coordonnées du centre du cercle
 * \param r             rayon du cercle
 *
 * \see fillCircle, setColor
 */
void DrawingWindow::drawCircle(int x, int y, int r)
{
    QRect rect;
    rect.setCoords(x - r, y - r, x + r - 1, y + r - 1);
    safeLock(imageMutex);
    painter->drawEllipse(rect);
    rect.adjust(0, 0, 1, 1);
    dirty(rect);
    safeUnlock(imageMutex);
}

//! Dessine un disque.
/*!
 * Dessine un disque (cercle plein) de centre (x, y) et de rayon r.
 * Utilise la couleur de dessin courante.
 *
 * \param x, y          coordonnées du centre du disque
 * \param r             rayon du disque
 *
 * \see drawCircle, setColor
 */
void DrawingWindow::fillCircle(int x, int y, int r)
{
    painter->setBrush(getColor());
    drawCircle(x, y, r);
    painter->setBrush(Qt::NoBrush);
}

//! Dessine un triangle.
/*!
 * Dessine un triangle défini par les coordonnées de ses sommets:
 * (x1, y1), (x2, y2) et (x3, y3).  Utilise la couleur de dessin
 * courante.
 *
 * \param x1, y1        coordonnées du premier sommet du triangle
 * \param x2, y2        coordonnées du deuxième sommet du triangle
 * \param x3, y3        coordonnées du troisième sommet du triangle
 *
 * \see fillTriangle, setColor
 */
void DrawingWindow::drawTriangle(int x1, int y1, int x2, int y2, int x3, int y3)
{
    QPolygon poly(3);
    poly.putPoints(0, 3, x1, y1, x2, y2, x3, y3);
    safeLock(imageMutex);
    painter->drawConvexPolygon(poly);
    dirty(poly.boundingRect());
    safeUnlock(imageMutex);
}

//! Dessine un triangle plein.
/*!
 * Dessine un triangle plein défini par les coordonnées de ses
 * sommets: (x1, y1), (x2, y2) et (x3, y3).  Utilise la couleur de
 * dessin courante.
 *
 * \param x1, y1        coordonnées du premier sommet du triangle
 * \param x2, y2        coordonnées du deuxième sommet du triangle
 * \param x3, y3        coordonnées du troisième sommet du triangle
 *
 * \see drawTriangle, setColor
 */
void DrawingWindow::fillTriangle(int x1, int y1, int x2, int y2, int x3, int y3)
{
    painter->setBrush(getColor());
    drawTriangle(x1, y1, x2, y2, x3, y3);
    painter->setBrush(Qt::NoBrush);
}

//! Écrit du texte.
/*!
 * Écrit le texte text, aux coordonnées (x, y) et avec les paramètres
 * d'alignement flags.  Le texte est écrit avec la couleur de dessin
 * courante.  Les flags sont une combinaison (ou binaire) de
 * Qt::AlignLeft, Qt::AligneRight, Qt::AlignHCenter, Qt::AlignTop,
 * Qt::AlignBottom, Qt::AlignVCenter, Qt::AlignCenter.  Par défaut, le
 * texte est aligné en haut à gauche.
 *
 * \param x, y          coordonnées du texte
 * \param text          texte à écrire
 * \param flags         paramètres d'alignement
 *
 * \see drawTextBg, setColor
 * \see QPainter::drawText
 */
void DrawingWindow::drawText(int x, int y, const char *text, int flags)
{
    safeLock(syncMutex);
    if (!terminateThread) {
        qApp->postEvent(this, new DrawTextEvent(x, y, text, flags));
        syncCondition.wait(&syncMutex);
    }
    safeUnlock(syncMutex);
}

//! Écrit du texte sur fond coloré.
/*!
 * Écrit du texte comme drawText, mais l'arrière-plan est coloré avec
 * la couleur de fond courante.
 *
 * \param x, y          coordonnées du texte
 * \param text          texte à écrire
 * \param flags         paramètres d'alignement
 *
 * \see drawText, setColor, setColorBg
 */
void DrawingWindow::drawTextBg(int x, int y, const char *text, int flags)
{
    painter->setBackgroundMode(Qt::OpaqueMode);
    drawText(x, y, text, flags);
    painter->setBackgroundMode(Qt::TransparentMode);
}


//! Retourne la couleur d'un pixel.
/*!
 * Retourne la couleur du pixel de coordonnées (x, y).  La valeur
 * retournée peut servir de paramètres à setColor(unsigned int) ou
 * setBgColor(unsigned int).
 *
 * \param x, y          coordonnées du pixel
 * \return              couleur du pixel
 *
 * \see setColor(unsigned int), setBgColor(unsigned int)
 */
unsigned int DrawingWindow::getPointColor(int x, int y)
{
    return image->pixel(x, y);
}

//! Attend l'appui sur un des boutons de la souris.
/*!
 * Attend l'appui sur un des boutons de la souris.  Retourne le bouton
 * qui a été pressé et les coordonnées du pointeur de souris à ce
 * moment-là.
 *
 * \param x, y          coordonnées du pointeur de souris
 * \param button        numéro du bouton qui a été pressé
 *                      (1: gauche, 2: droit, 3: milieu, 0 sinon)
 * \param time          durée maximale de l'attente
 * \return              true si un bouton a été pressé
 *
 * \bug                 expérimental
 */
bool DrawingWindow::waitMousePress(int &x, int &y, int &button,
                                   unsigned long time)
{
    bool pressed;
    safeLock(mouseMutex);
    if (terminateThread) {
        pressed = false;
    } else {
        pressed = mouseCondition.wait(&mouseMutex, time);
        if (pressed) {
            x = mousePos.x();
            y = mousePos.y();
            if (mouseButton & Qt::LeftButton)
                button = 1;
            else if (mouseButton & Qt::RightButton)
                button = 2;
            else if (mouseButton & Qt::MidButton)
                button = 3;
            else
                button = 0;
        }
    }
    safeUnlock(mouseMutex);
    return pressed;
}

//! Synchronise le contenu de la fenêtre.
/*!
 * Pour des raisons d'efficacités, le résultat des fonctions de dessin
 * n'est pas affiché immédiatement.  L'appel à sync permet de
 * synchroniser le contenu de la fenêtre.  Autrement dit, cela bloque
 * l'exécution du programme jusqu'à ce que le contenu de la fenêtre
 * soit à jour.
 *
 * \param time          durée maximale de l'attente
 * \return              true si la fenêtre a pu être synchronisée
 */
bool DrawingWindow::sync(unsigned long time)
{
    bool synced;
    safeLock(syncMutex);
    if (terminateThread) {
        synced = false;
    } else {
        qApp->postEvent(this, new SyncRequestEvent());
        synced = syncCondition.wait(&syncMutex, time);
    }
    safeUnlock(syncMutex);
    return synced;
}

//! Ferme la fenêtre graphique.
void DrawingWindow::closeGraph()
{
    qApp->postEvent(this, new CloseRequestEvent());
}

//! Suspend l'exécution pendant un certain temps.
/*!
 * \param secs          temps d'attente en seconde
 */
void DrawingWindow::sleep(unsigned long secs)
{
    DrawingThread::sleep(secs);
}

//! Suspend l'exécution pendant un certain temps.
/*!
 * \param msecs          temps d'attente en millisecondes
 */
void DrawingWindow::msleep(unsigned long msecs)
{
    DrawingThread::msleep(msecs);
}

//! Suspend l'exécution pendant un certain temps.
/*!
 * \param usecs          temps d'attente en microsecondes
 */
void DrawingWindow::usleep(unsigned long usecs)
{
    DrawingThread::usleep(usecs);
}

/*!
 * \see QWidget
 */
void DrawingWindow::closeEvent(QCloseEvent *ev)
{
    timer.stop();
    thread->terminate();
    syncMutex.lock();
    mouseMutex.lock();
    terminateThread = true;     // this flag is needed for the case
                                // where the following wakeAll() call
                                // occurs between the
                                // setTerminationEnable(false) and the
                                // mutex lock in safeLock() called
                                // from sync()
    syncCondition.wakeAll();
    mouseCondition.wakeAll();
    mouseMutex.unlock();
    syncMutex.unlock();
    QWidget::closeEvent(ev);
    thread->wait();
}

/*!
 * \see QWidget
 */
void DrawingWindow::customEvent(QEvent *ev)
{
    switch ((int )ev->type()) {
    case SyncRequest:
        realSync();
        break;
    case CloseRequest:
        close();
        break;
    case DrawTextRequest:
        DrawTextEvent* tev = dynamic_cast<DrawTextEvent *>(ev);
        realDrawText(tev->x, tev->y, tev->text, tev->flags);
        break;
    }
}

/*!
 * \see QWidget
 *
 * \bug                 expérimental
 */
void DrawingWindow::mousePressEvent(QMouseEvent *ev)
{
    mouseMutex.lock();
    mousePos = ev->pos();
    mouseButton = ev->button();
    ev->accept();
    mouseCondition.wakeAll();
    mouseMutex.unlock();
}

/*!
 * \see QWidget
 */
void DrawingWindow::keyPressEvent(QKeyEvent *ev)
{
    bool accept = true;
    switch (ev->key()) {
    case Qt::Key_Escape:
        close();
        break;
    default:
        accept = false;
        break;
    }
    if (accept)
        ev->accept();
}

/*!
 * \see QWidget
 */
void DrawingWindow::paintEvent(QPaintEvent *ev)
{
    QPainter widgetPainter(this);
    imageMutex.lock();
    QImage imageCopy(*image);
    imageMutex.unlock();
    QRect rect = ev->rect();
    widgetPainter.drawImage(rect, imageCopy, rect);
}

/*!
 * \see QWidget
 */
void DrawingWindow::showEvent(QShowEvent *ev)
{
    QWidget::showEvent(ev);
    qApp->flush();
    qApp->syncX();
    timer.start(paintInterval, this);
    thread->start_once(QThread::IdlePriority);
}

/*!
 * \see QWidget
 */
void DrawingWindow::timerEvent(QTimerEvent *ev)
{
    if (ev->timerId() == timer.timerId()) {
        mayUpdate();
        timer.start(paintInterval, this);
    } else {
        QWidget::timerEvent(ev);
    }
}

//--- DrawingWindow (private methods) ----------------------------------

//! Fonction d'initialisation.
/*!
 * Fonction appelée par les différents constructeurs.
 *
 * \param fun           fonction de dessin
 */
void DrawingWindow::initialize(DrawingWindow::ThreadFunction fun)
{
    terminateThread = false;
    lockCount = 0;
    image = new QImage(width, height, QImage::Format_RGB32);
    painter = new QPainter(image);
    thread = new DrawingThread(*this, fun);

    setFocusPolicy(Qt::StrongFocus);
    setFixedSize(image->size());
    setAttribute(Qt::WA_OpaquePaintEvent);
    setFocus();

    setColor("black");
    setBgColor("white");
    clearGraph();

    dirtyFlag = false;
}

//! Change la couleur de dessin.
/*!
 * \param color                 couleur
 */
inline
void DrawingWindow::setColor(const QColor& color)
{
    QPen pen(painter->pen());
    pen.setColor(color);
    painter->setPen(pen);
}

//! Change la couleur de fond.
/*!
 * \param color                 couleur
 */
inline
void DrawingWindow::setBgColor(const QColor& color)
{
    painter->setBackground(color);
}

//! Retourne la couleur de dessin courante.
/*!
 * \return              couleur de dessin courante
 */
inline
QColor DrawingWindow::getColor()
{
    return painter->pen().color();
}

//! Retourne la couleur de fond courante.
/*!
 * \return              couleur de fond courante
 */
inline
QColor DrawingWindow::getBgColor()
{
    return painter->background().color();
}

//! Verrouille un mutex.
/*!
 * S'assure que le thread courant ne peut pas être terminé s'il
 * détient un mutex.  Pendant de safeUnlock.
 *
 * \param mutex         le mutex à verrouiller
 *
 * \see safeUnlock
 */
inline
void DrawingWindow::safeLock(QMutex &mutex)
{
    if (lockCount++ == 0)
        thread->setTerminationEnabled(false);
    mutex.lock();
}

//! Déverrouille un mutex.
/*!
 * S'assure que le thread courant ne peut pas être terminé s'il
 * détient un mutex.  Pendant de safeLock.
 *
 * \param mutex         le mutex à déverrouiller
 *
 * \see safeLock
 */
inline
void DrawingWindow::safeUnlock(QMutex &mutex)
{
    mutex.unlock();
    if (--lockCount == 0)
        thread->setTerminationEnabled(true);
}

//! Marque l'image entière comme non à jour.
inline
void DrawingWindow::dirty()
{
    dirtyFlag = true;
    dirtyRect = image->rect();
}

//! Marque un point de l'image comme non à jour.
/*!
 * \param x, y          coordonnées du point
 */
inline
void DrawingWindow::dirty(int x, int y)
{
    dirty(QRect(x, y, 1, 1));
}

//! Marque une zone de l'image comme non à jour.
/*!
 * La zone est définie par un rectangle dont les coordonnées de deux
 * sommets oppposés sont données.
 *
 * \param x1, y1        coordonnées d'un sommet du rectangle
 * \param x2, y2        coordonnées du sommet opposé du rectangle
 */
inline
void DrawingWindow::dirty(int x1, int y1, int x2, int y2)
{
    QRect r;
    r.setCoords(x1, y1, x2, y2);
    dirty(r.normalized());
}

//! Marque une zone de l'image comme non à jour.
/*!
 * \param rect          rectangle délimitant la zone
 */
void DrawingWindow::dirty(const QRect &rect)
{
    if (dirtyFlag) {
        dirtyRect |= rect;
    } else {
        dirtyFlag = true;
        dirtyRect = rect;
    }
}

//! Génère un update si besoin.
/*!
 * Génère une demande de mise à jour de la fenêtre (appel à update)
 * s'il y en a besoin.
 */
void DrawingWindow::mayUpdate()
{
    imageMutex.lock();
    bool dirty = dirtyFlag;
    QRect rect = dirtyRect;
    dirtyFlag = false;
    imageMutex.unlock();
    if (dirty)
        update(rect);
}

//! Fonction bas-niveau pour sync.
/*!
 * Fonction de synchronisation dans le thread principal.
 *
 * \see sync, customEvent
 */
void DrawingWindow::realSync()
{
    mayUpdate();
    qApp->sendPostedEvents(this, QEvent::UpdateLater);
    qApp->sendPostedEvents(this, QEvent::UpdateRequest);
    qApp->sendPostedEvents(this, QEvent::Paint);
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents |
                        QEventLoop::ExcludeSocketNotifiers |
                        QEventLoop::DeferredDeletion |
                        QEventLoop::X11ExcludeTimers);
    qApp->flush();
    qApp->syncX();
    syncMutex.lock();
    syncCondition.wakeAll();
    syncMutex.unlock();
}

//! Fonction bas-niveau pour drawText.
/*!
 * Le rendu de texte doit être fait dans le thread principal.  D'où
 * les manipulations tordues et la synchronisation qui s'en suit.
 *
 * \param x, y, text, flags     cf. drawText
 *
 * \see drawText, customEvent
 */
void DrawingWindow::realDrawText(int x, int y, const char *text, int flags)
{
    QRect r(image->rect());
    switch (flags & Qt::AlignHorizontal_Mask) {
    case Qt::AlignRight:
        r.setRight(x);
        break;
    case Qt::AlignHCenter:
        if (x < width / 2)
            r.setLeft(2 * x - width + 1);
        else
            r.setRight(2 * x);
        break;
    default:
        r.setLeft(x);
    }
    switch (flags & Qt::AlignVertical_Mask) {
    case Qt::AlignBottom:
        r.setBottom(y);
        break;
    case Qt::AlignVCenter:
        if (y < height / 2)
            r.setTop(2 * y - height + 1);
        else
            r.setBottom(2 * y);
        break;
    default:
        r.setTop(y);
    }
    syncMutex.lock();
    painter->drawText(r, flags, QString::fromUtf8(text), &r);
    dirty(r);
    syncCondition.wakeAll();
    syncMutex.unlock();
}

//--- DrawingThread ----------------------------------------------------

//! Constructeur.
DrawingThread::DrawingThread(DrawingWindow &w, DrawingWindow::ThreadFunction f)
    : drawingWindow(w)
    , threadFunction(f)
    , started_once(false)
{
}

//! Démarre le thread si ce n'a pas encore été fait.
void DrawingThread::start_once(Priority priority)
{
    if (!started_once) {
        started_once = true;
        start(priority);
    }
}

//! La vraie fonction pour le thread.
void DrawingThread::run()
{
    threadFunction(drawingWindow);
}
