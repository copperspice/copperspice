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

#include <qkeymapper_p.h>
#include <qdebug.h>
#include <qevent_p.h>
#include <qlocale_p.h>

QT_BEGIN_NAMESPACE

QT_USE_NAMESPACE

QKeyMapperPrivate::QKeyMapperPrivate()
{
   keyboardInputLocale = QLocale::system();
   keyboardInputDirection = keyboardInputLocale.textDirection();
}

QKeyMapperPrivate::~QKeyMapperPrivate()
{
   // clearMappings();
}

void QKeyMapperPrivate::clearMappings()
{
}

QList<int> QKeyMapperPrivate::possibleKeys(QKeyEvent *e)
{
   QList<int> result;
   if (e->key() && (e->key() != Qt::Key_unknown)) {
      result << int(e->key() + e->modifiers());
   } else if (!e->text().isEmpty()) {
      result << int(e->text().at(0).unicode() + e->modifiers());
   }
   return result;
}

QT_END_NAMESPACE
