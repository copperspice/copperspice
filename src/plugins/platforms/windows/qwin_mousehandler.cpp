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

#include <qwin_mousehandler.h>
#include <qwin_keymapper.h>
#include <qwin_context.h>
#include <qwin_window.h>
#include <qwin_integration.h>
#include <qwin_screen.h>

#include <qwindowsysteminterface.h>
#include <qapplication.h>
#include <qscreen.h>
#include <qwindow.h>
#include <qcursor.h>
#include <qdebug.h>
#include <qscopedarraypointer.h>

#include <windowsx.h>

static inline void compressMouseMove(MSG *msg)
{
   // Compress mouse move events
   if (msg->message == WM_MOUSEMOVE) {
      MSG mouseMsg;

      while (PeekMessage(&mouseMsg, msg->hwnd, WM_MOUSEFIRST, WM_MOUSELAST, PM_NOREMOVE)) {

         if (mouseMsg.message == WM_MOUSEMOVE) {

#define PEEKMESSAGE_IS_BROKEN 1
#ifdef PEEKMESSAGE_IS_BROKEN

            // Since the Windows PeekMessage() function doesn't
            // correctly return the wParam for WM_MOUSEMOVE events
            // if there is a key release event in the queue
            // _before_ the mouse event, we have to also consider
            // key release events (kls 2003-05-13):
            MSG keyMsg;
            bool done = false;
            while (PeekMessage(&keyMsg, nullptr, WM_KEYFIRST, WM_KEYLAST, PM_NOREMOVE)) {

               if (keyMsg.time < mouseMsg.time) {
                  if ((keyMsg.lParam & 0xC0000000) == 0x40000000) {
                     PeekMessage(&keyMsg, nullptr, keyMsg.message, keyMsg.message, PM_REMOVE);
                  } else {
                     done = true;
                     break;
                  }

               } else {
                  break; // no key event before the WM_MOUSEMOVE event
               }
            }

            if (done) {
               break;
            }
#else
            // Actually the following 'if' should work instead of
            // the above key event checking, but apparently
            // PeekMessage() is broken :-(
            if (mouseMsg.wParam != msg.wParam) {
               break;   // leave the message in the queue because
            }

            // the key state has changed
#endif
            // Update the passed in MSG structure with the
            // most recent one.
            msg->lParam = mouseMsg.lParam;
            msg->wParam = mouseMsg.wParam;

            // Extract the x,y coordinates from the lParam as we do in the WndProc
            msg->pt.x = GET_X_LPARAM(mouseMsg.lParam);
            msg->pt.y = GET_Y_LPARAM(mouseMsg.lParam);
            ClientToScreen(msg->hwnd, &(msg->pt));

            // Remove the mouse move message
            PeekMessage(&mouseMsg, msg->hwnd, WM_MOUSEMOVE, WM_MOUSEMOVE, PM_REMOVE);
         } else {
            break; // there was no more WM_MOUSEMOVE event
         }
      }
   }
}

static inline QTouchDevice *createTouchDevice()
{
   enum {
      QT_SM_TABLETPC          = 86,
      QT_SM_DIGITIZER         = 94,
      QT_SM_MAXIMUMTOUCHES    = 95,
      QT_NID_INTEGRATED_TOUCH = 0x1,
      QT_NID_EXTERNAL_TOUCH   = 0x02,
      QT_NID_MULTI_INPUT      = 0x40,
      QT_NID_READY            = 0x80
   };

   if (QSysInfo::windowsVersion() < QSysInfo::WV_WINDOWS7) {
      return nullptr;
   }

   const int digitizers = GetSystemMetrics(QT_SM_DIGITIZER);

   if (! (digitizers & (QT_NID_INTEGRATED_TOUCH | QT_NID_EXTERNAL_TOUCH))) {
      return nullptr;
   }

   const int maxTouchPoints = GetSystemMetrics(QT_SM_MAXIMUMTOUCHES);

#if defined(CS_SHOW_DEBUG_PLATFORM)
   const int tabletPc = GetSystemMetrics(QT_SM_TABLETPC);

   qDebug() << "createTouchDevice() Digitizers =" << hex << showbase << (digitizers & ~QT_NID_READY) << "\n  "
      << "Ready =" << (digitizers & QT_NID_READY) << dec << noshowbase
      << "Tablet PC =" << tabletPc << "Max touch points =" << maxTouchPoints;
#endif

   QTouchDevice *result = new QTouchDevice;
   result->setType(digitizers & QT_NID_INTEGRATED_TOUCH
      ? QTouchDevice::TouchScreen : QTouchDevice::TouchPad);

   QTouchDevice::Capabilities capabilities = QTouchDevice::Position |
         QTouchDevice::Area | QTouchDevice::NormalizedPosition;

   if (result->type() == QTouchDevice::TouchPad) {
      capabilities |= QTouchDevice::MouseEmulation;
   }

   result->setCapabilities(capabilities);
   result->setMaximumTouchPoints(maxTouchPoints);

   return result;
}

