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

#include <qwin_integration.h>

#include <qplatform_nativeinterface.h>
#include <qvariant.h>
#include <qwin_context.h>
#include <qwin_fontdatabase.h>
#include <qwin_inputcontext.h>
#include <qwin_keymapper.h>
#include <qwin_opengl_context.h>
#include <qwin_opengl_tester.h>
#include <qwin_screen.h>
#include <qwin_services.h>
#include <qwin_theme.h>
#include <qwin_window.h>
#include <qwindowsysteminterface.h>

#ifndef QT_NO_ACCESSIBILITY
#  include <qwin_accessibility.h>
#endif

#if defined(QT_USE_FREETYPE)
#  include <qwin_fontdatabase_ft.h>
#endif

#ifndef QT_NO_CLIPBOARD
#  include <qwin_clipboard.h>
#ifndef QT_NO_DRAGANDDROP
#  include <qwin_drag.h>
#endif
#endif

#if ! defined(QT_NO_SESSIONMANAGER)
#  include <qwin_session_manager.h>
#endif

#if defined(QT_OPENGL_ES_2) || defined(QT_OPENGL_DYNAMIC)
#  include <qwin_egl_context.h>
#endif

#if ! defined(QT_NO_OPENGL) && ! defined(QT_OPENGL_ES_2)
#  include <qwin_gl_context.h>
#endif

#include <qapplication_p.h>
#include <qhighdpiscaling_p.h>
#include <qplatform_inputcontextfactory_p.h>
#include <qwin_gui_eventdispatcher_p.h>

#include <limits.h>

struct QWindowsIntegrationPrivate {
   explicit QWindowsIntegrationPrivate(const QStringList &paramList);
   ~QWindowsIntegrationPrivate();

   unsigned m_options;
   QWindowsContext m_context;
   QPlatformFontDatabase *m_fontDatabase;

#ifndef QT_NO_CLIPBOARD
   QWindowsClipboard m_clipboard;

#ifndef QT_NO_DRAGANDDROP
   QWindowsDrag m_drag;
#endif

#endif

#ifndef QT_NO_OPENGL
   QMutex m_staticContextLock;
   QScopedPointer<QWindowsStaticOpenGLContext> m_staticOpenGLContext;
#endif

   QScopedPointer<QPlatformInputContext> m_inputContext;

#ifndef QT_NO_ACCESSIBILITY
   QWindowsAccessibility m_accessibility;
#endif

   QWindowsServices m_services;
};

template <typename IntType>
bool parseIntOption(const QString &parameter, const QString &option, IntType minimumValue,
      IntType maximumValue, IntType *target)
{
   const int valueLength = parameter.size() - option.size() - 1;

   if (valueLength < 1 || ! parameter.startsWith(option) || parameter.at(option.size()) != '=') {
      return false;
   }

   bool ok;

   const QStringView valueView = parameter.rightView(valueLength);
   const int value = QStringParser::toInteger<int>(valueView, &ok);

   if (ok) {
      if (value >= minimumValue && value <= maximumValue) {
         *target = static_cast<IntType>(value);

      } else {
         qWarning() << "parseIntOption() Value = " << value << "for option " << option << " out of range"
            << minimumValue << ".." << maximumValue;
      }

   } else {
      qWarning() << "parseIntOption() Invalid value = " << valueView << " for option" << option;
   }

   return true;
}

static inline unsigned parseOptions(const QStringList &paramList,
   int *tabletAbsoluteRange, QtWindows::ProcessDpiAwareness *dpiAwareness)
{
   unsigned options = 0;

   for (const QString &param : paramList) {
      if (param.startsWith("fontengine=")) {

         if (param.endsWith("freetype")) {
            options |= QWindowsIntegration::FontDatabaseFreeType;

         } else if (param.endsWith("native")) {
            options |= QWindowsIntegration::FontDatabaseNative;
         }

      } else if (param.startsWith("dialogs=")) {

         if (param.endsWith("xp")) {
            options |= QWindowsIntegration::XpNativeDialogs;

         } else if (param.endsWith("none")) {
            options |= QWindowsIntegration::NoNativeDialogs;
         }

      } else if (param == "gl=gdi") {
         options |= QWindowsIntegration::DisableArb;

      } else if (param == "nomousefromtouch") {
         options |= QWindowsIntegration::DontPassOsMouseEventsSynthesizedFromTouch;

       } else if (parseIntOption(param, "tabletabsoluterange", 0, INT_MAX, tabletAbsoluteRange) ||
            parseIntOption(param, "dpiawareness", QtWindows::ProcessDpiUnaware,
            QtWindows::ProcessPerMonitorDpiAware, dpiAwareness)) {

      } else {
         qWarning() << "parseOptions() Unknown option" << param;
      }
   }

   return options;
}

