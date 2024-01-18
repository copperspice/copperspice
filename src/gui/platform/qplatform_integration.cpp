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

#include <qplatform_integration.h>

#include <qplatform_accessibility.h>
#include <qplatform_clipboard.h>
#include <qplatform_fontdatabase.h>
#include <qplatform_theme.h>

#include <qapplication_p.h>
#include <qpixmap_raster_p.h>
#include <qdnd_p.h>
#include <qsimpledrag_p.h>

#ifndef QT_NO_SESSIONMANAGER
#include <qplatform_sessionmanager.h>
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
QPlatformClipboard *QPlatformIntegration::clipboard() const
{
   static QPlatformClipboard *clipboard = nullptr;

   if (! clipboard) {
      clipboard = new QPlatformClipboard;
   }

   return clipboard;
}
#endif

#ifndef QT_NO_DRAGANDDROP
QPlatformDrag *QPlatformIntegration::drag() const
{
   static QSimpleDrag *drag = nullptr;

   if (! drag) {
      drag = new QSimpleDrag;
   }

   return drag;
}
#endif

QPlatformNativeInterface *QPlatformIntegration::nativeInterface() const
{
   return nullptr;
}

QPlatformServices *QPlatformIntegration::services() const
{
   return nullptr;
}

bool QPlatformIntegration::hasCapability(Capability cap) const
{
   return cap == NonFullScreenWindows || cap == NativeWidgets || cap == WindowManagement;
}

QPlatformPixmap *QPlatformIntegration::createPlatformPixmap(QPlatformPixmap::PixelType type) const
{
   return new QRasterPlatformPixmap(type);
}

#ifndef QT_NO_OPENGL
QPlatformOpenGLContext *QPlatformIntegration::createPlatformOpenGLContext(QOpenGLContext *context) const
{
   (void) context;

   qWarning("QPlatformIntegration::createPlatformOpenGLContext() Plugin does not support OpenGL");
   return nullptr;
}
#endif

QPlatformSharedGraphicsCache *QPlatformIntegration::createPlatformSharedGraphicsCache(const char *cacheId) const
{
   qWarning("QPlatformIntegration::createPlatformSharedGraphicsCache() Plugin does not support cacheId %s", cacheId);
   return nullptr;
}

QPaintEngine *QPlatformIntegration::createImagePaintEngine(QPaintDevice *paintDevice) const
{
   (void) paintDevice;

   return nullptr;
}

void QPlatformIntegration::initialize()
{
}

void QPlatformIntegration::destroy()
{
}

QPlatformInputContext *QPlatformIntegration::inputContext() const
{
   return nullptr;
}

#ifndef QT_NO_ACCESSIBILITY
QPlatformAccessibility *QPlatformIntegration::accessibility() const
{
   return nullptr;
}
#endif

QVariant QPlatformIntegration::styleHint(StyleHint hint) const
{
   switch (hint) {
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
      case ShowIsFullScreen:
         return false;
      case ShowIsMaximized:
         return false;
      case PasswordMaskDelay:
         return QPlatformTheme::defaultThemeHint(QPlatformTheme::PasswordMaskDelay);
      case PasswordMaskCharacter:
         return QPlatformTheme::defaultThemeHint(QPlatformTheme::PasswordMaskCharacter);
      case FontSmoothingGamma:
         return qreal(1.7);
      case StartDragVelocity:
         return QPlatformTheme::defaultThemeHint(QPlatformTheme::StartDragVelocity);
      case UseRtlExtensions:
         return QVariant(false);
      case SetFocusOnTouchRelease:
         return QVariant(false);
      case MousePressAndHoldInterval:
         return QPlatformTheme::defaultThemeHint(QPlatformTheme::MousePressAndHoldInterval);
      case TabFocusBehavior:
         return QPlatformTheme::defaultThemeHint(QPlatformTheme::TabFocusBehavior);
      case ReplayMousePressOutsidePopup:
         return true;
      case ItemViewActivateItemOnSingleClick:
         return QPlatformTheme::defaultThemeHint(QPlatformTheme::ItemViewActivateItemOnSingleClick);
   }

   return 0;
}

