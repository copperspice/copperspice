/***********************************************************************
*
* Copyright (c) 2012-2024 Barbara Geller
* Copyright (c) 2012-2024 Ansel Sermersheim
*
* Copyright (c) 2015 The Qt Company Ltd.
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
*
* This file is part of CopperSpice.
*
* CopperSpice is free software. You can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
* https://www.gnu.org/licenses/
*
***********************************************************************/

#include <qdeclarativeview.h>
#include <qdeclarative.h>
#include <qdeclarativeitem.h>
#include <qdeclarativeengine.h>
#include <qdeclarativecontext.h>
#include <qdeclarativeglobal_p.h>
#include <qdeclarativeguard_p.h>
#include <qdeclarativedebugtrace_p.h>
#include <qdeclarativeinspectorservice_p.h>

#include <qscriptvalueiterator.h>
#include <qdebug.h>
#include <qtimer.h>
#include <qevent.h>
#include <qdir.h>
#include <qcoreapplication.h>
#include <qfontdatabase.h>
#include <qicon.h>
#include <qurl.h>
#include <qlayout.h>
#include <qwidget.h>
#include <qgraphicswidget.h>
#include <qbasictimer.h>
#include <QtCore/qabstractanimation.h>
#include <qgraphicsview_p.h>
#include <qdeclarativeitem_p.h>
#include <qabstractanimation_p.h>
#include <qdeclarativeitemchangelistener_p.h>

QT_BEGIN_NAMESPACE

DEFINE_BOOL_CONFIG_OPTION(frameRateDebug, QML_SHOW_FRAMERATE)

class QDeclarativeScene : public QGraphicsScene
{
 public:
   QDeclarativeScene(QObject *parent = nullptr);

 protected:
   virtual void keyPressEvent(QKeyEvent *);
   virtual void keyReleaseEvent(QKeyEvent *);

   virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *);
   virtual void mousePressEvent(QGraphicsSceneMouseEvent *);
   virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *);
};

QDeclarativeScene::QDeclarativeScene(QObject *parent) : QGraphicsScene(parent)
{
}

void QDeclarativeScene::keyPressEvent(QKeyEvent *e)
{
   QDeclarativeDebugTrace::addEvent(QDeclarativeDebugTrace::Key);

   QGraphicsScene::keyPressEvent(e);
}

void QDeclarativeScene::keyReleaseEvent(QKeyEvent *e)
{
   QDeclarativeDebugTrace::addEvent(QDeclarativeDebugTrace::Key);

   QGraphicsScene::keyReleaseEvent(e);
}

void QDeclarativeScene::mouseMoveEvent(QGraphicsSceneMouseEvent *e)
{
   QDeclarativeDebugTrace::addEvent(QDeclarativeDebugTrace::Mouse);

   QGraphicsScene::mouseMoveEvent(e);
}

void QDeclarativeScene::mousePressEvent(QGraphicsSceneMouseEvent *e)
{
   QDeclarativeDebugTrace::addEvent(QDeclarativeDebugTrace::Mouse);

   QGraphicsScene::mousePressEvent(e);
}

void QDeclarativeScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *e)
{
   QDeclarativeDebugTrace::addEvent(QDeclarativeDebugTrace::Mouse);

   QGraphicsScene::mouseReleaseEvent(e);
}

class QDeclarativeViewPrivate : public QGraphicsViewPrivate, public QDeclarativeItemChangeListener
{
   Q_DECLARE_PUBLIC(QDeclarativeView)
 public:
   QDeclarativeViewPrivate()
      : root(0), declarativeItemRoot(0), graphicsWidgetRoot(0), component(0),
        resizeMode(QDeclarativeView::SizeViewToRootObject), initialSize(0, 0) {}
   ~QDeclarativeViewPrivate() {
      delete root;
      delete engine;
   }
   void execute();
   void itemGeometryChanged(QDeclarativeItem *item, const QRectF &newGeometry, const QRectF &oldGeometry);
   void initResize();
   void updateSize();
   inline QSize rootObjectSize() const;

