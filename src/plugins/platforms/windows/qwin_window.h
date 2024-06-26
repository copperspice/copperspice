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

#ifndef QWINDOWSWINDOW_H
#define QWINDOWSWINDOW_H

#include <qwin_additional.h>
#include <qwin_cursor.h>

#include <qplatform_window.h>
#include <platformheaders/qwindowswindowfunctions.h>

class QWindowsOleDropTarget;
class QDebug;

struct QWindowsGeometryHint {
   QWindowsGeometryHint()
   {
   }

   explicit QWindowsGeometryHint(const QWindow *w, const QMargins &customMargins);
   static QMargins frame(DWORD style, DWORD exStyle);
   static bool handleCalculateSize(const QMargins &customMargins, const MSG &msg, LRESULT *result);

   void applyToMinMaxInfo(DWORD style, DWORD exStyle, MINMAXINFO *mmi) const;
   void applyToMinMaxInfo(HWND hwnd, MINMAXINFO *mmi) const;

   bool validSize(const QSize &s) const;

   static inline QPoint mapToGlobal(HWND hwnd, const QPoint &);
   static inline QPoint mapToGlobal(const QWindow *w, const QPoint &);
   static inline QPoint mapFromGlobal(const HWND hwnd, const QPoint &);
   static inline QPoint mapFromGlobal(const QWindow *w, const QPoint &);

   static bool positionIncludesFrame(const QWindow *w);

   QSize minimumSize;
   QSize maximumSize;
   QMargins customMargins;
};

struct QWindowCreationContext {
   QWindowCreationContext(const QWindow *w, const QRect &r,
      const QMargins &customMargins,
      DWORD style, DWORD exStyle);

   void applyToMinMaxInfo(MINMAXINFO *mmi) const {
      geometryHint.applyToMinMaxInfo(style, exStyle, mmi);
   }

   QWindowsGeometryHint geometryHint;
   const QWindow *window;
   DWORD style;
   DWORD exStyle;
   QRect requestedGeometry;
   QRect obtainedGeometry;
   QMargins margins;
   QMargins customMargins;  // User-defined, additional frame for WM_NCCALCSIZE
   int frameX; // Passed on to CreateWindowEx(), including frame.
   int frameY;
   int frameWidth;
   int frameHeight;
};

struct QWindowsWindowData {
   QWindowsWindowData()
      : hwnd(nullptr), embedded(false)
   {
   }

   Qt::WindowFlags flags;
   QRect geometry;
   QMargins frame;             // Do not use directly for windows, see FrameDirty.
   QMargins customMargins;     // User-defined, additional frame for NCCALCSIZE
   HWND hwnd;
   bool embedded;

   static QWindowsWindowData create(const QWindow *w,
      const QWindowsWindowData &parameters,
      const QString &title);
};

class QWindowsWindow : public QPlatformWindow
{
 public:
   enum Flags {
      AutoMouseCapture = 0x1, //! Automatic mouse capture on button press.
      WithinSetParent = 0x2,
      FrameDirty = 0x4,            //! Frame outdated by setStyle, recalculate in next query.
      OpenGLSurface = 0x10,
      OpenGL_ES2 = 0x20,
      OpenGLDoubleBuffered = 0x40,
      OpenGlPixelFormatInitialized = 0x80,
      BlockedByModal = 0x100,
      SizeGripOperation = 0x200,
      FrameStrutEventsEnabled = 0x400,
      SynchronousGeometryChangeEvent = 0x800,
      WithinSetStyle = 0x1000,
      WithinDestroy = 0x2000,
      TouchRegistered = 0x4000,
      AlertState = 0x8000,
      Exposed = 0x10000,
      WithinCreate = 0x20000,
      WithinMaximize = 0x40000,
      MaximizeToFullScreen = 0x80000,
      InputMethodDisabled = 0x100000,
      Compositing = 0x200000,
      HasBorderInFullScreen = 0x400000
   };

   QWindowsWindow(QWindow *window, const QWindowsWindowData &data);
   ~QWindowsWindow();

   using QPlatformWindow::screenForGeometry;

   QSurfaceFormat format() const override {
      return m_format;
   }

   void setGeometry(const QRect &rect) override;
   QRect geometry() const override {
      return m_data.geometry;
   }

   QRect normalGeometry() const override;

