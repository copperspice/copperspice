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

#include "qwindowswindow.h"
#include "qwindowsnativeimage.h"
#include "qwindowscontext.h"
#include "qwindowsdrag.h"
#include "qwindowsscreen.h"
#include "qwindowsintegration.h"

#ifdef QT_OPENGL_DYNAMIC
#  include <qwindowsglcontext.h>
#else
#  include <qwindowsopenglcontext.h>
#endif

#ifdef QT_NO_CURSOR
#  include <qwindowscursor.h>
#endif

#include <QApplication>
#include <QDebug>
#include <QScreen>
#include <QWindow>
#include <QRegion>
#include <QOpenGLContext>
#include <qwindowsysteminterface.h>

#include <qsystemlibrary_p.h>
#include <qwindow_p.h>
#include <qapplication_p.h>
#include <qhighdpiscaling_p.h>

enum {
   defaultWindowWidth = 160,
   defaultWindowHeight = 160
};

Q_GUI_EXPORT HICON qt_pixmapToWinHICON(const QPixmap &);

static QByteArray debugWinStyle(DWORD style)
{
   QByteArray rc = "0x";
   rc += QByteArray::number(quint64(style), 16);
   if (style & WS_POPUP) {
      rc += " WS_POPUP";
   }
   if (style & WS_CHILD) {
      rc += " WS_CHILD";
   }
   if (style & WS_OVERLAPPED) {
      rc += " WS_OVERLAPPED";
   }
   if (style & WS_CLIPSIBLINGS) {
      rc += " WS_CLIPSIBLINGS";
   }
   if (style & WS_CLIPCHILDREN) {
      rc += " WS_CLIPCHILDREN";
   }
   if (style & WS_THICKFRAME) {
      rc += " WS_THICKFRAME";
   }
   if (style & WS_DLGFRAME) {
      rc += " WS_DLGFRAME";
   }
   if (style & WS_SYSMENU) {
      rc += " WS_SYSMENU";
   }
   if (style & WS_MINIMIZEBOX) {
      rc += " WS_MINIMIZEBOX";
   }
   if (style & WS_MAXIMIZEBOX) {
      rc += " WS_MAXIMIZEBOX";
   }
   return rc;
}

static QByteArray debugWinExStyle(DWORD exStyle)
{
   QByteArray rc = "0x";
   rc += QByteArray::number(quint64(exStyle), 16);
   if (exStyle & WS_EX_TOOLWINDOW) {
      rc += " WS_EX_TOOLWINDOW";
   }
   if (exStyle & WS_EX_CONTEXTHELP) {
      rc += " WS_EX_CONTEXTHELP";
   }
   if (exStyle & WS_EX_LAYERED) {
      rc += " WS_EX_LAYERED";
   }
   if (exStyle & WS_EX_DLGMODALFRAME) {
      rc += " WS_EX_DLGMODALFRAME";
   }
   return rc;
}

static inline QSize qSizeOfRect(const RECT &rect)
{
   return QSize(rect.right - rect.left, rect.bottom - rect.top);
}

static inline QRect qrectFromRECT(const RECT &rect)
{
   return QRect(QPoint(rect.left, rect.top), qSizeOfRect(rect));
}

static inline RECT RECTfromQRect(const QRect &rect)
{
   const int x = rect.left();
   const int y = rect.top();
   RECT result = { x, y, x + rect.width(), y + rect.height() };
   return result;
}

QDebug operator<<(QDebug d, const RECT &r)
{
   QDebugStateSaver saver(d);
   d.nospace();
   d << "RECT: left/top=" << r.left << ',' << r.top
      << " right/bottom=" << r.right << ',' << r.bottom;
   return d;
}

QDebug operator<<(QDebug d, const POINT &p)
{
   d << p.x << ',' << p.y;
   return d;
}

QDebug operator<<(QDebug d, const NCCALCSIZE_PARAMS &p)
{
   QDebugStateSaver saver(d);
   d.nospace();
   d << "NCCALCSIZE_PARAMS " << qrectFromRECT(p.rgrc[0])
      << ' ' << qrectFromRECT(p.rgrc[1]) << ' ' << qrectFromRECT(p.rgrc[2]);
   return d;
}

QDebug operator<<(QDebug d, const MINMAXINFO &i)
{
   QDebugStateSaver saver(d);
   d.nospace();
   d << "MINMAXINFO maxSize=" << i.ptMaxSize.x << ','
      << i.ptMaxSize.y << " maxpos=" << i.ptMaxPosition.x
      << ',' << i.ptMaxPosition.y << " mintrack="
      << i.ptMinTrackSize.x << ',' << i.ptMinTrackSize.y
      << " maxtrack=" << i.ptMaxTrackSize.x << ',' << i.ptMaxTrackSize.y;
   return d;
}

QDebug operator<<(QDebug d, const WINDOWPLACEMENT &wp)
{
   QDebugStateSaver saver(d);
   d.nospace();

   d <<  "WINDOWPLACEMENT(flags=0x" << hex << wp.flags << dec << ", showCmd="
      << wp.showCmd << ", ptMinPosition=" << wp.ptMinPosition << ", ptMaxPosition=" << wp.ptMaxPosition
      << ", rcNormalPosition=" << wp.rcNormalPosition;
   return d;
}

// QTBUG-43872, for windows that do not have WS_EX_TOOLWINDOW set, WINDOWPLACEMENT
// is in workspace/available area coordinates.
static QPoint windowPlacementOffset(HWND hwnd, const QPoint &point)
{

   if (GetWindowLongPtr(hwnd, GWL_EXSTYLE) & WS_EX_TOOLWINDOW) {
      return QPoint(0, 0);
   }

   const QWindowsScreenManager &screenManager = QWindowsContext::instance()->screenManager();
   const QWindowsScreen *screen = screenManager.screens().size() == 1
      ? screenManager.screens().constFirst() : screenManager.screenAtDp(point);

   if (screen) {
      return screen->availableGeometry().topLeft() - screen->geometry().topLeft();
   }

   return QPoint(0, 0);
}

// Return the frame geometry relative to the parent
// if there is one.
static inline QRect frameGeometry(HWND hwnd, bool topLevel)
{
   RECT rect = { 0, 0, 0, 0 };

   if (topLevel) {
      WINDOWPLACEMENT windowPlacement;
      windowPlacement.length = sizeof(WINDOWPLACEMENT);
      GetWindowPlacement(hwnd, &windowPlacement);
      if (windowPlacement.showCmd == SW_SHOWMINIMIZED) {
         const QRect result = qrectFromRECT(windowPlacement.rcNormalPosition);
         return result.translated(windowPlacementOffset(hwnd, result.topLeft()));
      }
   }

   GetWindowRect(hwnd, &rect); // Screen coordinates.
   const HWND parent = GetParent(hwnd);
   if (parent && !topLevel) {
      const int width = rect.right - rect.left;
      const int height = rect.bottom - rect.top;
      POINT leftTop = { rect.left, rect.top };
      ScreenToClient(parent, &leftTop);
      rect.left = leftTop.x;
      rect.top = leftTop.y;
      rect.right = leftTop.x + width;
      rect.bottom = leftTop.y + height;
   }
   return qrectFromRECT(rect);
}

// Return the visibility of the Window (except full screen since it is not a window state).
static QWindow::Visibility windowVisibility_sys(HWND hwnd)
{
   if (!IsWindowVisible(hwnd)) {
      return QWindow::Hidden;
   }

   WINDOWPLACEMENT windowPlacement;
   windowPlacement.length = sizeof(WINDOWPLACEMENT);
   if (GetWindowPlacement(hwnd, &windowPlacement)) {
      switch (windowPlacement.showCmd) {
         case SW_SHOWMINIMIZED:
         case SW_MINIMIZE:
         case SW_FORCEMINIMIZE:
            return QWindow::Minimized;
         case SW_SHOWMAXIMIZED:
            return QWindow::Maximized;
         default:
            break;
      }
   }

   return QWindow::Windowed;
}

static inline bool windowIsOpenGL(const QWindow *w)
{
   switch (w->surfaceType()) {
      case QSurface::OpenGLSurface:
         return true;
      case QSurface::RasterGLSurface:
         return qt_window_private(const_cast<QWindow *>(w))->compositing;
      default:
         return false;
   }
}

static bool applyBlurBehindWindow(HWND hwnd)
{
   enum { dwmBbEnable = 0x1, dwmBbBlurRegion = 0x2 };

   struct DwmBlurBehind {
      DWORD dwFlags;
      BOOL  fEnable;
      HRGN  hRgnBlur;
      BOOL  fTransitionOnMaximized;
   };

   typedef HRESULT (WINAPI * PtrDwmEnableBlurBehindWindow)(HWND, const DwmBlurBehind *);
   typedef HRESULT (WINAPI * PtrDwmIsCompositionEnabled)(BOOL *);

   // DWM API is available only from Windows Vista
   if (QSysInfo::windowsVersion() < QSysInfo::WV_VISTA) {
      return false;
   }

   static bool functionPointersResolved = false;
   static PtrDwmEnableBlurBehindWindow dwmBlurBehind = 0;
   static PtrDwmIsCompositionEnabled dwmIsCompositionEnabled = 0;

   if (! functionPointersResolved) {
      QSystemLibrary library(QString("dwmapi"));

      if (library.load()) {
         dwmBlurBehind = (PtrDwmEnableBlurBehindWindow)(library.resolve("DwmEnableBlurBehindWindow"));
         dwmIsCompositionEnabled = (PtrDwmIsCompositionEnabled)(library.resolve("DwmIsCompositionEnabled"));
      }

      functionPointersResolved = true;
   }

   if (! dwmBlurBehind || !dwmIsCompositionEnabled) {
      return false;
   }

   BOOL compositionEnabled;
   if (dwmIsCompositionEnabled(&compositionEnabled) != S_OK) {
      return false;
   }

   DwmBlurBehind blurBehind = {0, 0, 0, 0};

   if (compositionEnabled) {
      blurBehind.dwFlags = dwmBbEnable | dwmBbBlurRegion;
      blurBehind.fEnable = TRUE;
      blurBehind.hRgnBlur = CreateRectRgn(0, 0, -1, -1);
   } else {
      blurBehind.dwFlags = dwmBbEnable;
      blurBehind.fEnable = FALSE;
   }

   const bool result = dwmBlurBehind(hwnd, &blurBehind) == S_OK;

   if (blurBehind.hRgnBlur) {
      DeleteObject(blurBehind.hRgnBlur);
   }

   return result;

}

// from qwidget_win.cpp, pass flags separately in case they have been "autofixed".
static bool shouldShowMaximizeButton(const QWindow *w, Qt::WindowFlags flags)
{
   if ((flags & Qt::MSWindowsFixedSizeDialogHint) || !(flags & Qt::WindowMaximizeButtonHint)) {
      return false;
   }
   // if the user explicitly asked for the maximize button, we try to add
   // it even if the window has fixed size.
   return (flags & Qt::CustomizeWindowHint) ||
      w->maximumSize() == QSize(QWINDOWSIZE_MAX, QWINDOWSIZE_MAX);
}

