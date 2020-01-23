/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#include "qwindowcontainer_p.h"
#include "qwidget_p.h"
#include <qwindow.h>
#include <qguiapplication_p.h>
#include <qplatform_integration.h>
#include <QDebug>
#include <QMdiSubWindow>
#include <QAbstractScrollArea>

class QWindowContainerPrivate : public QWidgetPrivate
{
 public:
   Q_DECLARE_PUBLIC(QWindowContainer)

   QWindowContainerPrivate()
      : window(0)
      , oldFocusWindow(0)
      , usesNativeWidgets(false) {
   }

   ~QWindowContainerPrivate() { }

   static QWindowContainerPrivate *get(QWidget *w) {
      QWindowContainer *wc = qobject_cast<QWindowContainer *>(w);
      if (wc) {
         return wc->d_func();
      }
      return 0;
   }

   void updateGeometry() {
      Q_Q(QWindowContainer);
      if (!q->isWindow() && (q->geometry().bottom() <= 0 || q->geometry().right() <= 0))
         /* Qt (e.g. QSplitter) sometimes prefer to hide a widget by *not* calling
            setVisible(false). This is often done by setting its coordinates to a sufficiently
            negative value so that its clipped outside the parent. Since a QWindow is not clipped
            to widgets in general, it needs to be dealt with as a special case.
         */
      {
         window->setGeometry(q->geometry());
      } else if (usesNativeWidgets) {
         window->setGeometry(q->rect());
      } else {
         window->setGeometry(QRect(q->mapTo(q->window(), QPoint()), q->size()));
      }
   }

   void updateUsesNativeWidgets() {
      if (usesNativeWidgets || window->parent() == 0) {
         return;
      }
      Q_Q(QWindowContainer);
      if (q->internalWinId()) {
         // Allow use native widgets if the window container is already a native widget
         usesNativeWidgets = true;
         return;
      }

      QWidget *p = q->parentWidget();

      while (p) {
         if (

#ifndef QT_NO_MDIAREA
            qobject_cast<QMdiSubWindow *>(p) != 0 ||
#endif

            qobject_cast<QAbstractScrollArea *>(p) != 0) {
            q->winId();
            usesNativeWidgets = true;
            break;
         }
         p = p->parentWidget();
      }
   }

   void markParentChain() {
      Q_Q(QWindowContainer);

      QWidget *parentContainer = q;

      while (parentContainer) {
         QWidgetPrivate *obj = static_cast<QWidgetPrivate *>(QWidgetPrivate::get(parentContainer));

         obj->createExtra();
         obj->extra->hasWindowContainer = true;

         parentContainer = parentContainer->parentWidget();
      }
   }

   bool isStillAnOrphan() const {
      return window->parent() == &fakeParent;
   }

   QPointer<QWindow> window;
   QWindow *oldFocusWindow;
   QWindow fakeParent;

   uint usesNativeWidgets : 1;
};



/*!
    \fn QWidget *QWidget::createWindowContainer(QWindow *window, QWidget *parent, Qt::WindowFlags flags);

    Creates a QWidget that makes it possible to embed \a window into
    a QWidget-based application.

    The window container is created as a child of \a parent and with
    window flags \a flags.

    Once the window has been embedded into the container, the
    container will control the window's geometry and
    visibility. Explicit calls to QWindow::setGeometry(),
    QWindow::show() or QWindow::hide() on an embedded window is not
    recommended.

    The container takes over ownership of \a window. The window can
    be removed from the window container with a call to
    QWindow::setParent().

    The window container is attached as a native child window to the
    toplevel window it is a child of. When a window container is used
    as a child of a QAbstractScrollArea or QMdiArea, it will
    create a \l {Native Widgets vs Alien Widgets} {native window} for
    every widget in its parent chain to allow for proper stacking and
    clipping in this use case. Creating a native window for the window
    container also allows for proper stacking and clipping. This must
    be done before showing the window container. Applications with
    many native child windows may suffer from performance issues.

    The window container has a number of known limitations:

    \list

    \li Stacking order; The embedded window will stack on top of the
    widget hierarchy as an opaque box. The stacking order of multiple
    overlapping window container instances is undefined.

    \li Rendering Integration; The window container does not interoperate
    with QGraphicsProxyWidget, QWidget::render() or similar functionality.

    \li Focus Handling; It is possible to let the window container
    instance have any focus policy and it will delegate focus to the
    window via a call to QWindow::requestActivate(). However,
    returning to the normal focus chain from the QWindow instance will
    be up to the QWindow instance implementation itself. For instance,
    when entering a Qt Quick based window with tab focus, it is quite
    likely that further tab presses will only cycle inside the QML
    application. Also, whether QWindow::requestActivate() actually
    gives the window focus, is platform dependent.

    \li Using many window container instances in a QWidget-based
    application can greatly hurt the overall performance of the
    application.

    \endlist
 */

QWidget *QWidget::createWindowContainer(QWindow *window, QWidget *parent, Qt::WindowFlags flags)
{
   return new QWindowContainer(window, parent, flags);
}



/*!
    \internal
 */

QWindowContainer::QWindowContainer(QWindow *embeddedWindow, QWidget *parent, Qt::WindowFlags flags)
   : QWidget(*new QWindowContainerPrivate, parent, flags)
{
   Q_D(QWindowContainer);
   if (!embeddedWindow) {
      qWarning("QWindowContainer: embedded window cannot be null");
      return;
   }

   // The embedded QWindow must use the same logic as QWidget when it comes to the surface type.
   // Otherwise we may end up with BadMatch failures on X11.
   if (embeddedWindow->surfaceType() == QSurface::RasterSurface
      && QGuiApplicationPrivate::platformIntegration()->hasCapability(QPlatformIntegration::RasterGLSurface)
      && !QApplication::testAttribute(Qt::AA_ForceRasterWidgets)) {
      embeddedWindow->setSurfaceType(QSurface::RasterGLSurface);
   }

   d->window = embeddedWindow;
   d->window->setParent(&d->fakeParent);
   setAcceptDrops(true);

   connect(QGuiApplication::instance(), SIGNAL(focusWindowChanged(QWindow *)), this, SLOT(focusWindowChanged(QWindow *)));
}

