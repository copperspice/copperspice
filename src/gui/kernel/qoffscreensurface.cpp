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

#include <qoffscreensurface.h>

#include "qscreen.h"
#include "qplatform_offscreensurface.h"
#include "qplatform_integration.h"
#include "qplatform_window.h"
#include "qwindow.h"

#include <qapplication_p.h>

class Q_GUI_EXPORT QOffscreenSurfacePrivate
{
   Q_DECLARE_PUBLIC(QOffscreenSurface)

 public:
   QOffscreenSurfacePrivate()
      : surfaceType(QSurface::OpenGLSurface), platformOffscreenSurface(nullptr), offscreenWindow(nullptr),
        requestedFormat(QSurfaceFormat::defaultFormat()), screen(nullptr), size(1, 1)
   {
   }

   ~QOffscreenSurfacePrivate() {
   }

   QSurface::SurfaceType surfaceType;
   QPlatformOffscreenSurface *platformOffscreenSurface;
   QWindow *offscreenWindow;
   QSurfaceFormat requestedFormat;
   QScreen *screen;
   QSize size;

 protected:
   QOffscreenSurface *q_ptr;

};

QOffscreenSurface::QOffscreenSurface(QScreen *targetScreen)
   : QSurface(Offscreen), d_ptr(new QOffscreenSurfacePrivate)
{
   d_ptr->q_ptr = this;
   Q_D(QOffscreenSurface);

   d->screen = targetScreen;
   if (! d->screen) {
      d->screen = QGuiApplication::primaryScreen();
   }

   //if your applications aborts here, then chances are your creating a QOffscreenSurface before
   //the screen list is populated.
   Q_ASSERT(d->screen);

   connect(d->screen, &QScreen::destroyed, this, &QOffscreenSurface::screenDestroyed);
}

QOffscreenSurface::~QOffscreenSurface()
{
   destroy();
}

/*!
    Returns the surface type of the offscreen surface.

    The surface type of an offscreen surface is always QSurface::OpenGLSurface.
*/
QOffscreenSurface::SurfaceType QOffscreenSurface::surfaceType() const
{
   Q_D(const QOffscreenSurface);
   return d->surfaceType;
}

/*!
    Allocates the platform resources associated with the offscreen surface.

    It is at this point that the surface format set using setFormat() gets resolved
    into an actual native surface.

    Call destroy() to free the platform resources if necessary.

    \note Some platforms require this function to be called on the main (GUI) thread.

    \sa destroy()
*/
void QOffscreenSurface::create()
{
   Q_D(QOffscreenSurface);
   if (!d->platformOffscreenSurface && !d->offscreenWindow) {
      d->platformOffscreenSurface = QGuiApplicationPrivate::platformIntegration()->createPlatformOffscreenSurface(this);

      // No platform offscreen surface, fallback to an invisible window
      if (!d->platformOffscreenSurface) {
         if (QThread::currentThread() != qGuiApp->thread()) {
            qWarning("QOffscreenSurface::create() Attempting to create an off screen surface outside of the GUI thread");
         }

         d->offscreenWindow = new QWindow(d->screen);
         d->offscreenWindow->setObjectName(QLatin1String("QOffscreenSurface"));
         // Remove this window from the global list since we do not want it to be destroyed when closing the app.
         // The QOffscreenSurface has to be usable even after exiting the event loop.
         QGuiApplicationPrivate::window_list.removeOne(d->offscreenWindow);
         d->offscreenWindow->setSurfaceType(QWindow::OpenGLSurface);
         d->offscreenWindow->setFormat(d->requestedFormat);
         d->offscreenWindow->setGeometry(0, 0, d->size.width(), d->size.height());
         d->offscreenWindow->create();
      }

      QPlatformSurfaceEvent e(QPlatformSurfaceEvent::SurfaceCreated);
      QGuiApplication::sendEvent(this, &e);
   }
}

/*!
    Releases the native platform resources associated with this offscreen surface.

    \sa create()
*/
void QOffscreenSurface::destroy()
{
   Q_D(QOffscreenSurface);

   QPlatformSurfaceEvent e(QPlatformSurfaceEvent::SurfaceAboutToBeDestroyed);
   QGuiApplication::sendEvent(this, &e);

   delete d->platformOffscreenSurface;
   d->platformOffscreenSurface = nullptr;

   if (d->offscreenWindow) {
      d->offscreenWindow->destroy();
      delete d->offscreenWindow;
      d->offscreenWindow = nullptr;
   }
}