// internal
QWindowsMouseHandler::QWindowsMouseHandler()
   : m_windowUnderMouse(nullptr), m_trackedWindow(nullptr), m_touchDevice(nullptr),
     m_leftButtonDown(false), m_previousCaptureWindow(nullptr)
{
}

QTouchDevice *QWindowsMouseHandler::ensureTouchDevice()
{
   if (! m_touchDevice) {
      m_touchDevice = createTouchDevice();
   }

   return m_touchDevice;
}

Qt::MouseButtons QWindowsMouseHandler::queryMouseButtons()
{
   Qt::MouseButtons result = Qt::EmptyFlag;
   const bool mouseSwapped = GetSystemMetrics(SM_SWAPBUTTON);

   if (GetAsyncKeyState(VK_LBUTTON) < 0) {
      result |= mouseSwapped ? Qt::RightButton : Qt::LeftButton;
   }

   if (GetAsyncKeyState(VK_RBUTTON) < 0) {
      result |= mouseSwapped ? Qt::LeftButton : Qt::RightButton;
   }

   if (GetAsyncKeyState(VK_MBUTTON) < 0) {
      result |= Qt::MiddleButton;
   }

   if (GetAsyncKeyState(VK_XBUTTON1) < 0) {
      result |= Qt::XButton1;
   }

   if (GetAsyncKeyState(VK_XBUTTON2) < 0) {
      result |= Qt::XButton2;
   }

   return result;
}