// Set the WS_EX_LAYERED flag on a HWND if required. This is required for
// translucent backgrounds, not fully opaque windows and for
// Qt::WindowTransparentForInput (in combination with WS_EX_TRANSPARENT).
bool QWindowsWindow::setWindowLayered(HWND hwnd, Qt::WindowFlags flags, bool hasAlpha, qreal opacity)
{
   // maybe needs revisiting WS_EX_LAYERED
   const LONG exStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
   const bool needsLayered = (flags & Qt::WindowTransparentForInput)
      || (hasAlpha && (flags & Qt::FramelessWindowHint)) || opacity < 1.0;

   const bool isLayered = (exStyle & WS_EX_LAYERED);
   if (needsLayered != isLayered) {
      if (needsLayered) {
         SetWindowLong(hwnd, GWL_EXSTYLE, exStyle | WS_EX_LAYERED);
      } else {
         SetWindowLong(hwnd, GWL_EXSTYLE, exStyle & ~WS_EX_LAYERED);
      }
   }

   return needsLayered;
}

static void setWindowOpacity(HWND hwnd, Qt::WindowFlags flags, bool hasAlpha, bool openGL, qreal level)
{
   if (QWindowsWindow::setWindowLayered(hwnd, flags, hasAlpha, level)) {
      if (hasAlpha && !openGL && (flags & Qt::FramelessWindowHint)) {
         // Non-GL windows with alpha: Use blend function to update.
         BLENDFUNCTION blend = {AC_SRC_OVER, 0, (BYTE)(255.0 * level), AC_SRC_ALPHA};
         QWindowsContext::user32dll.updateLayeredWindow(hwnd, NULL, NULL, NULL, NULL, NULL, 0, &blend, ULW_ALPHA);
      } else {
         QWindowsContext::user32dll.setLayeredWindowAttributes(hwnd, 0, (int)(level * 255), LWA_ALPHA);
      }
   } else if (IsWindowVisible(hwnd)) { // Repaint when switching from layered.
      InvalidateRect(hwnd, NULL, TRUE);
   }
}

static inline void updateGLWindowSettings(const QWindow *w, HWND hwnd, Qt::WindowFlags flags, qreal opacity)
{
   const bool isGL = windowIsOpenGL(w);
   const bool hasAlpha = w->format().hasAlpha();

   if (isGL && hasAlpha) {
      applyBlurBehindWindow(hwnd);
   }

   setWindowOpacity(hwnd, flags, hasAlpha, isGL, opacity);
}

/*!
    \class WindowCreationData
    \brief Window creation code.

    This struct gathers all information required to create a window.
    Window creation is split in 3 steps:

    \list
    \li fromWindow() Gather all required information
    \li create() Create the system handle.
    \li initialize() Post creation initialization steps.
    \endlist

    The reason for this split is to also enable changing the QWindowFlags
    by calling:

    \list
    \li fromWindow() Gather information and determine new system styles
    \li applyWindowFlags() to apply the new window system styles.
    \li initialize() Post creation initialization steps.
    \endlist

    Contains the window creation code formerly in qwidget_win.cpp.

    \sa QWindowCreationContext
    \internal
    \ingroup qt-lighthouse-win
*/

struct WindowCreationData {
   typedef QWindowsWindowData WindowData;
   enum Flags { ForceChild = 0x1, ForceTopLevel = 0x2 };

   WindowCreationData() : parentHandle(0), type(Qt::Widget), style(0), exStyle(0),
      topLevel(false), popup(false), dialog(false), desktop(false),
      tool(false), embedded(false), hasAlpha(false) {}

   void fromWindow(const QWindow *w, const Qt::WindowFlags flags, unsigned creationFlags = 0);
   inline WindowData create(const QWindow *w, const WindowData &data, QString title) const;
   inline void applyWindowFlags(HWND hwnd) const;
   void initialize(const QWindow *w, HWND h, bool frameChange, qreal opacityLevel) const;

   Qt::WindowFlags flags;
   HWND parentHandle;
   Qt::WindowType type;
   unsigned style;
   unsigned exStyle;
   bool topLevel;
   bool popup;
   bool dialog;
   bool desktop;
   bool tool;
   bool embedded;
   bool hasAlpha;
};

QDebug operator<<(QDebug debug, const WindowCreationData &d)
{
   QDebugStateSaver saver(debug);

   debug.nospace();
   debug.noquote();

   debug << "WindowCreationData: " << d.flags
         << "\n  topLevel=" << d.topLevel;

   if (d.parentHandle) {
      debug << " parent=" << d.parentHandle;
   }

   debug << " popup=" << d.popup << " dialog=" << d.dialog << " desktop=" << d.desktop
      << " embedded=" << d.embedded << " tool=" << d.tool
      << "\n  style=" << debugWinStyle(d.style);

   if (d.exStyle) {
      debug << "\n  exStyle=" << debugWinExStyle(d.exStyle);
   }

   return debug;
}

// Fix top level window flags in case only the type flags are passed.
static inline void fixTopLevelWindowFlags(Qt::WindowFlags &flags)
{
   // Not supported on Windows, also do correction when it is set.
   flags &= ~Qt::WindowFullscreenButtonHint;
   switch (flags) {
      case Qt::Window:
         flags |= Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowMinimizeButtonHint
            | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint;
         break;
      case Qt::Dialog:
         flags |= Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowContextHelpButtonHint | Qt::WindowCloseButtonHint;
         break;
      case Qt::Tool:
         flags |= Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint;
         break;
      default:
         break;
   }
   if ((flags & Qt::WindowType_Mask) == Qt::SplashScreen) {
      flags |= Qt::FramelessWindowHint;
   }
}

void WindowCreationData::fromWindow(const QWindow *w, const Qt::WindowFlags flagsIn,
   unsigned creationFlags)
{
   flags = flagsIn;

   // Sometimes QWindow doesn't have a QWindow parent but does have a native parent window,
   // e.g. in case of embedded ActiveQt servers. They should not be considered a top-level
   // windows in such cases.
   QVariant prop = w->property("_q_embedded_native_parent_handle");
   if (prop.isValid()) {
      embedded = true;
      parentHandle = (HWND)prop.value<WId>();
   }

   if (creationFlags & ForceChild) {
      topLevel = false;
   } else if (embedded) {
      // Embedded native windows (for example Active X server windows) are by
      // definition never toplevel, even though they do not have QWindow parents.
      topLevel = false;
   } else {
      topLevel = (creationFlags & ForceTopLevel) ? true : w->isTopLevel();
   }

   if (topLevel) {
      fixTopLevelWindowFlags(flags);
   }

   type = static_cast<Qt::WindowType>(int(flags) & Qt::WindowType_Mask);
   switch (type) {
      case Qt::Dialog:
      case Qt::Sheet:
         dialog = true;
         break;
      case Qt::Drawer:
      case Qt::Tool:
         tool = true;
         break;
      case Qt::Popup:
         popup = true;
         break;
      case Qt::Desktop:
         desktop = true;
         break;
      default:
         break;
   }
   if ((flags & Qt::MSWindowsFixedSizeDialogHint)) {
      dialog = true;
   }

   // Parent: Use transient parent for top levels.
   if (popup) {
      flags |= Qt::WindowStaysOnTopHint; // a popup stays on top, no parent.
   } else if (!embedded) {
      if (const QWindow *parentWindow = topLevel ? w->transientParent() : w->parent()) {
         parentHandle = QWindowsWindow::handleOf(parentWindow);
      }
   }

   if (popup || (type == Qt::ToolTip) || (type == Qt::SplashScreen)) {
      style = WS_POPUP;
   } else if (topLevel && !desktop) {
      if (flags & Qt::FramelessWindowHint) {
         style = WS_POPUP;   // no border
      } else if (flags & Qt::WindowTitleHint) {
         style = WS_OVERLAPPED;
      } else {
         style = 0;
      }
   } else {
      style = WS_CHILD;
   }

   if (!desktop) {
      // if (!testAttribute(Qt::WA_PaintUnclipped))
      // ### Commented out for now as it causes some problems, but
      // this should be correct anyway, so dig some more into this
#ifdef Q_FLATTEN_EXPOSE
      if (windowIsOpenGL(w)) { // a bit incorrect since the is-opengl status may change from false to true at any time later on
         style |= WS_CLIPSIBLINGS | WS_CLIPCHILDREN;   // see SetPixelFormat
      }
#else
      style |= WS_CLIPSIBLINGS | WS_CLIPCHILDREN ;
#endif
      if (topLevel) {
         if ((type == Qt::Window || dialog || tool)) {
            if (!(flags & Qt::FramelessWindowHint)) {
               style |= WS_POPUP;
               if (flags & Qt::MSWindowsFixedSizeDialogHint) {
                  style |= WS_DLGFRAME;
               } else {
                  style |= WS_THICKFRAME;
               }
               if (flags & Qt::WindowTitleHint) {
                  style |= WS_CAPTION;   // Contains WS_DLGFRAME
               }
            }
            if (flags & Qt::WindowSystemMenuHint) {
               style |= WS_SYSMENU;
            } else if (dialog && (flags & Qt::WindowCloseButtonHint) && !(flags & Qt::FramelessWindowHint)) {
               style |= WS_SYSMENU | WS_BORDER; // QTBUG-2027, dialogs without system menu.
               exStyle |= WS_EX_DLGMODALFRAME;
            }
            if (flags & Qt::WindowMinimizeButtonHint) {
               style |= WS_MINIMIZEBOX;
            }
            if (shouldShowMaximizeButton(w, flags)) {
               style |= WS_MAXIMIZEBOX;
            }
            if (tool) {
               exStyle |= WS_EX_TOOLWINDOW;
            }
            if (flags & Qt::WindowContextHelpButtonHint) {
               exStyle |= WS_EX_CONTEXTHELP;
            }
         } else {
            exStyle |= WS_EX_TOOLWINDOW;
         }

         // make mouse events fall through this window
         // NOTE: WS_EX_TRANSPARENT flag can make mouse inputs fall through a layered window
         if (flagsIn & Qt::WindowTransparentForInput) {
            exStyle |= WS_EX_LAYERED | WS_EX_TRANSPARENT;
         }
      }
   }
}

