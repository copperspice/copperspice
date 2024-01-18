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

#ifndef QPLATFORM_INTEGRATION_H
#define QPLATFORM_INTEGRATION_H

#include <qopenglcontext.h>
#include <qsurfaceformat.h>
#include <qwindowdefs.h>
#include <qplatform_screen.h>

class QPlatformWindow;
class QWindow;
class QPlatformBackingStore;
class QPlatformFontDatabase;
class QPlatformClipboard;
class QPlatformNativeInterface;
class QPlatformDrag;
class QPlatformOpenGLContext;
class QGuiGLFormat;
class QAbstractEventDispatcher;
class QPlatformInputContext;
class QPlatformAccessibility;
class QPlatformTheme;
class QPlatformDialogHelper;
class QPlatformSharedGraphicsCache;
class QPlatformServices;
class QPlatformSessionManager;
class QKeyEvent;
class QPlatformOffscreenSurface;
class QOffscreenSurface;

class Q_GUI_EXPORT QPlatformIntegration
{
 public:
   enum Capability {
      ThreadedPixmaps = 1,
      OpenGL,
      ThreadedOpenGL,
      SharedGraphicsCache,
      BufferQueueingOpenGL,
      WindowMasks,
      MultipleWindows,
      ApplicationState,
      ForeignWindows,
      NonFullScreenWindows,
      NativeWidgets,
      WindowManagement,
      SyncState,
      RasterGLSurface,
      AllGLFunctionsQueryable,
      ApplicationIcon,
      SwitchableWidgetComposition
   };

   virtual ~QPlatformIntegration() { }
   virtual bool hasCapability(Capability cap) const;
   virtual QPlatformPixmap *createPlatformPixmap(QPlatformPixmap::PixelType type) const;
   virtual QPlatformWindow *createPlatformWindow(QWindow *window) const = 0;
   virtual QPlatformBackingStore *createPlatformBackingStore(QWindow *window) const = 0;

#ifndef QT_NO_OPENGL
   virtual QPlatformOpenGLContext *createPlatformOpenGLContext(QOpenGLContext *context) const;
#endif

   virtual QPlatformSharedGraphicsCache *createPlatformSharedGraphicsCache(const char *cacheId) const;
   virtual QPaintEngine *createImagePaintEngine(QPaintDevice *paintDevice) const;
   virtual QAbstractEventDispatcher *createEventDispatcher() const = 0;
   virtual void initialize();
   virtual void destroy();

   //Deeper window system integrations
   virtual QPlatformFontDatabase *fontDatabase() const;

#ifndef QT_NO_CLIPBOARD
   virtual QPlatformClipboard *clipboard() const;
#endif

#ifndef QT_NO_DRAGANDDROP
   virtual QPlatformDrag *drag() const;
#endif

   virtual QPlatformInputContext *inputContext() const;

#ifndef QT_NO_ACCESSIBILITY
   virtual QPlatformAccessibility *accessibility() const;
#endif

   virtual QPlatformNativeInterface *nativeInterface() const;
   virtual QPlatformServices *services() const;
   enum StyleHint {
      CursorFlashTime,
      KeyboardInputInterval,
      MouseDoubleClickInterval,
      StartDragDistance,
      StartDragTime,
      KeyboardAutoRepeatRate,
      ShowIsFullScreen,
      PasswordMaskDelay,
      FontSmoothingGamma,
      StartDragVelocity,
      UseRtlExtensions,
      PasswordMaskCharacter,
      SetFocusOnTouchRelease,
      ShowIsMaximized,
      MousePressAndHoldInterval,
      TabFocusBehavior,
      ReplayMousePressOutsidePopup,
      ItemViewActivateItemOnSingleClick
   };

   virtual QVariant styleHint(StyleHint hint) const;
   virtual Qt::WindowState defaultWindowState(Qt::WindowFlags flags) const;
   virtual Qt::KeyboardModifiers queryKeyboardModifiers() const;
   virtual QList<int> possibleKeys(const QKeyEvent *event) const;

   virtual QStringList themeNames() const;
   virtual QPlatformTheme *createPlatformTheme(const QString &name) const;

   virtual QPlatformOffscreenSurface *createPlatformOffscreenSurface(QOffscreenSurface *surface) const;

#ifndef QT_NO_SESSIONMANAGER
   virtual QPlatformSessionManager *createPlatformSessionManager(const QString &id, const QString &key) const;
#endif

   virtual void sync();

#ifndef QT_NO_OPENGL
   virtual QOpenGLContext::OpenGLModuleType openGLModuleType();
#endif

   virtual void setApplicationIcon(const QIcon &icon) const;

   void removeScreen(QScreen *screen);

 protected:
   void screenAdded(QPlatformScreen *screen, bool isPrimary = false);
   void destroyScreen(QPlatformScreen *screen);
   void setPrimaryScreen(QPlatformScreen *newPrimary);
};

#endif
