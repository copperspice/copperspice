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

#ifndef QAtomicCasters_P_H
#define QAtomicCasters_P_H

#include <qatomiccaster_p.h>
#include <qdecimal_p.h>
#include <qderivedinteger_p.h>
#include <qderivedstring_p.h>
#include <qinteger_p.h>
#include <qvalidationerror_p.h>

namespace QPatternist {

template<TypeOfDerivedString DerivedType>
class ToStringCaster : public AtomicCaster
{
 public:
   Item castFrom(const Item &from, const QExplicitlySharedDataPointer<DynamicContext> &context) const override {
      Q_ASSERT(from);
      return DerivedString<DerivedType>::fromLexical(context->namePool(), from.stringValue());
   }
};


class ToUntypedAtomicCaster : public AtomicCaster
{
 public:
   Item castFrom(const Item &from, const QExplicitlySharedDataPointer<DynamicContext> &context) const override;
};

class ToAnyURICaster : public AtomicCaster
{
 public:
   Item castFrom(const Item &from, const QExplicitlySharedDataPointer<DynamicContext> &context) const override;
};

class HexBinaryToBase64BinaryCaster : public AtomicCaster
{
 public:
   Item castFrom(const Item &from, const QExplicitlySharedDataPointer<DynamicContext> &context) const override;
};


class Base64BinaryToHexBinaryCaster : public AtomicCaster
{
 public:
   Item castFrom(const Item &from, const QExplicitlySharedDataPointer<DynamicContext> &context) const override;
};


class StringToBase64BinaryCaster : public AtomicCaster
{
 public:
   Item castFrom(const Item &from, const QExplicitlySharedDataPointer<DynamicContext> &context) const override;
};


class StringToHexBinaryCaster : public AtomicCaster
{
 public:
   Item castFrom(const Item &from, const QExplicitlySharedDataPointer<DynamicContext> &context) const override;
};


class NumericToBooleanCaster : public AtomicCaster
{
 public:
   Item castFrom(const Item &from, const QExplicitlySharedDataPointer<DynamicContext> &context) const override;
};


class StringToBooleanCaster : public AtomicCaster
{
 public:
   Item castFrom(const Item &from, const QExplicitlySharedDataPointer<DynamicContext> &context) const override;
};


template <const bool IsInteger>
class NumericToDecimalCaster : public AtomicCaster
{
 public:
   /**
    * Used by NumericToDerivedIntegerCaster in addition to this class.
    */
   static inline QString errorMessage() {
      return QtXmlPatterns::tr("When casting to %1 from %2, the source value cannot be %3.");
   }

   Item castFrom(const Item &from, const QExplicitlySharedDataPointer<DynamicContext> &context) const override {
      const ItemType::Ptr t(from.type());
      const Numeric *const num = from.template as<Numeric>();

      if (BuiltinTypes::xsDouble->xdtTypeMatches(t) || BuiltinTypes::xsFloat->xdtTypeMatches(t)) {
         if (num->isInf() || num->isNaN()) {

            return ValidationError::createError(errorMessage()
                  .formatArg(formatType(context->namePool(), IsInteger ? BuiltinTypes::xsInteger : BuiltinTypes::xsDecimal))
                  .formatArg(formatType(context->namePool(), t))
                  .formatArg(formatData(num->stringValue())), ReportContext::FOCA0002);
         }
      }

      if (IsInteger) {
         return Integer::fromValue(num->toInteger());
      } else {
         return toItem(Decimal::fromValue(num->toDecimal()));
      }
   }
};


class StringToDecimalCaster : public AtomicCaster
{
 public:
   Item castFrom(const Item &from, const QExplicitlySharedDataPointer<DynamicContext> &context) const override;
};


class StringToIntegerCaster : public AtomicCaster
{
 public:
   Item castFrom(const Item &from, const QExplicitlySharedDataPointer<DynamicContext> &context) const override;
};


class BooleanToDecimalCaster : public AtomicCaster
{
 public:
   Item castFrom(const Item &from, const QExplicitlySharedDataPointer<DynamicContext> &context) const override;
};


class BooleanToIntegerCaster : public AtomicCaster
{
 public:
   Item castFrom(const Item &from, const QExplicitlySharedDataPointer<DynamicContext> &context) const override;
};


class SelfToSelfCaster : public AtomicCaster
{
 public:

