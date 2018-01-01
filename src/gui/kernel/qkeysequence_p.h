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

#ifndef QKEYSEQUENCE_P_H
#define QKEYSEQUENCE_P_H

#include <qkeysequence.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_SHORTCUT
struct QKeyBinding {
   QKeySequence::StandardKey standardKey;
   uchar priority;
   uint shortcut;
   uint platform;
};

class QKeySequencePrivate
{
 public:
   inline QKeySequencePrivate() {
      ref = 1;
      key[0] = key[1] = key[2] = key[3] =  0;
   }
   inline QKeySequencePrivate(const QKeySequencePrivate &copy) {
      ref = 1;
      key[0] = copy.key[0];
      key[1] = copy.key[1];
      key[2] = copy.key[2];
      key[3] = copy.key[3];
   }
   QAtomicInt ref;
   int key[4];
   static QString encodeString(int key, QKeySequence::SequenceFormat format);
   static int decodeString(const QString &keyStr, QKeySequence::SequenceFormat format);

   static const QKeyBinding keyBindings[];
   static const uint numberOfKeyBindings;

};
#endif // QT_NO_SHORTCUT

QT_END_NAMESPACE

#endif //QKEYSEQUENCE_P_H
