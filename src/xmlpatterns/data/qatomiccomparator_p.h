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

#ifndef QAtomicComparator_P_H
#define QAtomicComparator_P_H

#include <qflags.h>
#include <qitem_p.h>
#include <qstringfwd.h>

#include <qatomictypedispatch_p.h>

namespace QPatternist {
class AtomicComparator : public AtomicTypeVisitorResult
{
 public:
   AtomicComparator();
   virtual ~AtomicComparator();

   typedef QExplicitlySharedDataPointer<AtomicComparator> Ptr;

   enum Operator {
      OperatorEqual           = 1,
      OperatorNotEqual        = 1 << 1,
      OperatorGreaterThan     = 1 << 2,
      OperatorLessThan        = 1 << 3,
      OperatorLessThanNaNLeast    = 1 << 4,
      OperatorLessThanNaNGreatest    = 1 << 5,
      OperatorGreaterOrEqual  = OperatorEqual | OperatorGreaterThan,
      OperatorLessOrEqual     = OperatorEqual | OperatorLessThan
   };

   typedef QFlags<Operator> Operators;

   enum ComparisonResult {
      LessThan     = 1,
      Equal        = 2,
      GreaterThan  = 4,
      Incomparable = 8
   };

   virtual ComparisonResult compare(const Item &op1,
                                    const AtomicComparator::Operator op,
                                    const Item &op2) const;

   virtual bool equals(const Item &op1,
                       const Item &op2) const = 0;

   enum ComparisonType {
      AsGeneralComparison = 1,
      AsValueComparison
   };

   static QString displayName(const AtomicComparator::Operator op,
                              const ComparisonType type);

};

Q_DECLARE_OPERATORS_FOR_FLAGS(AtomicComparator::Operators)
}



#endif