   void setVisible(bool visible) override;
   bool isVisible() const;

   bool isExposed() const override {
      return testFlag(Exposed);
   }

   bool isActive() const override;
   bool isEmbedded(const QPlatformWindow *parentWindow = nullptr) const override;
   QPoint mapToGlobal(const QPoint &pos) const override;
   QPoint mapFromGlobal(const QPoint &pos) const override;

   void setWindowFlags(Qt::WindowFlags flags) override;
   void setWindowState(Qt::WindowState state) override;

   HWND handle() const {
      return m_data.hwnd;
   }

   WId winId() const override {
      return WId(m_data.hwnd);
   }

   void *nativeHandle() override {
      return &m_data.hwnd;
   }

   void setParent(const QPlatformWindow *window) override;

   void setWindowTitle(const QString &title) override;
   void raise() override;
   void lower() override;

   void windowEvent(QEvent *event) override;

   void propagateSizeHints() override;
   static bool handleGeometryChangingMessage(MSG *message, const QWindow *qWindow, const QMargins &marginsDp);
   bool handleGeometryChanging(MSG *message) const;
   QMargins frameMargins() const override;

   void setOpacity(qreal level) override;
   void setMask(const QRegion &region) override;
   qreal opacity() const {
      return m_opacity;
   }
   void requestActivateWindow() override;

   bool setKeyboardGrabEnabled(bool grab) override;
   bool setMouseGrabEnabled(bool grab) override;
   inline bool hasMouseCapture() const {
      return GetCapture() == m_data.hwnd;
   }

   bool startSystemResize(const QPoint &pos, Qt::Corner corner) override;

   void setFrameStrutEventsEnabled(bool enabled) override;
   bool frameStrutEventsEnabled() const override {
      return testFlag(FrameStrutEventsEnabled);
   }

   QMargins customMargins() const {
      return m_data.customMargins;
   }
   void setCustomMargins(const QMargins &m);

   inline unsigned style() const {
      return GetWindowLongPtr(m_data.hwnd, GWL_STYLE);
   }
   void setStyle(unsigned s) const;

   inline unsigned exStyle() const {
      return GetWindowLongPtr(m_data.hwnd, GWL_EXSTYLE);
   }
   void setExStyle(unsigned s) const;