   /**
    * This function simply returns @p from.
    */
   Item castFrom(const Item &from, const QExplicitlySharedDataPointer<DynamicContext> &context) const override;
};


class StringToGYearCaster : public AtomicCaster
{
 public:
   Item castFrom(const Item &from, const QExplicitlySharedDataPointer<DynamicContext> &context) const override;
};


class StringToGDayCaster : public AtomicCaster
{
 public:
   Item castFrom(const Item &from, const QExplicitlySharedDataPointer<DynamicContext> &context) const override;
};

class StringToGMonthCaster : public AtomicCaster
{
 public:
   Item castFrom(const Item &from, const QExplicitlySharedDataPointer<DynamicContext> &context) const override;
};


class StringToGYearMonthCaster : public AtomicCaster
{
 public:
   Item castFrom(const Item &from, const QExplicitlySharedDataPointer<DynamicContext> &context) const override;
};


class StringToGMonthDayCaster : public AtomicCaster
{
 public:
   Item castFrom(const Item &from, const QExplicitlySharedDataPointer<DynamicContext> &context) const override;
};


class StringToDateTimeCaster : public AtomicCaster
{
 public:
   Item castFrom(const Item &from, const QExplicitlySharedDataPointer<DynamicContext> &context) const override;
};


class StringToTimeCaster : public AtomicCaster
{
 public:
   Item castFrom(const Item &from, const QExplicitlySharedDataPointer<DynamicContext> &context) const override;
};


class StringToDateCaster : public AtomicCaster
{
 public:
   Item castFrom(const Item &from, const QExplicitlySharedDataPointer<DynamicContext> &context) const override;
};


class StringToDurationCaster : public AtomicCaster
{
 public:
   Item castFrom(const Item &from, const QExplicitlySharedDataPointer<DynamicContext> &context) const override;
};


class StringToDayTimeDurationCaster : public AtomicCaster
{
 public:
   Item castFrom(const Item &from, const QExplicitlySharedDataPointer<DynamicContext> &context) const override;
};


class StringToYearMonthDurationCaster : public AtomicCaster
{
 public:
   Item castFrom(const Item &from, const QExplicitlySharedDataPointer<DynamicContext> &context) const override;
};


class AbstractDateTimeToGYearCaster : public AtomicCaster
{
 public:
   Item castFrom(const Item &from, const QExplicitlySharedDataPointer<DynamicContext> &context) const override;
};

class AbstractDateTimeToGYearMonthCaster : public AtomicCaster
{
 public:
   Item castFrom(const Item &from, const QExplicitlySharedDataPointer<DynamicContext> &context) const override;
};


class AbstractDateTimeToGMonthCaster : public AtomicCaster
{
 public:
   Item castFrom(const Item &from, const QExplicitlySharedDataPointer<DynamicContext> &context) const override;
};


class AbstractDateTimeToGMonthDayCaster : public AtomicCaster
{
 public:
   Item castFrom(const Item &from, const QExplicitlySharedDataPointer<DynamicContext> &context) const override;
};


class AbstractDateTimeToGDayCaster : public AtomicCaster
{
 public:
   Item castFrom(const Item &from, const QExplicitlySharedDataPointer<DynamicContext> &context) const override;
};


class AbstractDateTimeToDateTimeCaster : public AtomicCaster
{
 public:
   Item castFrom(const Item &from, const QExplicitlySharedDataPointer<DynamicContext> &context) const override;
};


class AbstractDateTimeToDateCaster : public AtomicCaster
{
 public:
   Item castFrom(const Item &from, const QExplicitlySharedDataPointer<DynamicContext> &context) const override;
};

class AbstractDateTimeToTimeCaster : public AtomicCaster
{
 public:
   Item castFrom(const Item &from, const QExplicitlySharedDataPointer<DynamicContext> &context) const override;
};

class AbstractDurationToDurationCaster : public AtomicCaster
{
 public:
   Item castFrom(const Item &from, const QExplicitlySharedDataPointer<DynamicContext> &context) const override;
};


class AbstractDurationToDayTimeDurationCaster : public AtomicCaster
{
 public:
   Item castFrom(const Item &from, const QExplicitlySharedDataPointer<DynamicContext> &context) const override;
};


class AbstractDurationToYearMonthDurationCaster : public AtomicCaster
{
 public:
   Item castFrom(const Item &from, const QExplicitlySharedDataPointer<DynamicContext> &context) const override;
};

template<TypeOfDerivedInteger type>
class StringToDerivedIntegerCaster : public AtomicCaster
{
 public:
   Item castFrom(const Item &from, const QExplicitlySharedDataPointer<DynamicContext> &context) const override {
      return DerivedInteger<type>::fromLexical(context->namePool(), from.stringValue());
   }
};


template<TypeOfDerivedInteger type>
class BooleanToDerivedIntegerCaster : public AtomicCaster
{
 public:
   Item castFrom(const Item &from, const QExplicitlySharedDataPointer<DynamicContext> &context) const  override{
      return DerivedInteger<type>::fromValue(context->namePool(),
                  from.template as<AtomicValue>()->evaluateEBV(context) ? 1 : 0);
   }
};


template<TypeOfDerivedString type>
class AnyToDerivedStringCaster : public AtomicCaster
{
 public:
   Item castFrom(const Item &from, const QExplicitlySharedDataPointer<DynamicContext> &context) const override {
      return DerivedString<type>::fromLexical(context->namePool(), from.stringValue());
   }
};

template<TypeOfDerivedInteger type>
class NumericToDerivedIntegerCaster : public AtomicCaster
{
 public:
   Item castFrom(const Item &from, const QExplicitlySharedDataPointer<DynamicContext> &context) const override {
      const ItemType::Ptr t(from.type());
      const Numeric *const num = from.template as<Numeric>();

      if (BuiltinTypes::xsDouble->xdtTypeMatches(t) || BuiltinTypes::xsFloat->xdtTypeMatches(t)) {
         if (num->isInf() || num->isNaN()) {
            return ValidationError::createError(NumericToDecimalCaster<false>::errorMessage()
                                                .formatArg(formatType(context->namePool(), DerivedInteger<type>::itemType()))
                                                .formatArg(formatType(context->namePool(), t))
                                                .formatArg(formatData(num->stringValue())), ReportContext::FOCA0002);
         }
      }

      return toItem(DerivedInteger<type>::fromValue(context->namePool(), from.template as<Numeric>()->toInteger()));
   }
};
}

#endif