QWindow *QWindowContainer::containedWindow() const
{
   Q_D(const QWindowContainer);
   return d->window;
}

/*!
    \internal
 */

QWindowContainer::~QWindowContainer()
{
   Q_D(QWindowContainer);
   delete d->window;
}



/*!
    \internal
 */

void QWindowContainer::focusWindowChanged(QWindow *focusWindow)
{
   Q_D(QWindowContainer);
   d->oldFocusWindow = focusWindow;
   if (focusWindow == d->window) {
      QWidget *widget = QApplication::focusWidget();
      if (widget) {
         widget->clearFocus();
      }
   }
}

/*!
    \internal
 */

bool QWindowContainer::event(QEvent *e)
{
   Q_D(QWindowContainer);
   if (!d->window) {
      return QWidget::event(e);
   }

   QEvent::Type type = e->type();
   switch (type) {
      case QEvent::ChildRemoved: {
         QChildEvent *ce = static_cast<QChildEvent *>(e);
         if (ce->child() == d->window) {
            d->window = 0;
         }
         break;
      }
      // The only thing we are interested in is making sure our sizes stay
      // in sync, so do a catch-all case.
      case QEvent::Resize:
         d->updateGeometry();
         break;
      case QEvent::Move:
         d->updateGeometry();
         break;
      case QEvent::PolishRequest:
         d->updateGeometry();
         break;
      case QEvent::Show:
         d->updateUsesNativeWidgets();
         if (d->isStillAnOrphan()) {
            d->window->setParent(d->usesNativeWidgets
               ? windowHandle()
               : window()->windowHandle());
         }
         if (d->window->parent()) {
            d->markParentChain();
            d->window->show();
         }
         break;
      case QEvent::Hide:
         if (d->window->parent()) {
            d->window->hide();
         }
         break;
      case QEvent::FocusIn:
         if (d->window->parent()) {
            if (d->oldFocusWindow != d->window) {
               d->window->requestActivate();
            } else {
               QWidget *next = nextInFocusChain();
               next->setFocus();
            }
         }
         break;
#ifndef QT_NO_DRAGANDDROP
      case QEvent::Drop:
      case QEvent::DragMove:
      case QEvent::DragLeave:
         QCoreApplication::sendEvent(d->window, e);
         return e->isAccepted();
      case QEvent::DragEnter:
         // Don't reject drag events for the entire widget when one
         // item rejects the drag enter
         QCoreApplication::sendEvent(d->window, e);
         e->accept();
         return true;
#endif
      default:
         break;
   }

   return QWidget::event(e);
}

typedef void (*qwindowcontainer_traverse_callback)(QWidget *parent);

static void qwindowcontainer_traverse(QWidget *parent, qwindowcontainer_traverse_callback callback)
{
   const QObjectList &children = parent->children();

   for (int i = 0; i < children.size(); ++i) {
      QWidget *w = qobject_cast<QWidget *>(children.at(i));

      if (w) {
         QWidgetPrivate *wd = static_cast<QWidgetPrivate *>(QWidgetPrivate::get(w));

         if (wd->extra && wd->extra->hasWindowContainer) {
            callback(w);
         }
      }
   }
}

void QWindowContainer::toplevelAboutToBeDestroyed(QWidget *parent)
{
   if (QWindowContainerPrivate *d = QWindowContainerPrivate::get(parent)) {
      d->window->setParent(&d->fakeParent);
   }

   qwindowcontainer_traverse(parent, toplevelAboutToBeDestroyed);
}

void QWindowContainer::parentWasChanged(QWidget *parent)
{
   if (QWindowContainerPrivate *d = QWindowContainerPrivate::get(parent)) {

      if (d->window->parent()) {
         d->updateUsesNativeWidgets();
         d->markParentChain();

         QWidget *toplevel = d->usesNativeWidgets ? parent : parent->window();

         if (!toplevel->windowHandle()) {
            QWidgetPrivate *tld = static_cast<QWidgetPrivate *>(QWidgetPrivate::get(toplevel));
            tld->createTLExtra();
            tld->createTLSysExtra();
            Q_ASSERT(toplevel->windowHandle());
         }

         d->window->setParent(toplevel->windowHandle());
         d->updateGeometry();
      }
   }

   qwindowcontainer_traverse(parent, parentWasChanged);
}

void QWindowContainer::parentWasMoved(QWidget *parent)
{
   if (QWindowContainerPrivate *d = QWindowContainerPrivate::get(parent)) {
      if (d->window->parent()) {
         d->updateGeometry();
      }
   }

   qwindowcontainer_traverse(parent, parentWasMoved);
}

void QWindowContainer::parentWasRaised(QWidget *parent)
{
   if (QWindowContainerPrivate *d = QWindowContainerPrivate::get(parent)) {
      if (d->window->parent()) {
         d->window->raise();
      }
   }

   qwindowcontainer_traverse(parent, parentWasRaised);
}

void QWindowContainer::parentWasLowered(QWidget *parent)
{
   if (QWindowContainerPrivate *d = QWindowContainerPrivate::get(parent)) {
      if (d->window->parent()) {
         d->window->lower();
      }
   }

   qwindowcontainer_traverse(parent, parentWasLowered);
}