QWindowsIntegrationPrivate::QWindowsIntegrationPrivate(const QStringList &paramList)
   : m_options(0), m_fontDatabase(nullptr)
{
   static bool dpiAwarenessSet = false;
   int tabletAbsoluteRange = -1;

   // Default to per-monitor awareness to avoid being scaled when monitors with different DPI
   // are connected to Windows 8.1
   QtWindows::ProcessDpiAwareness dpiAwareness = QtWindows::ProcessPerMonitorDpiAware;
   m_options = parseOptions(paramList, &tabletAbsoluteRange, &dpiAwareness);

   if (tabletAbsoluteRange >= 0) {
      m_context.setTabletAbsoluteRange(tabletAbsoluteRange);
   }

   if (! dpiAwarenessSet) {
      // Set only once in case of repeated instantiations of QApplication.
      m_context.setProcessDpiAwareness(dpiAwareness);
      dpiAwarenessSet = true;
   }

   m_context.initTouch(m_options);
}

QWindowsIntegrationPrivate::~QWindowsIntegrationPrivate()
{
   if (m_fontDatabase) {
      delete m_fontDatabase;
   }
}

QWindowsIntegration *QWindowsIntegration::m_instance = nullptr;

QWindowsIntegration::QWindowsIntegration(const QStringList &paramList)
   : d(new QWindowsIntegrationPrivate(paramList))
{
   m_instance = this;

#ifndef QT_NO_CLIPBOARD
   d->m_clipboard.registerViewer();
#endif

   d->m_context.screenManager().handleScreenChanges();
}

QWindowsIntegration::~QWindowsIntegration()
{
   m_instance = nullptr;
}

void QWindowsIntegration::initialize()
{
   QString icStr = QPlatformInputContextFactory::requested();

   icStr.isEmpty() ? d->m_inputContext.reset(new QWindowsInputContext)
      : d->m_inputContext.reset(QPlatformInputContextFactory::create(icStr));
}

bool QWindowsIntegration::hasCapability(QPlatformIntegration::Capability cap) const
{
   switch (cap) {
      case ThreadedPixmaps:
         return true;

#ifndef QT_NO_OPENGL
      case OpenGL:
         return true;

      case ThreadedOpenGL:
         if (const QWindowsStaticOpenGLContext *glContext = QWindowsIntegration::staticOpenGLContext()) {
            return glContext->supportsThreadedOpenGL();
         }
         return false;
#endif

      case WindowMasks:
         return true;

      case MultipleWindows:
         return true;

      case ForeignWindows:
         return true;

      case RasterGLSurface:
         return true;

      case AllGLFunctionsQueryable:
         return true;

      case SwitchableWidgetComposition:
         return true;

      default:
         return QPlatformIntegration::hasCapability(cap);
   }
}

QPlatformWindow *QWindowsIntegration::createPlatformWindow(QWindow *window) const
{
   QWindowsWindowData requested;
   requested.flags = window->flags();
   requested.geometry = QHighDpi::toNativePixels(window->geometry(), window);

   // Apply custom margins (see  QWindowsWindow::setCustomMargins())).
   const QVariant customMarginsV = window->property("_q_windowsCustomMargins");

   if (customMarginsV.isValid()) {
      requested.customMargins = customMarginsV.value<QMargins>();
   }

   QWindowsWindowData obtained = QWindowsWindowData::create(window, requested, window->title());

   if (! obtained.hwnd) {
      return nullptr;
   }

   QWindowsWindow *result = createPlatformWindowHelper(window, obtained);
   Q_ASSERT(result);

   if (requested.flags != obtained.flags) {
      window->setFlags(obtained.flags);
   }
   // Trigger geometry change (unless it has a special state in which case setWindowState()
   // will send the message) and screen change signals of QWindow.
   if ((obtained.flags & Qt::Desktop) != Qt::Desktop) {
      const Qt::WindowState state = window->windowState();
      if (state != Qt::WindowMaximized && state != Qt::WindowFullScreen
         && requested.geometry != obtained.geometry) {
         QWindowSystemInterface::handleGeometryChange(window, obtained.geometry);
      }

      QPlatformScreen *screen = result->screenForGeometry(obtained.geometry);
      if (screen && result->screen() != screen) {
         QWindowSystemInterface::handleWindowScreenChanged(window, screen->screen());
      }
   }

   return result;
}

// Overridden to return a QWindowsDirect2DWindow in Direct2D plugin
QWindowsWindow *QWindowsIntegration::createPlatformWindowHelper(QWindow *window, const QWindowsWindowData &data) const
{
   return new QWindowsWindow(window, data);
}

#if ! defined(QT_NO_OPENGL)

