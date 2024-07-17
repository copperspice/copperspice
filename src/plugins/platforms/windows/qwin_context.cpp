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

#include <qwin_context.h>
#include <qwin_integration.h>
#include <qwin_window.h>
#include <qwin_keymapper.h>
#include <qwin_mousehandler.h>
#include <qwin_global.h>
#include <qwin_mime.h>
#include <qwin_inputcontext.h>
#include <qwin_theme.h>
#include <qwin_screen.h>
#include <qwin_theme.h>

#ifndef QT_NO_ACCESSIBILITY
# include <qwin_accessibility.h>
#endif

#if ! defined(QT_NO_SESSIONMANAGER)
# include <qsessionmanager_p.h>
# include <qwin_session_manager.h>
#endif

#include <qwindow.h>
#include <qwindowsysteminterface.h>
#include <qplatform_nativeinterface.h>
#include <QApplication>
#include <QSet>
#include <QHash>
#include <QStringList>
#include <QDebug>
#include <QSysInfo>
#include <QScopedArrayPointer>

#include <qapplication_p.h>
#include <qsystemlibrary_p.h>
#include <qwin_gui_eventdispatcher_p.h>

#include <stdlib.h>
#include <stdio.h>
#include <windowsx.h>
#include <comdef.h>

#if ! defined(LANG_SYRIAC)
#  define LANG_SYRIAC 0x5a
#endif

static inline bool useRTL_Extensions(QSysInfo::WinVersion ver)
{
   if ((ver & QSysInfo::WV_NT_based) && (ver >= QSysInfo::WV_VISTA)) {
      // Since the IsValidLanguageGroup/IsValidLocale functions always return true on
      // Vista, check the Keyboard Layouts for enabling RTL.

      if (const int nLayouts = GetKeyboardLayoutList(0, nullptr)) {
         QScopedArrayPointer<HKL> lpList(new HKL[nLayouts]);
         GetKeyboardLayoutList(nLayouts, lpList.data());

         for (int i = 0; i < nLayouts; ++i) {
            switch (PRIMARYLANGID((quintptr)lpList[i])) {
               case LANG_ARABIC:
               case LANG_HEBREW:
               case LANG_FARSI:
               case LANG_SYRIAC:
                  return true;
               default:
                  break;
            }
         }
      }
      return false;
   } // NT/Vista

   // Pre-NT: figure out whether a RTL language is installed
   return IsValidLanguageGroup(LGRPID_ARABIC, LGRPID_INSTALLED)
      || IsValidLanguageGroup(LGRPID_HEBREW, LGRPID_INSTALLED)
      || IsValidLocale(MAKELCID(MAKELANGID(LANG_ARABIC, SUBLANG_DEFAULT), SORT_DEFAULT), LCID_INSTALLED)
      || IsValidLocale(MAKELCID(MAKELANGID(LANG_HEBREW, SUBLANG_DEFAULT), SORT_DEFAULT), LCID_INSTALLED)
      || IsValidLocale(MAKELCID(MAKELANGID(LANG_SYRIAC, SUBLANG_DEFAULT), SORT_DEFAULT), LCID_INSTALLED)
      || IsValidLocale(MAKELCID(MAKELANGID(LANG_FARSI, SUBLANG_DEFAULT), SORT_DEFAULT), LCID_INSTALLED);
}

#if ! defined(QT_NO_SESSIONMANAGER)
static inline QWindowsSessionManager *platformSessionManager()
{
   QApplicationPrivate *guiPrivate     = static_cast<QApplicationPrivate *>(QApplicationPrivate::instance());
   QSessionManagerPrivate *managerPrivate = QSessionManagerPrivate::get(guiPrivate->session_manager);

   return static_cast<QWindowsSessionManager *>(managerPrivate->platformSessionManager);
}
#endif

QWindowsUser32DLL::QWindowsUser32DLL()
   : setLayeredWindowAttributes(nullptr), updateLayeredWindow(nullptr), updateLayeredWindowIndirect(nullptr),
     isHungAppWindow(nullptr), isTouchWindow(nullptr), registerTouchWindow(nullptr), unregisterTouchWindow(nullptr),
     getTouchInputInfo(nullptr), closeTouchInputHandle(nullptr), setProcessDPIAware(nullptr),  addClipboardFormatListener(nullptr),
     removeClipboardFormatListener(nullptr), getDisplayAutoRotationPreferences(nullptr), setDisplayAutoRotationPreferences(nullptr)
{
}

void QWindowsUser32DLL::init()
{
   QSystemLibrary library("user32");

   // MinGW (g++ 3.4.5) accepts only C casts.
   setLayeredWindowAttributes = (SetLayeredWindowAttributes)(library.resolve("SetLayeredWindowAttributes"));
   updateLayeredWindow = (UpdateLayeredWindow)(library.resolve("UpdateLayeredWindow"));

   if (!setLayeredWindowAttributes || !updateLayeredWindow) {
      qFatal("This version of Windows is not supported (User32.dll is missing the symbols 'SetLayeredWindowAttributes', 'UpdateLayeredWindow').");
   }

   updateLayeredWindowIndirect = (UpdateLayeredWindowIndirect)(library.resolve("UpdateLayeredWindowIndirect"));
   isHungAppWindow = (IsHungAppWindow)library.resolve("IsHungAppWindow");
   setProcessDPIAware = (SetProcessDPIAware)library.resolve("SetProcessDPIAware");

   if (QSysInfo::windowsVersion() >= QSysInfo::WV_VISTA) {
      addClipboardFormatListener = (AddClipboardFormatListener)library.resolve("AddClipboardFormatListener");
      removeClipboardFormatListener = (RemoveClipboardFormatListener)library.resolve("RemoveClipboardFormatListener");
   }

   getDisplayAutoRotationPreferences = (GetDisplayAutoRotationPreferences)library.resolve("GetDisplayAutoRotationPreferences");
   setDisplayAutoRotationPreferences = (SetDisplayAutoRotationPreferences)library.resolve("SetDisplayAutoRotationPreferences");
}