QWindowsWindowData WindowCreationData::create(const QWindow *w, const WindowData &data, QString title) const
{
   typedef QSharedPointer<QWindowCreationContext> QWindowCreationContextPtr;

   WindowData result;
   result.flags = flags;

   if (desktop) {                        // desktop widget. No frame, hopefully?
      result.hwnd = GetDesktopWindow();
      result.geometry = frameGeometry(result.hwnd, true);
      result.embedded = false;

      return result;
   }
   if ((flags & Qt::WindowType_Mask) == Qt::ForeignWindow) {
      result.hwnd = reinterpret_cast<HWND>(w->winId());
      Q_ASSERT(result.hwnd);

      const LONG_PTR style   = GetWindowLongPtr(result.hwnd, GWL_STYLE);
      const LONG_PTR exStyle = GetWindowLongPtr(result.hwnd, GWL_EXSTYLE);
      result.embedded = false;
      result.frame = QWindowsGeometryHint::frame(style, exStyle);
      result.geometry = frameGeometry(result.hwnd, !GetParent(result.hwnd)).marginsRemoved(result.frame);

      qDebug() << "Foreign window: " << w << result.hwnd << result.geometry;

      return result;
   }

   const HINSTANCE appinst = (HINSTANCE)GetModuleHandle(0);

   const QString windowClassName = QWindowsContext::instance()->registerWindowClass(w);

   const QRect rect = QPlatformWindow::initialGeometry(w, data.geometry, defaultWindowWidth, defaultWindowHeight);

   if (title.isEmpty() && (result.flags & Qt::WindowTitleHint)) {
      title = topLevel ? qAppName() : w->objectName();
   }

   std::wstring tmp1 = title.toStdWString();
   std::wstring tmp2 = windowClassName.toStdWString();

   const wchar_t *titleUtf16     = tmp1.data();
   const wchar_t *classNameUtf16 = tmp2.data();

   // Capture events before CreateWindowEx() returns. The context is cleared in
   // the QWindowsWindow constructor.
   const QWindowCreationContextPtr context(new QWindowCreationContext(w, rect, data.customMargins, style, exStyle));
   QWindowsContext::instance()->setWindowCreationContext(context);

#if defined(CS_SHOW_DEBUG)
   qDebug() << "WindowCreationData::create(): Window =" << w << "\n  "
            << "Class =" << windowClassName << "Title =" << title << "\n  "
            << "Requested Size =" << rect << " Size Created =" << context->frameWidth
            << "x" << context->frameHeight
            << "+" << context->frameX << "+" << context->frameY << "\n  "
            << "Custom Margins =" << context->customMargins;
#endif

   result.hwnd = CreateWindowEx(exStyle, classNameUtf16, titleUtf16, style,
         context->frameX, context->frameY,
         context->frameWidth, context->frameHeight,
         parentHandle, NULL, appinst, NULL);

#if defined(CS_SHOW_DEBUG)
   qDebug() << "WindowCreationData::create(): Returns =" << w  << "\n  "
            << "Handle =" << result.hwnd << "\n  "
            << "Obtained geometry =" << context->obtainedGeometry << " Margins =" << context->margins;
#endif

   if (! result.hwnd) {
      qErrnoWarning("WindowCreationData::create(): CreateWindowEx failed");
      return result;
   }

   result.geometry = context->obtainedGeometry;
   result.frame = context->margins;
   result.embedded = embedded;
   result.customMargins = context->customMargins;

   return result;
}

void WindowCreationData::applyWindowFlags(HWND hwnd) const
{
   // Keep enabled and visible from the current style.
   const LONG_PTR oldStyle = GetWindowLongPtr(hwnd, GWL_STYLE);
   const LONG_PTR oldExStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);

   const LONG_PTR newStyle = style | (oldStyle & (WS_DISABLED | WS_VISIBLE));
   if (oldStyle != newStyle) {
      SetWindowLongPtr(hwnd, GWL_STYLE, newStyle);
   }
   const LONG_PTR newExStyle = exStyle;

   if (newExStyle != oldExStyle) {
      SetWindowLongPtr(hwnd, GWL_EXSTYLE, newExStyle);
   }

#if defined(CS_SHOW_DEBUG)
   qDebug() << "WindowCreationData::applyWindowFlags(): Handle =" << hwnd << *this << "\n  "
      << "Style from =" << debugWinStyle(oldStyle) << " To =" << debugWinStyle(newStyle) << "\n  "
      << "ExStyle from =" << debugWinExStyle(oldExStyle) << " To =" << debugWinExStyle(newExStyle);
#endif
}

void WindowCreationData::initialize(const QWindow *w, HWND hwnd, bool frameChange, qreal opacityLevel) const
{
   if (desktop || !hwnd) {
      return;
   }

   UINT swpFlags = SWP_NOMOVE | SWP_NOSIZE;
   if (frameChange) {
      swpFlags |= SWP_FRAMECHANGED;
   }

   if (topLevel) {
      swpFlags |= SWP_NOACTIVATE;
      if ((flags & Qt::WindowStaysOnTopHint) || (type == Qt::ToolTip)) {
         SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, swpFlags);
         if (flags & Qt::WindowStaysOnBottomHint) {
            qWarning() << "QWidget: Incompatible window flags: the window can't be on top and on bottom at the same time";
         }
      } else if (flags & Qt::WindowStaysOnBottomHint) {
         SetWindowPos(hwnd, HWND_BOTTOM, 0, 0, 0, 0, swpFlags);
      } else if (frameChange) { // Force WM_NCCALCSIZE with wParam=1 in case of custom margins.
         SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, swpFlags);
      }
      if (flags & (Qt::CustomizeWindowHint | Qt::WindowTitleHint)) {
         HMENU systemMenu = GetSystemMenu(hwnd, FALSE);
         if (flags & Qt::WindowCloseButtonHint) {
            EnableMenuItem(systemMenu, SC_CLOSE, MF_BYCOMMAND | MF_ENABLED);
         } else {
            EnableMenuItem(systemMenu, SC_CLOSE, MF_BYCOMMAND | MF_GRAYED);
         }
      }
      updateGLWindowSettings(w, hwnd, flags, opacityLevel);
   } else { // child.
      SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0, swpFlags);
   }
}


// Scaling helpers for size constraints.
static QSize toNativeSizeConstrained(QSize dip, const QWindow *w)
{
   if (QHighDpiScaling::isActive()) {
      const qreal factor = QHighDpiScaling::factor(w);
      if (dip.width() > 0 && dip.width() < QWINDOWSIZE_MAX) {
         dip.rwidth() *= factor;
      }
      if (dip.height() > 0 && dip.height() < QWINDOWSIZE_MAX) {
         dip.rheight() *= factor;
      }
   }
   return dip;
}

/*!
    \class QWindowsGeometryHint
    \brief Stores geometry constraints and provides utility functions.

    Geometry constraints ready to apply to a MINMAXINFO taking frame
    into account.

    \internal
    \ingroup qt-lighthouse-win
*/

QWindowsGeometryHint::QWindowsGeometryHint(const QWindow *w, const QMargins &cm) :
   minimumSize(toNativeSizeConstrained(w->minimumSize(), w)),
   maximumSize(toNativeSizeConstrained(w->maximumSize(), w)),
   customMargins(cm)
{
}

bool QWindowsGeometryHint::validSize(const QSize &s) const
{
   const int width = s.width();
   const int height = s.height();

   return width >= minimumSize.width() && width <= maximumSize.width()
      && height >= minimumSize.height() && height <= maximumSize.height();
}

QMargins QWindowsGeometryHint::frame(DWORD style, DWORD exStyle)
{
   RECT rect = {0, 0, 0, 0};

   style &= ~(WS_OVERLAPPED);

   if (! AdjustWindowRectEx(&rect, style, FALSE, exStyle)) {
      qErrnoWarning("QWindowsGeometryHint::frame(): AdjustWindowRectEx failed");
   }

   const QMargins result(qAbs(rect.left), qAbs(rect.top), qAbs(rect.right), qAbs(rect.bottom));

#if defined(CS_SHOW_DEBUG)
   qDebug() << "QWindowsGeometryHint::frame(): Style =" << showbase << hex << style << "\n  "
      << "ExStyle =" << exStyle << dec << noshowbase << " Rect =" << rect << " Result =" << result;
#endif

   return result;
}

bool QWindowsGeometryHint::handleCalculateSize(const QMargins &customMargins, const MSG &msg, LRESULT *result)
{
   // NCCALCSIZE_PARAMS structure if wParam==TRUE
   if (!msg.wParam || customMargins.isNull()) {
      return false;
   }

   *result = DefWindowProc(msg.hwnd, msg.message, msg.wParam, msg.lParam);
   NCCALCSIZE_PARAMS *ncp = reinterpret_cast<NCCALCSIZE_PARAMS *>(msg.lParam);

   const RECT oldClientArea = ncp->rgrc[0];
   ncp->rgrc[0].left += customMargins.left();
   ncp->rgrc[0].top += customMargins.top();
   ncp->rgrc[0].right -= customMargins.right();
   ncp->rgrc[0].bottom -= customMargins.bottom();
   result = 0;

   return true;
}

void QWindowsGeometryHint::applyToMinMaxInfo(HWND hwnd, MINMAXINFO *mmi) const
{
   return applyToMinMaxInfo(GetWindowLong(hwnd, GWL_STYLE),
         GetWindowLong(hwnd, GWL_EXSTYLE), mmi);
}

void QWindowsGeometryHint::applyToMinMaxInfo(DWORD style, DWORD exStyle, MINMAXINFO *mmi) const
{
   const QMargins margins = QWindowsGeometryHint::frame(style, exStyle);
   const int frameWidth = margins.left() + margins.right() + customMargins.left() + customMargins.right();
   const int frameHeight = margins.top() + margins.bottom() + customMargins.top() + customMargins.bottom();

   if (minimumSize.width() > 0) {
      mmi->ptMinTrackSize.x = minimumSize.width() + frameWidth;
   }

   if (minimumSize.height() > 0) {
      mmi->ptMinTrackSize.y = minimumSize.height() + frameHeight;
   }

   const int maximumWidth = qMax(maximumSize.width(), minimumSize.width());
   const int maximumHeight = qMax(maximumSize.height(), minimumSize.height());
   if (maximumWidth < QWINDOWSIZE_MAX) {
      mmi->ptMaxTrackSize.x = maximumWidth + frameWidth;
   }   if (maximumHeight < QWINDOWSIZE_MAX) {
      mmi->ptMaxTrackSize.y = maximumHeight + frameHeight;
   }
}

bool QWindowsGeometryHint::positionIncludesFrame(const QWindow *w)
{
   return qt_window_private(const_cast<QWindow *>(w))->positionPolicy
      == QWindowPrivate::WindowFrameInclusive;
}

QWindowCreationContext::QWindowCreationContext(const QWindow *w, const QRect &geometry, const QMargins &cm,
                  DWORD style_, DWORD exStyle_)
   : geometryHint(w, cm), window(w), style(style_), exStyle(exStyle_), requestedGeometry(geometry),
     obtainedGeometry(geometry), margins(QWindowsGeometryHint::frame(style, exStyle)), customMargins(cm),
     frameX(CW_USEDEFAULT), frameY(CW_USEDEFAULT),frameWidth(CW_USEDEFAULT), frameHeight(CW_USEDEFAULT)
{
   // Geometry of toplevels does not consider window frames.
   // TODO: No concept of WA_wasMoved yet that would indicate a
   // CW_USEDEFAULT unless set. For now, assume that 0,0 means 'default' for toplevels.

   if (geometry.isValid()) {
      frameX = geometry.x();
      frameY = geometry.y();
      const QMargins effectiveMargins = margins + customMargins;
      frameWidth = effectiveMargins.left() + geometry.width() + effectiveMargins.right();
      frameHeight = effectiveMargins.top() + geometry.height() + effectiveMargins.bottom();
      const bool isDefaultPosition = !frameX && !frameY && w->isTopLevel();

      if (!QWindowsGeometryHint::positionIncludesFrame(w) && !isDefaultPosition) {
         frameX -= effectiveMargins.left();
         frameY -= effectiveMargins.top();
      }
   }

#if defined(CS_SHOW_DEBUG)
   qDebug() << "QWindowCreationContext():  Window =" << w << "\n  "
            << "Geometry =" << geometry << " Pos including frame =" << QWindowsGeometryHint::positionIncludesFrame(w)
            << " Frame =" << frameWidth << 'x' << frameHeight << '+'
            << frameX << '+' << frameY << "\n  "
            << "Min =" << geometryHint.minimumSize << " Max =" << geometryHint.maximumSize
            << " Custom Margins =" << customMargins;
#endif
}