QWindowsStaticOpenGLContext *QWindowsStaticOpenGLContext::doCreate()
{
#if defined(QT_OPENGL_DYNAMIC)
   QWindowsOpenGLTester::Renderer requestedRenderer = QWindowsOpenGLTester::requestedRenderer();

   switch (requestedRenderer) {
      case QWindowsOpenGLTester::DesktopGl:
         if (QWindowsStaticOpenGLContext *glCtx = QOpenGLStaticContext::create()) {
            if ((QWindowsOpenGLTester::supportedRenderers() & QWindowsOpenGLTester::DisableRotationFlag)
                  && !QWindowsScreen::setOrientationPreference(Qt::LandscapeOrientation)) {
               qWarning("QWindowsStaticOpenGLContext::doCreate() Unable to disable rotation");
            }
            return glCtx;
         }

         qWarning("QWindowsStaticOpenGLContext::doCreate() System OpenGL failed, Falling back to Software OpenGL");
         return QOpenGLStaticContext::create(true);

      // If ANGLE is requested use it, do not try anything else
      case QWindowsOpenGLTester::AngleRendererD3d9:
      case QWindowsOpenGLTester::AngleRendererD3d11:
      case QWindowsOpenGLTester::AngleRendererD3d11Warp:
         return QWindowsEGLStaticContext::create(requestedRenderer);

      case QWindowsOpenGLTester::Gles:
         return QWindowsEGLStaticContext::create(QWindowsOpenGLTester::supportedGlesRenderers());

      case QWindowsOpenGLTester::SoftwareRasterizer:
         if (QWindowsStaticOpenGLContext *swCtx = QOpenGLStaticContext::create(true)) {
            return swCtx;
         }

         qWarning("QWindowsStaticOpenGLContext::doCreate() Software OpenGL failed, Falling back to system OpenGL");

         if (QWindowsOpenGLTester::supportedRenderers() & QWindowsOpenGLTester::DesktopGl) {
            return QOpenGLStaticContext::create();
         }

         return nullptr;

      default:
         break;
   }

   const QWindowsOpenGLTester::Renderers supportedRenderers = QWindowsOpenGLTester::supportedRenderers();

   if (supportedRenderers & QWindowsOpenGLTester::DesktopGl) {
      if (QWindowsStaticOpenGLContext *glCtx = QOpenGLStaticContext::create()) {

         if ((supportedRenderers & QWindowsOpenGLTester::DisableRotationFlag)
               && ! QWindowsScreen::setOrientationPreference(Qt::LandscapeOrientation)) {
            qWarning("QWindowsStaticOpenGLContext::doCreate() Unable to disable rotation");
         }
         return glCtx;
      }
   }

   if (QWindowsOpenGLTester::Renderers glesRenderers = supportedRenderers & QWindowsOpenGLTester::GlesMask) {
      if (QWindowsEGLStaticContext *eglCtx = QWindowsEGLStaticContext::create(glesRenderers)) {
         return eglCtx;
      }
   }

   return QOpenGLStaticContext::create(true);

#elif defined(QT_OPENGL_ES_2)
   QWindowsOpenGLTester::Renderers glesRenderers = QWindowsOpenGLTester::requestedGlesRenderer();

   if (glesRenderers == QWindowsOpenGLTester::InvalidRenderer) {
      glesRenderers = QWindowsOpenGLTester::supportedGlesRenderers();
   }

   return QWindowsEGLStaticContext::create(glesRenderers);

#else
   return QOpenGLStaticContext::create();

#endif
}

QWindowsStaticOpenGLContext *QWindowsStaticOpenGLContext::create()
{
   return QWindowsStaticOpenGLContext::doCreate();
}

QPlatformOpenGLContext *QWindowsIntegration::createPlatformOpenGLContext(QOpenGLContext *context) const
{
   QWindowsStaticOpenGLContext *staticOpenGLContext = QWindowsIntegration::staticOpenGLContext();

   if (staticOpenGLContext != nullptr) {
      QScopedPointer<QWindowsOpenGLContext> result(staticOpenGLContext->createContext(context));

      if (result->isValid()) {
         return result.take();
      }
   }

   return nullptr;
}

QOpenGLContext::OpenGLModuleType QWindowsIntegration::openGLModuleType()
{
#if defined(QT_OPENGL_ES_2)
   return QOpenGLContext::LibGLES;

#elif ! defined(QT_OPENGL_DYNAMIC)
   return QOpenGLContext::LibGL;

#else
   if (const QWindowsStaticOpenGLContext *staticOpenGLContext = QWindowsIntegration::staticOpenGLContext()) {
      return staticOpenGLContext->moduleType();
   }

   return QOpenGLContext::LibGL;
#endif
}