bool QWindowsMouseHandler::translateMouseEvent(QWindow *window, HWND hwnd,
      QtWindows::WindowsEventType et, MSG msg, LRESULT *result)
{
   static constexpr const quint64 signatureMask = 0xffffff00;
   static constexpr const quint64 miWpSignature = 0xff515700;

   if (et == QtWindows::MouseWheelEvent) {
      return translateMouseWheelEvent(window, hwnd, msg, result);
   }

   Qt::MouseEventSource source = Qt::MouseEventNotSynthesized;

   // Check for events synthesized from touch. Lower byte is touch index, 0 means pen.
   static const bool passSynthesizedMouseEvents =
      !(QWindowsIntegration::instance()->options() & QWindowsIntegration::DontPassOsMouseEventsSynthesizedFromTouch);

   // Check for events synthesized from touch. Lower 7 bits are touch/pen index, bit 8 indicates touch.
   // However, when tablet support is active, extraInfo is a packet serial number. This is not a problem
   // since we do not want to ignore mouse events coming from a tablet.
   const quint64 extraInfo = quint64(GetMessageExtraInfo());

   if ((extraInfo & signatureMask) == miWpSignature) {
      if (extraInfo & 0x80) {
         // Bit 7 indicates touch event, else tablet pen.
         source = Qt::MouseEventSynthesizedBySystem;

         if (! passSynthesizedMouseEvents) {
            return false;
         }
      }
   }

   const QPoint winEventPosition(GET_X_LPARAM(msg.lParam), GET_Y_LPARAM(msg.lParam));

   if (et & QtWindows::NonClientEventFlag) {
      const QPoint globalPosition = winEventPosition;
      const QPoint clientPosition = QWindowsGeometryHint::mapFromGlobal(hwnd, globalPosition);
      const Qt::MouseButtons buttons = QWindowsMouseHandler::queryMouseButtons();

      QWindowSystemInterface::handleFrameStrutMouseEvent(window, clientPosition,
         globalPosition, buttons, QWindowsKeyMapper::queryKeyboardModifiers(), source);

      return false; // Allow further event processing (dragging of windows).
   }

   *result = 0;
   if (msg.message == WM_MOUSELEAVE) {
      // When moving out of a window, WM_MOUSEMOVE within the moved-to window is received first,
      // so if m_trackedWindow is not the window here, it means the cursor has left the application.

      if (window == m_trackedWindow) {
         QWindow *leaveTarget = m_windowUnderMouse ? m_windowUnderMouse : m_trackedWindow;
         QWindowSystemInterface::handleLeaveEvent(leaveTarget);
         m_trackedWindow    = nullptr;
         m_windowUnderMouse = nullptr;
      }

      return true;
   }

   QWindowsWindow *platformWindow = static_cast<QWindowsWindow *>(window->handle());
   const Qt::MouseButtons buttons = keyStateToMouseButtons(int(msg.wParam));

   // If the window was recently resized via mouse doubleclick on the frame or title bar,
   // we don't get WM_LBUTTONDOWN or WM_LBUTTONDBLCLK for the second click,
   // but we will get at least one WM_MOUSEMOVE with left button down and the WM_LBUTTONUP,
   // which will result undesired mouse press and release events.
   // To avoid those, we ignore any events with left button down if we didn't
   // get the original WM_LBUTTONDOWN/WM_LBUTTONDBLCLK.

   if (msg.message == WM_LBUTTONDOWN || msg.message == WM_LBUTTONDBLCLK) {
      m_leftButtonDown = true;
   } else {
      const bool actualLeftDown = buttons & Qt::LeftButton;
      if (!m_leftButtonDown && actualLeftDown) {
         // Autocapture the mouse for current window to and ignore further events until release.
         // Capture is necessary so we don't get WM_MOUSELEAVEs to confuse matters.
         // This autocapture is released normally when button is released.

         if (!platformWindow->hasMouseCapture()) {
            QWindowsWindow::baseWindowOf(window)->applyCursor();
            platformWindow->setMouseGrabEnabled(true);
            platformWindow->setFlag(QWindowsWindow::AutoMouseCapture);
         }

         m_previousCaptureWindow = window;
         return true;

      } else if (m_leftButtonDown && !actualLeftDown) {
         m_leftButtonDown = false;
      }
   }

   const QPoint globalPosition = QWindowsGeometryHint::mapToGlobal(hwnd, winEventPosition);
   // In this context, neither an invisible nor a transparent window (transparent regarding mouse
   // events, "click-through") can be considered as the window under mouse.
   QWindow *currentWindowUnderMouse = platformWindow->hasMouseCapture() ?
      QWindowsScreen::windowAt(globalPosition, CWP_SKIPINVISIBLE | CWP_SKIPTRANSPARENT) : window;

   // QTBUG-44332: When running at low integrity level and a Window is parented on a Window
   // of a higher integrity process, using QWindow::fromWinId() (for example in a browser plugin)
   // ChildWindowFromPointEx() may not find the window (failing with ERROR_ACCESS_DENIED)

   if (!currentWindowUnderMouse) {
      const QRect clientRect(QPoint(0, 0), window->size());
      if (clientRect.contains(winEventPosition)) {
         currentWindowUnderMouse = window;
      }
   }

   compressMouseMove(&msg);

   // expects the platform plugin to capture the mouse on any button press until release.

   if (!platformWindow->hasMouseCapture()
      && (msg.message == WM_LBUTTONDOWN || msg.message == WM_MBUTTONDOWN
         || msg.message == WM_RBUTTONDOWN || msg.message == WM_XBUTTONDOWN
         || msg.message == WM_LBUTTONDBLCLK || msg.message == WM_MBUTTONDBLCLK
         || msg.message == WM_RBUTTONDBLCLK || msg.message == WM_XBUTTONDBLCLK)) {
      platformWindow->setMouseGrabEnabled(true);
      platformWindow->setFlag(QWindowsWindow::AutoMouseCapture);

      // Implement "Click to focus" for native child windows (unless it is a native widget window).
      if (!window->isTopLevel() && !window->inherits("QWidgetWindow") && QApplication::focusWindow() != window) {
         window->requestActivate();
      }
   } else if (platformWindow->hasMouseCapture()
      && platformWindow->testFlag(QWindowsWindow::AutoMouseCapture)
      && (msg.message == WM_LBUTTONUP || msg.message == WM_MBUTTONUP
         || msg.message == WM_RBUTTONUP || msg.message == WM_XBUTTONUP) && ! buttons) {
      platformWindow->setMouseGrabEnabled(false);
   }

   const bool hasCapture = platformWindow->hasMouseCapture();
   const bool currentNotCapturing = hasCapture && currentWindowUnderMouse != window;

   // Enter new window: track to generate leave event.
   // If there is an active capture, only track if the current window is capturing,
   // so we don't get extra leave when cursor leaves the application.
   if (window != m_trackedWindow && !currentNotCapturing) {
      TRACKMOUSEEVENT tme;
      tme.cbSize = sizeof(TRACKMOUSEEVENT);
      tme.dwFlags = TME_LEAVE;
      tme.hwndTrack = hwnd;
      tme.dwHoverTime = HOVER_DEFAULT;

      if (!TrackMouseEvent(&tme)) {
         qWarning("QWindowsMouseHandler::translateMouseEvent() TrackMouseEvent failed");
      }
      m_trackedWindow =  window;
   }

   // No enter or leave events are sent as long as there is an autocapturing window.
   if (!hasCapture || !platformWindow->testFlag(QWindowsWindow::AutoMouseCapture)) {
      // Leave is needed if:
      // 1) There is no capture and we move from a window to another window.
      //    Note: Leaving the application entirely is handled in WM_MOUSELEAVE case.
      // 2) There is capture and we move out of the capturing window.
      // 3) There is a new capture and we were over another window.

      if ((m_windowUnderMouse && m_windowUnderMouse != currentWindowUnderMouse
            && (!hasCapture || window == m_windowUnderMouse))
            || (hasCapture && m_previousCaptureWindow != window && m_windowUnderMouse
            && m_windowUnderMouse != window)) {

         QWindowSystemInterface::handleLeaveEvent(m_windowUnderMouse);
         if (currentNotCapturing) {
            // Clear tracking if capturing and current window is not the capturing window
            // to avoid leave when mouse actually leaves the application.
            m_trackedWindow = nullptr;

            // We are not officially in any window, but we need to set some cursor to clear
            // whatever cursor the left window had, so apply the cursor of the capture window.
            QWindowsWindow::baseWindowOf(window)->applyCursor();
         }
      }

      // Enter is needed if:
      // 1) There is no capture and we move to a new window.
      // 2) There is capture and we move into the capturing window.
      // 3) The capture just ended and we are over non-capturing window.
      if ((currentWindowUnderMouse && m_windowUnderMouse != currentWindowUnderMouse
            && (!hasCapture || currentWindowUnderMouse == window))
            || (m_previousCaptureWindow && window != m_previousCaptureWindow && currentWindowUnderMouse
            && currentWindowUnderMouse != m_previousCaptureWindow)) {

         QWindowsWindow::baseWindowOf(currentWindowUnderMouse)->applyCursor();
         QWindowSystemInterface::handleEnterEvent(currentWindowUnderMouse,
            currentWindowUnderMouse->mapFromGlobal(globalPosition),
            globalPosition);
      }
      // We need to track m_windowUnderMouse separately from m_trackedWindow, as
      // Windows mouse tracking will not trigger WM_MOUSELEAVE for leaving window when
      // mouse capture is set.
      m_windowUnderMouse = currentWindowUnderMouse;
   }

   QWindowSystemInterface::handleMouseEvent(window, winEventPosition, globalPosition, buttons,
      QWindowsKeyMapper::queryKeyboardModifiers(), source);

   m_previousCaptureWindow = hasCapture ? window : nullptr;

   // QTBUG-48117, force synchronous handling for the extra buttons so that WM_APPCOMMAND
   // is sent for unhandled WM_XBUTTONDOWN.

   return (msg.message != WM_XBUTTONUP && msg.message != WM_XBUTTONDOWN && msg.message != WM_XBUTTONDBLCLK)
      || QWindowSystemInterface::flushWindowSystemEvents();
}

