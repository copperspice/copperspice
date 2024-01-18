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

#ifndef QKEYSEQUENCEEDIT_P_H
#define QKEYSEQUENCEEDIT_P_H

#include <qkeysequenceedit.h>

#include <qwidget_p.h>
#include <qkeysequence_p.h>

#ifndef QT_NO_KEYSEQUENCEEDIT

class QLineEdit;

class QKeySequenceEditPrivate : public QWidgetPrivate
{
   Q_DECLARE_PUBLIC(QKeySequenceEdit)

 public:
   void init();
   int translateModifiers(Qt::KeyboardModifiers state, const QString &text);
   void resetState();
   void finishEditing();

   QLineEdit *lineEdit;
   QKeySequence keySequence;
   int keyNum;
   int key[QKeySequencePrivate::MaxKeyCount];
   int prevKey;
   int releaseTimer;
};

#endif // QT_NO_KEYSEQUENCEEDIT

#endif