bool QWindowsUser32DLL::initTouch()
{
   if (!isTouchWindow && QSysInfo::windowsVersion() >= QSysInfo::WV_WINDOWS7) {
      QSystemLibrary library(QString("user32"));
      isTouchWindow = (IsTouchWindow)(library.resolve("IsTouchWindow"));
      registerTouchWindow = (RegisterTouchWindow)(library.resolve("RegisterTouchWindow"));
      unregisterTouchWindow = (UnregisterTouchWindow)(library.resolve("UnregisterTouchWindow"));
      getTouchInputInfo = (GetTouchInputInfo)(library.resolve("GetTouchInputInfo"));
      closeTouchInputHandle = (CloseTouchInputHandle)(library.resolve("CloseTouchInputHandle"));
   }
   return isTouchWindow && registerTouchWindow && unregisterTouchWindow && getTouchInputInfo && closeTouchInputHandle;
}

QWindowsShell32DLL::QWindowsShell32DLL()
   : sHCreateItemFromParsingName(nullptr), sHGetKnownFolderIDList(nullptr), sHGetStockIconInfo(nullptr),
     sHGetImageList(nullptr), sHCreateItemFromIDList(nullptr)
{
}

void QWindowsShell32DLL::init()
{
   QSystemLibrary library(QString("shell32"));
   sHCreateItemFromParsingName = (SHCreateItemFromParsingName)(library.resolve("SHCreateItemFromParsingName"));
   sHGetKnownFolderIDList = (SHGetKnownFolderIDList)(library.resolve("SHGetKnownFolderIDList"));
   sHGetStockIconInfo = (SHGetStockIconInfo)library.resolve("SHGetStockIconInfo");
   sHGetImageList = (SHGetImageList)library.resolve("SHGetImageList");
   sHCreateItemFromIDList = (SHCreateItemFromIDList)library.resolve("SHCreateItemFromIDList");
}

QWindowsShcoreDLL::QWindowsShcoreDLL()
   : getProcessDpiAwareness(nullptr), setProcessDpiAwareness(nullptr), getDpiForMonitor(nullptr)
{
}

void QWindowsShcoreDLL::init()
{
   if (QSysInfo::windowsVersion() < QSysInfo::WV_WINDOWS8_1) {
      return;
   }

   QSystemLibrary library("SHCore");
   getProcessDpiAwareness = (GetProcessDpiAwareness)library.resolve("GetProcessDpiAwareness");
   setProcessDpiAwareness = (SetProcessDpiAwareness)library.resolve("SetProcessDpiAwareness");
   getDpiForMonitor = (GetDpiForMonitor)library.resolve("GetDpiForMonitor");
}

QWindowsUser32DLL QWindowsContext::user32dll;
QWindowsShell32DLL QWindowsContext::shell32dll;
QWindowsShcoreDLL QWindowsContext::shcoredll;

QWindowsContext *QWindowsContext::m_instance = nullptr;

typedef QHash<HWND, QWindowsWindow *> HandleBaseWindowHash;

struct QWindowsContextPrivate {

   QWindowsContextPrivate();

   unsigned m_systemInfo;
   QSet<QString> m_registeredWindowClassNames;
   HandleBaseWindowHash m_windows;
   HDC m_displayContext;
   int m_defaultDPI;

   QWindowsKeyMapper m_keyMapper;
   QWindowsMouseHandler m_mouseHandler;
   QWindowsMimeConverter m_mimeConverter;
   QWindowsScreenManager m_screenManager;
   QSharedPointer<QWindowCreationContext> m_creationContext;

   const HRESULT m_oleInitializeResult;
   const QByteArray m_eventType;
   QWindow *m_lastActiveWindow;
   bool m_asyncExpose;
};

QWindowsContextPrivate::QWindowsContextPrivate()
   : m_systemInfo(0), m_oleInitializeResult(OleInitialize(nullptr)),
     m_eventType("windows_generic_MSG"), m_lastActiveWindow(nullptr), m_asyncExpose(false)
{
   const QSysInfo::WinVersion ver = QSysInfo::windowsVersion();

   QWindowsContext::user32dll.init();
   QWindowsContext::shell32dll.init();
   QWindowsContext::shcoredll.init();

   if (m_mouseHandler.touchDevice() && QWindowsContext::user32dll.initTouch()) {
      m_systemInfo |= QWindowsContext::SI_SupportsTouch;
   }

   m_displayContext = GetDC(nullptr);
   m_defaultDPI = GetDeviceCaps(m_displayContext, LOGPIXELSY);

   if (useRTL_Extensions(ver)) {
      m_systemInfo |= QWindowsContext::SI_RTL_Extensions;
      m_keyMapper.setUseRTLExtensions(true);
   }

   if (FAILED(m_oleInitializeResult)) {
      qWarning() << "QWindowsContext: OleInitialize() Failed: "
         << QWindowsContext::comErrorString(m_oleInitializeResult);
   }
}

QWindowsContext::QWindowsContext()
   : d(new QWindowsContextPrivate)
{
   m_instance = this;
}

QWindowsContext::~QWindowsContext()
{
   unregisterWindowClasses();
   if (d->m_oleInitializeResult == S_OK || d->m_oleInitializeResult == S_FALSE) {
      OleUninitialize();
   }

   d->m_screenManager.clearScreens(); // Order: Potentially calls back to the windows.
   m_instance = nullptr;
}

bool QWindowsContext::initTouch()
{
   return initTouch(QWindowsIntegration::instance()->options());
}

