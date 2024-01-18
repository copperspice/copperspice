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

#ifndef QWINDOWSKEYMAPPER_H
#define QWINDOWSKEYMAPPER_H

#include <qwin_additional.h>
#include <qlocale.h>

class QKeyEvent;
class QWindow;

/*
    \internal
    A Windows KeyboardLayoutItem has 8 possible states:
        1. Unmodified
        2. Shift
        3. Control
        4. Control + Shift
        5. Alt
        6. Alt + Shift
        7. Alt + Control
        8. Alt + Control + Shift
*/
struct KeyboardLayoutItem {
   uint dirty : 1;
   uint exists : 1; // whether this item has been initialized (by updatePossibleKeyCodes)
   quint8 deadkeys;
   static const size_t NumQtKeys = 9;
   quint32 qtKey[NumQtKeys]; // Can by any Qt::Key_<foo>, or unicode character
};

class QWindowsKeyMapper
{
 public:
   explicit QWindowsKeyMapper();

   QWindowsKeyMapper(const QWindowsKeyMapper &) = delete;
   QWindowsKeyMapper &operator=(const QWindowsKeyMapper &) = delete;

   ~QWindowsKeyMapper();

   void changeKeyboard();

   void setUseRTLExtensions(bool e) {
      m_useRTLExtensions = e;
   }

   bool useRTLExtensions() const    {
      return m_useRTLExtensions;
   }

   bool translateKeyEvent(QWindow *widget, HWND hwnd, const MSG &msg, LRESULT *result);

   QWindow *keyGrabber() const      {
      return m_keyGrabber;
   }

   void setKeyGrabber(QWindow *w)   {
      m_keyGrabber = w;
   }

   static Qt::KeyboardModifiers queryKeyboardModifiers();
   QList<int> possibleKeys(const QKeyEvent *e) const;

 private:
   bool translateKeyEventInternal(QWindow *receiver, const MSG &msg, bool grab);
   bool translateMultimediaKeyEventInternal(QWindow *receiver, const MSG &msg);
   void updateKeyMap(const MSG &msg);

   bool m_useRTLExtensions;

   QLocale keyboardInputLocale;
   Qt::LayoutDirection keyboardInputDirection;

   void updatePossibleKeyCodes(unsigned char *kbdBuffer, quint32 scancode, quint32 vk_key);
   void deleteLayouts();

   QWindow *m_keyGrabber;
   char16_t m_lastHighSurrogate;
   static const size_t NumKeyboardLayoutItems = 256;
   KeyboardLayoutItem keyLayout[NumKeyboardLayoutItems];
};

enum WindowsNativeModifiers {
   ShiftLeft            = 0x00000001,
   ControlLeft          = 0x00000002,
   AltLeft              = 0x00000004,
   MetaLeft             = 0x00000008,
   ShiftRight           = 0x00000010,
   ControlRight         = 0x00000020,
   AltRight             = 0x00000040,
   MetaRight            = 0x00000080,
   CapsLock             = 0x00000100,
   NumLock              = 0x00000200,
   ScrollLock           = 0x00000400,
   ExtendedKey          = 0x01000000,

   // Convenience mappings
   ShiftAny             = 0x00000011,
   ControlAny           = 0x00000022,
   AltAny               = 0x00000044,
   MetaAny              = 0x00000088,
   LockAny              = 0x00000700
};

#endif
