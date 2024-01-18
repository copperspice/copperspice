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

#ifndef QCastingPlatform_P_H
#define QCastingPlatform_P_H

#include <qatomiccasterlocator_p.h>
#include <qatomiccaster_p.h>
#include <qatomicstring_p.h>
#include <qatomictype_p.h>
#include <qbuiltintypes_p.h>
#include <qcommonsequencetypes_p.h>
#include <qpatternistlocale_p.h>
#include <qqnamevalue_p.h>
#include <qschematypefactory_p.h>
#include <qstaticcontext_p.h>
#include <qvalidationerror_p.h>

namespace QPatternist {

template<typename TSubClass, const bool issueError>
class CastingPlatform
{
 protected:

   inline CastingPlatform(const ReportContext::ErrorCode code = ReportContext::FORG0001) : m_errorCode(code) {
   }

   Item cast(const Item &sourceValue, const ReportContext::Ptr &context) const;

   bool prepareCasting(const ReportContext::Ptr &context, const ItemType::Ptr &sourceType);

   void checkTargetType(const ReportContext::Ptr &context) const;

 private:
   inline Item castWithCaster(const Item &sourceValue,
                              const AtomicCaster::Ptr &caster,
                              const ReportContext::Ptr &context) const;

   /**
    * Locates the caster for casting values of type @p sourceType to targetType(), if
    * possible.
    *
    * @p castImpossible is not initialized. Initialize it to @c false.
    */
   static AtomicCaster::Ptr locateCaster(const ItemType::Ptr &sourceType,
                                         const ReportContext::Ptr &context,
                                         bool &castImpossible,
                                         const SourceLocationReflection *const location,
                                         const ItemType::Ptr &targetType);
 private:
   inline Item castWithCaster(const Item &sourceValue,
                              const AtomicCaster::Ptr &caster,
                              const DynamicContext::Ptr &context) const;


   inline ItemType::Ptr targetType() const {
      Q_ASSERT(static_cast<const TSubClass *>(this)->targetType());
      return static_cast<const TSubClass *>(this)->targetType();
   }

   void issueCastError(const Item &validationError,
                       const Item &sourceValue,
                       const ReportContext::Ptr &context) const;

   CastingPlatform(const CastingPlatform &) = delete;
   CastingPlatform &operator=(const CastingPlatform &) = delete;

   AtomicCaster::Ptr m_caster;
   const ReportContext::ErrorCode m_errorCode;
};

#include "qcastingplatform.cpp"

}

#endif