/*!
    Returns \c true if this offscreen surface is valid; otherwise returns \c false.

    The offscreen surface is valid if the platform resources have been successfuly allocated.

    \sa create()
*/
bool QOffscreenSurface::isValid() const
{
   Q_D(const QOffscreenSurface);
   return (d->platformOffscreenSurface && d->platformOffscreenSurface->isValid())
      || (d->offscreenWindow && d->offscreenWindow->handle());
}

/*!
    Sets the offscreen surface \a format.

    The surface format will be resolved in the create() function. Calling
    this function after create() will not re-resolve the surface format of the native surface.

    \sa create(), destroy()
*/
void QOffscreenSurface::setFormat(const QSurfaceFormat &format)
{
   Q_D(QOffscreenSurface);
   d->requestedFormat = format;
}

/*!
    Returns the requested surfaceformat of this offscreen surface.

    If the requested format was not supported by the platform implementation,
    the requestedFormat will differ from the actual offscreen surface format.

    This is the value set with setFormat().

    \sa setFormat(), format()
 */
QSurfaceFormat QOffscreenSurface::requestedFormat() const
{
   Q_D(const QOffscreenSurface);
   return d->requestedFormat;
}

/*!
    Returns the actual format of this offscreen surface.

    After the offscreen surface has been created, this function will return the actual
    surface format of the surface. It might differ from the requested format if the requested
    format could not be fulfilled by the platform.

    \sa create(), requestedFormat()
*/
QSurfaceFormat QOffscreenSurface::format() const
{
   Q_D(const QOffscreenSurface);
   if (d->platformOffscreenSurface) {
      return d->platformOffscreenSurface->format();
   }
   if (d->offscreenWindow) {
      return d->offscreenWindow->format();
   }
   return d->requestedFormat;
}

/*!
    Returns the size of the offscreen surface.
*/
QSize QOffscreenSurface::size() const
{
   Q_D(const QOffscreenSurface);
   return d->size;
}

/*!
    Returns the screen to which the offscreen surface is connected.

    \sa setScreen()
*/
QScreen *QOffscreenSurface::screen() const
{
   Q_D(const QOffscreenSurface);
   return d->screen;
}

/*!
    Sets the screen to which the offscreen surface is connected.

    If the offscreen surface has been created, it will be recreated on the \a newScreen.

    \sa screen()
*/
void QOffscreenSurface::setScreen(QScreen *newScreen)
{
   Q_D(QOffscreenSurface);
   if (!newScreen) {
      newScreen = QGuiApplication::primaryScreen();
   }

   if (newScreen != d->screen) {
      const bool wasCreated = d->platformOffscreenSurface != nullptr || d->offscreenWindow != nullptr;

      if (wasCreated) {
         destroy();
      }

      if (d->screen) {
         disconnect(d->screen, &QScreen::destroyed, this, &QOffscreenSurface::screenDestroyed);
      }

      d->screen = newScreen;
      if (newScreen) {
         connect(d->screen, &QScreen::destroyed, this, &QOffscreenSurface::screenDestroyed);

         if (wasCreated) {
            create();
         }
      }
      emit screenChanged(newScreen);
   }
}

/*!
    Called when the offscreen surface's screen is destroyed.

    \internal
*/
void QOffscreenSurface::screenDestroyed(QObject *object)
{
   Q_D(QOffscreenSurface);
   if (object == static_cast<QObject *>(d->screen)) {
      setScreen(nullptr);
   }
}

/*!
    \fn QOffscreenSurface::screenChanged(QScreen *screen)

    This signal is emitted when an offscreen surface's \a screen changes, either
    by being set explicitly with setScreen(), or automatically when
    the window's screen is removed.
*/

/*!
    Returns the platform offscreen surface corresponding to the offscreen surface.

    \internal
*/
QPlatformOffscreenSurface *QOffscreenSurface::handle() const
{
   Q_D(const QOffscreenSurface);
   return d->platformOffscreenSurface;
}

/*!
    Returns the platform surface corresponding to the offscreen surface.

    \internal
*/
QPlatformSurface *QOffscreenSurface::surfaceHandle() const
{
   Q_D(const QOffscreenSurface);
   if (d->offscreenWindow) {
      return d->offscreenWindow->handle();
   }

   return d->platformOffscreenSurface;
}