bool QWindowsContext::initTouch(unsigned integrationOptions)
{
   if (d->m_systemInfo & QWindowsContext::SI_SupportsTouch) {
      return true;
   }

   QTouchDevice *touchDevice = d->m_mouseHandler.ensureTouchDevice();
   if (!touchDevice) {
      return false;
   }

   if (!QWindowsContext::user32dll.initTouch()) {
      delete touchDevice;
      return false;
   }

   if (!(integrationOptions & QWindowsIntegration::DontPassOsMouseEventsSynthesizedFromTouch)) {
      touchDevice->setCapabilities(touchDevice->capabilities() | QTouchDevice::MouseEmulation);
   }

   QWindowSystemInterface::registerTouchDevice(touchDevice);

   d->m_systemInfo |= QWindowsContext::SI_SupportsTouch;
   return true;
}

void QWindowsContext::setTabletAbsoluteRange(int a)
{
   (void) a;
}

int QWindowsContext::processDpiAwareness()
{
   int result;
   if (QWindowsContext::shcoredll.getProcessDpiAwareness
               && SUCCEEDED(QWindowsContext::shcoredll.getProcessDpiAwareness(nullptr, &result))) {
      return result;
   }

   return -1;
}

void QWindowsContext::setProcessDpiAwareness(QtWindows::ProcessDpiAwareness dpiAwareness)
{

#if defined(CS_SHOW_DEBUG_PLATFORM)
   qDebug() << "QWindowsContext::setProcessDpiAwareness() Dpi =" << dpiAwareness;
#endif

   if (QWindowsContext::shcoredll.isValid()) {
      const HRESULT hr = QWindowsContext::shcoredll.setProcessDpiAwareness(dpiAwareness);
      // E_ACCESSDENIED means set externally (MSVC manifest or external app loading plugin).
      // Silence warning in that case unless debug is enabled.

      if (FAILED(hr) && (hr != E_ACCESSDENIED)) {
         qWarning() << " QWindowsContext::setProcessDpiAwareness() Dpi = " << dpiAwareness << "\n  "
            << "Failed = " << QWindowsContext::comErrorString(hr) << " Using = " << QWindowsContext::processDpiAwareness();
      }

   } else {
      if (dpiAwareness != QtWindows::ProcessDpiUnaware && QWindowsContext::user32dll.setProcessDPIAware) {
         if (! QWindowsContext::user32dll.setProcessDPIAware()) {
            qErrnoWarning("SetProcessDPIAware() Failed");
         }
      }
   }
}

QWindowsContext *QWindowsContext::instance()
{
   return m_instance;
}

unsigned QWindowsContext::systemInfo() const
{
   return d->m_systemInfo;
}

bool QWindowsContext::useRTLExtensions() const
{
   return d->m_keyMapper.useRTLExtensions();
}

QList<int> QWindowsContext::possibleKeys(const QKeyEvent *e) const
{
   return d->m_keyMapper.possibleKeys(e);
}

void QWindowsContext::setWindowCreationContext(const QSharedPointer<QWindowCreationContext> &ctx)
{
   d->m_creationContext = ctx;
}

int QWindowsContext::defaultDPI() const
{
   return d->m_defaultDPI;
}

HDC QWindowsContext::displayContext() const
{
   return d->m_displayContext;
}

QWindow *QWindowsContext::keyGrabber() const
{
   return d->m_keyMapper.keyGrabber();
}

void QWindowsContext::setKeyGrabber(QWindow *w)
{
   d->m_keyMapper.setKeyGrabber(w);
}

// Window class registering code (from qapplication_win.cpp)

QString QWindowsContext::registerWindowClass(const QWindow *w)
{
   Q_ASSERT(w);

   const Qt::WindowFlags flags = w->flags();
   const Qt::WindowFlags type = flags & Qt::WindowType_Mask;

   // Determine style and icon.
   uint style = CS_DBLCLKS;
   bool icon = true;

   // The following will not set CS_OWNDC for any widget window, even if it contains a
   // QOpenGLWidget or QQuickWidget later on. That cannot be detected at this stage.
   if (w->surfaceType() == QSurface::OpenGLSurface || (flags & Qt::MSWindowsOwnDC)) {
      style |= CS_OWNDC;
   }

   if (!(flags & Qt::NoDropShadowWindowHint) && (QSysInfo::WindowsVersion & QSysInfo::WV_NT_based)
      && (type == Qt::Popup || w->property("_q_windowsDropShadow").toBool())) {
      style |= CS_DROPSHADOW;
   }

   switch (type) {
      case Qt::Tool:
      case Qt::ToolTip:
      case Qt::Popup:
         style |= CS_SAVEBITS; // Save/restore background
         icon = false;
         break;
      case Qt::Dialog:
         if (!(flags & Qt::WindowSystemMenuHint)) {
            icon = false;
         }
         break;
   }

   // Create a unique name for the flag combination
   QString cname = QString("CsQWindow");

   switch (type) {
      case Qt::Tool:
         cname += QString("Tool");
         break;

      case Qt::ToolTip:
         cname += QString("ToolTip");
         break;

      case Qt::Popup:
         cname += QString("Popup");
         break;

      default:
         break;
   }

   if (style & CS_DROPSHADOW) {
      cname += QString("DropShadow");
   }
   if (style & CS_SAVEBITS) {
      cname += QString("SaveBits");
   }
   if (style & CS_OWNDC) {
      cname += QString("OwnDC");
   }
   if (icon) {
      cname += QString("Icon");
   }

   return registerWindowClass(cname, qWindowsWndProc, style, GetSysColorBrush(COLOR_WINDOW), icon);
}