static bool isValidWheelReceiver(QWindow *candidate)
{
   if (candidate) {
      const QWindow *toplevel = QWindowsWindow::topLevelOf(candidate);
      if (const QWindowsWindow *ww = QWindowsWindow::baseWindowOf(toplevel)) {
         return !ww->testFlag(QWindowsWindow::BlockedByModal);
      }
   }

   return false;
}

static void redirectWheelEvent(QWindow *window, const QPoint &globalPos, int delta,
   Qt::Orientation orientation, Qt::KeyboardModifiers mods)
{
   // Redirect wheel event to one of the following, in order of preference:
   // 1) The window under mouse
   // 2) The window receiving the event
   // If a window is blocked by modality, it can't get the event.

   QWindow *receiver = QWindowsScreen::windowAt(globalPos, CWP_SKIPINVISIBLE);
   bool handleEvent = true;

   if (!isValidWheelReceiver(receiver)) {
      receiver = window;
      if (!isValidWheelReceiver(receiver)) {
         handleEvent = false;
      }
   }

   if (handleEvent) {
      QWindowSystemInterface::handleWheelEvent(receiver,
         QWindowsGeometryHint::mapFromGlobal(receiver, globalPos),
         globalPos, delta, orientation, mods);
   }
}

bool QWindowsMouseHandler::translateMouseWheelEvent(QWindow *window, HWND, MSG msg, LRESULT *)
{
   const Qt::KeyboardModifiers mods = keyStateToModifiers(int(msg.wParam));

   int delta;
   if (msg.message == WM_MOUSEWHEEL || msg.message == WM_MOUSEHWHEEL) {
      delta = GET_WHEEL_DELTA_WPARAM(msg.wParam);
   } else {
      delta = int(msg.wParam);
   }

   Qt::Orientation orientation = (msg.message == WM_MOUSEHWHEEL || (mods & Qt::AltModifier))
                  ? Qt::Horizontal : Qt::Vertical;

   // according to the MSDN documentation on WM_MOUSEHWHEEL:
   // a positive value indicates that the wheel was rotated to the right;
   // a negative value indicates that the wheel was rotated to the left.
   // defines this value as the exact opposite, so we have to flip the value

   if (msg.message == WM_MOUSEHWHEEL) {
      delta = -delta;
   }

   const QPoint globalPos(GET_X_LPARAM(msg.lParam), GET_Y_LPARAM(msg.lParam));
   redirectWheelEvent(window, globalPos, delta, orientation, mods);

   return true;
}