QWindowsWindow::QWindowsWindow(QWindow *aWindow, const QWindowsWindowData &data)
   : QPlatformWindow(aWindow), m_data(data), m_flags(WithinCreate), m_hdc(0), m_windowState(Qt::WindowNoState),
     m_opacity(1.0), m_cursor(new CursorHandle), m_dropTarget(0), m_savedStyle(0),
     m_format(aWindow->requestedFormat()), m_iconSmall(0), m_iconBig(0), m_surface(0)
{
   // Clear the creation context as the window can be found in QWindowsContext's map.
   QWindowsContext::instance()->setWindowCreationContext(QSharedPointer<QWindowCreationContext>());
   QWindowsContext::instance()->addWindow(m_data.hwnd, this);

   const Qt::WindowType type = aWindow->type();
   if (type == Qt::Desktop) {
      return;   // No further handling for Qt::Desktop
   }

#ifndef QT_NO_OPENGL
   if (aWindow->surfaceType() == QWindow::OpenGLSurface) {
      if (QOpenGLContext::openGLModuleType() == QOpenGLContext::LibGL) {
         setFlag(OpenGLSurface);
      } else {
         setFlag(OpenGL_ES2);
      }
   }
#endif

   updateDropSite(window()->isTopLevel());

   registerTouchWindow();
   setWindowState(aWindow->windowState());
   const qreal opacity = qt_window_private(aWindow)->opacity;
   if (!qFuzzyCompare(opacity, qreal(1.0))) {
      setOpacity(opacity);
   }
   if (aWindow->isTopLevel()) {
      setWindowIcon(aWindow->icon());
   }
   clearFlag(WithinCreate);
}

QWindowsWindow::~QWindowsWindow()
{
   setFlag(WithinDestroy);

   if (testFlag(TouchRegistered)) {
      QWindowsContext::user32dll.unregisterTouchWindow(m_data.hwnd);
   }

   destroyWindow();
   destroyIcon();
}

void QWindowsWindow::fireExpose(const QRegion &region, bool force)
{
   if (region.isEmpty() && !force) {
      clearFlag(Exposed);
   } else {
      setFlag(Exposed);
   }
   QWindowSystemInterface::handleExposeEvent(window(), region);
}

static inline QWindow *findTransientChild(const QWindow *parent)
{
   for (QWindow *w : QApplication::topLevelWindows())
      if (w->transientParent() == parent) {
         return w;
      }
   return 0;
}

void QWindowsWindow::destroyWindow()
{
   if (m_data.hwnd) { // Stop event dispatching before Window is destroyed.
      setFlag(WithinDestroy);

      // Clear any transient child relationships as Windows will otherwise destroy them (QTBUG-35499, QTBUG-36666)
      if (QWindow *transientChild = findTransientChild(window()))
         if (QWindowsWindow *tw = QWindowsWindow::baseWindowOf(transientChild)) {
            tw->updateTransientParent();
         }

      QWindowsContext *context = QWindowsContext::instance();
      if (context->windowUnderMouse() == window()) {
         context->clearWindowUnderMouse();
      }

      if (hasMouseCapture()) {
         setMouseGrabEnabled(false);
      }

      setDropSiteEnabled(false);

#ifndef QT_NO_OPENGL
      if (m_surface) {
         if (QWindowsStaticOpenGLContext *staticOpenGLContext = QWindowsIntegration::staticOpenGLContext()) {
            staticOpenGLContext->destroyWindowSurface(m_surface);
         }
         m_surface = 0;
      }
#endif

      if (m_data.hwnd != GetDesktopWindow() && window()->type() != Qt::ForeignWindow) {
         DestroyWindow(m_data.hwnd);
      }
      context->removeWindow(m_data.hwnd);
      m_data.hwnd = 0;
   }
}

void QWindowsWindow::updateDropSite(bool topLevel)
{
   bool enabled = false;
   bool parentIsEmbedded = false;

   if (!topLevel) {
      // if the parent window is a foreign window wrapped via QWindow::fromWinId, we need to enable the drop site
      // on the first child window
      const QWindow *parent = window()->parent();
      if (parent && (parent->type() == Qt::ForeignWindow)) {
         parentIsEmbedded = true;
      }
   }

   if (topLevel || parentIsEmbedded) {
      switch (window()->type()) {
         case Qt::Window:
         case Qt::Dialog:
         case Qt::Sheet:
         case Qt::Drawer:
         case Qt::Popup:
         case Qt::Tool:
            enabled = true;
            break;
         default:
            break;
      }
   }
   setDropSiteEnabled(enabled);
}

void QWindowsWindow::setDropSiteEnabled(bool dropEnabled)
{
   if (isDropSiteEnabled() == dropEnabled) {
      return;
   }

#if !defined(QT_NO_CLIPBOARD) && !defined(QT_NO_DRAGANDDROP)
   if (dropEnabled) {
      Q_ASSERT(m_data.hwnd);
      m_dropTarget = new QWindowsOleDropTarget(window());
      RegisterDragDrop(m_data.hwnd, m_dropTarget);
      CoLockObjectExternal(m_dropTarget, true, true);
   } else {
      CoLockObjectExternal(m_dropTarget, false, true);
      m_dropTarget->Release();
      RevokeDragDrop(m_data.hwnd);
      m_dropTarget = 0;
   }
#endif
}

// Returns topmost QWindowsWindow ancestor even if there are embedded windows in the chain.
// Returns this window if it is the topmost ancestor.
QWindow *QWindowsWindow::topLevelOf(QWindow *w)
{
   while (QWindow *parent = w->parent()) {
      w = parent;
   }

   if (const QPlatformWindow *handle = w->handle()) {
      const QWindowsWindow *ww = static_cast<const QWindowsWindow *>(handle);
      if (ww->isEmbedded()) {
         HWND parentHWND = GetAncestor(ww->handle(), GA_PARENT);
         const HWND desktopHwnd = GetDesktopWindow();
         const QWindowsContext *ctx = QWindowsContext::instance();
         while (parentHWND && parentHWND != desktopHwnd) {
            if (QWindowsWindow *ancestor = ctx->findPlatformWindow(parentHWND)) {
               return topLevelOf(ancestor->window());
            }
            parentHWND = GetAncestor(parentHWND, GA_PARENT);
         }
      }
   }
   return w;
}

QWindowsWindowData QWindowsWindowData::create(const QWindow *w,
   const QWindowsWindowData &parameters,
   const QString &title)
{
   WindowCreationData creationData;
   creationData.fromWindow(w, parameters.flags);
   QWindowsWindowData result = creationData.create(w, parameters, title);
   // Force WM_NCCALCSIZE (with wParam=1) via SWP_FRAMECHANGED for custom margin.
   creationData.initialize(w, result.hwnd, !parameters.customMargins.isNull(), 1);
   return result;
}

void QWindowsWindow::setVisible(bool visible)
{
   const QWindow *win = window();

#if defined(CS_SHOW_DEBUG)
   qDebug() << "QWindowsWindow::setVisible(): Window =" << this << win << m_data.hwnd << visible;
#endif

   if (m_data.hwnd) {
      if (visible) {
         show_sys();

         // When the window is layered, we won't get WM_PAINT, and "we" are in control
         // over the rendering of the window
         // There is nobody waiting for this, so we don't need to flush afterwards.
         if (isLayered()) {
            fireExpose(QRect(0, 0, win->width(), win->height()));
         }
         // QTBUG-44928, QTBUG-7386: This is to resolve the problem where popups are
         // opened from the system tray and not being implicitly activated

         if (win->type() == Qt::Popup && !win->parent() && !QApplication::focusWindow()) {
            SetForegroundWindow(m_data.hwnd);
         }
      } else {
         if (hasMouseCapture()) {
            setMouseGrabEnabled(false);
         }
         hide_sys();
         fireExpose(QRegion());
      }
   }
}

bool QWindowsWindow::isVisible() const
{
   return m_data.hwnd && IsWindowVisible(m_data.hwnd);
}

bool QWindowsWindow::isActive() const
{
   // Check for native windows or children of the active native window.
   if (const HWND activeHwnd = GetForegroundWindow())
      if (m_data.hwnd == activeHwnd || IsChild(activeHwnd, m_data.hwnd)) {
         return true;
      }
   return false;
}

bool QWindowsWindow::isEmbedded(const QPlatformWindow *parentWindow) const
{
   if (parentWindow) {
      const QWindowsWindow *ww = static_cast<const QWindowsWindow *>(parentWindow);
      const HWND hwnd = ww->handle();
      if (!IsChild(hwnd, m_data.hwnd)) {
         return false;
      }
   }

   if (!m_data.embedded && parent()) {
      return parent()->isEmbedded();
   }

   return m_data.embedded;
}

QPoint QWindowsWindow::mapToGlobal(const QPoint &pos) const
{
   if (m_data.hwnd) {
      return QWindowsGeometryHint::mapToGlobal(m_data.hwnd, pos);
   } else {
      return pos;
   }
}

QPoint QWindowsWindow::mapFromGlobal(const QPoint &pos) const
{
   if (m_data.hwnd) {
      return QWindowsGeometryHint::mapFromGlobal(m_data.hwnd, pos);
   } else {
      return pos;
   }
}

static inline HWND transientParentHwnd(HWND hwnd)
{
   if (GetAncestor(hwnd, GA_PARENT) == GetDesktopWindow()) {
      const HWND rootOwnerHwnd = GetAncestor(hwnd, GA_ROOTOWNER);
      if (rootOwnerHwnd != hwnd) { // May return itself for toplevels.
         return rootOwnerHwnd;
      }
   }
   return 0;
}

// Update the transient parent for a toplevel window. The concept does not
// really exist on Windows, the relationship is set by passing a parent along with !WS_CHILD
// to window creation or by setting the parent using  GWL_HWNDPARENT (as opposed to
// SetParent, which would make it a real child).
void QWindowsWindow::updateTransientParent() const
{
   if (window()->type() == Qt::Popup) {
      return;   // QTBUG-34503, // a popup stays on top, no parent, see also WindowCreationData::fromWindow().
   }

   // Update transient parent.
   const HWND oldTransientParent = transientParentHwnd(m_data.hwnd);
   HWND newTransientParent = 0;

   if (const QWindow *tp = window()->transientParent())
      if (const QWindowsWindow *tw = QWindowsWindow::baseWindowOf(tp))
         if (!tw->testFlag(WithinDestroy)) { // Prevent destruction by parent window (QTBUG-35499, QTBUG-36666)
            newTransientParent = tw->handle();
         }

   if (newTransientParent != oldTransientParent) {
      SetWindowLongPtr(m_data.hwnd, GWL_HWNDPARENT, (LONG_PTR)newTransientParent);
   }
}

