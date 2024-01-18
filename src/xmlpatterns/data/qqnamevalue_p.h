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

#ifndef QQNameValue_P_H
#define QQNameValue_P_H

#include "qitem_p.h"
#include "qxmlname.h"

namespace QPatternist {
class QNameValue : public AtomicValue
{
 public:
   friend class CommonValues;
   friend class QNameComparator;

   typedef QExplicitlySharedDataPointer<QNameValue> Ptr;

   static QNameValue::Ptr fromValue(const NamePool::Ptr &np, const QXmlName name);

   QString stringValue() const override;
   ItemType::Ptr type() const override;

   QXmlName qName() const {
      return m_qName;
   }

 private:
   QNameValue(const NamePool::Ptr &np, const QXmlName name);

   const QXmlName m_qName;
   const NamePool::Ptr m_namePool;
};
}

#endif
