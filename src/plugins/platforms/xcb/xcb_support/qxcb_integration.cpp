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

#include <qxcb_integration.h>

#include <qfileinfo.h>
#include <qoffscreensurface.h>
#include <qopenglcontext.h>
#include <qplatform_inputcontext.h>
#include <qscreen.h>
#include <qxcb_backingstore.h>
#include <qxcb_clipboard.h>
#include <qxcb_connection.h>
#include <qxcb_cursor.h>
#include <qxcb_drag.h>
#include <qxcb_gl_integration.h>
#include <qxcb_keyboard.h>
#include <qxcb_nativeinterface.h>
#include <qxcb_screen.h>
#include <qxcb_window.h>

#ifndef QT_NO_SESSIONMANAGER
#include <qxcb_sessionmanager.h>
#endif

#ifndef QT_NO_ACCESSIBILITY
#include <qplatform_accessibility.h>

#ifndef QT_NO_ACCESSIBILITY_ATSPI_BRIDGE
#include <bridge_p.h>
#endif

#endif

#include <qgenericunix_eventdispatcher_p.h>
#include <qgenericunix_fontdatabase_p.h>
#include <qgenericunix_services_p.h>
#include <qgenericunix_theme_p.h>
#include <qplatform_inputcontextfactory_p.h>

// must be included before egl since egl pulls in X headers
#include <qapplication_p.h>

#include <xcb/xcb.h>

#ifdef XCB_USE_EGL
#include <EGL/egl.h>
#endif

#ifdef XCB_USE_XLIB
#include <X11/Xlib.h>
#endif

#include <stdio.h>

QXcbIntegration *QXcbIntegration::m_instance = nullptr;

// Find out if our parent process is gdb by looking at the 'exe' symlink under /proc,.
// or, for older Linuxes, read out 'cmdline'.
static bool runningUnderDebugger()
{
#if defined(CS_SHOW_DEBUG_PLATFORM) && defined(Q_OS_LINUX)
   const QString parentProc = QString("/proc/") + QString::number(getppid());
   const QFileInfo parentProcExe(parentProc + QString("/exe"));

   if (parentProcExe.isSymLink()) {
      return parentProcExe.symLinkTarget().endsWith("/gdb");
   }

   QFile f(parentProc + QString("/cmdline"));
   if (! f.open(QIODevice::ReadOnly)) {
      return false;
   }

   QByteArray s;
   char c;

   while (f.getChar(&c) && c) {
      if (c == '/') {
         s.clear();
      } else {
         s += c;
      }
   }

   return s == "gdb";

#else
   return false;

#endif
}

QXcbIntegration::QXcbIntegration(const QStringList &parameters, int &argc, char **argv)
   : m_services(new QGenericUnixServices), m_instanceName(nullptr), m_canGrab(true), m_defaultVisualId(UINT_MAX)
{
   m_instance = this;

#ifdef XCB_USE_XLIB
   XInitThreads();
#endif

   m_nativeInterface.reset(new QXcbNativeInterface);

   // Parse arguments
   const char *displayName = nullptr;
   bool noGrabArg = false;
   bool doGrabArg = false;

   if (argc) {
      int j = 1;
      for (int i = 1; i < argc; i++) {
         QByteArray arg(argv[i]);

         if (arg.startsWith("--")) {
            arg.remove(0, 1);
         }

         if (arg == "-display" && i < argc - 1) {
            displayName = argv[++i];

         } else if (arg == "-name" && i < argc - 1) {
            m_instanceName = argv[++i];

         } else if (arg == "-nograb") {
            noGrabArg = true;

         } else if (arg == "-dograb") {
            doGrabArg = true;

         } else if (arg == "-visual" && i < argc - 1) {
            bool ok = false;
            m_defaultVisualId = QByteArray(argv[++i]).toUInt(&ok, 0);

            if (!ok) {
               m_defaultVisualId = UINT_MAX;
            }
         } else {
            argv[j++] = argv[i];
         }
      }

      argc = j;
   }

   bool underDebugger = runningUnderDebugger();

   if (noGrabArg && doGrabArg && underDebugger) {
      qWarning() << "QXcbIntegration() Both -nograb and -dograb command line arguments specified, only one can be used at a time";
      doGrabArg = false;
   }

#if defined(CS_SHOW_DEBUG_PLATFORM)
   if (! noGrabArg && ! doGrabArg && underDebugger) {
      qDebug("QXcbIntegration() Debugger detected, use -dograb option to enable grabbing");
   }
#endif

   m_canGrab = (!underDebugger && !noGrabArg) || (underDebugger && doGrabArg);

   static bool canNotGrabEnv = qgetenv("QT_XCB_NO_GRAB_SERVER").isEmpty();
   if (canNotGrabEnv) {
      m_canGrab = false;
   }

   const int numParameters = parameters.size();
   m_connections << new QXcbConnection(m_nativeInterface.data(), m_canGrab, m_defaultVisualId, displayName);

   for (int i = 0; i < numParameters - 1; i += 2) {

#if defined(CS_SHOW_DEBUG_PLATFORM)
      qDebug() << "QXcbIntegration() Connecting to additional display = " << parameters.at(i) << parameters.at(i + 1);
#endif

      QString display = parameters.at(i) + QLatin1Char(':') + parameters.at(i + 1);
      m_connections << new QXcbConnection(m_nativeInterface.data(), m_canGrab, m_defaultVisualId, display.toLatin1().constData());
   }

   m_fontDatabase.reset(new QGenericUnixFontDatabase());
}

