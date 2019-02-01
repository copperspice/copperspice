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
#include <QScopedPointer>


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
   static QList<int> possibleKeys(QKeyEvent *e);

 protected:
   QScopedPointer<QKeyMapperPrivate> d_ptr;

 private:
   Q_DECLARE_PRIVATE(QKeyMapper)
   Q_DISABLE_COPY(QKeyMapper)

   friend QKeyMapperPrivate *qt_keymapper_private();
};

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

 protected:
   QKeyMapper *q_ptr;

};

QKeyMapperPrivate *qt_keymapper_private(); // from qkeymapper.cpp

#endif