QString QWindowsContext::registerWindowClass(QString cname, WNDPROC proc, unsigned style, HBRUSH brush, bool icon)
{
   // since multiple versions can be used in one process
   // each one has to have window class names with a unique name
   // The first instance gets the unmodified name; if the class
   // has already been registered by another instance of CS then
   // add an instance-specific ID, the address of the window proc.
   static int classExists = -1;

   const HINSTANCE appInstance = static_cast<HINSTANCE>(GetModuleHandle(nullptr));
   if (classExists == -1) {
      WNDCLASS wcinfo;

      classExists = GetClassInfo(appInstance, cname.toStdWString().data(), &wcinfo);
      classExists = classExists && wcinfo.lpfnWndProc != proc;
   }

   if (classExists) {
      cname += QString::number(reinterpret_cast<quintptr>(proc));
   }

   if (d->m_registeredWindowClassNames.contains(cname)) {
      // already registered in our list
      return cname;
   }

   WNDCLASSEX wc;
   wc.cbSize       = sizeof(WNDCLASSEX);

   wc.style        = style;
   wc.lpfnWndProc  = proc;
   wc.cbClsExtra   = 0;
   wc.cbWndExtra   = 0;
   wc.hInstance    = appInstance;
   wc.hCursor      = nullptr;

   wc.hbrBackground = brush;

   if (icon) {
      wc.hIcon = static_cast<HICON>(LoadImage(appInstance, L"IDI_ICON1", IMAGE_ICON, 0, 0, LR_DEFAULTSIZE));
      if (wc.hIcon) {
         int sw = GetSystemMetrics(SM_CXSMICON);
         int sh = GetSystemMetrics(SM_CYSMICON);
         wc.hIconSm = static_cast<HICON>(LoadImage(appInstance, L"IDI_ICON1", IMAGE_ICON, sw, sh, 0));

      } else {
         wc.hIcon = static_cast<HICON>(LoadImage(nullptr, IDI_APPLICATION, IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_SHARED));
         wc.hIconSm = nullptr;
      }

   } else {
      wc.hIcon    = nullptr;
      wc.hIconSm  = nullptr;
   }

   std::wstring tmp = cname.toStdWString();

   wc.lpszMenuName  = nullptr;
   wc.lpszClassName = tmp.data();

   ATOM atom = RegisterClassEx(&wc);

   if (! atom) {
      qErrnoWarning("QApplication::regClass() Registering window class %s failed", csPrintable(cname));
   }

   d->m_registeredWindowClassNames.insert(cname);

#if defined(CS_SHOW_DEBUG_PLATFORM)
   qDebug() << "QWindowsContext::registerWindowClass() ClassName =" << cname << "\n"
      << "  Style = 0x" << hex << style << dec
      << "Brush =" << brush << " Icon =" << icon << " Atom =" << atom;
#endif

   return cname;
}

void QWindowsContext::unregisterWindowClasses()
{
   const HINSTANCE appInstance = static_cast<HINSTANCE>(GetModuleHandle(nullptr));

   for (const QString &name : d->m_registeredWindowClassNames) {
      if (! UnregisterClass(name.toStdWString().data(), appInstance)) {
         qErrnoWarning("UnregisterClass failed for %s", csPrintable(name));
      }
   }

   d->m_registeredWindowClassNames.clear();
}

int QWindowsContext::screenDepth() const
{
   return GetDeviceCaps(d->m_displayContext, BITSPIXEL);
}

QString QWindowsContext::windowsErrorMessage(unsigned long errorCode)
{
   QString rc = QString("#%1: ").formatArg(errorCode);
   wchar_t *lpMsgBuf;

   const DWORD len = FormatMessage(
         FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
         nullptr, errorCode, 0, reinterpret_cast<LPTSTR>(&lpMsgBuf), 0, nullptr);

   if (len != 0) {
      std::wstring tmp(lpMsgBuf, int(len));
      rc = QString::fromStdWString(tmp);

      LocalFree(lpMsgBuf);

   } else {
      rc += "<unknown error>";
   }

   return rc;
}

void QWindowsContext::addWindow(HWND hwnd, QWindowsWindow *w)
{
   d->m_windows.insert(hwnd, w);
}

void QWindowsContext::removeWindow(HWND hwnd)
{
   const HandleBaseWindowHash::iterator it = d->m_windows.find(hwnd);

   if (it != d->m_windows.end()) {
      if (d->m_keyMapper.keyGrabber() == it.value()->window()) {
         d->m_keyMapper.setKeyGrabber(nullptr);
      }

      d->m_windows.erase(it);
   }
}

QWindowsWindow *QWindowsContext::findPlatformWindow(HWND hwnd) const
{
   return d->m_windows.value(hwnd);
}

QWindowsWindow *QWindowsContext::findClosestPlatformWindow(HWND hwnd) const
{
   QWindowsWindow *window = d->m_windows.value(hwnd);

   // Requested hwnd may also be a child of a platform window in case of embedded native windows.
   // Find the closest parent that has a platform window.

   if (! window) {
      for (HWND w = hwnd; w; w = GetParent(w)) {
         window = d->m_windows.value(w);

         if (window) {
            break;
         }
      }
   }

   return window;
}

QWindow *QWindowsContext::findWindow(HWND hwnd) const
{
   if (const QWindowsWindow *bw = findPlatformWindow(hwnd)) {
      return bw->window();
   }

   return nullptr;
}

QWindow *QWindowsContext::windowUnderMouse() const
{
   return d->m_mouseHandler.windowUnderMouse();
}

void QWindowsContext::clearWindowUnderMouse()
{
   d->m_mouseHandler.clearWindowUnderMouse();
}