Qt::WindowState QPlatformIntegration::defaultWindowState(Qt::WindowFlags flags) const
{
   if (flags & Qt::Popup & ~Qt::Window) {
      return Qt::WindowNoState;
   }

   if (styleHint(QPlatformIntegration::ShowIsFullScreen).toBool()) {
      return Qt::WindowFullScreen;
   } else if (styleHint(QPlatformIntegration::ShowIsMaximized).toBool()) {
      return Qt::WindowMaximized;
   }

   return Qt::WindowNoState;
}

Qt::KeyboardModifiers QPlatformIntegration::queryKeyboardModifiers() const
{
   return QGuiApplication::keyboardModifiers();
}

QList<int> QPlatformIntegration::possibleKeys(const QKeyEvent *) const
{
   return QList<int>();
}

void QPlatformIntegration::screenAdded(QPlatformScreen *ps, bool isPrimary)
{
   QScreen *screen = new QScreen(ps);

   if (isPrimary) {
      QGuiApplicationPrivate::screen_list.prepend(screen);
   } else {
      QGuiApplicationPrivate::screen_list.append(screen);
   }

   emit qGuiApp->screenAdded(screen);

   if (isPrimary) {
      emit qGuiApp->primaryScreenChanged(screen);
   }
}

void QPlatformIntegration::removeScreen(QScreen *screen)
{
   const bool wasPrimary = (!QGuiApplicationPrivate::screen_list.isEmpty() && QGuiApplicationPrivate::screen_list[0] == screen);
   QGuiApplicationPrivate::screen_list.removeOne(screen);

   if (wasPrimary && qGuiApp && !QGuiApplicationPrivate::screen_list.isEmpty()) {
      emit qGuiApp->primaryScreenChanged(QGuiApplicationPrivate::screen_list[0]);
   }
}

void QPlatformIntegration::destroyScreen(QPlatformScreen *screen)
{
   QScreen *qScreen = screen->screen();
   removeScreen(qScreen);
   delete qScreen;
   delete screen;
}

void QPlatformIntegration::setPrimaryScreen(QPlatformScreen *newPrimary)
{
   QScreen *newPrimaryScreen = newPrimary->screen();

   int idx = QGuiApplicationPrivate::screen_list.indexOf(newPrimaryScreen);
   Q_ASSERT(idx >= 0);

   if (idx == 0) {
      return;
   }

   QGuiApplicationPrivate::screen_list.swap(0, idx);
   emit qGuiApp->primaryScreenChanged(newPrimaryScreen);
}

QStringList QPlatformIntegration::themeNames() const
{
   return QStringList();
}

class QPlatformTheme *QPlatformIntegration::createPlatformTheme(const QString &name) const
{
   (void) name;
   return new QPlatformTheme;
}

QPlatformOffscreenSurface *QPlatformIntegration::createPlatformOffscreenSurface(QOffscreenSurface *surface) const
{
   (void) surface;
   return nullptr;
}

#ifndef QT_NO_SESSIONMANAGER
QPlatformSessionManager *QPlatformIntegration::createPlatformSessionManager(const QString &id, const QString &key) const
{
   return new QPlatformSessionManager(id, key);
}
#endif

void QPlatformIntegration::sync()
{
}

#ifndef QT_NO_OPENGL
QOpenGLContext::OpenGLModuleType QPlatformIntegration::openGLModuleType()
{
   qWarning("QPlatformIntegration::openGLModuleType() Plugin does not support dynamic OpenGL loading");
   return QOpenGLContext::LibGL;
}
#endif

void QPlatformIntegration::setApplicationIcon(const QIcon &icon) const
{
   (void) icon;
}
