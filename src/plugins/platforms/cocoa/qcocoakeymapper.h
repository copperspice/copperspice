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

#ifndef QCOCOAKEYMAPPER_H
#define QCOCOAKEYMAPPER_H

#include <qcocoahelpers.h>

#include <Cocoa/Cocoa.h>
#include <Carbon/Carbon.h>

#include <QList>
#include <QKeyEvent>

/*
    \internal
    A Mac KeyboardLayoutItem has 8 possible states:
        1. Unmodified
        2. Shift
        3. Control
        4. Control + Shift
        5. Alt
        6. Alt + Shift
        7. Alt + Control
        8. Alt + Control + Shift
        9. Meta
        10. Meta + Shift
        11. Meta + Control
        12. Meta + Control + Shift
        13. Meta + Alt
        14. Meta + Alt + Shift
        15. Meta + Alt + Control
        16. Meta + Alt + Control + Shift
*/
struct KeyboardLayoutItem {
   bool dirty;
   quint32 qtKey[16]; // Can by any Qt::Key_<foo>, or unicode character
};


class QCocoaKeyMapper
{
 public:
   QCocoaKeyMapper();
   ~QCocoaKeyMapper();
   static Qt::KeyboardModifiers queryKeyboardModifiers();
   QList<int> possibleKeys(const QKeyEvent *event) const;
   bool updateKeyboard();
   void deleteLayouts();
   void updateKeyMap(unsigned short macVirtualKey, QChar unicodeKey);
   void clearMappings();

 private:
   QCFType<TISInputSourceRef> currentInputSource;

   QLocale keyboardInputLocale;
   Qt::LayoutDirection keyboardInputDirection;
   enum { NullMode, UnicodeMode, OtherMode } keyboard_mode;
   union {
      const UCKeyboardLayout *unicode;
      void *other;
   } keyboard_layout_format;
   KeyboardLayoutKind keyboard_kind;
   UInt32 keyboard_dead;
   KeyboardLayoutItem *keyLayout[256];
};

#endif

