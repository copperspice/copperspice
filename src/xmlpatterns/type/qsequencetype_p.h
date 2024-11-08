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

#ifndef QSequenceType_P_H
#define QSequenceType_P_H

#include <qshareddata.h>
#include <qcontainerfwd.h>

#include <qcardinality_p.h>
#include <qitemtype_p.h>

namespace QPatternist {
class ItemType;

class SequenceType : public virtual QSharedData
{
 public:
   SequenceType()
   { }

   typedef QExplicitlySharedDataPointer<const SequenceType> Ptr;
   typedef QList<SequenceType::Ptr> List;

   virtual ~SequenceType();

   virtual QString displayName(const NamePool::Ptr &np) const = 0;

   virtual Cardinality cardinality() const = 0;

   virtual ItemType::Ptr itemType() const = 0;

   bool matches(const SequenceType::Ptr other) const;

   bool is(const SequenceType::Ptr &other) const;

 private:
   SequenceType(const SequenceType &) = delete;
   SequenceType &operator=(const SequenceType &) = delete;
};

}

#endif