bool QWindowsMouseHandler::translateScrollEvent(QWindow *window, HWND, MSG msg, LRESULT *)
{
   // This is a workaround against some touchpads that send WM_HSCROLL instead of WM_MOUSEHWHEEL.
   // We could also handle vertical scroll here but there's no reason to, there's no bug for vertical
   // (broken vertical scroll would have been noticed long time ago), so lets keep the change small
   // and minimize the chance for regressions.

   int delta = 0;
   switch (LOWORD(msg.wParam)) {
      case SB_LINELEFT:
         delta = 120;
         break;

      case SB_LINERIGHT:
         delta = -120;
         break;

      case SB_PAGELEFT:
         delta = 240;
         break;

      case SB_PAGERIGHT:
         delta = -240;
         break;

      default:
         return false;
   }

   redirectWheelEvent(window, QCursor::pos(), delta, Qt::Horizontal, Qt::NoModifier);

   return true;
}

// from bool QApplicationPrivate::translateTouchEvent()
bool QWindowsMouseHandler::translateTouchEvent(QWindow *window, HWND,
   QtWindows::WindowsEventType, MSG msg, LRESULT *)
{
   typedef QWindowSystemInterface::TouchPoint QTouchPoint;
   typedef QList<QWindowSystemInterface::TouchPoint> QTouchPointList;

   if (! QWindowsContext::instance()->initTouch()) {
      qWarning("QWindowsMouseHandler::translateTouchEvent() Unable to initialize touch handling");
      return true;
   }

   const QScreen *screen = window->screen();

   if (! screen) {
      screen = QApplication::primaryScreen();
   }

   if (! screen) {
      return true;
   }

   const QRect screenGeometry = screen->geometry();

   const int winTouchPointCount = int(msg.wParam);
   QScopedArrayPointer<TOUCHINPUT> winTouchInputs(new TOUCHINPUT[winTouchPointCount]);
   memset(winTouchInputs.data(), 0, sizeof(TOUCHINPUT) * size_t(winTouchPointCount));

   QTouchPointList touchPoints;
   Qt::TouchPointStates allStates = Qt::EmptyFlag;

   QWindowsContext::user32dll.getTouchInputInfo(reinterpret_cast<HANDLE>(msg.lParam),
      UINT(msg.wParam), winTouchInputs.data(), sizeof(TOUCHINPUT));

   for (int i = 0; i < winTouchPointCount; ++i) {
      const TOUCHINPUT &winTouchInput = winTouchInputs[i];
      int id = m_touchInputIDToTouchPointID.value(winTouchInput.dwID, -1);

      if (id == -1) {
         id = m_touchInputIDToTouchPointID.size();
         m_touchInputIDToTouchPointID.insert(winTouchInput.dwID, id);
      }

      QTouchPoint touchPoint;
      touchPoint.pressure = 1.0;
      touchPoint.id = id;

      if (m_lastTouchPositions.contains(id)) {
         touchPoint.normalPosition = m_lastTouchPositions.value(id);
      }

      const QPointF screenPos = QPointF(winTouchInput.x, winTouchInput.y) / qreal(100.);
      if (winTouchInput.dwMask & TOUCHINPUTMASKF_CONTACTAREA) {
         touchPoint.area.setSize(QSizeF(winTouchInput.cxContact, winTouchInput.cyContact) / qreal(100.));
      }

      touchPoint.area.moveCenter(screenPos);
      QPointF normalPosition = QPointF(screenPos.x() / screenGeometry.width(),
            screenPos.y() / screenGeometry.height());
      const bool stationaryTouchPoint = (normalPosition == touchPoint.normalPosition);
      touchPoint.normalPosition = normalPosition;

      if (winTouchInput.dwFlags & TOUCHEVENTF_DOWN) {
         touchPoint.state = Qt::TouchPointPressed;

         m_lastTouchPositions.insert(id, touchPoint.normalPosition);
      } else if (winTouchInput.dwFlags & TOUCHEVENTF_UP) {
         touchPoint.state = Qt::TouchPointReleased;
         m_lastTouchPositions.remove(id);

      } else {
         touchPoint.state = (stationaryTouchPoint
               ? Qt::TouchPointStationary : Qt::TouchPointMoved);
         m_lastTouchPositions.insert(id, touchPoint.normalPosition);
      }

      allStates |= touchPoint.state;

      touchPoints.append(touchPoint);
   }

   QWindowsContext::user32dll.closeTouchInputHandle(reinterpret_cast<HANDLE>(msg.lParam));

   // all touch points released, forget the ids already seen since they may not be reused
   if (allStates == Qt::TouchPointReleased) {
      m_touchInputIDToTouchPointID.clear();
   }

   QWindowSystemInterface::handleTouchEvent(window, m_touchDevice, touchPoints);

   return true;
}

bool QWindowsMouseHandler::translateGestureEvent(QWindow *window, HWND hwnd,
   QtWindows::WindowsEventType, MSG msg, LRESULT *)
{
   (void) window;
   (void) hwnd;
   (void) msg;

   return false;
}

