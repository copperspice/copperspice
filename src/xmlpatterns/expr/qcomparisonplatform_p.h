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

#ifndef QComparisonPlatform_P_H
#define QComparisonPlatform_P_H

#include <qatomiccomparators_p.h>
#include <qitem_p.h>
#include <qcommonsequencetypes_p.h>
#include <qdynamiccontext_p.h>
#include <qbuiltintypes_p.h>
#include <qitemtype_p.h>
#include <qpatternistlocale_p.h>

namespace QPatternist {

template <typename TSubClass,
          bool issueError,
          AtomicComparator::ComparisonType comparisonType = AtomicComparator::AsValueComparison,
          ReportContext::ErrorCode errorCode = ReportContext::XPTY0004>
class ComparisonPlatform
{
 protected:
   void prepareComparison(const AtomicComparator::Ptr &comparator);

   ComparisonPlatform() {
   }

   AtomicComparator::Ptr
   fetchComparator(const ItemType::Ptr &type1,
                   const ItemType::Ptr &type2,
                   const ReportContext::Ptr &context) const;

   bool compare(const Item &i1,
                const Item &i2,
                const AtomicComparator::Ptr &comp,
                const AtomicComparator::Operator op) const;

   bool
   flexibleCompare(const Item &it1,
                   const Item &it2,
                   const DynamicContext::Ptr &context) const;

   AtomicComparator::ComparisonResult
   detailedFlexibleCompare(const Item &it1,
                           const Item &it2,
                           const DynamicContext::Ptr &context) const;

   const AtomicComparator::Ptr &comparator() const {
      return m_comparator;
   }

   void useCaseInsensitiveComparator() {
      m_comparator = AtomicComparator::Ptr(new CaseInsensitiveStringComparator());
   }

 private:
   AtomicComparator::Operator operatorID() const {
      Q_ASSERT(static_cast<const TSubClass *>(this)->operatorID());
      return static_cast<const TSubClass *>(this)->operatorID();
   }

   AtomicComparator::Ptr m_comparator;
};

#include "qcomparisonplatform.cpp"

}

#endif