QXcbIntegration::~QXcbIntegration()
{
   qDeleteAll(m_connections);
   m_instance = nullptr;
}

QPlatformWindow *QXcbIntegration::createPlatformWindow(QWindow *window) const
{
   QXcbScreen *screen = static_cast<QXcbScreen *>(window->screen()->handle());
   QXcbGlIntegration *glIntegration = screen->connection()->glIntegration();

   if (window->type() != Qt::Desktop) {
      if (glIntegration) {
         QXcbWindow *xcbWindow = glIntegration->createWindow(window);
         xcbWindow->create();
         return xcbWindow;
      }
   }

   Q_ASSERT(window->type() == Qt::Desktop || !window->supportsOpenGL()
      || (!glIntegration && window->surfaceType() == QSurface::RasterGLSurface)); // for VNC
   QXcbWindow *xcbWindow = new QXcbWindow(window);
   xcbWindow->create();
   return xcbWindow;
}

#ifndef QT_NO_OPENGL
QPlatformOpenGLContext *QXcbIntegration::createPlatformOpenGLContext(QOpenGLContext *context) const
{
   QXcbScreen *screen = static_cast<QXcbScreen *>(context->screen()->handle());
   QXcbGlIntegration *glIntegration = screen->connection()->glIntegration();
   if (!glIntegration) {
      qWarning("QXcbIntegration: Cannot create platform OpenGL context, neither GLX nor EGL are enabled");
      return nullptr;
   }
   return glIntegration->createPlatformOpenGLContext(context);
}
#endif

QPlatformBackingStore *QXcbIntegration::createPlatformBackingStore(QWindow *window) const
{
   return new QXcbBackingStore(window);
}

QPlatformOffscreenSurface *QXcbIntegration::createPlatformOffscreenSurface(QOffscreenSurface *surface) const
{
   QXcbScreen *screen = static_cast<QXcbScreen *>(surface->screen()->handle());
   QXcbGlIntegration *glIntegration = screen->connection()->glIntegration();
   if (!glIntegration) {
      qWarning("QXcbIntegration: Cannot create platform offscreen surface, neither GLX nor EGL are enabled");
      return nullptr;
   }
   return glIntegration->createPlatformOffscreenSurface(surface);
}

bool QXcbIntegration::hasCapability(QPlatformIntegration::Capability cap) const
{
   switch (cap) {
      case ThreadedPixmaps:
         return true;
      case OpenGL:
         return m_connections.first()->glIntegration();
      case ThreadedOpenGL:
         return m_connections.at(0)->threadedEventHandling()
            && m_connections.at(0)->glIntegration()
            && m_connections.at(0)->glIntegration()->supportsThreadedOpenGL();
      case WindowMasks:
         return true;
      case MultipleWindows:
         return true;
      case ForeignWindows:
         return true;
      case SyncState:
         return true;
      case RasterGLSurface:
         return true;
      case SwitchableWidgetComposition:
         return m_connections.at(0)->glIntegration()
            && m_connections.at(0)->glIntegration()->supportsSwitchableWidgetComposition();
      default:
         return QPlatformIntegration::hasCapability(cap);
   }
}

QAbstractEventDispatcher *QXcbIntegration::createEventDispatcher() const
{
   QAbstractEventDispatcher *dispatcher = createUnixEventDispatcher();

   for (int i = 0; i < m_connections.size(); i++) {
      m_connections[i]->eventReader()->registerEventDispatcher(dispatcher);
   }

   return dispatcher;
}

void QXcbIntegration::initialize()
{
   // Perform everything that may potentially need the event dispatcher (timers, socket
   // notifiers) here instead of the constructor.
   QString icStr = QPlatformInputContextFactory::requested();

   if (icStr.isEmpty()) {
      icStr = "compose";
   }

   m_inputContext.reset(QPlatformInputContextFactory::create(icStr));
}

void QXcbIntegration::moveToScreen(QWindow *window, int screen)
{
   (void) window;
   (void) screen;
}

QPlatformFontDatabase *QXcbIntegration::fontDatabase() const
{
   return m_fontDatabase.data();
}

QPlatformNativeInterface *QXcbIntegration::nativeInterface() const
{
   return m_nativeInterface.data();
}

#ifndef QT_NO_CLIPBOARD
QPlatformClipboard *QXcbIntegration::clipboard() const
{
   return m_connections.at(0)->clipboard();
}
#endif

#ifndef QT_NO_DRAGANDDROP
QPlatformDrag *QXcbIntegration::drag() const
{
   return m_connections.at(0)->drag();
}
#endif

QPlatformInputContext *QXcbIntegration::inputContext() const
{
   return m_inputContext.data();
}