   QDeclarativeGuard<QGraphicsObject> root;
   QDeclarativeGuard<QDeclarativeItem> declarativeItemRoot;
   QDeclarativeGuard<QGraphicsWidget> graphicsWidgetRoot;

   QUrl source;

   QDeclarativeEngine *engine;
   QDeclarativeComponent *component;
   QBasicTimer resizetimer;

   QDeclarativeView::ResizeMode resizeMode;
   QSize initialSize;
   QElapsedTimer frameTimer;

   void init();
};

void QDeclarativeViewPrivate::execute()
{
   Q_Q(QDeclarativeView);
   if (root) {
      delete root;
      root = 0;
   }
   if (component) {
      delete component;
      component = 0;
   }
   if (!source.isEmpty()) {
      component = new QDeclarativeComponent(engine, source, q);
      if (!component->isLoading()) {
         q->continueExecute();
      } else {
         QObject::connect(component, SIGNAL(statusChanged(QDeclarativeComponent::Status)), q, SLOT(continueExecute()));
      }
   }
}

void QDeclarativeViewPrivate::itemGeometryChanged(QDeclarativeItem *resizeItem, const QRectF &newGeometry,
      const QRectF &oldGeometry)
{
   Q_Q(QDeclarativeView);
   if (resizeItem == root && resizeMode == QDeclarativeView::SizeViewToRootObject) {
      // wait for both width and height to be changed
      resizetimer.start(0, q);
   }
   QDeclarativeItemChangeListener::itemGeometryChanged(resizeItem, newGeometry, oldGeometry);
}

/*!
    \class QDeclarativeView
    \since 4.7
    \brief The QDeclarativeView class provides a widget for displaying a Qt Declarative user interface.

    QDeclarativeItem objects can be placed on a standard QGraphicsScene and
    displayed with QGraphicsView. QDeclarativeView is a QGraphicsView subclass
    provided as a convenience for displaying QML files, and connecting between
    QML and C++ Qt objects.

    QDeclarativeView provides:

    \list
    \o Management of QDeclarativeComponent loading and object creation
    \o Initialization of QGraphicsView for optimal performance with QML using these settings:
        \list
        \o QGraphicsView::setOptimizationFlags(QGraphicsView::DontSavePainterState)
        \o QGraphicsView::setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate)
        \o QGraphicsScene::setItemIndexMethod(QGraphicsScene::NoIndex)
        \endlist
    \o Initialization of QGraphicsView for QML key handling using these settings:
        \list
        \o QGraphicsView::viewport()->setFocusPolicy(Qt::NoFocus)
        \o QGraphicsView::setFocusPolicy(Qt::StrongFocus)
        \o QGraphicsScene::setStickyFocus(true)
        \endlist
    \endlist

    Typical usage:

    \code
    QDeclarativeView *view = new QDeclarativeView;
    view->setSource(QUrl::fromLocalFile("myqmlfile.qml"));
    view->show();
    \endcode

    Since QDeclarativeView is a QWidget-based class, it can be used to
    display QML interfaces within QWidget-based GUI applications that do not
    use the Graphics View framework.

    To receive errors related to loading and executing QML with QDeclarativeView,
    you can connect to the statusChanged() signal and monitor for QDeclarativeView::Error.
    The errors are available via QDeclarativeView::errors().

    If you're using your own QGraphicsScene-based scene with QDeclarativeView, remember to
    enable scene's sticky focus mode and to set itemIndexMethod to QGraphicsScene::NoIndex.

    \sa {Integrating QML Code with Existing Qt UI Code}, {Using QML Bindings in C++ Applications}
*/


/*! \fn void QDeclarativeView::sceneResized(QSize size)
  This signal is emitted when the view is resized to \a size.
*/

