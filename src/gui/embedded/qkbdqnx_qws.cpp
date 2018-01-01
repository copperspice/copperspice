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

#include <qkbdqnx_qws.h>
#include <qplatformdefs.h>
#include <qsocketnotifier.h>
#include <qcore_unix_p.h>
#include <QtCore/qdebug.h>
#include <sys/dcmd_input.h>
#include <sys/keycodes.h>
#include <errno.h>

QT_BEGIN_NAMESPACE

/*!
    \class QWSQnxKeyboardHandler
    \preliminary
    \ingroup qws
    \since 4.6
    \internal

    \brief The QWSQnxKeyboardHandler class implements a keyboard driver
    for the QNX \c{devi-hid} input manager.

    To be able to compile this mouse handler, \l{Qt for Embedded Linux}
    must be configured with the \c -qt-kbd-qnx option, see the
    \l{Qt for Embedded Linux Character Input} documentation for details.

    In order to use this keyboard handler, the \c{devi-hid} input manager
    must be set up and run with the resource manager interface (option \c{-r}).
    Also, Photon must not be running.

    Example invocation from command line: \c{/usr/photon/bin/devi-hid -Pr kbd mouse}
    Note that after running \c{devi-hid}, you will not be able to use the local
    shell anymore. It is suggested to run the command in a shell script, that launches
    a Qt application after invocation of \c{devi-hid}.

    To make \l{Qt for Embedded Linux} explicitly choose the qnx keyboard
    handler, set the QWS_KEYBOARD environment variable to \c{qnx}. By default,
    the first keyboard device (\c{/dev/devi/keyboard0}) is used. To override, pass a device
    name as the first and only parameter, for example
    \c{QWS_KEYBOARD=qnx:/dev/devi/keyboard1; export QWS_KEYBOARD}.

    \sa {Qt for Embedded Linux Character Input}, {Qt for Embedded Linux}
*/

/*!
    Constructs a keyboard handler for the specified \a device, defaulting to
    \c{/dev/devi/keyboard0}.

    Note that you should never instanciate this class, instead let QKbdDriverFactory
    handle the keyboard handlers.

    \sa QKbdDriverFactory
 */
QWSQnxKeyboardHandler::QWSQnxKeyboardHandler(const QString &device)
{
   // open the keyboard device
   keyboardFD = QT_OPEN(device.isEmpty() ? "/dev/devi/keyboard0" : device.toLatin1().constData(),
                        QT_OPEN_RDONLY);
   if (keyboardFD == -1) {
      qErrnoWarning(errno, "QWSQnxKeyboardHandler: Unable to open device");
   } else {
      // create a socket notifier so we'll wake up whenever keyboard input is detected.
      QSocketNotifier *notifier = new QSocketNotifier(keyboardFD, QSocketNotifier::Read, this);
      connect(notifier, SIGNAL(activated(int)), SLOT(socketActivated()));

      qDebug("QWSQnxKeyboardHandler: connected.");
   }
}

/*!
    Destroys this keyboard handler and closes the connection to the keyboard device.
 */
QWSQnxKeyboardHandler::~QWSQnxKeyboardHandler()
{
   if (keyboardFD != -1) {
      QT_CLOSE(keyboardFD);
   }
}

// similar to PhKeyToMb
static inline bool key_sym_displayable(unsigned long sym)
{
   if (sym >= 0xF000) {
      return sym >= 0xF100 && (sizeof(wchar_t) > 2 || sym < 0x10000);
   }
   return (sym & ~0x9F) != 0; // exclude 0...0x1F and 0x80...0x9F
}

/*! \internal
    Translates the QNX keyboard events to Qt keyboard events
 */