#ifndef QT_NO_ACCESSIBILITY
QPlatformAccessibility *QXcbIntegration::accessibility() const
{
#ifndef QT_NO_ACCESSIBILITY_ATSPI_BRIDGE
   if (! m_accessibility) {
      Q_ASSERT_X(QCoreApplication::eventDispatcher(), "QXcbIntegration", "Initializing accessibility without event-dispatcher");
      m_accessibility.reset(new QSpiAccessibleBridge());
   }
#endif

   return m_accessibility.data();
}
#endif

QPlatformServices *QXcbIntegration::services() const
{
   return m_services.data();
}

Qt::KeyboardModifiers QXcbIntegration::queryKeyboardModifiers() const
{
   int keybMask = 0;
   QXcbConnection *conn = m_connections.at(0);
   QXcbCursor::queryPointer(conn, nullptr, nullptr, &keybMask);
   return conn->keyboard()->translateModifiers(keybMask);
}

QList<int> QXcbIntegration::possibleKeys(const QKeyEvent *e) const
{
   return m_connections.at(0)->keyboard()->possibleKeys(e);
}

QStringList QXcbIntegration::themeNames() const
{
   return QGenericUnixTheme::themeNames();
}

QPlatformTheme *QXcbIntegration::createPlatformTheme(const QString &name) const
{
   return QGenericUnixTheme::createUnixTheme(name);
}

QVariant QXcbIntegration::styleHint(QPlatformIntegration::StyleHint hint) const
{
   switch (hint) {
      case QPlatformIntegration::CursorFlashTime:
      case QPlatformIntegration::KeyboardInputInterval:
      case QPlatformIntegration::MouseDoubleClickInterval:
      case QPlatformIntegration::StartDragTime:
      case QPlatformIntegration::KeyboardAutoRepeatRate:
      case QPlatformIntegration::PasswordMaskDelay:
      case QPlatformIntegration::StartDragVelocity:
      case QPlatformIntegration::UseRtlExtensions:
      case QPlatformIntegration::PasswordMaskCharacter:
         // TODO using various xcb, gnome or KDE settings
         break; // Not implemented, use defaults

      case QPlatformIntegration::FontSmoothingGamma:
         return qreal(1.0);

      case QPlatformIntegration::StartDragDistance: {
         // The default (in QPlatformTheme::defaultThemeHint) is 10 pixels, but
         // on a high-resolution screen it makes sense to increase it.
         qreal dpi = 100.0;

         if (const QXcbScreen *screen = defaultConnection()->primaryScreen()) {
            if (screen->logicalDpi().first > dpi) {
               dpi = screen->logicalDpi().first;
            }
            if (screen->logicalDpi().second > dpi) {
               dpi = screen->logicalDpi().second;
            }
         }
         return 10.0 * dpi / 100.0;
      }
      case QPlatformIntegration::ShowIsFullScreen:
         // X11 always has support for windows, but the
         // window manager could prevent it (e.g. matchbox)
         return false;
      case QPlatformIntegration::ReplayMousePressOutsidePopup:
         return false;
      default:
         break;
   }
   return QPlatformIntegration::styleHint(hint);
}

static QString argv0BaseName()
{
   QString result;
   const QStringList arguments = QCoreApplication::arguments();

   if (!arguments.isEmpty() && !arguments.front().isEmpty()) {
      result = arguments.front();

      const int lastSlashPos = result.lastIndexOf('/');

      if (lastSlashPos != -1) {
         result.remove(0, lastSlashPos + 1);
      }
   }

   return result;
}

QByteArray QXcbIntegration::wmClass() const
{
   if (m_wmClass.isEmpty()) {
      // Instance name according to ICCCM 4.1.2.5
      QString name;

      if (m_instanceName != nullptr) {
         name = QString::fromUtf8(m_instanceName);
      }

      if (name.isEmpty()) {
         QByteArray tmp = qgetenv("RESOURCE_NAME");

         if (! tmp.isEmpty()) {
            name = QString::fromUtf8(tmp);
         }
      }

      if (name.isEmpty()) {
         name = argv0BaseName();
      }

      // QCoreApplication::applicationName() can not be called from the QApplication constructor
      QString className = QCoreApplication::applicationName();

      if (className.isEmpty()) {
         className = argv0BaseName();

         if (! className.isEmpty()) {
            QChar ch = className.at(0);

            if (ch.isLower()) {
               className.replace(0, 1, ch.toUpper());
            }
         }
      }

      if (! name.isEmpty() && ! className.isEmpty()) {
         m_wmClass = name.toUtf8();
         m_wmClass.append('\0');
         m_wmClass.append(className.toUtf8());
         m_wmClass.append('\0');
      }
   }

   return m_wmClass;
}

#if ! defined(QT_NO_SESSIONMANAGER) && defined(XCB_USE_SM)
QPlatformSessionManager *QXcbIntegration::createPlatformSessionManager(const QString &id, const QString &key) const
{
   return new QXcbSessionManager(id, key);
}
#endif

void QXcbIntegration::sync()
{
   for (int i = 0; i < m_connections.size(); i++) {
      m_connections.at(i)->sync();
   }
}