/*! \fn void QDeclarativeView::statusChanged(QDeclarativeView::Status status)
    This signal is emitted when the component's current \a status changes.
*/

/*!
  \fn QDeclarativeView::QDeclarativeView(QWidget *parent)

  Constructs a QDeclarativeView with the given \a parent.
*/
QDeclarativeView::QDeclarativeView(QWidget *parent)
   : QGraphicsView(*(new QDeclarativeViewPrivate), parent)
{
   Q_D(QDeclarativeView);
   setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
   d->init();
}

/*!
  \fn QDeclarativeView::QDeclarativeView(const QUrl &source, QWidget *parent)

  Constructs a QDeclarativeView with the given QML \a source and \a parent.
*/
QDeclarativeView::QDeclarativeView(const QUrl &source, QWidget *parent)
   : QGraphicsView(*(new QDeclarativeViewPrivate), parent)
{
   Q_D(QDeclarativeView);
   setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
   d->init();
   setSource(source);
}

void QDeclarativeViewPrivate::init()
{
   Q_Q(QDeclarativeView);
   engine = new QDeclarativeEngine();
   q->setScene(new QDeclarativeScene(q));

   q->setOptimizationFlags(QGraphicsView::DontSavePainterState);
   q->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
   q->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
   q->setFrameStyle(0);

   // These seem to give the best performance
   q->setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
   q->scene()->setItemIndexMethod(QGraphicsScene::NoIndex);
   q->viewport()->setFocusPolicy(Qt::NoFocus);
   q->setFocusPolicy(Qt::StrongFocus);

   q->scene()->setStickyFocus(true);  //### needed for correct focus handling

#ifdef QDECLARATIVEVIEW_NOBACKGROUND
   q->setAttribute(Qt::WA_OpaquePaintEvent);
   q->setAttribute(Qt::WA_NoSystemBackground);
   q->viewport()->setAttribute(Qt::WA_OpaquePaintEvent);
   q->viewport()->setAttribute(Qt::WA_NoSystemBackground);
#endif

   QDeclarativeInspectorService::instance()->addView(q);
}

/*!
    Destroys the view.
 */
QDeclarativeView::~QDeclarativeView()
{
   QDeclarativeInspectorService::instance()->removeView(this);
}

/*! \property QDeclarativeView::source
  \brief The URL of the source of the QML component.

  Changing this property causes the QML component to be reloaded.

    Ensure that the URL provided is full and correct, in particular, use
    \l QUrl::fromLocalFile() when loading a file from the local filesystem.

    \sa {Network Transparency}{Loading Resources in QML}
 */

/*!
    Sets the source to the \a url, loads the QML component and instantiates it.

    Ensure that the URL provided is full and correct, in particular, use
    \l QUrl::fromLocalFile() when loading a file from the local filesystem.

    Calling this methods multiple times with the same url will result
    in the QML being reloaded.
 */
void QDeclarativeView::setSource(const QUrl &url)
{
   Q_D(QDeclarativeView);
   d->source = url;
   d->execute();
}

/*!
  Returns the source URL, if set.

  \sa setSource()
 */
QUrl QDeclarativeView::source() const
{
   Q_D(const QDeclarativeView);
   return d->source;
}

/*!
  Returns a pointer to the QDeclarativeEngine used for instantiating
  QML Components.
 */
QDeclarativeEngine *QDeclarativeView::engine() const
{
   Q_D(const QDeclarativeView);
   return d->engine;
}

/*!
  This function returns the root of the context hierarchy.  Each QML
  component is instantiated in a QDeclarativeContext.  QDeclarativeContext's are
  essential for passing data to QML components.  In QML, contexts are
  arranged hierarchically and this hierarchy is managed by the
  QDeclarativeEngine.
 */
QDeclarativeContext *QDeclarativeView::rootContext() const
{
   Q_D(const QDeclarativeView);
   return d->engine->rootContext();
}

