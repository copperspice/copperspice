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

#include "qatomiccaster_p.h"
#include "qatomicstring_p.h"
#include "qcastingplatform_p.h"
#include "qvaluefactory_p.h"

using namespace QPatternist;

/**
 * @short Helper class for ValueFactory::fromLexical() which exposes
 * CastingPlatform appropriately.
 *
 * @relates ValueFactory
 */
class PerformValueConstruction : public CastingPlatform<PerformValueConstruction, false>
   , public SourceLocationReflection
{
 public:
   PerformValueConstruction(const SourceLocationReflection *const sourceLocationReflection,
                            const SchemaType::Ptr &toType) : m_sourceReflection(sourceLocationReflection)
      , m_targetType(AtomicType::Ptr(toType)) {
      Q_ASSERT(m_sourceReflection);
   }

   AtomicValue::Ptr operator()(const AtomicValue::Ptr &lexicalValue,
                               const SchemaType::Ptr & /*type*/,
                               const ReportContext::Ptr &context) {
      prepareCasting(context, BuiltinTypes::xsString);
      return AtomicValue::Ptr(const_cast<AtomicValue *>(cast(lexicalValue, context).asAtomicValue()));
   }

   const SourceLocationReflection *actualReflection() const  override {
      return m_sourceReflection;
   }

   ItemType::Ptr targetType() const {
      return m_targetType;
   }

 private:
   const SourceLocationReflection *const m_sourceReflection;
   const ItemType::Ptr                   m_targetType;
};

AtomicValue::Ptr ValueFactory::fromLexical(const QString &lexicalValue,
      const SchemaType::Ptr &type,
      const ReportContext::Ptr &context,
      const SourceLocationReflection *const sourceLocationReflection)
{
   Q_ASSERT(context);
   Q_ASSERT(type);
   Q_ASSERT_X(type->category() == SchemaType::SimpleTypeAtomic, Q_FUNC_INFO,
              "We can only construct for atomic values.");

   return PerformValueConstruction(sourceLocationReflection, type)(AtomicString::fromValue(lexicalValue),
          type,
          context);
}