static inline bool testShowWithoutActivating(const QWindow *window)
{
   // QWidget-attribute Qt::WA_ShowWithoutActivating .
   const QVariant showWithoutActivating = window->property("_q_showWithoutActivating");
   return showWithoutActivating.isValid() && showWithoutActivating.toBool();
}

// partially from QWidgetPrivate::show_sys()
void QWindowsWindow::show_sys() const
{
   int sm = SW_SHOWNORMAL;
   bool fakedMaximize = false;
   const QWindow *w = window();
   const Qt::WindowFlags flags = w->flags();
   const Qt::WindowType type = w->type();
   if (w->isTopLevel()) {
      const Qt::WindowState state = w->windowState();
      if (state & Qt::WindowMinimized) {
         sm = SW_SHOWMINIMIZED;
         if (!isVisible()) {
            sm = SW_SHOWMINNOACTIVE;
         }
      } else {
         updateTransientParent();
         if (state & Qt::WindowMaximized) {
            sm = SW_SHOWMAXIMIZED;
            // Windows will not behave correctly when we try to maximize a window which does not
            // have minimize nor maximize buttons in the window frame. Windows would then ignore
            // non-available geometry, and rather maximize the widget to the full screen, minus the
            // window frame (caption). So, we do a trick here, by adding a maximize button before
            // maximizing the widget, and then remove the maximize button afterwards.
            if (flags & Qt::WindowTitleHint &&
               !(flags & (Qt::WindowMinMaxButtonsHint | Qt::FramelessWindowHint))) {
               fakedMaximize = TRUE;
               setStyle(style() | WS_MAXIMIZEBOX);
            }
         } // Qt::WindowMaximized
      } // !Qt::WindowMinimized
   }
   if (type == Qt::Popup || type == Qt::ToolTip || type == Qt::Tool || testShowWithoutActivating(w)) {
      sm = SW_SHOWNOACTIVATE;
   }

   if (w->windowState() & Qt::WindowMaximized) {
      setFlag(WithinMaximize);   // QTBUG-8361
   }

   ShowWindow(m_data.hwnd, sm);

   clearFlag(WithinMaximize);

   if (fakedMaximize) {
      setStyle(style() & ~WS_MAXIMIZEBOX);
      SetWindowPos(m_data.hwnd, 0, 0, 0, 0, 0,
         SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER
         | SWP_FRAMECHANGED);
   }
}