/*!
    \enum QDeclarativeView::Status
    Specifies the loading status of the QDeclarativeView.

    \value Null This QDeclarativeView has no source set.
    \value Ready This QDeclarativeView has loaded and created the QML component.
    \value Loading This QDeclarativeView is loading network data.
    \value Error One or more errors has occurred. Call errors() to retrieve a list
           of errors.
*/

/*! \enum QDeclarativeView::ResizeMode

  This enum specifies how to resize the view.

  \value SizeViewToRootObject The view resizes with the root item in the QML.
  \value SizeRootObjectToView The view will automatically resize the root item to the size of the view.
*/

/*!
    \property QDeclarativeView::status
    The component's current \l{QDeclarativeView::Status} {status}.
*/

QDeclarativeView::Status QDeclarativeView::status() const
{
   Q_D(const QDeclarativeView);
   if (!d->component) {
      return QDeclarativeView::Null;
   }

   return QDeclarativeView::Status(d->component->status());
}

/*!
    Return the list of errors that occurred during the last compile or create
    operation.  When the status is not Error, an empty list is returned.
*/
QList<QDeclarativeError> QDeclarativeView::errors() const
{
   Q_D(const QDeclarativeView);
   if (d->component) {
      return d->component->errors();
   }
   return QList<QDeclarativeError>();
}

/*!
    \property QDeclarativeView::resizeMode
    \brief whether the view should resize the canvas contents

    If this property is set to SizeViewToRootObject (the default), the view
    resizes with the root item in the QML.

    If this property is set to SizeRootObjectToView, the view will
    automatically resize the root item.

    Regardless of this property, the sizeHint of the view
    is the initial size of the root item. Note though that
    since QML may load dynamically, that size may change.
*/

void QDeclarativeView::setResizeMode(ResizeMode mode)
{
   Q_D(QDeclarativeView);
   if (d->resizeMode == mode) {
      return;
   }

   if (d->declarativeItemRoot) {
      if (d->resizeMode == SizeViewToRootObject) {
         QDeclarativeItemPrivate *p =
            static_cast<QDeclarativeItemPrivate *>(QGraphicsItemPrivate::get(d->declarativeItemRoot));
         p->removeItemChangeListener(d, QDeclarativeItemPrivate::Geometry);
      }
   } else if (d->graphicsWidgetRoot) {
      if (d->resizeMode == QDeclarativeView::SizeViewToRootObject) {
         d->graphicsWidgetRoot->removeEventFilter(this);
      }
   }

   d->resizeMode = mode;
   if (d->root) {
      d->initResize();
   }
}

void QDeclarativeViewPrivate::initResize()
{
   Q_Q(QDeclarativeView);
   if (declarativeItemRoot) {
      if (resizeMode == QDeclarativeView::SizeViewToRootObject) {
         QDeclarativeItemPrivate *p =
            static_cast<QDeclarativeItemPrivate *>(QGraphicsItemPrivate::get(declarativeItemRoot));
         p->addItemChangeListener(this, QDeclarativeItemPrivate::Geometry);
      }
   } else if (graphicsWidgetRoot) {
      if (resizeMode == QDeclarativeView::SizeViewToRootObject) {
         graphicsWidgetRoot->installEventFilter(q);
      }
   }
   updateSize();
}