static inline bool findPlatformWindowHelper(const POINT &screenPoint, unsigned cwexFlags,
   const QWindowsContext *context, HWND *hwnd, QWindowsWindow **result)
{
   POINT point = screenPoint;
   ScreenToClient(*hwnd, &point);

   // Returns parent if inside & none matched.
   const HWND child = ChildWindowFromPointEx(*hwnd, point, cwexFlags);

   if (! child || child == *hwnd) {
      return false;
   }
   if (QWindowsWindow *window = context->findPlatformWindow(child)) {
      *result = window;
      *hwnd   = child;

      return true;
   }

   // QTBUG-40555: despite CWP_SKIPINVISIBLE, it is possible to hit on invisible
   // full screen windows of other applications that have WS_EX_TRANSPARENT set
   // (for example created by  screen sharing applications). In that case, try to
   // find a window by searching again with CWP_SKIPTRANSPARENT.
   // uses WS_EX_TRANSPARENT for Qt::WindowTransparentForInput as well.

   if (!(cwexFlags & CWP_SKIPTRANSPARENT)
      && (GetWindowLongPtr(child, GWL_EXSTYLE) & WS_EX_TRANSPARENT)) {
      const HWND nonTransparentChild = ChildWindowFromPointEx(*hwnd, point, cwexFlags | CWP_SKIPTRANSPARENT);

      if (QWindowsWindow *nonTransparentWindow = context->findPlatformWindow(nonTransparentChild)) {
         *result = nonTransparentWindow;
         *hwnd = nonTransparentChild;
         return true;
      }
   }

   *hwnd = child;

   return true;
}

QWindowsWindow *QWindowsContext::findPlatformWindowAt(HWND parent,
   const QPoint &screenPointIn, unsigned cwex_flags) const
{
   QWindowsWindow *result = nullptr;
   const POINT screenPoint = { screenPointIn.x(), screenPointIn.y() };

   while (findPlatformWindowHelper(screenPoint, cwex_flags, this, &parent, &result)) {
   }

   return result;
}

QWindowsMimeConverter &QWindowsContext::mimeConverter() const
{
   return d->m_mimeConverter;
}

QWindowsScreenManager &QWindowsContext::screenManager()
{
   return d->m_screenManager;
}

HWND QWindowsContext::createDummyWindow(const QString &classNameIn,
   const wchar_t *windowName, WNDPROC wndProc, DWORD style)
{
   if (! wndProc) {
      wndProc = DefWindowProc;
   }

   QString className = registerWindowClass(classNameIn, wndProc);

   return CreateWindowEx(0, className.toStdWString().data(),
         windowName, style, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
         HWND_MESSAGE, nullptr, static_cast<HINSTANCE>(GetModuleHandle(nullptr)), nullptr);
}


// Re-engineered from the inline function _com_error::ErrorMessage().
// We cannot use it directly since it uses swprintf_s(), which is not
// present in the MSVCRT.DLL found on Windows XP (QTBUG-35617).
static inline QString errorMessageFromComError(const _com_error &comError)
{
   TCHAR *message = nullptr;

   FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
      nullptr, DWORD(comError.Error()), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), message, 0, nullptr);

   if (message) {
      const QString result = QString::fromStdWString(std::wstring(message)).trimmed();
      LocalFree(static_cast<HLOCAL>(message));
      return result;
   }

   if (const WORD wCode = comError.WCode()) {
      return QString("IDispatch error #") + QString::number(wCode);
   }
   return QString("Unknown error 0x0") + QString::number(comError.Error(), 16);
}

QByteArray QWindowsContext::comErrorString(HRESULT hr)
{
   QByteArray result = "COM error 0x" + QByteArray::number(quintptr(hr), 16) + ' ';

   switch (hr) {
      case S_OK:
         result += "S_OK";
         break;
      case S_FALSE:
         result += "S_FALSE";
         break;
      case E_UNEXPECTED:
         result += "E_UNEXPECTED";
         break;
      case E_ACCESSDENIED:
         result += "E_ACCESSDENIED";
         break;
      case CO_E_ALREADYINITIALIZED:
         result += "CO_E_ALREADYINITIALIZED";
         break;
      case CO_E_NOTINITIALIZED:
         result += "CO_E_NOTINITIALIZED";
         break;
      case RPC_E_CHANGED_MODE:
         result += "RPC_E_CHANGED_MODE";
         break;
      case OLE_E_WRONGCOMPOBJ:
         result += "OLE_E_WRONGCOMPOBJ";
         break;
      case CO_E_NOT_SUPPORTED:
         result += "CO_E_NOT_SUPPORTED";
         break;
      case E_NOTIMPL:
         result += "E_NOTIMPL";
         break;
      case E_INVALIDARG:
         result += "E_INVALIDARG";
         break;
      case E_NOINTERFACE:
         result += "E_NOINTERFACE";
         break;
      case E_POINTER:
         result += "E_POINTER";
         break;
      case E_HANDLE:
         result += "E_HANDLE";
         break;
      case E_ABORT:
         result += "E_ABORT";
         break;
      case E_FAIL:
         result += "E_FAIL";
         break;
      case RPC_E_WRONG_THREAD:
         result += "RPC_E_WRONG_THREAD";
         break;
      case RPC_E_THREAD_NOT_INIT:
         result += "RPC_E_THREAD_NOT_INIT";
         break;
      default:
         break;
   }

   _com_error error(hr);
   result += " (" + errorMessageFromComError(error).toUtf8() + ')';

   return result;
}

static inline QWindowsInputContext *windowsInputContext()
{
   return dynamic_cast<QWindowsInputContext *>(QWindowsIntegration::instance()->inputContext());
}