// partially from QWidgetPrivate::hide_sys()
void QWindowsWindow::hide_sys() const
{
   const Qt::WindowFlags flags = window()->flags();
   if (flags != Qt::Desktop) {
      if (flags & Qt::Popup) {
         ShowWindow(m_data.hwnd, SW_HIDE);
      } else {
         SetWindowPos(m_data.hwnd, 0, 0, 0, 0, 0, SWP_HIDEWINDOW | SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
      }
   }
}

void QWindowsWindow::setParent(const QPlatformWindow *newParent)
{
   if (m_data.hwnd) {
      setParent_sys(newParent);
   }
}

void QWindowsWindow::setParent_sys(const QPlatformWindow *parent)
{
   // Use GetAncestor instead of GetParent, as GetParent can return owner window for toplevels
   HWND oldParentHWND = GetAncestor(m_data.hwnd, GA_PARENT);
   HWND newParentHWND = 0;
   if (parent) {
      const QWindowsWindow *parentW = static_cast<const QWindowsWindow *>(parent);
      newParentHWND = parentW->handle();

   }

   // NULL handle means desktop window, which also has its proper handle -> disambiguate
   HWND desktopHwnd = GetDesktopWindow();
   if (oldParentHWND == desktopHwnd) {
      oldParentHWND = 0;
   }
   if (newParentHWND == desktopHwnd) {
      newParentHWND = 0;
   }

   if (newParentHWND != oldParentHWND) {
      const bool wasTopLevel = oldParentHWND == 0;
      const bool isTopLevel = newParentHWND == 0;

      setFlag(WithinSetParent);
      SetParent(m_data.hwnd, newParentHWND);
      clearFlag(WithinSetParent);

      // WS_CHILD/WS_POPUP must be manually set/cleared in addition
      // to dialog frames, etc (see  SetParent() ) if the top level state changes.
      // Force toplevel state as QWindow::isTopLevel cannot be relied upon here.
      if (wasTopLevel != isTopLevel) {
         setDropSiteEnabled(false);
         setWindowFlags_sys(window()->flags(), unsigned(isTopLevel ? WindowCreationData::ForceTopLevel : WindowCreationData::ForceChild));
         updateDropSite(isTopLevel);
      }
   }
}

void QWindowsWindow::handleHidden()
{
   fireExpose(QRegion());
}

void QWindowsWindow::handleCompositionSettingsChanged()
{
   const QWindow *w = window();
   if (w->surfaceType() == QWindow::OpenGLSurface && w->format().hasAlpha()) {
      applyBlurBehindWindow(handle());
   }
}

static QRect normalFrameGeometry(HWND hwnd)
{
   WINDOWPLACEMENT wp;
   wp.length = sizeof(WINDOWPLACEMENT);

   if (GetWindowPlacement(hwnd, &wp)) {
      const QRect result = qrectFromRECT(wp.rcNormalPosition);
      return result.translated(windowPlacementOffset(hwnd, result.topLeft()));
   }

   return QRect();
}

QRect QWindowsWindow::normalGeometry() const
{
   // Check for fake 'fullscreen' mode.
   const bool fakeFullScreen = m_savedFrameGeometry.isValid() && window()->windowState() == Qt::WindowFullScreen;
   const QRect frame = fakeFullScreen ? m_savedFrameGeometry : normalFrameGeometry(m_data.hwnd);
   const QMargins margins = fakeFullScreen ? QWindowsGeometryHint::frame(m_savedStyle, 0) : frameMargins();

   return frame.isValid() ? frame.marginsRemoved(margins) : frame;
}

void QWindowsWindow::setGeometry(const QRect &rectIn)
{
   QRect rect = rectIn;
   // This means it is a call from QWindow::setFramePosition() and
   // the coordinates include the frame (size is still the contents rectangle).
   if (QWindowsGeometryHint::positionIncludesFrame(window())) {
      const QMargins margins = frameMargins();
      rect.moveTopLeft(rect.topLeft() + QPoint(margins.left(), margins.top()));
   }

   if (m_windowState == Qt::WindowMinimized) {
      m_data.geometry = rect;   // Otherwise set by handleGeometryChange() triggered by event.
   }

   if (m_data.hwnd) {
      // ResizeEvent with resulting geometry will be sent, if we cannot achieve that size
      // (for example, window title minimal constraint) notify and warn
      setGeometry_sys(rect);

      if (m_data.geometry != rect) {
         qWarning("QWindowsWindow::setGeometry(): Unable to set geometry %dx%d+%d+%d on %s/'%s'.\n"
            "Resulting geometry = %dx%d+%d+%d, Frame = %d, %d, %d, %d \n"
            "Custom margin = %d, %d, %d, %d, Minimum size = %dx%d, Maximum size = %dx%d)\n",
            rect.width(), rect.height(), rect.x(), rect.y(),
            csPrintable(window()->metaObject()->className()), csPrintable(window()->objectName()),
            m_data.geometry.width(), m_data.geometry.height(),
            m_data.geometry.x(), m_data.geometry.y(),
            m_data.frame.left(), m_data.frame.top(),
            m_data.frame.right(), m_data.frame.bottom(),
            m_data.customMargins.left(), m_data.customMargins.top(),
            m_data.customMargins.right(), m_data.customMargins.bottom(),
            window()->minimumWidth(), window()->minimumHeight(),
            window()->maximumWidth(), window()->maximumHeight());
      }

   } else {
      QPlatformWindow::setGeometry(rect);
   }
}

void QWindowsWindow::handleMoved()
{
   // Minimize/Set parent can send nonsensical move events.
   if (! IsIconic(m_data.hwnd) && !testFlag(WithinSetParent)) {
      handleGeometryChange();
   }
}

void QWindowsWindow::handleResized(int wParam)
{
   switch (wParam) {
      case SIZE_MAXHIDE: // Some other window affected.
      case SIZE_MAXSHOW:
         return;

      case SIZE_MINIMIZED:
         handleWindowStateChange(Qt::WindowMinimized);
         return;

      case SIZE_MAXIMIZED:
         handleWindowStateChange(Qt::WindowMaximized);
         handleGeometryChange();
         break;

      case SIZE_RESTORED:
         if (isFullScreen_sys()) {
            handleWindowStateChange(Qt::WindowFullScreen);
         } else if (m_windowState != Qt::WindowNoState && !testFlag(MaximizeToFullScreen)) {
            handleWindowStateChange(Qt::WindowNoState);
         }

         handleGeometryChange();
         break;
   }
}

void QWindowsWindow::handleGeometryChange()
{
   // Prevent recursive resizes for Windows CE
   if (testFlag(WithinSetStyle)) {
      return;
   }

   const QRect previousGeometry = m_data.geometry;
   m_data.geometry = geometry_sys();
   QPlatformWindow::setGeometry(m_data.geometry);
   QWindowSystemInterface::handleGeometryChange(window(), m_data.geometry);

   // QTBUG-32121: OpenGL/normal windows (with exception of ANGLE) do not receive
   // expose events when shrinking, synthesize.
   if (! testFlag(OpenGL_ES2) && isExposed()
         && m_data.geometry.size() != previousGeometry.size() // Exclude plain move
         // One dimension grew -> Windows will send expose, no need to synthesize.
         && !(m_data.geometry.width() > previousGeometry.width() || m_data.geometry.height() > previousGeometry.height())) {
      fireExpose(QRect(QPoint(0, 0), m_data.geometry.size()), true);
   }

   if (previousGeometry.topLeft() != m_data.geometry.topLeft()) {
      QPlatformScreen *newScreen = screenForGeometry(m_data.geometry);
      if (newScreen != screen()) {
         QWindowSystemInterface::handleWindowScreenChanged(window(), newScreen->screen());
      }
   }

   if (testFlag(SynchronousGeometryChangeEvent)) {
      QWindowSystemInterface::flushWindowSystemEvents(QEventLoop::ExcludeUserInputEvents);
   }

#if defined(CS_SHOW_DEBUG)
   qDebug() << "QWindowsWindow::handleGeometryChange(): Window =" << window() << "\n  "
            << "Geometry =" << m_data.geometry;
#endif
}

void QWindowsWindow::setGeometry_sys(const QRect &rect) const
{
   const QMargins margins = frameMargins();
   const QRect frameGeometry = rect + margins;

#if defined(CS_SHOW_DEBUG)
   qDebug() << "QWindowsWindow::setGeometry_sys(): Window =" << window() << "\n  "
      << "From =" << geometry_sys() << "Frame =" << margins << " To =" << rect << "\n  "
      << "New frame =" << frameGeometry;
#endif

   bool result = false;

   WINDOWPLACEMENT windowPlacement;
   windowPlacement.length = sizeof(WINDOWPLACEMENT);
   GetWindowPlacement(m_data.hwnd, &windowPlacement);

   // If the window is hidden and in maximized state or minimized, instead of moving the
   // window, set the normal position of the window.
   if ((windowPlacement.showCmd == SW_MAXIMIZE && !IsWindowVisible(m_data.hwnd))
                  || windowPlacement.showCmd == SW_SHOWMINIMIZED) {

      windowPlacement.rcNormalPosition =
                  RECTfromQRect(frameGeometry.translated(-windowPlacementOffset(m_data.hwnd, frameGeometry.topLeft())));

      windowPlacement.showCmd = windowPlacement.showCmd == SW_SHOWMINIMIZED ? SW_SHOWMINIMIZED : SW_HIDE;
      result = SetWindowPlacement(m_data.hwnd, &windowPlacement);

   } else {
      result = MoveWindow(m_data.hwnd, frameGeometry.x(), frameGeometry.y(),
            frameGeometry.width(), frameGeometry.height(), true);
   }
}

QRect QWindowsWindow::frameGeometry_sys() const
{
   bool isRealTopLevel = window()->isTopLevel() && !m_data.embedded;
   return frameGeometry(m_data.hwnd, isRealTopLevel);
}

QRect QWindowsWindow::geometry_sys() const
{
   return frameGeometry_sys().marginsRemoved(frameMargins());
}

/*!
    Allocates a HDC for the window or returns the temporary one
    obtained from WinAPI BeginPaint within a WM_PAINT event.

    \sa releaseDC()
*/

HDC QWindowsWindow::getDC()
{
   if (!m_hdc) {
      m_hdc = GetDC(handle());
   }
   return m_hdc;
}

/*!
    Relases the HDC for the window or does nothing in
    case it was obtained from WinAPI BeginPaint within a WM_PAINT event.

    \sa getDC()
*/

void QWindowsWindow::releaseDC()
{
   if (m_hdc) {
      ReleaseDC(handle(), m_hdc);
      m_hdc = 0;
   }
}

bool QWindowsWindow::handleWmPaint(HWND hwnd, UINT message, WPARAM, LPARAM)
{
   if (message == WM_ERASEBKGND) { // Backing store - ignored.
      return true;
   }
   // Ignore invalid update bounding rectangles
   if (!GetUpdateRect(m_data.hwnd, 0, FALSE)) {
      return false;
   }
   PAINTSTRUCT ps;

#ifdef QT_OPENGL_DYNAMIC
   // QTBUG-58178: GL software rendering needs InvalidateRect() to suppress
   // artifacts while resizing.
   if (testFlag(OpenGLSurface)
      && QOpenGLStaticContext::opengl32.moduleIsNotOpengl32()
      && QOpenGLContext::openGLModuleType() == QOpenGLContext::LibGL) {
      InvalidateRect(hwnd, 0, false);
   }
#endif

   BeginPaint(hwnd, &ps);

   // Observed painting problems with Aero style disabled (QTBUG-7865).
   // 5.8: Consider making it dependent on !DwmIsCompositionEnabled().
   if (testFlag(OpenGLSurface) && testFlag(OpenGLDoubleBuffered)) {
      SelectClipRgn(ps.hdc, NULL);
   }

   // If the a window is obscured by another window (such as a child window)
   // we still need to send isExposed=true, for compatibility.
   // Our tests depend on it.
   fireExpose(QRegion(qrectFromRECT(ps.rcPaint)), true);
   if (!QWindowsContext::instance()->asyncExpose()) {
      QWindowSystemInterface::flushWindowSystemEvents(QEventLoop::ExcludeUserInputEvents);
   }

   EndPaint(hwnd, &ps);
   return true;
}

void QWindowsWindow::setWindowTitle(const QString &title)
{
   if (m_data.hwnd) {
      const QString fullTitle = formatWindowTitle(title, QString(" - "));
      SetWindowText(m_data.hwnd, fullTitle.toStdWString().data());
   }
}

void QWindowsWindow::setWindowFlags(Qt::WindowFlags flags)
{
   const QRect oldGeometry = geometry();

   if (m_data.flags != flags) {
      m_data.flags = flags;
      if (m_data.hwnd) {
         m_data = setWindowFlags_sys(flags);
         updateDropSite(window()->isTopLevel());
      }
   }

   // When switching to a frameless window, geometry
   // may change without a WM_MOVE. Report change manually.
   // Do not send synchronously as not to clobber the widget
   // geometry in a sequence of setting flags and geometry.

   const QRect newGeometry = geometry_sys();
   if (oldGeometry != newGeometry) {
      handleGeometryChange();
   }
}

QWindowsWindowData QWindowsWindow::setWindowFlags_sys(Qt::WindowFlags wt,
   unsigned flags) const
{
   WindowCreationData creationData;
   creationData.fromWindow(window(), wt, flags);
   creationData.applyWindowFlags(m_data.hwnd);
   creationData.initialize(window(), m_data.hwnd, true, m_opacity);

   QWindowsWindowData result = m_data;
   result.flags = creationData.flags;
   result.embedded = creationData.embedded;
   setFlag(FrameDirty);
   return result;
}

void QWindowsWindow::handleWindowStateChange(Qt::WindowState state)
{
   setFlag(FrameDirty);
   m_windowState = state;

   QWindowSystemInterface::handleWindowStateChanged(window(), state);
   switch (state) {
      case Qt::WindowMinimized:
         handleHidden();
         QWindowSystemInterface::flushWindowSystemEvents(QEventLoop::ExcludeUserInputEvents); // Tell QQuickWindow to stop rendering now.
         break;
      case Qt::WindowMaximized:
      case Qt::WindowFullScreen:
      case Qt::WindowNoState: {
         // QTBUG-17548: We send expose events when receiving WM_Paint, but for
         // layered windows and transient children, we won't receive any WM_Paint.
         QWindow *w = window();
         bool exposeEventsSent = false;
         if (isLayered()) {
            fireExpose(QRegion(0, 0, w->width(), w->height()));
            exposeEventsSent = true;
         }

         for (QWindow *child : QApplication::allWindows()) {
            if (child != w && child->isVisible() && child->transientParent() == w) {
               QWindowsWindow *platformWindow = QWindowsWindow::baseWindowOf(child);
               if (platformWindow->isLayered()) {
                  platformWindow->fireExpose(QRegion(0, 0, child->width(), child->height()));
                  exposeEventsSent = true;
               }
            }
         }
         if (exposeEventsSent && !QWindowsContext::instance()->asyncExpose()) {
            QWindowSystemInterface::flushWindowSystemEvents(QEventLoop::ExcludeUserInputEvents);
         }
      }
      break;
      default:
         break;
   }
}

void QWindowsWindow::setWindowState(Qt::WindowState state)
{
   if (m_data.hwnd) {
      setWindowState_sys(state);
      m_windowState = state;
   }
}

bool QWindowsWindow::isFullScreen_sys() const
{
   const QWindow *w = window();
   if (!w->isTopLevel()) {
      return false;
   }
   const QScreen *screen = w->screen();
   if (!screen) {
      screen = QApplication::primaryScreen();
   }
   return screen && geometry_sys() == QHighDpi::toNativePixels(screen->geometry(), w);
}

void QWindowsWindow::setWindowState_sys(Qt::WindowState newState)
{
   const Qt::WindowState oldState = m_windowState;
   if (oldState == newState) {
      return;
   }

   const bool visible = isVisible();

   setFlag(FrameDirty);

   if ((oldState == Qt::WindowFullScreen) != (newState == Qt::WindowFullScreen)) {

      if (newState == Qt::WindowFullScreen) {

#ifndef Q_FLATTEN_EXPOSE
         UINT newStyle = WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_POPUP;
#else
         UINT newStyle = WS_POPUP;
#endif
         // Save geometry and style to be restored when fullscreen
         // is turned off again, since on Windows, it is not a real
         // Window state but emulated by changing geometry and style.
         if (!m_savedStyle) {
            m_savedStyle = style();

            if (oldState == Qt::WindowMinimized || oldState == Qt::WindowMaximized) {
               const QRect nf = normalFrameGeometry(m_data.hwnd);
               if (nf.isValid()) {
                  m_savedFrameGeometry = nf;
               }

            } else {
               m_savedFrameGeometry = frameGeometry_sys();

            }

         }
         if (m_savedStyle & WS_SYSMENU) {
            newStyle |= WS_SYSMENU;
         }
         if (visible) {
            newStyle |= WS_VISIBLE;
         }
         if (testFlag(HasBorderInFullScreen)) {
            newStyle |= WS_BORDER;
         }
         setStyle(newStyle);
         // Use geometry of QWindow::screen() within creation or the virtual screen the
         // window is in (QTBUG-31166, QTBUG-30724).
         const QScreen *screen = window()->screen();
         if (!screen) {
            screen = QApplication::primaryScreen();
         }
         const QRect r = screen ? QHighDpi::toNativePixels(screen->geometry(), window()) : m_savedFrameGeometry;
         const UINT swpf = SWP_FRAMECHANGED | SWP_NOACTIVATE;
         const bool wasSync = testFlag(SynchronousGeometryChangeEvent);
         setFlag(SynchronousGeometryChangeEvent);
         SetWindowPos(m_data.hwnd, HWND_TOP, r.left(), r.top(), r.width(), r.height(), swpf);
         if (!wasSync) {
            clearFlag(SynchronousGeometryChangeEvent);
         }
         QWindowSystemInterface::handleGeometryChange(window(), r);
         QWindowSystemInterface::flushWindowSystemEvents(QEventLoop::ExcludeUserInputEvents);
      } else if (newState != Qt::WindowMinimized) {
         // Restore saved state.
         unsigned newStyle = m_savedStyle ? m_savedStyle : style();
         if (visible) {
            newStyle |= WS_VISIBLE;
         }
         setStyle(newStyle);

         UINT swpf = SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOACTIVATE;
         if (!m_savedFrameGeometry.isValid()) {
            swpf |= SWP_NOSIZE | SWP_NOMOVE;
         }
         const bool wasSync = testFlag(SynchronousGeometryChangeEvent);
         setFlag(SynchronousGeometryChangeEvent);
         // After maximized/fullscreen; the window can be in a maximized state. Clear
         // it before applying the normal geometry.
         if (windowVisibility_sys(m_data.hwnd) == QWindow::Maximized) {
            ShowWindow(m_data.hwnd, SW_SHOWNOACTIVATE);
         }
         SetWindowPos(m_data.hwnd, 0, m_savedFrameGeometry.x(), m_savedFrameGeometry.y(),
            m_savedFrameGeometry.width(), m_savedFrameGeometry.height(), swpf);
         if (!wasSync) {
            clearFlag(SynchronousGeometryChangeEvent);
         }
         // preserve maximized state
         if (visible) {
            setFlag(WithinMaximize);
            ShowWindow(m_data.hwnd, (newState == Qt::WindowMaximized) ? SW_MAXIMIZE : SW_SHOWNA);
            clearFlag(WithinMaximize);
         }
         m_savedStyle = 0;
         m_savedFrameGeometry = QRect();
      }
   } else if ((oldState == Qt::WindowMaximized) != (newState == Qt::WindowMaximized)) {
      if (visible && !(newState == Qt::WindowMinimized)) {
         setFlag(WithinMaximize);
         if (newState == Qt::WindowFullScreen) {
            setFlag(MaximizeToFullScreen);
         }
         ShowWindow(m_data.hwnd, (newState == Qt::WindowMaximized) ? SW_MAXIMIZE : SW_SHOWNOACTIVATE);
         clearFlag(WithinMaximize);
         clearFlag(MaximizeToFullScreen);
      }
   }

   if ((oldState == Qt::WindowMinimized) != (newState == Qt::WindowMinimized)) {
      if (visible)
         ShowWindow(m_data.hwnd, (newState == Qt::WindowMinimized) ? SW_MINIMIZE :
            (newState == Qt::WindowMaximized) ? SW_MAXIMIZE : SW_SHOWNORMAL);
   }
}

void QWindowsWindow::setStyle(unsigned s) const
{
   setFlag(WithinSetStyle);
   setFlag(FrameDirty);
   SetWindowLongPtr(m_data.hwnd, GWL_STYLE, s);
   clearFlag(WithinSetStyle);
}

void QWindowsWindow::setExStyle(unsigned s) const
{
   setFlag(FrameDirty);
   SetWindowLongPtr(m_data.hwnd, GWL_EXSTYLE, s);
}

void QWindowsWindow::raise()
{
   SetWindowPos(m_data.hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
}

void QWindowsWindow::lower()
{
   if (m_data.hwnd) {
      SetWindowPos(m_data.hwnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
   }
}

void QWindowsWindow::windowEvent(QEvent *event)
{
   switch (event->type()) {
      case QEvent::WindowBlocked: // Blocked by another modal window.
         setEnabled(false);
         setFlag(BlockedByModal);
         if (hasMouseCapture()) {
            ReleaseCapture();
         }
         break;
      case QEvent::WindowUnblocked:
         setEnabled(true);
         clearFlag(BlockedByModal);
         break;
      default:
         break;
   }
}

void QWindowsWindow::propagateSizeHints()
{
}

bool QWindowsWindow::handleGeometryChangingMessage(MSG *message, const QWindow *qWindow, const QMargins &margins)
{
   if (!qWindow->isTopLevel()) {
      // Implement hasHeightForWidth().
      return false;
   }

   WINDOWPOS *windowPos = reinterpret_cast<WINDOWPOS *>(message->lParam);
   if ((windowPos->flags & (SWP_NOCOPYBITS | SWP_NOSIZE))) {
      return false;
   }

   const QRect suggestedFrameGeometry(windowPos->x, windowPos->y,
      windowPos->cx, windowPos->cy);
   const QRect suggestedGeometry = suggestedFrameGeometry - margins;
   const QRectF correctedGeometryF = QPlatformWindow::closestAcceptableGeometry(qWindow, suggestedGeometry);
   if (!correctedGeometryF.isValid()) {
      return false;
   }
   const QRect correctedFrameGeometry = correctedGeometryF.toRect() + margins;
   if (correctedFrameGeometry == suggestedFrameGeometry) {
      return false;
   }
   windowPos->x = correctedFrameGeometry.left();
   windowPos->y = correctedFrameGeometry.top();
   windowPos->cx = correctedFrameGeometry.width();
   windowPos->cy = correctedFrameGeometry.height();

   return true;
}

bool QWindowsWindow::handleGeometryChanging(MSG *message) const
{
   const QMargins margins = window()->isTopLevel() ? frameMargins() : QMargins();
   return QWindowsWindow::handleGeometryChangingMessage(message, window(), margins);
}

QMargins QWindowsWindow::frameMargins() const
{
   // Frames are invalidated by style changes (window state, flags).
   // As they are also required for geometry calculations in resize
   // event sequences, introduce a dirty flag mechanism to be able
   // to cache results.
   if (testFlag(FrameDirty)) {
      // Always skip calculating style-dependent margins for windows claimed to be frameless.
      // This allows users to remove the margins by handling WM_NCCALCSIZE with WS_THICKFRAME set
      // to ensure Areo snap still works (QTBUG-40578).
      m_data.frame = m_data.flags & Qt::FramelessWindowHint
         ? QMargins(0, 0, 0, 0)
         : QWindowsGeometryHint::frame(style(), exStyle());
      clearFlag(FrameDirty);
   }
   return m_data.frame + m_data.customMargins;
}

void QWindowsWindow::setOpacity(qreal level)
{
   if (m_opacity != level) {
      m_opacity = level;

      if (m_data.hwnd)
         setWindowOpacity(m_data.hwnd, m_data.flags,
            window()->format().hasAlpha(), testFlag(OpenGLSurface), level);
   }
}

static inline HRGN createRectRegion(const QRect &r)
{
   return CreateRectRgn(r.left(), r.top(), r.x() + r.width(), r.y() + r.height());
}

static inline void addRectToWinRegion(const QRect &rect, HRGN *winRegion)
{
   if (const HRGN rectRegion = createRectRegion(rect)) {
      HRGN result = CreateRectRgn(0, 0, 0, 0);
      if (CombineRgn(result, *winRegion, rectRegion, RGN_OR)) {
         DeleteObject(*winRegion);
         *winRegion = result;
      }
      DeleteObject(rectRegion);
   }
}

static HRGN qRegionToWinRegion(const QRegion &region)
{
   const QVector<QRect> rects = region.rects();
   if (rects.isEmpty()) {
      return NULL;
   }
   const int rectCount = rects.size();
   if (rectCount == 1) {
      return createRectRegion(region.boundingRect());
   }
   HRGN hRegion = createRectRegion(rects.front());
   for (int i = 1; i < rectCount; ++i) {
      addRectToWinRegion(rects.at(i), &hRegion);
   }
   return hRegion;
}

void QWindowsWindow::setMask(const QRegion &region)
{
   if (region.isEmpty()) {
      SetWindowRgn(m_data.hwnd, 0, true);
      return;
   }
   const HRGN winRegion = qRegionToWinRegion(region);

   // Mask is in client area coordinates, so offset it in case we have a frame
   if (window()->isTopLevel()) {
      const QMargins margins = frameMargins();
      OffsetRgn(winRegion, margins.left(), margins.top());
   }

   // SetWindowRgn takes ownership.
   if (!SetWindowRgn(m_data.hwnd, winRegion, true)) {
      DeleteObject(winRegion);
   }
}

void QWindowsWindow::requestActivateWindow()
{
   // 'Active' state handling is based in focus since it needs to work for child windows as well.
   if (m_data.hwnd) {
      SetForegroundWindow(m_data.hwnd);
      SetFocus(m_data.hwnd);
   }
}

bool QWindowsWindow::setKeyboardGrabEnabled(bool grab)
{
   if (! m_data.hwnd) {
      qWarning("QWindowsWindow::setKeyboardGrabEnabled(): No handle");
      return false;
   }

   QWindowsContext *context = QWindowsContext::instance();

   if (grab) {
      context->setKeyGrabber(window());
   } else {
      if (context->keyGrabber() == window()) {
         context->setKeyGrabber(0);
      }
   }

   return true;
}

bool QWindowsWindow::setMouseGrabEnabled(bool grab)
{
   if (! m_data.hwnd) {
      qWarning("QWindowsWindow::setMouseGrabEnabled(): No handle");
      return false;
   }

   if (! isVisible() && grab) {
      qWarning("QWindowsWindow::setMouseGrabEnabled(): Unable to set mouse grab for invisible window %s/'%s'",
         csPrintable(window()->metaObject()->className()), csPrintable(window()->objectName()));

      return false;
   }

   // release grab or an explicit grab overriding autocapture: Clear flag.
   clearFlag(QWindowsWindow::AutoMouseCapture);
   if (hasMouseCapture() != grab) {
      if (grab) {
         SetCapture(m_data.hwnd);
      } else {
         ReleaseCapture();
      }
   }
   return grab;
}

static inline DWORD cornerToWinOrientation(Qt::Corner corner)
{
   switch (corner) {
      case Qt::TopLeftCorner:
         return 0xf004; // SZ_SIZETOPLEFT;

      case Qt::TopRightCorner:
         return 0xf005; // SZ_SIZETOPRIGHT

      case Qt::BottomLeftCorner:
         return 0xf007; // SZ_SIZEBOTTOMLEFT

      case Qt::BottomRightCorner:
         return 0xf008; // SZ_SIZEBOTTOMRIGHT
   }

   return 0;
}

bool QWindowsWindow::startSystemResize(const QPoint &, Qt::Corner corner)
{
   if (!GetSystemMenu(m_data.hwnd, FALSE)) {
      return false;
   }

   ReleaseCapture();
   PostMessage(m_data.hwnd, WM_SYSCOMMAND, cornerToWinOrientation(corner), 0);
   setFlag(SizeGripOperation);
   return true;
}

void QWindowsWindow::setFrameStrutEventsEnabled(bool enabled)
{
   if (enabled) {
      setFlag(FrameStrutEventsEnabled);
   } else {
      clearFlag(FrameStrutEventsEnabled);
   }
}

void QWindowsWindow::getSizeHints(MINMAXINFO *mmi) const
{
   const QWindowsGeometryHint hint(window(), m_data.customMargins);
   hint.applyToMinMaxInfo(m_data.hwnd, mmi);

   if ((testFlag(WithinMaximize) || (window()->windowState() == Qt::WindowMinimized))
      && (m_data.flags & Qt::FramelessWindowHint)) {
      // This block fixes QTBUG-8361: Frameless windows shouldn't cover the
      // taskbar when maximized
      const QScreen *screen = window()->screen();

      // Documentation of MINMAXINFO states that it will only work for the primary screen
      if (screen && screen == QApplication::primaryScreen()) {
         const QRect availableGeometry = QHighDpi::toNativePixels(screen->availableGeometry(), screen);
         mmi->ptMaxSize.y = availableGeometry.height();

         // Width, because you can have the taskbar on the sides too.
         mmi->ptMaxSize.x = availableGeometry.width();

         // If you have the taskbar on top, or on the left you don't want it at (0,0):
         mmi->ptMaxPosition.x = availableGeometry.x();
         mmi->ptMaxPosition.y = availableGeometry.y();
      } else if (!screen) {
         qWarning() << "window()->screen() returned a null screen";
      }
   }
}

bool QWindowsWindow::handleNonClientHitTest(const QPoint &globalPos, LRESULT *result) const
{
   // QTBUG-32663, suppress resize cursor for fixed size windows.
   const QWindow *w = window();
   if (!w->isTopLevel() // Task 105852, minimized windows need to respond to user input.
      || (m_windowState != Qt::WindowNoState && m_windowState != Qt::WindowActive)
      || (m_data.flags & Qt::FramelessWindowHint)) {
      return false;
   }
   const QSize minimumSize = w->minimumSize();
   if (minimumSize.isEmpty()) {
      return false;
   }
   const QSize maximumSize = w->maximumSize();
   const bool fixedWidth = minimumSize.width() == maximumSize.width();
   const bool fixedHeight = minimumSize.height() == maximumSize.height();
   if (!fixedWidth && !fixedHeight) {
      return false;
   }
   const QPoint localPos = w->mapFromGlobal(QHighDpi::fromNativePixels(globalPos, w));
   const QSize size = w->size();
   if (fixedHeight) {
      if (localPos.y() >= size.height()) {
         *result = HTBORDER; // Unspecified border, no resize cursor.
         return true;
      }
      if (localPos.y() < 0) {
         const QMargins margins = frameMargins();
         const int topResizeBarPos = margins.left() - margins.top();
         if (localPos.y() < topResizeBarPos) {
            *result = HTCAPTION; // Extend caption over top resize bar, let's user move the window.
            return true;
         }
      }
   }
   if (fixedWidth && (localPos.x() < 0 || localPos.x() >= size.width())) {
      *result = HTBORDER; // Unspecified border, no resize cursor.
      return true;
   }
   return false;
}

#ifndef QT_NO_CURSOR

// Return the default cursor (Arrow) from QWindowsCursor's cache.
static inline CursorHandlePtr defaultCursor(const QWindow *w)
{
   if (QScreen *screen = w->screen())
      if (const QPlatformScreen *platformScreen = screen->handle())
         if (QPlatformCursor *cursor = platformScreen->cursor()) {
            return static_cast<QWindowsCursor *>(cursor)->standardWindowCursor(Qt::ArrowCursor);
         }
   return CursorHandlePtr(new CursorHandle(QWindowsCursor::createCursorFromShape(Qt::ArrowCursor)));
}

// Check whether to apply a new cursor. Either the window in question is
// currently under mouse, or it is the parent of the window under mouse and
// there is no other window with an explicitly set cursor in-between.
static inline bool applyNewCursor(const QWindow *w)
{
   const QWindow *underMouse = QWindowsContext::instance()->windowUnderMouse();
   if (underMouse == w) {
      return true;
   }
   for (const QWindow *p = underMouse; p ; p = p->parent()) {
      if (p == w) {
         return true;
      }
      if (!QWindowsWindow::baseWindowOf(p)->cursor()->isNull()) {
         return false;
      }
   }
   return false;
}
#endif // !QT_NO_CURSOR

/*!
    \brief Applies to cursor property set on the window to the global cursor.

    \sa QWindowsCursor
*/

void QWindowsWindow::applyCursor()
{
#ifndef QT_NO_CURSOR
   if (m_cursor->isNull()) { // Recurse up to parent with non-null cursor. Set default for toplevel.
      if (const QWindow *p = window()->parent()) {
         QWindowsWindow::baseWindowOf(p)->applyCursor();
      } else {
         SetCursor(defaultCursor(window())->handle());
      }
   } else {
      SetCursor(m_cursor->handle());
   }
#endif
}

void QWindowsWindow::setCursor(const CursorHandlePtr &c)
{
#ifndef QT_NO_CURSOR
   if (c->handle() != m_cursor->handle()) {
      const bool apply = applyNewCursor(window());

      m_cursor = c;
      if (apply) {
         applyCursor();
      }
   }
#endif
}

void QWindowsWindow::setAlertState(bool enabled)
{
   if (isAlertState() == enabled) {
      return;
   }
   if (enabled) {
      alertWindow(0);
      setFlag(AlertState);
   } else {
      stopAlertWindow();
      clearFlag(AlertState);
   }
}

void QWindowsWindow::alertWindow(int durationMs)
{
   DWORD timeOutMs = GetCaretBlinkTime();
   if (!timeOutMs || timeOutMs == INFINITE) {
      timeOutMs = 250;
   }

   FLASHWINFO info;
   info.cbSize = sizeof(info);
   info.hwnd = m_data.hwnd;
   info.dwFlags = FLASHW_TRAY;
   info.dwTimeout = timeOutMs;
   info.uCount = durationMs == 0 ? 10 : durationMs / timeOutMs;
   FlashWindowEx(&info);
}

void QWindowsWindow::stopAlertWindow()
{
   FLASHWINFO info;
   info.cbSize = sizeof(info);
   info.hwnd = m_data.hwnd;
   info.dwFlags = FLASHW_STOP;
   info.dwTimeout = 0;
   info.uCount = 0;
   FlashWindowEx(&info);
}

bool QWindowsWindow::isEnabled() const
{
   return (style() & WS_DISABLED) == 0;
}

void QWindowsWindow::setEnabled(bool enabled)
{
   const unsigned oldStyle = style();
   unsigned newStyle = oldStyle;
   if (enabled) {
      newStyle &= ~WS_DISABLED;
   } else {
      newStyle |= WS_DISABLED;
   }
   if (newStyle != oldStyle) {
      setStyle(newStyle);
   }
}

static HICON createHIcon(const QIcon &icon, int xSize, int ySize)
{
   if (!icon.isNull()) {
      const QPixmap pm = icon.pixmap(icon.actualSize(QSize(xSize, ySize)));
      if (!pm.isNull()) {
         return qt_pixmapToWinHICON(pm);
      }
   }
   return 0;
}

void QWindowsWindow::setWindowIcon(const QIcon &icon)
{
   if (m_data.hwnd) {
      destroyIcon();

      m_iconSmall = createHIcon(icon, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON));
      m_iconBig = createHIcon(icon, GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON));

      if (m_iconBig) {
         SendMessage(m_data.hwnd, WM_SETICON, 0 /* ICON_SMALL */, (LPARAM)m_iconSmall);
         SendMessage(m_data.hwnd, WM_SETICON, 1 /* ICON_BIG */, (LPARAM)m_iconBig);
      } else {
         SendMessage(m_data.hwnd, WM_SETICON, 0 /* ICON_SMALL */, (LPARAM)m_iconSmall);
         SendMessage(m_data.hwnd, WM_SETICON, 1 /* ICON_BIG */, (LPARAM)m_iconSmall);
      }
   }
}

/*!
    \brief Sets custom margins to be added to the default margins determined by
    the windows style in the handling of the WM_NCCALCSIZE message.

    This is currently used to give the Aero-style QWizard a smaller top margin.
    The property can be set using QPlatformNativeInterface::setWindowProperty() or,
    before platform window creation, by setting a dynamic property
    on the QWindow (see QWindowsIntegration::createPlatformWindow()).
*/

void QWindowsWindow::setCustomMargins(const QMargins &newCustomMargins)
{
   if (newCustomMargins != m_data.customMargins) {
      const QMargins oldCustomMargins = m_data.customMargins;
      m_data.customMargins = newCustomMargins;

      // Re-trigger WM_NCALCSIZE with wParam=1 by passing SWP_FRAMECHANGED
      const QRect currentFrameGeometry = frameGeometry_sys();
      const QPoint topLeft = currentFrameGeometry.topLeft();

      QRect newFrame = currentFrameGeometry.marginsRemoved(oldCustomMargins) + m_data.customMargins;
      newFrame.moveTo(topLeft);
      setFlag(FrameDirty);

      SetWindowPos(m_data.hwnd, 0, newFrame.x(), newFrame.y(), newFrame.width(), newFrame.height(),
                  SWP_NOZORDER | SWP_FRAMECHANGED);
   }
}

void *QWindowsWindow::surface(void *nativeConfig, int *err)
{
#ifdef QT_NO_OPENGL
   return 0;
#else
   if (!m_surface) {
      if (QWindowsStaticOpenGLContext *staticOpenGLContext = QWindowsIntegration::staticOpenGLContext()) {
         m_surface = staticOpenGLContext->createWindowSurface(m_data.hwnd, nativeConfig, err);
      }
   }

   return m_surface;
#endif
}

void QWindowsWindow::invalidateSurface()
{
#ifndef QT_NO_OPENGL
   if (m_surface) {
      if (QWindowsStaticOpenGLContext *staticOpenGLContext = QWindowsIntegration::staticOpenGLContext()) {
         staticOpenGLContext->destroyWindowSurface(m_surface);
      }
      m_surface = 0;
   }
#endif
}

void QWindowsWindow::setTouchWindowTouchTypeStatic(QWindow *window, QWindowsWindowFunctions::TouchWindowTouchTypes touchTypes)
{
   if (!window->handle()) {
      return;
   }
   static_cast<QWindowsWindow *>(window->handle())->registerTouchWindow(touchTypes);
}

void QWindowsWindow::registerTouchWindow(QWindowsWindowFunctions::TouchWindowTouchTypes touchTypes)
{
   if ((QWindowsContext::instance()->systemInfo() & QWindowsContext::SI_SupportsTouch)
      && window()->type() != Qt::ForeignWindow) {
      ULONG touchFlags = 0;
      const bool ret = QWindowsContext::user32dll.isTouchWindow(m_data.hwnd, &touchFlags);
      // Return if it is not a touch window or the flags are already set by a hook
      // such as HCBT_CREATEWND
      if (ret || touchFlags != 0) {
         return;
      }
      if (QWindowsContext::user32dll.registerTouchWindow(m_data.hwnd, (ULONG)touchTypes)) {
         setFlag(TouchRegistered);
      } else {
         qErrnoWarning("RegisterTouchWindow() failed for window '%s'.", qPrintable(window()->objectName()));
      }
   }
}

void QWindowsWindow::aboutToMakeCurrent()
{
#ifndef QT_NO_OPENGL
   // For RasterGLSurface windows, that become OpenGL windows dynamically, it might be
   // time to set up some GL specifics.  This is particularly important for layered
   // windows (WS_EX_LAYERED due to alpha > 0).
   const bool isCompositing = qt_window_private(window())->compositing;
   if (isCompositing != testFlag(Compositing)) {
      if (isCompositing) {
         setFlag(Compositing);
      } else {
         clearFlag(Compositing);
      }

      updateGLWindowSettings(window(), m_data.hwnd, m_data.flags, m_opacity);
   }
#endif
}

void QWindowsWindow::setHasBorderInFullScreenStatic(QWindow *window, bool border)
{
   if (QPlatformWindow *handle = window->handle()) {
      static_cast<QWindowsWindow *>(handle)->setHasBorderInFullScreen(border);
   } else {
      qWarning("%s invoked without window handle; call has no effect.", Q_FUNC_INFO);
   }
}

void QWindowsWindow::setHasBorderInFullScreen(bool border)
{
   if (border) {
      setFlag(HasBorderInFullScreen);
   } else {
      clearFlag(HasBorderInFullScreen);
   }
}