void QDeclarativeViewPrivate::updateSize()
{
   Q_Q(QDeclarativeView);
   if (!root) {
      return;
   }
   if (declarativeItemRoot) {
      if (resizeMode == QDeclarativeView::SizeViewToRootObject) {
         QSize newSize = QSize(declarativeItemRoot->width(), declarativeItemRoot->height());
         if (newSize.isValid() && newSize != q->size()) {
            q->resize(newSize);
         }
      } else if (resizeMode == QDeclarativeView::SizeRootObjectToView) {
         if (!qFuzzyCompare(q->width(), declarativeItemRoot->width())) {
            declarativeItemRoot->setWidth(q->width());
         }
         if (!qFuzzyCompare(q->height(), declarativeItemRoot->height())) {
            declarativeItemRoot->setHeight(q->height());
         }
      }
   } else if (graphicsWidgetRoot) {
      if (resizeMode == QDeclarativeView::SizeViewToRootObject) {
         QSize newSize = QSize(graphicsWidgetRoot->size().width(), graphicsWidgetRoot->size().height());
         if (newSize.isValid() && newSize != q->size()) {
            q->resize(newSize);
         }
      } else if (resizeMode == QDeclarativeView::SizeRootObjectToView) {
         QSizeF newSize = QSize(q->size().width(), q->size().height());
         if (newSize.isValid() && newSize != graphicsWidgetRoot->size()) {
            graphicsWidgetRoot->resize(newSize);
         }
      }
   }
   q->updateGeometry();
}

QSize QDeclarativeViewPrivate::rootObjectSize() const
{
   QSize rootObjectSize(0, 0);
   int widthCandidate = -1;
   int heightCandidate = -1;
   if (root) {
      QSizeF size = root->boundingRect().size();
      widthCandidate = size.width();
      heightCandidate = size.height();
   }
   if (widthCandidate > 0) {
      rootObjectSize.setWidth(widthCandidate);
   }
   if (heightCandidate > 0) {
      rootObjectSize.setHeight(heightCandidate);
   }
   return rootObjectSize;
}

QDeclarativeView::ResizeMode QDeclarativeView::resizeMode() const
{
   Q_D(const QDeclarativeView);
   return d->resizeMode;
}

/*!
  \internal
 */
void QDeclarativeView::continueExecute()
{
   Q_D(QDeclarativeView);
   disconnect(d->component, SIGNAL(statusChanged(QDeclarativeComponent::Status)), this, SLOT(continueExecute()));

   if (d->component->isError()) {
      QList<QDeclarativeError> errorList = d->component->errors();
      foreach (const QDeclarativeError & error, errorList) {
         qWarning() << error;
      }
      emit statusChanged(status());
      return;
   }

   QObject *obj = d->component->create();

   if (d->component->isError()) {
      QList<QDeclarativeError> errorList = d->component->errors();
      foreach (const QDeclarativeError & error, errorList) {
         qWarning() << error;
      }
      emit statusChanged(status());
      return;
   }

   setRootObject(obj);
   emit statusChanged(status());
}

/*!
  \internal
*/
void QDeclarativeView::setRootObject(QObject *obj)
{
   Q_D(QDeclarativeView);
   if (d->root == obj || !scene()) {
      return;
   }
   if (QDeclarativeItem *declarativeItem = qobject_cast<QDeclarativeItem *>(obj)) {
      scene()->addItem(declarativeItem);
      d->root = declarativeItem;
      d->declarativeItemRoot = declarativeItem;
   } else if (QGraphicsObject *graphicsObject = qobject_cast<QGraphicsObject *>(obj)) {
      scene()->addItem(graphicsObject);
      d->root = graphicsObject;
      if (graphicsObject->isWidget()) {
         d->graphicsWidgetRoot = static_cast<QGraphicsWidget *>(graphicsObject);
      } else {
         qWarning() << "QDeclarativeView::resizeMode is not honored for components of type QGraphicsObject";
      }
   } else if (obj) {
      qWarning() << "QDeclarativeView only supports loading of root objects that derive from QGraphicsObject";
      if (QWidget *widget  = qobject_cast<QWidget *>(obj)) {
         window()->setAttribute(Qt::WA_OpaquePaintEvent, false);
         window()->setAttribute(Qt::WA_NoSystemBackground, false);
         if (layout() && layout()->count()) {
            // Hide the QGraphicsView in GV mode.
            QLayoutItem *item = layout()->itemAt(0);
            if (item->widget()) {
               item->widget()->hide();
            }
         }
         widget->setParent(this);
         if (isVisible()) {
            widget->setVisible(true);
         }
         resize(widget->size());
      }
   }

   if (d->root) {
      d->initialSize = d->rootObjectSize();
      if (d->initialSize != size()) {
         if (!(parentWidget() && parentWidget()->layout())) {
            resize(d->initialSize);
         }
      }
      d->initResize();
   }
}