bool QWindowsContext::windowsProc(HWND hwnd, UINT message, QtWindows::WindowsEventType et,
      WPARAM wParam, LPARAM lParam, LRESULT *result)
{
   *result = 0;

   MSG msg;
   msg.hwnd = hwnd;         // re-create MSG structure
   msg.message = message;   // time and pt fields ignored
   msg.wParam = wParam;
   msg.lParam = lParam;
   msg.pt.x = msg.pt.y = 0;

   if (et != QtWindows::CursorEvent && (et & (QtWindows::MouseEventFlag | QtWindows::NonClientEventFlag))) {
      msg.pt.x = GET_X_LPARAM(lParam);
      msg.pt.y = GET_Y_LPARAM(lParam);
      // For non-client-area messages, these are screen coordinates (as expected
      // in the MSG structure), otherwise they are client coordinates.
      if (! (et & QtWindows::NonClientEventFlag)) {
         ClientToScreen(msg.hwnd, &msg.pt);
      }

   } else {
      GetCursorPos(&msg.pt);

   }

   // Run the native event filters
   long filterResult = 0;
   QAbstractEventDispatcher *dispatcher = QAbstractEventDispatcher::instance();

   if (dispatcher && dispatcher->filterNativeEvent(d->m_eventType, &msg, &filterResult)) {
      *result = LRESULT(filterResult);
      return true;
   }

   QWindowsWindow *platformWindow = findPlatformWindow(hwnd);
   if (platformWindow) {
      filterResult = 0;

      if (QWindowSystemInterface::handleNativeEvent(platformWindow->window(), d->m_eventType, &msg, &filterResult)) {
         *result = LRESULT(filterResult);
         return true;
      }
   }

   if (et & QtWindows::InputMethodEventFlag) {
      QWindowsInputContext *windowsInputContext = ::windowsInputContext();

      // Disable IME assuming this is a special implementation hooking into keyboard input.
      // "Real" IME implementations should use a native event filter intercepting IME events.
      if (! windowsInputContext) {
         QWindowsInputContext::setWindowsImeEnabled(platformWindow, false);
         return false;
      }

      switch (et) {
         case QtWindows::InputMethodStartCompositionEvent:
            return windowsInputContext->startComposition(hwnd);

         case QtWindows::InputMethodCompositionEvent:
            return windowsInputContext->composition(hwnd, lParam);

         case QtWindows::InputMethodEndCompositionEvent:
            return windowsInputContext->endComposition(hwnd);

         case QtWindows::InputMethodRequest:
            return windowsInputContext->handleIME_Request(wParam, lParam, result);

         default:
            break;
      }

   }

   switch (et) {
      case QtWindows::GestureEvent:
#if ! defined(QT_NO_SESSIONMANAGER)
         return platformSessionManager()->isInteractionBlocked() ? true :
               d->m_mouseHandler.translateGestureEvent(platformWindow->window(), hwnd, et, msg, result);
#else
         return d->m_mouseHandler.translateGestureEvent(platformWindow->window(), hwnd, et, msg, result);
#endif

      case QtWindows::InputMethodOpenCandidateWindowEvent:
      case QtWindows::InputMethodCloseCandidateWindowEvent:
         // TODO: Release/regrab mouse if a popup has mouse grab
         return false;

      case QtWindows::DestroyEvent:
         if (platformWindow && !platformWindow->testFlag(QWindowsWindow::WithinDestroy)) {
            qWarning() << "QWindowsContext::windowsProc() External WM_DESTROY received for " << platformWindow->window()
               << ", parent = " << platformWindow->window()->parent()
               << ", transient parent = " << platformWindow->window()->transientParent();
         }
         return false;

      case QtWindows::ClipboardEvent:
         return false;

      case QtWindows::UnknownEvent:
         return false;

      case QtWindows::AccessibleObjectFromWindowRequest:
#ifndef QT_NO_ACCESSIBILITY
         return QWindowsAccessibility::handleAccessibleObjectFromWindowRequest(hwnd, wParam, lParam, result);
#else
         return false;
#endif

      case QtWindows::DisplayChangedEvent:
         if (QWindowsTheme *t = QWindowsTheme::instance()) {
            t->displayChanged();
         }

         return d->m_screenManager.handleDisplayChange(wParam, lParam);

      case QtWindows::SettingChangedEvent:
         return d->m_screenManager.handleScreenChanges();

      default:
         break;
   }

   // Before CreateWindowEx() returns, some events are sent,
   // for example WM_GETMINMAXINFO asking for size constraints for top levels.
   // Pass on to current creation context
   if (! platformWindow && ! d->m_creationContext.isNull()) {
      switch (et) {
         case QtWindows::QuerySizeHints:
            d->m_creationContext->applyToMinMaxInfo(reinterpret_cast<MINMAXINFO *>(lParam));
            return true;

         case QtWindows::ResizeEvent:
            d->m_creationContext->obtainedGeometry.setSize(QSize(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)));
            return true;

         case QtWindows::MoveEvent:
            d->m_creationContext->obtainedGeometry.moveTo(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            return true;

         case QtWindows::CalculateSize:
            return QWindowsGeometryHint::handleCalculateSize(d->m_creationContext->customMargins, msg, result);

         case QtWindows::GeometryChangingEvent:
            return QWindowsWindow::handleGeometryChangingMessage(&msg, d->m_creationContext->window,
                  d->m_creationContext->margins + d->m_creationContext->customMargins);
         default:
            break;
      }
   }

   if (platformWindow) {
      // Suppress events sent during DestroyWindow() for native children.
      if (platformWindow->testFlag(QWindowsWindow::WithinDestroy)) {
         return false;
      }

   } else {
      qWarning("QWindowsContext::windowsProc() No Window found for event = 0x%x (%s), hwnd = 0x%p.",
            message, QWindowsGuiEventDispatcher::windowsMessageName(message), static_cast<void *>(hwnd));
      return false;
   }

   switch (et) {
      case QtWindows::KeyboardLayoutChangeEvent:
         if (QWindowsInputContext *wic = windowsInputContext()) {
            wic->handleInputLanguageChanged(wParam, lParam);
         }
         [[fallthrough]];

      case QtWindows::KeyDownEvent:
      case QtWindows::KeyEvent:
      case QtWindows::InputMethodKeyEvent:
      case QtWindows::InputMethodKeyDownEvent:
      case QtWindows::AppCommandEvent:
#if ! defined(QT_NO_SESSIONMANAGER)
         return platformSessionManager()->isInteractionBlocked() ? true
               : d->m_keyMapper.translateKeyEvent(platformWindow->window(), hwnd, msg, result);
#else
         return d->m_keyMapper.translateKeyEvent(platformWindow->window(), hwnd, msg, result);
#endif

      case QtWindows::MoveEvent:
         platformWindow->handleMoved();
         return true;

      case QtWindows::ResizeEvent:
         platformWindow->handleResized(static_cast<int>(wParam));
         return true;

      case QtWindows::QuerySizeHints:
         platformWindow->getSizeHints(reinterpret_cast<MINMAXINFO *>(lParam));
         return true;   // maybe available on some SDKs revisit WM_NCCALCSIZE

      case QtWindows::CalculateSize:
         return QWindowsGeometryHint::handleCalculateSize(platformWindow->customMargins(), msg, result);

      case QtWindows::NonClientHitTest:
         return platformWindow->handleNonClientHitTest(QPoint(msg.pt.x, msg.pt.y), result);

      case QtWindows::GeometryChangingEvent:
         return platformWindow->QWindowsWindow::handleGeometryChanging(&msg);

      case QtWindows::ExposeEvent:
         return platformWindow->handleWmPaint(hwnd, message, wParam, lParam);

      case QtWindows::NonClientMouseEvent:
         if (platformWindow->frameStrutEventsEnabled())
#if ! defined(QT_NO_SESSIONMANAGER)
            return platformSessionManager()->isInteractionBlocked() ? true :
                  d->m_mouseHandler.translateMouseEvent(platformWindow->window(), hwnd, et, msg, result);
#else
            return d->m_mouseHandler.translateMouseEvent(platformWindow->window(), hwnd, et, msg, result);
#endif
         break;

      case QtWindows::ScrollEvent:
#if ! defined(QT_NO_SESSIONMANAGER)
         return platformSessionManager()->isInteractionBlocked() ? true :
                  d->m_mouseHandler.translateScrollEvent(platformWindow->window(), hwnd, msg, result);
#else
         return d->m_mouseHandler.translateScrollEvent(platformWindow->window(), hwnd, msg, result);
#endif

      case QtWindows::MouseWheelEvent:
      case QtWindows::MouseEvent:
      case QtWindows::LeaveEvent:

#if ! defined(QT_NO_SESSIONMANAGER)
         return platformSessionManager()->isInteractionBlocked() ? true :
               d->m_mouseHandler.translateMouseEvent(platformWindow->window(), hwnd, et, msg, result);
#else
         return d->m_mouseHandler.translateMouseEvent(platformWindow->window(), hwnd, et, msg, result);
#endif

      case QtWindows::TouchEvent:
#if !defined(QT_NO_SESSIONMANAGER)
         return platformSessionManager()->isInteractionBlocked() ? true :
               d->m_mouseHandler.translateTouchEvent(platformWindow->window(), hwnd, et, msg, result);
#else
         return d->m_mouseHandler.translateTouchEvent(platformWindow->window(), hwnd, et, msg, result);
#endif

      case QtWindows::FocusInEvent: // see QWindowsWindow::requestActivateWindow().
      case QtWindows::FocusOutEvent:
         handleFocusEvent(et, platformWindow);
         return true;

      case QtWindows::ShowEventOnParentRestoring:
         // QTBUG-40696, prevent Windows from re-showing hidden transient children (dialogs)
         if (! platformWindow->window()->isVisible()) {
            *result = 0;
            return true;
         }
         break;

      case QtWindows::HideEvent:
         platformWindow->handleHidden();
         return false;// Indicate transient children should be hidden by windows (SW_PARENTCLOSING)

      case QtWindows::CloseEvent:
         QWindowSystemInterface::handleCloseEvent(platformWindow->window());
         return true;

      case QtWindows::ThemeChanged: {
         // Switch from Aero to Classic changes margins
         const Qt::WindowFlags flags = platformWindow->window()->flags();
         if ((flags & Qt::WindowType_Mask) != Qt::Desktop && ! (flags & Qt::FramelessWindowHint)) {
            platformWindow->setFlag(QWindowsWindow::FrameDirty);
         }

         if (QWindowsTheme *theme = QWindowsTheme::instance()) {
            theme->windowsThemeChanged(platformWindow->window());
         }

         return true;
      }

      case QtWindows::CompositionSettingsChanged:
         platformWindow->handleCompositionSettingsChanged();
         return true;

      case QtWindows::ActivateWindowEvent:
         if (platformWindow->window()->flags() & Qt::WindowDoesNotAcceptFocus) {
            *result = LRESULT(MA_NOACTIVATE);
            return true;
         }

         if (platformWindow->testFlag(QWindowsWindow::BlockedByModal)) {
            if (const QWindow *modalWindow = QApplication::modalWindow()) {
               QWindowsWindow::baseWindowOf(modalWindow)->alertWindow();
            }
         }
         break;

      case QtWindows::MouseActivateWindowEvent:
         if (platformWindow->window()->flags() & Qt::WindowDoesNotAcceptFocus) {
            *result = LRESULT(MA_NOACTIVATE);
            return true;
         }
         break;

#ifndef QT_NO_CONTEXTMENU
      case QtWindows::ContextMenu:
         return handleContextMenuEvent(platformWindow->window(), msg);
#endif

      case QtWindows::WhatsThisEvent: {

#ifndef QT_NO_WHATSTHIS
         QWindowSystemInterface::handleEnterWhatsThisEvent();
         return true;
#endif
      }
      break;

#if ! defined(QT_NO_SESSIONMANAGER)
      case QtWindows::QueryEndSessionApplicationEvent: {
         QWindowsSessionManager *sessionManager = platformSessionManager();

         if (sessionManager->isActive()) {
            // bogus message from windows
            *result = sessionManager->wasCanceled() ? 0 : 1;

            return true;
         }

         sessionManager->setActive(true);
         sessionManager->blocksInteraction();
         sessionManager->clearCancellation();

         QApplicationPrivate *qGuiAppPriv = static_cast<QApplicationPrivate *>(QApplicationPrivate::instance());
         qGuiAppPriv->commitData();

         if (lParam & ENDSESSION_LOGOFF) {
            fflush(nullptr);
         }

         *result = sessionManager->wasCanceled() ? 0 : 1;
         return true;
      }

      case QtWindows::EndSessionApplicationEvent: {
         QWindowsSessionManager *sessionManager = platformSessionManager();

         sessionManager->setActive(false);
         sessionManager->allowsInteraction();
         const bool endsession = wParam != 0;

         // we receive the message for each toplevel window included internal hidden ones,
         // but the aboutToQuit signal should be emitted only once.
         QApplicationPrivate *qGuiAppPriv = static_cast<QApplicationPrivate *>(QApplicationPrivate::instance());

         if (endsession && !qGuiAppPriv->aboutToQuitEmitted) {
            qGuiAppPriv->aboutToQuitEmitted = true;

            qApp->aboutToQuit();

            // since the process will be killed immediately quit() has no real effect
            QApplication::quit();
         }
         return true;
      }
#endif // ! defined(QT_NO_SESSIONMANAGER)

      default:
         break;
   }

   return false;
}

