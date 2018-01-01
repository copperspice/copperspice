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

#ifndef QABSTRACTXMLPULLPROVIDER_P_H
#define QABSTRACTXMLPULLPROVIDER_P_H

#include <QtCore/QtGlobal>
#include <qcontainerfwd.h>

QT_BEGIN_NAMESPACE

class QXmlItem;
class QXmlName;
class QString;
class QVariant;

namespace QPatternist {
class AbstractXmlPullProviderPrivate;

class AbstractXmlPullProvider
{
 public:
   AbstractXmlPullProvider();
   virtual ~AbstractXmlPullProvider();

   enum Event {
      StartOfInput            = 1,
      AtomicValue             = 1 << 1,
      StartDocument           = 1 << 2,
      EndDocument             = 1 << 3,
      StartElement            = 1 << 4,
      EndElement              = 1 << 5,
      Text                    = 1 << 6,
      ProcessingInstruction   = 1 << 7,
      Comment                 = 1 << 8,
      Attribute               = 1 << 9,
      Namespace               = 1 << 10,
      EndOfInput              = 1 << 11
   };

   virtual Event next() = 0;
   virtual Event current() const = 0;
   virtual QXmlName name() const = 0;
   virtual QVariant atomicValue() const = 0;
   virtual QString stringValue() const = 0;

   virtual QHash<QXmlName, QString> attributes() = 0;
   virtual QHash<QXmlName, QXmlItem> attributeItems() = 0;

   /* *** The functions below are internal. */
 private:
   Q_DISABLE_COPY(AbstractXmlPullProvider)
   AbstractXmlPullProviderPrivate *d;
};
}

QT_END_NAMESPACE

#endif