QWindowsStaticOpenGLContext *QWindowsIntegration::staticOpenGLContext()
{
   QWindowsIntegration *integration = QWindowsIntegration::instance();

   if (! integration) {
      return nullptr;
   }

   QWindowsIntegrationPrivate *d = integration->d.data();
   QMutexLocker lock(&d->m_staticContextLock);

   if (d->m_staticOpenGLContext.isNull()) {
      d->m_staticOpenGLContext.reset(QWindowsStaticOpenGLContext::create());
   }

   return d->m_staticOpenGLContext.data();
}

#endif // ! QT_NO_OPENGL

QPlatformFontDatabase *QWindowsIntegration::fontDatabase() const
{
   if (! d->m_fontDatabase) {

#if defined(QT_USE_FREETYPE)
      if (d->m_options & QWindowsIntegration::FontDatabaseFreeType) {
         d->m_fontDatabase = new QWindowsFontDatabaseFT;

      } else if (d->m_options & QWindowsIntegration::FontDatabaseNative) {
         d->m_fontDatabase = new QWindowsFontDatabase;

      } else {
         d->m_fontDatabase = new QWindowsFontDatabase;

      }
#else
      d->m_fontDatabase = new QWindowsFontDatabase();
#endif

   }

   return d->m_fontDatabase;
}

#ifdef SPI_GETKEYBOARDSPEED
static inline int keyBoardAutoRepeatRateMS()
{
   DWORD time = 0;

   if (SystemParametersInfo(SPI_GETKEYBOARDSPEED, 0, &time, 0)) {
      return time ? 1000 / static_cast<int>(time) : 500;
   }
   return 30;
}
#endif

QVariant QWindowsIntegration::styleHint(QPlatformIntegration::StyleHint hint) const
{
   switch (hint) {
      case QPlatformIntegration::CursorFlashTime:
         if (const unsigned timeMS = GetCaretBlinkTime()) {
            return QVariant(timeMS != INFINITE ? int(timeMS) * 2 : 0);
         }
         break;

#ifdef SPI_GETKEYBOARDSPEED
      case KeyboardAutoRepeatRate:
         return QVariant(keyBoardAutoRepeatRateMS());
#endif

      case QPlatformIntegration::StartDragTime:
      case QPlatformIntegration::StartDragDistance:
      case QPlatformIntegration::KeyboardInputInterval:
      case QPlatformIntegration::ShowIsFullScreen:
      case QPlatformIntegration::PasswordMaskDelay:
      case QPlatformIntegration::StartDragVelocity:
         break; // Not implemented

      case QPlatformIntegration::FontSmoothingGamma:
         return QVariant(QWindowsFontDatabase::fontSmoothingGamma());

      case QPlatformIntegration::MouseDoubleClickInterval:
         if (const UINT ms = GetDoubleClickTime()) {
            return QVariant(int(ms));
         }
         break;

      case QPlatformIntegration::UseRtlExtensions:
         return QVariant(d->m_context.useRTLExtensions());

      default:
         break;
   }

   return QPlatformIntegration::styleHint(hint);
}

Qt::KeyboardModifiers QWindowsIntegration::queryKeyboardModifiers() const
{
   return QWindowsKeyMapper::queryKeyboardModifiers();
}

QList<int> QWindowsIntegration::possibleKeys(const QKeyEvent *e) const
{
   return d->m_context.possibleKeys(e);
}

#ifndef QT_NO_CLIPBOARD
QPlatformClipboard *QWindowsIntegration::clipboard() const
{
   return &d->m_clipboard;
}

#ifndef QT_NO_DRAGANDDROP
QPlatformDrag *QWindowsIntegration::drag() const
{
   return &d->m_drag;
}
#endif

#endif

QPlatformInputContext *QWindowsIntegration::inputContext() const
{
   return d->m_inputContext.data();
}

#ifndef QT_NO_ACCESSIBILITY
QPlatformAccessibility *QWindowsIntegration::accessibility() const
{
   return &d->m_accessibility;
}
#endif

unsigned QWindowsIntegration::options() const
{
   return d->m_options;
}

#if ! defined(QT_NO_SESSIONMANAGER)
QPlatformSessionManager *QWindowsIntegration::createPlatformSessionManager(const QString &id, const QString &key) const
{
   return new QWindowsSessionManager(id, key);
}
#endif

QAbstractEventDispatcher *QWindowsIntegration::createEventDispatcher() const
{
   return new QWindowsGuiEventDispatcher;
}

QStringList QWindowsIntegration::themeNames() const
{
   return QStringList(QWindowsTheme::name);
}

QPlatformTheme *QWindowsIntegration::createPlatformTheme(const QString &name) const
{
   if (name == QWindowsTheme::name) {
      return new QWindowsTheme;
   }

   return QPlatformIntegration::createPlatformTheme(name);
}

QPlatformServices *QWindowsIntegration::services() const
{
   return &d->m_services;
}