/* Compress activation events. If the next focus window is already known
 * at the time the current one receives focus-out, pass that to
 * QWindowSystemInterface instead of sending 0 and ignore its consecutive
 * focus-in event.
 * This helps applications that do handling in focus-out events. */
void QWindowsContext::handleFocusEvent(QtWindows::WindowsEventType et,
   QWindowsWindow *platformWindow)
{
   QWindow *nextActiveWindow = nullptr;

   if (et == QtWindows::FocusInEvent) {
      QWindow *topWindow   = QWindowsWindow::topLevelOf(platformWindow->window());
      QWindow *modalWindow = nullptr;

      if (QApplicationPrivate::instance()->isWindowBlocked(topWindow, &modalWindow) && topWindow != modalWindow) {
         modalWindow->requestActivate();
         return;
      }

      // QTBUG-32867: Invoking WinAPI SetParent() can cause focus-in for the
      // window which is not desired for native child widgets.

      if (platformWindow->testFlag(QWindowsWindow::WithinSetParent)) {
         QWindow *currentFocusWindow = QApplication::focusWindow();

         if (currentFocusWindow && currentFocusWindow != platformWindow->window()) {
            currentFocusWindow->requestActivate();
            return;
         }
      }

      nextActiveWindow = platformWindow->window();

   } else {
      // Focus out: Is the next window known and different from the receiving the focus out
      if (const HWND nextActiveHwnd = GetFocus())
         if (QWindowsWindow *nextActivePlatformWindow = findClosestPlatformWindow(nextActiveHwnd))
            if (nextActivePlatformWindow != platformWindow) {
               nextActiveWindow = nextActivePlatformWindow->window();
            }
   }

   if (nextActiveWindow != d->m_lastActiveWindow) {
      d->m_lastActiveWindow = nextActiveWindow;
      QWindowSystemInterface::handleWindowActivated(nextActiveWindow);
   }
}

