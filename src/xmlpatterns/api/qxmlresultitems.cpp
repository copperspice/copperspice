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

#include <qxmlresultitems.h>
#include <qxmlresultitems_p.h>

#include <qitem_p.h>

QXmlResultItems::QXmlResultItems() : d_ptr(new QXmlResultItemsPrivate())
{
}

QXmlResultItems::~QXmlResultItems()
{
}

QXmlItem QXmlResultItems::next()
{
   Q_D(QXmlResultItems);
   if (d->hasError) {
      return QXmlItem();
   }

   try {
      d->current = QPatternist::Item::toPublic(d->iterator->next());
      return d->current;
   } catch (const QPatternist::Exception) {
      d->current = QXmlItem();
      d->hasError = true;
      return QXmlItem();
   }
}

QXmlItem QXmlResultItems::current() const
{
   Q_D(const QXmlResultItems);

   if (d->hasError) {
      return QXmlItem();
   } else {
      return d->current;
   }
}

bool QXmlResultItems::hasError() const
{
   Q_D(const QXmlResultItems);
   return d->hasError;
}
