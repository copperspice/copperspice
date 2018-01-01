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

#ifndef QKEYMAPPER_P_H
#define QKEYMAPPER_P_H

#include <qobject.h>
#include <qkeysequence.h>
#include <qlist.h>
#include <qlocale.h>
#include <qevent.h>
#include <qhash.h>
#include <QScopedPointer>

#ifdef Q_OS_MAC
#include <qt_mac_p.h>
#endif

QT_BEGIN_NAMESPACE

class QKeyEvent;
class QKeyMapperPrivate;

struct KeyboardLayoutItem;

class QKeyMapper : public QObject
{
   GUI_CS_OBJECT(QKeyMapper)

 public:
   explicit QKeyMapper();
   ~QKeyMapper();

   static QKeyMapper *instance();
   static void changeKeyboard();
   static bool sendKeyEvent(QWidget *widget, bool grab, QEvent::Type type, int code, Qt::KeyboardModifiers modifiers,
                            const QString &text, bool autorepeat, int count, quint32 nativeScanCode, 
                            quint32 nativeVirtualKey, quint32 nativeModifiers, bool *unusedExceptForCocoa = 0);

   static QList<int> possibleKeys(QKeyEvent *e);

 protected:
   QScopedPointer<QKeyMapperPrivate> d_ptr;

 private:
   Q_DECLARE_PRIVATE(QKeyMapper)
   Q_DISABLE_COPY(QKeyMapper)

   friend QKeyMapperPrivate *qt_keymapper_private();
};


#if defined(Q_OS_WIN)
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

#if ! defined(tagMSG)
typedef struct tagMSG MSG;
#endif

#elif defined(Q_OS_MAC)

QT_BEGIN_INCLUDE_NAMESPACE
# include <qt_mac_p.h>
QT_END_INCLUDE_NAMESPACE

#elif defined(Q_WS_X11)

QT_BEGIN_INCLUDE_NAMESPACE
typedef ulong XID;
typedef XID KeySym;
QT_END_INCLUDE_NAMESPACE

struct QXCoreDesc {
   int min_keycode;
   int max_keycode;
   int keysyms_per_keycode;
   KeySym *keysyms;
   uchar mode_switch;
   uchar num_lock;
   KeySym lock_meaning;
};

#endif

class QKeyMapperPrivate
{
   Q_DECLARE_PUBLIC(QKeyMapper)

 public:
   QKeyMapperPrivate();
   virtual ~QKeyMapperPrivate();

   void clearMappings();
   QList<int> possibleKeys(QKeyEvent *e);

   QLocale keyboardInputLocale;
   Qt::LayoutDirection keyboardInputDirection;

#if defined(Q_OS_WIN)
   void clearRecordedKeys();
   void updateKeyMap(const MSG &msg);
   bool translateKeyEvent(QWidget *receiver, const MSG &msg, bool grab);
   void updatePossibleKeyCodes(unsigned char *kbdBuffer, quint32 scancode, quint32 vk_key);
   bool isADeadKey(unsigned int vk_key, unsigned int modifiers);
   void deleteLayouts();

   KeyboardLayoutItem *keyLayout[256];

#elif defined(Q_WS_X11)

   QList<int> possibleKeysXKB(QKeyEvent *event);
   QList<int> possibleKeysCore(QKeyEvent *event);

   bool translateKeyEventInternal(QWidget *keywidget, const XEvent *, KeySym &keysym, int &count, QString &text,
                                  Qt::KeyboardModifiers &modifiers, int &code, QEvent::Type &type, bool statefulTranslation = true);

   bool translateKeyEvent(QWidget *keywidget, const XEvent *, bool grab);

   int xkb_currentGroup;
   QXCoreDesc coreDesc;

#elif defined(Q_OS_MAC)
   bool updateKeyboard();
   void updateKeyMap(EventHandlerCallRef, EventRef, void *);
   bool translateKeyEvent(QWidget *, EventHandlerCallRef, EventRef, void *, bool);
   void deleteLayouts();

   enum { NullMode, UnicodeMode, OtherMode } keyboard_mode;

   union {
      const UCKeyboardLayout *unicode;
      void *other;
   } keyboard_layout_format;

   QCFType<TISInputSourceRef> m_currentInputSource;
   KeyboardLayoutKind m_keyboard_kind;
   UInt32 m_keyboard_dead;

   KeyboardLayoutItem *keyLayout[256];
#endif

 protected:
   QKeyMapper *q_ptr;

};

QKeyMapperPrivate *qt_keymapper_private(); // from qkeymapper.cpp

QT_END_NAMESPACE

#endif // QKEYMAPPER_P_H
