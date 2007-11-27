/*
 * Copyright (c) 2007, Arnaud Giersch <arnaud.giersch@iut-bm.univ-fcomte.fr>
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
 *  \brief Fen�tre de dessin.
 *
 * \author Arnaud Giersch <arnaud.giersch@iut-bm.univ-fcomte.fr>
 * \date novembre 2007
 *
 * Cette classe d�crit un widget Qt permettant d'�crire des
 * applications graphiques simples.  Pour cela, il faut d�finir une
 * fonction de dessin.  Cette fonction ne retourne rien et prend comme
 * unique param�tre une r�f�rence vers un objet de class
 * DrawingWindow.
 *
 * La fonction devra ensuite �tre pass�e en param�tre pour les
 * constructeurs de la classe, ainsi que les dimension requises pour
 * la fen�tre graphique.  Le programme est ensuite compil� comme
 * n'importe quel programme Qt.
 *
 * Concr�tement, la fonction sera ex�cut�e dans un nouveau thread,
 * tandis que le thread principal s'occupera de la gestion des
 * �v�nements et du rendu dans la fen�tre.
 *
 * <b>NB.</b> Pour toutes les m�thodes de dessin, le coin en haut � gauche
 * de la fen�tre a les coordonn�es (0, 0).  Le coin en bas � droite de
 * la fen�tre a les coordonn�es (largeur - 1, hauteur - 1), si la
 * fen�tre est de dimension largeur � hauteur.
 *
 * Un appui sur la touche <Esc> provoque la fermeture de la fen�tre.
 * Comme pour la plupart des applications, il est �galement possible
 * de fermer la fen�tre via le gestionnaire de fen�tres.
 *
 * Il est possible, dans une application, d'ouvrir plusieurs fen�tres,
 * avec des fonctions de dessin �ventuellement diff�rentes.
 * L'application se terminera normalement lorsque la derni�re fen�tre
 * sera ferm�e.
 */

/*! \example hello.cpp
 *
 * Voir le code source � la fin de la page.  Pour compiler et ex�cuter
 * ce programme, il faut :
 *
 * <b>1. Cr�er le fichier \c hello.pro</b>
 *
 * Pour simplifier, ce fichier contient la liste des fichiers sources
 * composant le programme.
 *
 * \include hello.pro
 *
 * <b>2. Cr�er le fichier \c Makefile avec la commande :</b>
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
 * <b>4. Ex�cuter le programme avec la commande :</b>
 *
 * \verbatim ./exemple \endverbatim
 *
 * <b>Code source de l'exemple</b>
 */

/*! \example exemple.cpp
 *
 * Un exemple un peu plus sophistiqu�.
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
    CloseRequest,               //!< Demande de fermeture de la fen�tre.
    DrawTextRequest,            //!< Demande d'�criture de texte.
};

//! Demande de synchronisation.
class SyncRequestEvent: public QEvent {
public:
    SyncRequestEvent(): QEvent(static_cast<QEvent::Type>(SyncRequest))
    { }
};

//! Demande de fermeture de fen�tre.
class CloseRequestEvent: public QEvent {
public:
    CloseRequestEvent(): QEvent(static_cast<QEvent::Type>(CloseRequest))
    { }
};

//! Demande de trac� de texte. 
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
 *  \brief Type de la fonction de dessin, pass�e en param�tre de construction.
 */
/*! \var DrawingWindow::DEFAULT_WIDTH
 *  \brief Largeur par d�faut de la fen�tre.
 */
/*! \var DrawingWindow::DEFAULT_HEIGHT
 *  \brief Hauteur par d�faut de la fen�tre.
 */
/*! \var DrawingWindow::width
 *  \brief Largeur de la fen�tre.
 */
/*! \var DrawingWindow::height
 *  \brief Hauteur de la fen�tre.
 */
/*! \var DrawingWindow::paintInterval
 *  \brief Intervalle de temps entre deux rendus (ms).
 */

