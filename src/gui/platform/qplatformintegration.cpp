/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#include <qplatformintegration.h>

#include <qscreen.h>
#include <qplatformfontdatabase.h>

// emerald    #include <qplatformclipboard.h>
// emerald    #include <qplatformaccessibility.h>
// emerald    #include <qplatformtheme.h>

#include <qapplication_p.h>
#include <qpixmap_raster_p.h>
#include <qdnd_p.h>

// emerald    #include <qsimpledrag_p.h>

#ifndef QT_NO_SESSIONMANAGER
// emerald    #include <qplatformsessionmanager.h>
#endif

QPlatformFontDatabase *QPlatformIntegration::fontDatabase() const
{
    static QPlatformFontDatabase *db = nullptr;
    if (! db) {
        db = new QPlatformFontDatabase;
    }

    return db;
}

#ifndef QT_NO_CLIPBOARD

/* emerald

QPlatformClipboard *QPlatformIntegration::clipboard() const
{
    static QPlatformClipboard *clipboard = nullptr;
    if (!clipboard) {
        clipboard = new QPlatformClipboard;
    }
    return clipboard;
}

*/

#endif

#ifndef QT_NO_DRAGANDDROP

/* emerald

QPlatformDrag *QPlatformIntegration::drag() const
{
    static QSimpleDrag *drag = 0;
    if (!drag) {
        drag = new QSimpleDrag;
    }
    return drag;
}

*/

#endif

QPlatformNativeInterface * QPlatformIntegration::nativeInterface() const
{
    return 0;
}

QPlatformServices *QPlatformIntegration::services() const
{
    return 0;
}

bool QPlatformIntegration::hasCapability(Capability cap) const
{
    return cap == NonFullScreenWindows || cap == NativeWidgets || cap == WindowManagement;
}

/* emerald

QPlatformPixmap *QPlatformIntegration::createPlatformPixmap(QPlatformPixmap::PixelType type) const
{
    return new QRasterPlatformPixmap(type);
}

*/

#ifndef QT_NO_OPENGL

/* emerald

QPlatformOpenGLContext *QPlatformIntegration::createPlatformOpenGLContext(QOpenGLContext *context) const
{
    qWarning("This plugin does not support createPlatformOpenGLContext!");
    return 0;
}

*/

#endif

/* emerald

QPlatformSharedGraphicsCache *QPlatformIntegration::createPlatformSharedGraphicsCache(const char *cacheId) const
{
    qWarning("This plugin does not support createPlatformSharedGraphicsBuffer for cacheId: %s!", cacheId);
    return 0;
}

QPaintEngine *QPlatformIntegration::createImagePaintEngine(QPaintDevice *paintDevice) const
{
    return 0;
}

*/

void QPlatformIntegration::initialize()
{
}

void QPlatformIntegration::destroy()
{
}


/* emerald


QPlatformInputContext *QPlatformIntegration::inputContext() const
{
    return 0;
}

*/


#ifndef QT_NO_ACCESSIBILITY

/* emerald

QPlatformAccessibility *QPlatformIntegration::accessibility() const
{
    return 0;
}

*/

#endif

QVariant QPlatformIntegration::styleHint(StyleHint hint) const
{
    switch (hint) {

/* emerald
       case CursorFlashTime:
              return QPlatformTheme::defaultThemeHint(QPlatformTheme::CursorFlashTime);

          case KeyboardInputInterval:
              return QPlatformTheme::defaultThemeHint(QPlatformTheme::KeyboardInputInterval);

          case KeyboardAutoRepeatRate:
              return QPlatformTheme::defaultThemeHint(QPlatformTheme::KeyboardAutoRepeatRate);

          case MouseDoubleClickInterval:
              return QPlatformTheme::defaultThemeHint(QPlatformTheme::MouseDoubleClickInterval);

          case StartDragDistance:
              return QPlatformTheme::defaultThemeHint(QPlatformTheme::StartDragDistance);

          case StartDragTime:
              return QPlatformTheme::defaultThemeHint(QPlatformTheme::StartDragTime);
*/

          case ShowIsFullScreen:
              return false;

          case ShowIsMaximized:
              return false;


/* emerald
          case PasswordMaskDelay:
              return QPlatformTheme::defaultThemeHint(QPlatformTheme::PasswordMaskDelay);

          case PasswordMaskCharacter:
              return QPlatformTheme::defaultThemeHint(QPlatformTheme::PasswordMaskCharacter);
*/

          case FontSmoothingGamma:
              return qreal(1.7);

/* emerald
          case StartDragVelocity:
              return QPlatformTheme::defaultThemeHint(QPlatformTheme::StartDragVelocity);
*/

          case UseRtlExtensions:
              return QVariant(false);

          case SetFocusOnTouchRelease:
              return QVariant(false);

/* emerald
          case MousePressAndHoldInterval:
              return QPlatformTheme::defaultThemeHint(QPlatformTheme::MousePressAndHoldInterval);

          case TabFocusBehavior:
              return QPlatformTheme::defaultThemeHint(QPlatformTheme::TabFocusBehavior);
*/

          case ReplayMousePressOutsidePopup:
              return true;

/* emerald
          case ItemViewActivateItemOnSingleClick:
              return QPlatformTheme::defaultThemeHint(QPlatformTheme::ItemViewActivateItemOnSingleClick);
*/
    }

    return 0;
}