#ifndef QT_NO_CONTEXTMENU
bool QWindowsContext::handleContextMenuEvent(QWindow *window, const MSG &msg)
{
   bool mouseTriggered = false;
   QPoint globalPos;
   QPoint pos;

   if (msg.lParam != int(0xffffffff)) {
      mouseTriggered = true;
      globalPos.setX(msg.pt.x);
      globalPos.setY(msg.pt.y);
      pos = QWindowsGeometryHint::mapFromGlobal(msg.hwnd, globalPos);

      RECT clientRect;

      if (GetClientRect(msg.hwnd, &clientRect)) {
         if (pos.x() < clientRect.left || pos.x() >= clientRect.right ||
            pos.y() < clientRect.top || pos.y() >= clientRect.bottom) {
            // This is the case that user has right clicked in the window's caption,
            // We should call DefWindowProc() to display a default shortcut menu
            // instead of sending a Qt window system event.
            return false;
         }
      }
   }

   QWindowSystemInterface::handleContextMenuEvent(window, mouseTriggered, pos, globalPos,
      QWindowsKeyMapper::queryKeyboardModifiers());

   return true;
}
#endif

bool QWindowsContext::asyncExpose() const
{
   return d->m_asyncExpose;
}

void QWindowsContext::setAsyncExpose(bool value)
{
   d->m_asyncExpose = value;
}

QTouchDevice *QWindowsContext::touchDevice() const
{
   return d->m_mouseHandler.touchDevice();
}

extern "C" LRESULT QT_WIN_CALLBACK qWindowsWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
   LRESULT result;
   const QtWindows::WindowsEventType et = windowsEventType(message, wParam, lParam);
   const bool handled = QWindowsContext::instance()->windowsProc(hwnd, message, et, wParam, lParam, &result);

   if (!handled) {
      result = DefWindowProc(hwnd, message, wParam, lParam);
   }

   return result;
}