   bool handleWmPaint(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

   void handleMoved();
   void handleResized(int wParam);
   void handleHidden();
   void handleCompositionSettingsChanged();

   static inline HWND handleOf(const QWindow *w);
   static inline QWindowsWindow *baseWindowOf(const QWindow *w);
   static QWindow *topLevelOf(QWindow *w);
   static inline void *userDataOf(HWND hwnd);
   static inline void setUserDataOf(HWND hwnd, void *ud);

   static bool setWindowLayered(HWND hwnd, Qt::WindowFlags flags, bool hasAlpha, qreal opacity);
   bool isLayered() const;

   HDC getDC();
   void releaseDC();

   void getSizeHints(MINMAXINFO *mmi) const;
   bool handleNonClientHitTest(const QPoint &globalPos, LRESULT *result) const;

#ifndef QT_NO_CURSOR
   CursorHandlePtr cursor() const {
      return m_cursor;
   }
#endif

   void setCursor(const CursorHandlePtr &c);
   void applyCursor();

   inline bool testFlag(unsigned f) const  {
      return (m_flags & f) != 0;
   }
   inline void setFlag(unsigned f) const   {
      m_flags |= f;
   }
   inline void clearFlag(unsigned f) const {
      m_flags &= ~f;
   }

   void setEnabled(bool enabled);
   bool isEnabled() const;
   void setWindowIcon(const QIcon &icon) override;

   void *surface(void *nativeConfig, int *err);
   void invalidateSurface() override;
   void aboutToMakeCurrent();

   void setAlertState(bool enabled) override;
   bool isAlertState() const override {
      return testFlag(AlertState);
   }
   void alertWindow(int durationMs = 0);
   void stopAlertWindow();

   static void setTouchWindowTouchTypeStatic(QWindow *window, QWindowsWindowFunctions::TouchWindowTouchTypes touchTypes);
   void registerTouchWindow(QWindowsWindowFunctions::TouchWindowTouchTypes touchTypes = QWindowsWindowFunctions::NormalTouch);
   static void setHasBorderInFullScreenStatic(QWindow *window, bool border);
   void setHasBorderInFullScreen(bool border);

 private:
   inline void show_sys() const;
   inline void hide_sys() const;
   inline void setGeometry_sys(const QRect &rect) const;
   inline QRect frameGeometry_sys() const;
   inline QRect geometry_sys() const;
   inline QWindowsWindowData setWindowFlags_sys(Qt::WindowFlags wt, unsigned flags = 0) const;
   inline bool isFullScreen_sys() const;
   inline void setWindowState_sys(Qt::WindowState newState);
   inline void setParent_sys(const QPlatformWindow *parent);
   inline void updateTransientParent() const;
   void destroyWindow();

   inline bool isDropSiteEnabled() const {
      return m_dropTarget != nullptr;
   }
   void setDropSiteEnabled(bool enabled);
   void updateDropSite(bool topLevel);
   void handleGeometryChange();
   void handleWindowStateChange(Qt::WindowState state);
   inline void destroyIcon();
   void fireExpose(const QRegion &region, bool force = false);

   mutable QWindowsWindowData m_data;
   mutable unsigned m_flags;
   HDC m_hdc;
   Qt::WindowState m_windowState;
   qreal m_opacity;

#ifndef QT_NO_CURSOR
   CursorHandlePtr m_cursor;
#endif

   QWindowsOleDropTarget *m_dropTarget;
   unsigned m_savedStyle;
   QRect m_savedFrameGeometry;
   const QSurfaceFormat m_format;

   HICON m_iconSmall;
   HICON m_iconBig;
   void *m_surface;
};

QDebug operator<<(QDebug debug, const RECT &r);
QDebug operator<<(QDebug debug, const POINT &);

QDebug operator<<(QDebug debug, const MINMAXINFO &i);
QDebug operator<<(QDebug debug, const NCCALCSIZE_PARAMS &p);
QDebug operator<<(QDebug debug, const WINDOWPLACEMENT &);

// ---------- QWindowsGeometryHint inline functions.
QPoint QWindowsGeometryHint::mapToGlobal(HWND hwnd, const QPoint &qp)
{
   POINT p = { qp.x(), qp.y() };
   ClientToScreen(hwnd, &p);

   return QPoint(p.x, p.y);
}

QPoint QWindowsGeometryHint::mapFromGlobal(const HWND hwnd, const QPoint &qp)
{
   POINT p = { qp.x(), qp.y() };
   ScreenToClient(hwnd, &p);

   return QPoint(p.x, p.y);
}

QPoint QWindowsGeometryHint::mapToGlobal(const QWindow *w, const QPoint &p)
{
   return QWindowsGeometryHint::mapToGlobal(QWindowsWindow::handleOf(w), p);
}

QPoint QWindowsGeometryHint::mapFromGlobal(const QWindow *w, const QPoint &p)
{
   return QWindowsGeometryHint::mapFromGlobal(QWindowsWindow::handleOf(w), p);
}


// ---------- QWindowsBaseWindow inline functions.

QWindowsWindow *QWindowsWindow::baseWindowOf(const QWindow *w)
{
   if (w) {
      if (QPlatformWindow *pw = w->handle()) {
         return static_cast<QWindowsWindow *>(pw);
      }
   }

   return nullptr;
}

HWND QWindowsWindow::handleOf(const QWindow *w)
{
   if (const QWindowsWindow *bw = QWindowsWindow::baseWindowOf(w)) {
      return bw->handle();
   }

   return nullptr;
}

void *QWindowsWindow::userDataOf(HWND hwnd)
{
   return (void *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
}

void QWindowsWindow::setUserDataOf(HWND hwnd, void *ud)
{
   SetWindowLongPtr(hwnd, GWLP_USERDATA, LONG_PTR(ud));
}

inline void QWindowsWindow::destroyIcon()
{
   if (m_iconBig) {
      DestroyIcon(m_iconBig);
      m_iconBig = nullptr;
   }

   if (m_iconSmall) {
      DestroyIcon(m_iconSmall);
      m_iconSmall = nullptr;
   }
}

inline bool QWindowsWindow::isLayered() const
{
   return GetWindowLongPtr(m_data.hwnd, GWL_EXSTYLE) & WS_EX_LAYERED;
}

#endif
