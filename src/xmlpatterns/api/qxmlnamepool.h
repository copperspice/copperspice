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

#ifndef QXMLNAMEPOOL_H
#define QXMLNAMEPOOL_H

#include <qshareddata.h>
#include <qstring.h>

namespace QPatternist {
class NamePool;
class XsdSchemaParser;
class XsdValidatingInstanceReader;
}

namespace QPatternistSDK {
class Global;
}

class QXmlQueryPrivate;
class QXmlName;

class Q_XMLPATTERNS_EXPORT QXmlNamePool
{
 public:
   QXmlNamePool();
   QXmlNamePool(const QXmlNamePool &other);
   ~QXmlNamePool();
   QXmlNamePool &operator=(const QXmlNamePool &other);

 private:
   QXmlNamePool(QPatternist::NamePool *namePool);
   friend class QXmlQueryPrivate;
   friend class QXmlQuery;
   friend class QXmlSchemaPrivate;
   friend class QXmlSchemaValidatorPrivate;
   friend class QXmlSerializerPrivate;
   friend class QXmlName;
   friend class QPatternist::XsdSchemaParser;
   friend class QPatternist::XsdValidatingInstanceReader;
   friend class QPatternistSDK::Global;
   QExplicitlySharedDataPointer<QPatternist::NamePool> d;
};

#endif