/*!
  \internal
  If the \l {QTimerEvent} {timer event} \a e is this
  view's resize timer, sceneResized() is emitted.
 */
void QDeclarativeView::timerEvent(QTimerEvent *e)
{
   Q_D(QDeclarativeView);
   if (!e || e->timerId() == d->resizetimer.timerId()) {
      d->updateSize();
      d->resizetimer.stop();
   }
}

/*! \internal */
bool QDeclarativeView::eventFilter(QObject *watched, QEvent *e)
{
   Q_D(QDeclarativeView);
   if (watched == d->root && d->resizeMode == SizeViewToRootObject) {
      if (d->graphicsWidgetRoot) {
         if (e->type() == QEvent::GraphicsSceneResize) {
            d->updateSize();
         }
      }
   }
   return QGraphicsView::eventFilter(watched, e);
}

/*!
    \internal
    Preferred size follows the root object geometry.
*/
QSize QDeclarativeView::sizeHint() const
{
   Q_D(const QDeclarativeView);
   QSize rootObjectSize = d->rootObjectSize();
   if (rootObjectSize.isEmpty()) {
      return size();
   } else {
      return rootObjectSize;
   }
}

/*!
  Returns the initial size of the root object
*/
QSize QDeclarativeView::initialSize() const
{
   Q_D(const QDeclarativeView);
   return d->initialSize;
}

/*!
  Returns the view's root \l {QGraphicsObject} {item}.
 */
QGraphicsObject *QDeclarativeView::rootObject() const
{
   Q_D(const QDeclarativeView);
   return d->root;
}

/*!
  \internal
  This function handles the \l {QResizeEvent} {resize event}
  \a e.
 */
void QDeclarativeView::resizeEvent(QResizeEvent *e)
{
   Q_D(QDeclarativeView);
   if (d->resizeMode == SizeRootObjectToView) {
      d->updateSize();
   }
   if (d->declarativeItemRoot) {
      setSceneRect(QRectF(0, 0, d->declarativeItemRoot->width(), d->declarativeItemRoot->height()));
   } else if (d->root) {
      setSceneRect(d->root->boundingRect());
   } else {
      setSceneRect(rect());
   }
   emit sceneResized(e->size());
   QGraphicsView::resizeEvent(e);
}

/*!
    \internal
*/
void QDeclarativeView::paintEvent(QPaintEvent *event)
{
   Q_D(QDeclarativeView);

   QDeclarativeDebugTrace::addEvent(QDeclarativeDebugTrace::FramePaint);
   QDeclarativeDebugTrace::startRange(QDeclarativeDebugTrace::Painting);

   int time = 0;
   if (frameRateDebug()) {
      time = d->frameTimer.restart();
   }

   QGraphicsView::paintEvent(event);

   QDeclarativeDebugTrace::endRange(QDeclarativeDebugTrace::Painting);

   if (frameRateDebug()) {
      qDebug() << "paintEvent:" << d->frameTimer.elapsed() << "time since last frame:" << time;
   }

#if QT_SHOW_DECLARATIVEVIEW_FPS
   static QTime timer;
   static int frames;

   if (frames == 0) {
      timer.start();
   } else if (timer.elapsed() > 5000) {
      qreal avgtime = timer.elapsed() / (qreal) frames;
      qDebug("Average time per frame: %f ms (%i fps)", avgtime, int(1000 / avgtime));
      timer.start();
      frames = 0;
   }
   ++frames;
   scene()->update();
#endif

}

QT_END_NAMESPACE