Qt::WindowState QPlatformIntegration::defaultWindowState(Qt::WindowFlags flags) const
{
    // Leave popup-windows as is
    if (flags & Qt::Popup & ~Qt::Window)
        return Qt::WindowNoState;

    if (styleHint(QPlatformIntegration::ShowIsFullScreen).toBool())
        return Qt::WindowFullScreen;

    else if (styleHint(QPlatformIntegration::ShowIsMaximized).toBool())
        return Qt::WindowMaximized;

    return Qt::WindowNoState;
}

Qt::KeyboardModifiers QPlatformIntegration::queryKeyboardModifiers() const
{
    return QApplication::keyboardModifiers();
}

QList<int> QPlatformIntegration::possibleKeys(const QKeyEvent *) const
{
    return QList<int>();
}

void QPlatformIntegration::screenAdded(QPlatformScreen *ps, bool isPrimary)
{
    QScreen *screen = new QScreen(ps);

/* emerald


    if (isPrimary) {
        QApplicationPrivate::screen_list.prepend(screen);
    } else {
        QApplicationPrivate::screen_list.append(screen);
    }

    emit qApp->screenAdded(screen);

    if (isPrimary)
        emit qApp->primaryScreenChanged(screen);
*/

}

void QPlatformIntegration::removeScreen(QScreen *screen)
{

/* emerald

    const bool wasPrimary = (! QApplicationPrivate::screen_list.isEmpty() && QApplicationPrivate::screen_list[0] == screen);
    QApplicationPrivate::screen_list.removeOne(screen);

    if (wasPrimary && qApp && ! QApplicationPrivate::screen_list.isEmpty())
        emit qApp->primaryScreenChanged(QApplicationPrivate::screen_list[0]);
*/

}

/*!
  Should be called by the implementation whenever a screen is removed.

  This removes the screen from QApplication::screens(), and deletes it.

  Failing to call this and manually deleting the QPlatformScreen instead may
  lead to a crash due to a pure virtual call.
*/
void QPlatformIntegration::destroyScreen(QPlatformScreen *screen)
{
    QScreen *qScreen = screen->screen();
    removeScreen(qScreen);

    delete qScreen;
    delete screen;
}

/*!
  Should be called whenever the primary screen changes.

  When the screen specified as primary changes, this method will notify
  QApplication and emit the QApplication::primaryScreenChanged signal.
 */

void QPlatformIntegration::setPrimaryScreen(QPlatformScreen *newPrimary)
{
    QScreen* newPrimaryScreen = newPrimary->screen();

/* emerald

    int idx = QApplicationPrivate::screen_list.indexOf(newPrimaryScreen);
    Q_ASSERT(idx >= 0);

    if (idx == 0)
        return;

    QApplicationPrivate::screen_list.swap(0, idx);
    emit qApp->primaryScreenChanged(newPrimaryScreen);
*/

}

QStringList QPlatformIntegration::themeNames() const
{
    return QStringList();
}

/* emerald

class QPlatformTheme *QPlatformIntegration::createPlatformTheme(const QString &name) const
{
   return new QPlatformTheme;
}

*/


/*!
   Factory function for QOffscreenSurface. An offscreen surface will typically be implemented with a
   pixel buffer (pbuffer). If the platform doesn't support offscreen surfaces, an invisible window
   will be used by QOffscreenSurface instead.
*/

/* emerald

QPlatformOffscreenSurface *QPlatformIntegration::createPlatformOffscreenSurface(QOffscreenSurface *surface) const
{
    return 0;
}

*/

#ifndef QT_NO_SESSIONMANAGER
/*!
   Factory function for QPlatformSessionManager. The default QPlatformSessionManager provides the same
   functionality as the QSessionManager.
*/

/* emerald


QPlatformSessionManager *QPlatformIntegration::createPlatformSessionManager(const QString &id, const QString &key) const
{
    return new QPlatformSessionManager(id, key);
}

*/

#endif

/*!
   Function to sync the platform integrations state with the window system.

   This is often implemented as a roundtrip from the platformintegration to the window system.

   This function should not call QWindowSystemInterface::flushWindowSystemEvents() or
   QCoreApplication::processEvents()
*/
void QPlatformIntegration::sync()
{
}

#ifndef QT_NO_OPENGL
/*!
  Platform integration function for querying the OpenGL implementation type.

  Used only when dynamic OpenGL implementation loading is enabled.

  Subclasses should reimplement this function and return a value based on
  the OpenGL implementation they have chosen to load.

  \note The return value does not indicate or limit the types of
  contexts that can be created by a given implementation. For example
  a desktop OpenGL implementation may be capable of creating OpenGL
  ES-compatible contexts too.

  \sa QOpenGLContext::openGLModuleType(), QOpenGLContext::isOpenGLES()

*/

/* emerald


QOpenGLContext::OpenGLModuleType QPlatformIntegration::openGLModuleType()
{
    qWarning("This plugin does not support dynamic OpenGL loading!");
    return QOpenGLContext::LibGL;
}

*/

#endif

void QPlatformIntegration::setApplicationIcon(const QIcon &icon) const
{
    Q_UNUSED(icon);
}


