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

#ifndef QPLATFORMINTEGRATION_H
#define QPLATFORMINTEGRATION_H

#include <qicon.h>
#include <qstringlist.h>
#include <qwindowdefs.h>
#include <qplatformscreen.h>

// emerald     #include <qsurfaceformat.h>
// emerald     #include <qopenglcontext.h>

// emerald    class QPlatformWindow;
// emerald    class QWindow;
// emerald    class QPlatformBackingStore;
class QPlatformFontDatabase;
// emerald    class QPlatformClipboard;
class QPlatformNativeInterface;
// emerald    class QPlatformDrag;
// emerald    class QPlatformOpenGLContext;
// emerald    class QGuiGLFormat;
class QAbstractEventDispatcher;
// emerald    class QPlatformInputContext;
// emerald    class QPlatformAccessibility;
class QPlatformTheme;
class QPlatformDialogHelper;
class QPlatformSharedGraphicsCache;
class QPlatformServices;
class QPlatformSessionManager;
class QKeyEvent;
// emerald    class QPlatformOffscreenSurface;
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

    // emerald    virtual QPlatformPixmap *createPlatformPixmap(QPlatformPixmap::PixelType type) const;
    // emerald    virtual QPlatformWindow *createPlatformWindow(QWindow *window) const = 0;
    // emerald    virtual QPlatformBackingStore *createPlatformBackingStore(QWindow *window) const = 0;

#ifndef QT_NO_OPENGL
    // emerald    virtual QPlatformOpenGLContext *createPlatformOpenGLContext(QOpenGLContext *context) const;
#endif

    virtual QPlatformSharedGraphicsCache *createPlatformSharedGraphicsCache(const char *cacheId) const;
    virtual QPaintEngine *createImagePaintEngine(QPaintDevice *paintDevice) const;

    // Event dispatcher
    virtual QAbstractEventDispatcher *createEventDispatcher() const = 0;
    virtual void initialize();
    virtual void destroy();

    //Deeper window system integrations
    virtual QPlatformFontDatabase *fontDatabase() const;

#ifndef QT_NO_CLIPBOARD
    // emerald    virtual QPlatformClipboard *clipboard() const;
#endif

#ifndef QT_NO_DRAGANDDROP
    // emerald    virtual QPlatformDrag *drag() const;
#endif

    // emerald    virtual QPlatformInputContext *inputContext() const;

#ifndef QT_NO_ACCESSIBILITY
    // emerald    virtual QPlatformAccessibility *accessibility() const;
#endif

    // Access native handles. The window handle is already available from Wid;
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
    virtual Qt::WindowState defaultWindowState(Qt::WindowFlags) const;

    virtual Qt::KeyboardModifiers queryKeyboardModifiers() const;
    virtual QList<int> possibleKeys(const QKeyEvent *) const;

    virtual QStringList themeNames() const;
    virtual QPlatformTheme *createPlatformTheme(const QString &name) const;

    // emerald    virtual QPlatformOffscreenSurface *createPlatformOffscreenSurface(QOffscreenSurface *surface) const;

#ifndef QT_NO_SESSIONMANAGER
    // emerald    virtual QPlatformSessionManager *createPlatformSessionManager(const QString &id, const QString &key) const;
#endif

    virtual void sync();

#ifndef QT_NO_OPENGL
    // emerald    virtual QOpenGLContext::OpenGLModuleType openGLModuleType();
#endif

    virtual void setApplicationIcon(const QIcon &icon) const;

    void removeScreen(QScreen *screen);

protected:
    void screenAdded(QPlatformScreen *screen, bool isPrimary = false);
    void destroyScreen(QPlatformScreen *screen);
    void setPrimaryScreen(QPlatformScreen *newPrimary);
};

#endif
