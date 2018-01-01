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

#ifndef QKBD_QWS_H
#define QKBD_QWS_H

#include <QtGui/qapplication.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_QWS_KEYBOARD

class QWSKbPrivate;

class  Q_GUI_EXPORT QWSKeyboardHandler
{
 public:
   QWSKeyboardHandler();
   QWSKeyboardHandler(const QString &device);
   virtual ~QWSKeyboardHandler();

   virtual void processKeyEvent(int unicode, int keycode, Qt::KeyboardModifiers modifiers, bool isPress, bool autoRepeat);

   enum KeycodeAction {
      None               = 0,

      CapsLockOff        = 0x01000000,
      CapsLockOn         = 0x01000001,
      NumLockOff         = 0x02000000,
      NumLockOn          = 0x02000001,
      ScrollLockOff      = 0x03000000,
      ScrollLockOn       = 0x03000001,

      Reboot             = 0x04000000,

      PreviousConsole    = 0x05000000,
      NextConsole        = 0x05000001,
      SwitchConsoleFirst = 0x06000000,
      SwitchConsoleLast  = 0x0600007f,
      SwitchConsoleMask  = 0x0000007f,
   };

   KeycodeAction processKeycode(quint16 keycode, bool pressed, bool autorepeat);

 protected:
   int transformDirKey(int key);
   void beginAutoRepeat(int uni, int code, Qt::KeyboardModifiers mod);
   void endAutoRepeat();

 private:
   QWSKbPrivate *d;
};

#endif // QT_NO_QWS_KEYBOARD

QT_END_NAMESPACE

#endif // QKBD_QWS_H
