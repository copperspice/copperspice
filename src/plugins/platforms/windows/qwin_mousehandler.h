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

#ifndef QWINDOWSMOUSEHANDLER_H
#define QWINDOWSMOUSEHANDLER_H

#include <qwin_global.h>
#include <qwin_additional.h>
#include <qpointf.h>
#include <qpointer.h>
#include <qhash.h>

class QWindow;
class QTouchDevice;

class QWindowsMouseHandler
{
 public:
   QWindowsMouseHandler();

   QWindowsMouseHandler(const QWindowsMouseHandler &) = delete;
   QWindowsMouseHandler &operator=(const QWindowsMouseHandler &) = delete;

   QTouchDevice *touchDevice() const {
      return m_touchDevice;
   }

   QTouchDevice *ensureTouchDevice();

   bool translateMouseEvent(QWindow *widget, HWND hwnd, QtWindows::WindowsEventType t, MSG msg, LRESULT *result);
   bool translateTouchEvent(QWindow *widget, HWND hwnd, QtWindows::WindowsEventType t, MSG msg, LRESULT *result);
   bool translateGestureEvent(QWindow *window, HWND hwnd, QtWindows::WindowsEventType, MSG msg, LRESULT *);
   bool translateScrollEvent(QWindow *window, HWND hwnd, MSG msg, LRESULT *result);

   static inline Qt::MouseButtons keyStateToMouseButtons(int);
   static inline Qt::KeyboardModifiers keyStateToModifiers(int);
   static inline int mouseButtonsToKeyState(Qt::MouseButtons);

   static Qt::MouseButtons queryMouseButtons();
   QWindow *windowUnderMouse() const {
      return m_windowUnderMouse.data();
   }

   void clearWindowUnderMouse() {
      m_windowUnderMouse = nullptr;
   }

 private:
   inline bool translateMouseWheelEvent(QWindow *window, HWND hwnd, MSG msg, LRESULT *result);

   QPointer<QWindow> m_windowUnderMouse;
   QPointer<QWindow> m_trackedWindow;
   QHash<DWORD, int> m_touchInputIDToTouchPointID;
   QHash<int, QPointF> m_lastTouchPositions;
   QTouchDevice *m_touchDevice;
   bool m_leftButtonDown;
   QWindow *m_previousCaptureWindow;
};

Qt::MouseButtons QWindowsMouseHandler::keyStateToMouseButtons(int wParam)
{
   Qt::MouseButtons mb(Qt::NoButton);

   if (wParam & MK_LBUTTON) {
      mb |= Qt::LeftButton;
   }

   if (wParam & MK_MBUTTON) {
      mb |= Qt::MiddleButton;
   }

   if (wParam & MK_RBUTTON) {
      mb |= Qt::RightButton;
   }

   if (wParam & MK_XBUTTON1) {
      mb |= Qt::XButton1;
   }

   if (wParam & MK_XBUTTON2) {
      mb |= Qt::XButton2;
   }

   return mb;
}

Qt::KeyboardModifiers QWindowsMouseHandler::keyStateToModifiers(int wParam)
{
   Qt::KeyboardModifiers mods(Qt::NoModifier);
   if (wParam & MK_CONTROL) {
      mods |= Qt::ControlModifier;
   }
   if (wParam & MK_SHIFT) {
      mods |= Qt::ShiftModifier;
   }
   if (GetKeyState(VK_MENU) < 0) {
      mods |= Qt::AltModifier;
   }
   return mods;
}

int QWindowsMouseHandler::mouseButtonsToKeyState(Qt::MouseButtons mb)
{
   int result = 0;
   if (mb & Qt::LeftButton) {
      result |= MK_LBUTTON;
   }
   if (mb & Qt::MiddleButton) {
      result |= MK_MBUTTON;
   }
   if (mb & Qt::RightButton) {
      result |= MK_RBUTTON;
   }
   if (mb & Qt::XButton1) {
      result |= MK_XBUTTON1;
   }
   if (mb & Qt::XButton2) {
      result |= MK_XBUTTON2;
   }
   return result;
}

#endif
