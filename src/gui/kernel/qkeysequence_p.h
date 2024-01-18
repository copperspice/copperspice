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

#ifndef QKEYSEQUENCE_P_H
#define QKEYSEQUENCE_P_H

#include <qkeysequence.h>

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
   static constexpr int MaxKeyCount = 4;

   QKeySequencePrivate()
      : ref(1)
   {
      key[0] = 0;
      key[1] = 0;
      key[2] = 0;
      key[3] = 0;
   }

   QKeySequencePrivate(const QKeySequencePrivate &copy)
      : ref(1)
   {
      key[0] = copy.key[0];
      key[1] = copy.key[1];
      key[2] = copy.key[2];
      key[3] = copy.key[3];
   }

   QAtomicInt ref;
   int key[MaxKeyCount];
   static QString encodeString(int key, QKeySequence::SequenceFormat format);

   // used in dbusmenu
   Q_GUI_EXPORT static QString keyName(int key, QKeySequence::SequenceFormat format);

   static int decodeString(const QString &keyStr, QKeySequence::SequenceFormat format);
};

#endif // QT_NO_SHORTCUT



#endif