void QWSQnxKeyboardHandler::socketActivated()
{
   _keyboard_packet packet;

   // read one keyboard event
   int bytesRead = QT_READ(keyboardFD, &packet, sizeof(_keyboard_packet));
   if (bytesRead == -1) {
      qErrnoWarning(errno, "QWSQnxKeyboardHandler::socketActivated(): Unable to read data.");
      return;
   }

   // the bytes read must be the size of a keyboard packet
   Q_ASSERT(bytesRead == sizeof(_keyboard_packet));

   if (packet.data.flags & KEY_SYM_VALID_EX) {
      packet.data.flags |= KEY_SYM_VALID;
   } else if (!(packet.data.flags & (KEY_SYM_VALID | KEY_CAP_VALID))) {
      return;
   }

   // QNX is nice enough to translate the raw keyboard data into generic format for us.
   // Now we just have to translate it into a format Qt understands.

   // figure out the modifiers that are currently pressed
   Qt::KeyboardModifiers modifiers = Qt::NoModifier;
   if (packet.data.modifiers & KEYMOD_SHIFT) {
      modifiers |= Qt::ShiftModifier;
   }
   if (packet.data.modifiers & KEYMOD_CTRL) {
      modifiers |= Qt::ControlModifier;
   }
   if (packet.data.modifiers & KEYMOD_ALT) {
      modifiers |= Qt::AltModifier;
   }
   if (packet.data.modifiers & KEYMOD_NUM_LOCK) {
      modifiers |= Qt::KeypadModifier;
   }

   // figure out whether it's a press
   bool isPress = packet.data.flags & KEY_DOWN;
   // figure out whether the key is still pressed and the key event is repeated
   bool isRepeat = packet.data.flags & KEY_REPEAT;

   int key = Qt::Key_unknown;
   int unicode = 0;

   if (((packet.data.flags & KEY_SYM_VALID) && key_sym_displayable(unicode = packet.data.key_sym))
         || ((packet.data.flags & KEY_CAP_VALID) && key_sym_displayable(unicode = packet.data.key_cap))) {
      if (unicode <= 0x0ff) {
         if (unicode >= 'a' && unicode <= 'z') {
            key = Qt::Key_A + unicode - 'a';
         } else {
            key = unicode;
         }
      }
      // Ctrl<something> or Alt<something> is not a displayable character
      if (modifiers & (Qt::ControlModifier | Qt::AltModifier)) {
         unicode = 0;
      }
   } else {
      unicode = 0;

      unsigned long sym = 0;
      if (packet.data.flags & KEY_SYM_VALID) {
         sym = packet.data.key_sym;
      } else if (packet.data.flags & KEY_CAP_VALID) {
         sym = packet.data.key_cap;
      }

      switch (sym) {
         case KEYCODE_ESCAPE:
            key = Qt::Key_Escape;
            unicode = 27;
            break;
         case KEYCODE_TAB:
            key = Qt::Key_Tab;
            unicode = 9;
            break;
         case KEYCODE_BACK_TAB:
            key = Qt::Key_Backtab;
            break;
         case KEYCODE_BACKSPACE:
            key = Qt::Key_Backspace;
            unicode = 127;
            break;
         case KEYCODE_RETURN:
            key = Qt::Key_Return;
            break;
         case KEYCODE_KP_ENTER:
            key = Qt::Key_Enter;
            break;
         case KEYCODE_INSERT:
         case KEYCODE_KP_INSERT:
            key = Qt::Key_Insert;
            break;
         case KEYCODE_KP_DELETE:
            if (modifiers & Qt::KeypadModifier) {
               key = Qt::Key_Comma;
               break;
            }
         // fall through
         case KEYCODE_DELETE:
            key = Qt::Key_Delete;
            break;
         case KEYCODE_PAUSE:
         case KEYCODE_BREAK:
            if (modifiers & (Qt::ControlModifier | Qt::AltModifier)) {
               return;   // sometimes occurs at the middle of a key sequence
            }
            key = Qt::Key_Pause;
            break;
         case KEYCODE_PRINT:
            if (modifiers & (Qt::ControlModifier | Qt::AltModifier)) {
               return;   // sometimes occurs at the middle of a key sequence
            }
            key = Qt::Key_Print;
            break;
         case KEYCODE_SYSREQ:
            key = Qt::Key_SysReq;
            break;
         case KEYCODE_HOME:
         case KEYCODE_KP_HOME:
            key = Qt::Key_Home;
            break;
         case KEYCODE_END:
         case KEYCODE_KP_END:
            key = Qt::Key_End;
            break;
         case KEYCODE_LEFT:
         case KEYCODE_KP_LEFT:
            key = Qt::Key_Left;
            break;
         case KEYCODE_UP:
         case KEYCODE_KP_UP:
            key = Qt::Key_Up;
            break;
         case KEYCODE_RIGHT:
         case KEYCODE_KP_RIGHT:
            key = Qt::Key_Right;
            break;
         case KEYCODE_DOWN:
         case KEYCODE_KP_DOWN:
            key = Qt::Key_Down;
            break;
         case KEYCODE_PG_UP:
         case KEYCODE_KP_PG_UP:
            key = Qt::Key_PageUp;
            break;
         case KEYCODE_PG_DOWN:
         case KEYCODE_KP_PG_DOWN:
            key = Qt::Key_PageDown;
            break;

         case KEYCODE_LEFT_SHIFT:
         case KEYCODE_RIGHT_SHIFT:
            key = Qt::Key_Shift;
            break;
         case KEYCODE_LEFT_CTRL:
         case KEYCODE_RIGHT_CTRL:
            key = Qt::Key_Control;
            break;
         case KEYCODE_LEFT_ALT:
         case KEYCODE_RIGHT_ALT:
            key = Qt::Key_Alt;
            break;
         case KEYCODE_CAPS_LOCK:
            key = Qt::Key_CapsLock;
            break;
         case KEYCODE_NUM_LOCK:
            key = Qt::Key_NumLock;
            break;
         case KEYCODE_SCROLL_LOCK:
            key = Qt::Key_ScrollLock;
            break;

         case KEYCODE_F1:
         case KEYCODE_F2:
         case KEYCODE_F3:
         case KEYCODE_F4:
         case KEYCODE_F5:
         case KEYCODE_F6:
         case KEYCODE_F7:
         case KEYCODE_F8:
         case KEYCODE_F9:
         case KEYCODE_F10:
         case KEYCODE_F11:
         case KEYCODE_F12:
            key = Qt::Key_F1 + sym - KEYCODE_F1;
            break;

         case KEYCODE_MENU:
            key = Qt::Key_Menu;
            break;
         case KEYCODE_LEFT_HYPER:
            key = Qt::Key_Hyper_L;
            break;
         case KEYCODE_RIGHT_HYPER:
            key = Qt::Key_Hyper_R;
            break;

         case KEYCODE_KP_PLUS:
            key = Qt::Key_Plus;
            break;
         case KEYCODE_KP_MINUS:
            key = Qt::Key_Minus;
            break;
         case KEYCODE_KP_MULTIPLY:
            key = Qt::Key_multiply;
            break;
         case KEYCODE_KP_DIVIDE:
            key = Qt::Key_Slash;
            break;
         case KEYCODE_KP_FIVE:
            if (!(modifiers & Qt::KeypadModifier)) {
               key = Qt::Key_5;
            }
            break;

         default: // none of the above
            break;
      }
   }

   if (key == Qt::Key_unknown && unicode == 0) {
      return;
   }

   // call processKeyEvent. This is where all the magic happens to insert a
   // key event into Qt's event loop.
   // Note that for repeated key events, isPress must be true
   // (on QNX, isPress is not set when the key event is repeated).
   processKeyEvent(unicode, key, modifiers, isPress || isRepeat, isRepeat);
}

QT_END_NAMESPACE
