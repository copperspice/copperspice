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

#ifndef QComparisonFactory_P_H
#define QComparisonFactory_P_H

#include <qatomiccomparator_p.h>
#include <qderivedstring_p.h>
#include <qitem_p.h>
#include <qreportcontext_p.h>
#include <qschematype_p.h>

namespace QPatternist {

class ComparisonFactory
{
 public:
   static bool compare(const AtomicValue::Ptr &operand1,
                       const AtomicComparator::Operator op,
                       const AtomicValue::Ptr &operand2,
                       const SchemaType::Ptr &type,
                       const ReportContext::Ptr &context,
                       const SourceLocationReflection *const sourceLocationReflection);

   static bool constructAndCompare(const DerivedString<TypeString>::Ptr &operand1,
                                   const AtomicComparator::Operator op,
                                   const DerivedString<TypeString>::Ptr &operand2,
                                   const SchemaType::Ptr &type,
                                   const ReportContext::Ptr &context,
                                   const SourceLocationReflection *const sourceLocationReflection);

 private:
   ComparisonFactory(const ComparisonFactory &) = delete;
   ComparisonFactory &operator=(const ComparisonFactory &) = delete;
};

}

#endif