//! Constructeur.
/*!
 * Construit une nouvelle fen�tre de dessin avec les dimensions
 * pass�es en param�tres.  La fonction fun sera ex�cut�e dans un
 * nouveau thread.
 *
 * \param fun           fonction de dessin
 * \param width_        largeur de la fen�tre
 * \param height_       hauteur de la fen�tre
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
 * Construit un nouveau widget de dessin avec les dimensions pass�es
 * en param�tres.  La fonction fun sera ex�cut�e dans un nouveau
 * thread.
 *
 * \param parent        widget parent
 * \param fun           fonction de dessin
 * \param width_        largeur de la fen�tre
 * \param height_       hauteur de la fen�tre
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
 * Construit un nouveau widget de dessin avec les dimensions pass�es
 * en param�tres.  La fonction fun sera ex�cut�e dans un nouveau
 * thread.
 *
 * \param parent        widget parent
 * \param flags         flags pass�s au constructeur de QWidget
 * \param fun           fonction de dessin
 * \param width_        largeur de la fen�tre
 * \param height_       hauteur de la fen�tre
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
 * La couleur est un entier, tel que retourn� par getPointColor.
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
 * Les composantes de rouge, vert et bleu de la couleur doivent �tre
 * compris entre 0 et 1.  Si le trois composantes sont � 0, on obtient
 * du noir; si les trois composantes sont � 1, on obtient du blanc.
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

//! Efface la fen�tre.
/*!
 * La fen�tre est effac�e avec la couleur de fond courante.
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
 * Dessine un point (pixel) aux coordonn�es (x, y), avec la couleur de
 * dessin courante.
 *
 * \param x, y          coordonn�es du point
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
 * Dessine un segement de droite entre les coordonn�es (x1, y1) et
 * (x2, y2), avec la couleur de dessin courante.
 *
 * \param x1, y1        coordonn�es d'une extr�mit� du segment
 * \param x2, y2        coordonn�es de l'autre extr�mit� du segment
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
 * Dessine le rectangle parall�le aux axes et d�fini par les
 * coordonn�es de deux sommets oppos�s (x1, y1) et (x2, y2).  Utilise
 * la couleur de dessin courante.
 *
 * \param x1, y1        coordonn�es d'un sommet du rectangle
 * \param x2, y2        coordonn�es du sommet oppos� du rectangle
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
 * Dessine le rectangle plein parall�le aux axes et d�fini par les
 * coordonn�es de deux sommets oppos�s (x1, y1) et (x2, y2).  Utilise
 * la couleur de dessin courante.
 *
 * \param x1, y1        coordonn�es d'un sommet du rectangle
 * \param x2, y2        coordonn�es du sommet oppos� du rectangle
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
 * \param x, y          coordonn�es du centre du cercle
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
 * \param x, y          coordonn�es du centre du disque
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

//! �crit du texte.
/*!
 * �crit le texte text, aux coordonn�es (x, y) et avec les param�tres
 * d'alignement flags.  Le texte est �crit avec la couleur de dessin
 * courante.  Les flags sont une combinaison (ou binaire) de
 * Qt::AlignLeft, Qt::AligneRight, Qt::AlignHCenter, Qt::AlignTop,
 * Qt::AlignBottom, Qt::AlignVCenter, Qt::AlignCenter.  Par d�faut, le
 * texte est align� en haut � gauche.
 *
 * \param x, y          coordonn�es du texte
 * \param text          texte � �crire
 * \param flags         param�tres d'alignement
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

//! �crit du texte sur fond color�.
/*!
 * �crit du texte comme drawText, mais l'arri�re-plan est color� avec
 * la couleur de fond courante.
 *
 * \param x, y          coordonn�es du texte
 * \param text          texte � �crire
 * \param flags         param�tres d'alignement
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
 * Retourne la couleur du pixel de coordonn�es (x, y).  La valeur
 * retourn�e peut servir de param�tres � setColor(unsigned int) ou
 * setBgColor(unsigned int).
 *
 * \param x, y          coordonn�es du pixel
 * \return              couleur du pixel
 *
 * \see setColor(unsigned int), setBgColor(unsigned int)
 */
unsigned int DrawingWindow::getPointColor(int x, int y)
{
    return image->pixel(x, y);
}

//! Synchronise le contenu de la fen�tre.
/*!
 * Pour des raisons d'efficacit�s, le r�sultat des fonctions de dessin
 * n'est pas affich� imm�diatement.  L'appel � sync permet de
 * synchroniser le contenu de la fen�tre.  Autrement dit, cela bloque
 * l'ex�cution du programme jusqu'� ce que le contenu de la fen�tre
 * soit � jour.
 *
 * \param time          dur�e maximale de l'attente
 * \return              true si la fen�tre a pu �tre synchronis�e
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

//! Ferme la fen�tre graphique.
void DrawingWindow::closeGraph()
{
    qApp->postEvent(this, new CloseRequestEvent());
}

//! Suspend l'ex�cution pendant un certain temps.
/*!
 * \param secs          temps d'attente en seconde
 */
void DrawingWindow::sleep(unsigned long secs)
{
    DrawingThread::sleep(secs);
}

//! Suspend l'ex�cution pendant un certain temps.
/*!
 * \param msecs          temps d'attente en millisecondes
 */
void DrawingWindow::msleep(unsigned long msecs)
{
    DrawingThread::msleep(msecs);
}

//! Suspend l'ex�cution pendant un certain temps.
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
    terminateThread = true;     // this flag is needed for the case
                                // where the following wakeAll() call
                                // occurs between the
                                // setTerminationEnable(false) and the
                                // mutex lock in safeLock() called
                                // from sync()
    syncCondition.wakeAll();
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
 * Fonction appel�e par les diff�rents constructeurs.
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
 * S'assure que le thread courant ne peut pas �tre termin� s'il
 * d�tient un mutex.  Pendant de safeUnlock.
 *
 * \param mutex         le mutex � verrouiller
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

//! D�verrouille un mutex.
/*!
 * S'assure que le thread courant ne peut pas �tre termin� s'il
 * d�tient un mutex.  Pendant de safeLock.
 *
 * \param mutex         le mutex � d�verrouiller
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

//! Marque l'image enti�re comme non � jour.
inline
void DrawingWindow::dirty()
{
    dirtyFlag = true;
    dirtyRect = image->rect();
}

//! Marque un point de l'image comme non � jour.
/*!
 * \param x, y          coordonn�es du point
 */
inline
void DrawingWindow::dirty(int x, int y)
{
    dirty(QRect(x, y, 1, 1));
}

//! Marque une zone de l'image comme non � jour.
/*!
 * La zone est d�finie par un rectangle dont les coordonn�es de deux
 * sommets opppos�s sont donn�es.
 *
 * \param x1, y1        coordonn�es d'un sommet du rectangle
 * \param x2, y2        coordonn�es du sommet oppos� du rectangle
 */
inline
void DrawingWindow::dirty(int x1, int y1, int x2, int y2)
{
    QRect r;
    r.setCoords(x1, y1, x2, y2);
    dirty(r.normalized());
}

//! Marque une zone de l'image comme non � jour.
/*!
 * \param rect          rectangle d�limitant la zone
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

//! G�n�re un update si besoin.
/*!
 * G�n�re une demande de mise � jour de la fen�tre (appel � update)
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
 * Le rendu de texte doit �tre fait dans le thread principal.  D'o�
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
    painter->drawText(r, flags, text, &r);
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

//! D�marre le thread si ce n'a pas encore �t� fait.
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
