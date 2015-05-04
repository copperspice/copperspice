/***********************************************************************
*
* Copyright (c) 2012-2015 Barbara Geller
* Copyright (c) 2012-2015 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or 
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see 
* <http://www.gnu.org/licenses/>.
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

QT_BEGIN_NAMESPACE

namespace QPatternist {

template<TypeOfDerivedString DerivedType>
class ToStringCaster : public AtomicCaster
{
 public:
   virtual Item castFrom(const Item &from,
                         const QExplicitlySharedDataPointer<DynamicContext> &context) const {
      Q_ASSERT(from);
      return DerivedString<DerivedType>::fromLexical(context->namePool(), from.stringValue());
   }
};


class ToUntypedAtomicCaster : public AtomicCaster
{
 public:
   virtual Item castFrom(const Item &from,
                         const QExplicitlySharedDataPointer<DynamicContext> &context) const;
};

class ToAnyURICaster : public AtomicCaster
{
 public:
   virtual Item castFrom(const Item &from,
                         const QExplicitlySharedDataPointer<DynamicContext> &context) const;
};

class HexBinaryToBase64BinaryCaster : public AtomicCaster
{
 public:
   virtual Item castFrom(const Item &from,
                         const QExplicitlySharedDataPointer<DynamicContext> &context) const;
};


class Base64BinaryToHexBinaryCaster : public AtomicCaster
{
 public:
   virtual Item castFrom(const Item &from,
                         const QExplicitlySharedDataPointer<DynamicContext> &context) const;
};


class StringToBase64BinaryCaster : public AtomicCaster
{
 public:
   virtual Item castFrom(const Item &from,
                         const QExplicitlySharedDataPointer<DynamicContext> &context) const;
};


class StringToHexBinaryCaster : public AtomicCaster
{
 public:
   virtual Item castFrom(const Item &from,
                         const QExplicitlySharedDataPointer<DynamicContext> &context) const;
};


class NumericToBooleanCaster : public AtomicCaster
{
 public:
   virtual Item castFrom(const Item &from,
                         const QExplicitlySharedDataPointer<DynamicContext> &context) const;
};


class StringToBooleanCaster : public AtomicCaster
{
 public:
   virtual Item castFrom(const Item &from,
                         const QExplicitlySharedDataPointer<DynamicContext> &context) const;
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

   virtual Item castFrom(const Item &from,
                         const QExplicitlySharedDataPointer<DynamicContext> &context) const {
      const ItemType::Ptr t(from.type());
      const Numeric *const num = from.template as<Numeric>();

      if (BuiltinTypes::xsDouble->xdtTypeMatches(t) || BuiltinTypes::xsFloat->xdtTypeMatches(t)) {
         if (num->isInf() || num->isNaN()) {
            return ValidationError::createError(errorMessage()
                                                .arg(formatType(context->namePool(), IsInteger ? BuiltinTypes::xsInteger : BuiltinTypes::xsDecimal))
                                                .arg(formatType(context->namePool(), t))
                                                .arg(formatData(num->stringValue())),
                                                ReportContext::FOCA0002);
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
   virtual Item castFrom(const Item &from,
                         const QExplicitlySharedDataPointer<DynamicContext> &context) const;
};


class StringToIntegerCaster : public AtomicCaster
{
 public:
   virtual Item castFrom(const Item &from,
                         const QExplicitlySharedDataPointer<DynamicContext> &context) const;
};


class BooleanToDecimalCaster : public AtomicCaster
{
 public:
   virtual Item castFrom(const Item &from,
                         const QExplicitlySharedDataPointer<DynamicContext> &context) const;
};


class BooleanToIntegerCaster : public AtomicCaster
{
 public:
   virtual Item castFrom(const Item &from,
                         const QExplicitlySharedDataPointer<DynamicContext> &context) const;
};


class SelfToSelfCaster : public AtomicCaster
{
 public:

   /**
    * This function simply returns @p from.
    */
   virtual Item castFrom(const Item &from,
                         const QExplicitlySharedDataPointer<DynamicContext> &context) const;
};


class StringToGYearCaster : public AtomicCaster
{
 public:
   virtual Item castFrom(const Item &from,
                         const QExplicitlySharedDataPointer<DynamicContext> &context) const;
};


class StringToGDayCaster : public AtomicCaster
{
 public:
   virtual Item castFrom(const Item &from,
                         const QExplicitlySharedDataPointer<DynamicContext> &context) const;
};

class StringToGMonthCaster : public AtomicCaster
{
 public:
   virtual Item castFrom(const Item &from,
                         const QExplicitlySharedDataPointer<DynamicContext> &context) const;
};


class StringToGYearMonthCaster : public AtomicCaster
{
 public:
   virtual Item castFrom(const Item &from,
                         const QExplicitlySharedDataPointer<DynamicContext> &context) const;
};


class StringToGMonthDayCaster : public AtomicCaster
{
 public:
   virtual Item castFrom(const Item &from,
                         const QExplicitlySharedDataPointer<DynamicContext> &context) const;
};


class StringToDateTimeCaster : public AtomicCaster
{
 public:
   virtual Item castFrom(const Item &from,
                         const QExplicitlySharedDataPointer<DynamicContext> &context) const;
};


class StringToTimeCaster : public AtomicCaster
{
 public:
   virtual Item castFrom(const Item &from,
                         const QExplicitlySharedDataPointer<DynamicContext> &context) const;
};


class StringToDateCaster : public AtomicCaster
{
 public:
   virtual Item castFrom(const Item &from,
                         const QExplicitlySharedDataPointer<DynamicContext> &context) const;
};


class StringToDurationCaster : public AtomicCaster
{
 public:
   virtual Item castFrom(const Item &from,
                         const QExplicitlySharedDataPointer<DynamicContext> &context) const;
};


class StringToDayTimeDurationCaster : public AtomicCaster
{
 public:
   virtual Item castFrom(const Item &from,
                         const QExplicitlySharedDataPointer<DynamicContext> &context) const;
};


class StringToYearMonthDurationCaster : public AtomicCaster
{
 public:
   virtual Item castFrom(const Item &from,
                         const QExplicitlySharedDataPointer<DynamicContext> &context) const;
};


class AbstractDateTimeToGYearCaster : public AtomicCaster
{
 public:
   virtual Item castFrom(const Item &from,
                         const QExplicitlySharedDataPointer<DynamicContext> &context) const;
};

class AbstractDateTimeToGYearMonthCaster : public AtomicCaster
{
 public:
   virtual Item castFrom(const Item &from,
                         const QExplicitlySharedDataPointer<DynamicContext> &context) const;
};


class AbstractDateTimeToGMonthCaster : public AtomicCaster
{
 public:
   virtual Item castFrom(const Item &from,
                         const QExplicitlySharedDataPointer<DynamicContext> &context) const;
};


class AbstractDateTimeToGMonthDayCaster : public AtomicCaster
{
 public:
   virtual Item castFrom(const Item &from,
                         const QExplicitlySharedDataPointer<DynamicContext> &context) const;
};


class AbstractDateTimeToGDayCaster : public AtomicCaster
{
 public:
   virtual Item castFrom(const Item &from,
                         const QExplicitlySharedDataPointer<DynamicContext> &context) const;
};


class AbstractDateTimeToDateTimeCaster : public AtomicCaster
{
 public:
   virtual Item castFrom(const Item &from,
                         const QExplicitlySharedDataPointer<DynamicContext> &context) const;
};


class AbstractDateTimeToDateCaster : public AtomicCaster
{
 public:
   virtual Item castFrom(const Item &from,
                         const QExplicitlySharedDataPointer<DynamicContext> &context) const;
};

class AbstractDateTimeToTimeCaster : public AtomicCaster
{
 public:
   virtual Item castFrom(const Item &from,
                         const QExplicitlySharedDataPointer<DynamicContext> &context) const;
};

class AbstractDurationToDurationCaster : public AtomicCaster
{
 public:
   virtual Item castFrom(const Item &from,
                         const QExplicitlySharedDataPointer<DynamicContext> &context) const;
};


class AbstractDurationToDayTimeDurationCaster : public AtomicCaster
{
 public:
   virtual Item castFrom(const Item &from,
                         const QExplicitlySharedDataPointer<DynamicContext> &context) const;
};


class AbstractDurationToYearMonthDurationCaster : public AtomicCaster
{
 public:
   virtual Item castFrom(const Item &from,
                         const QExplicitlySharedDataPointer<DynamicContext> &context) const;
};


template<TypeOfDerivedInteger type>
class StringToDerivedIntegerCaster : public AtomicCaster
{
 public:
   virtual Item
   castFrom(const Item &from,
            const QExplicitlySharedDataPointer<DynamicContext> &context) const {
      return DerivedInteger<type>::fromLexical(context->namePool(), from.stringValue());
   }
};


template<TypeOfDerivedInteger type>
class BooleanToDerivedIntegerCaster : public AtomicCaster
{
 public:
   virtual Item
   castFrom(const Item &from,
            const QExplicitlySharedDataPointer<DynamicContext> &context) const {
      return DerivedInteger<type>::fromValue(context->namePool(),
                                             from.template as<AtomicValue>()->evaluateEBV(context) ? 1 : 0);
   }
};


template<TypeOfDerivedString type>
class AnyToDerivedStringCaster : public AtomicCaster
{
 public:
   virtual Item
   castFrom(const Item &from,
            const QExplicitlySharedDataPointer<DynamicContext> &context) const {
      return DerivedString<type>::fromLexical(context->namePool(), from.stringValue());
   }
};

template<TypeOfDerivedInteger type>
class NumericToDerivedIntegerCaster : public AtomicCaster
{
 public:
   virtual Item
   castFrom(const Item &from,
            const QExplicitlySharedDataPointer<DynamicContext> &context) const {
      const ItemType::Ptr t(from.type());
      const Numeric *const num = from.template as<Numeric>();

      if (BuiltinTypes::xsDouble->xdtTypeMatches(t) || BuiltinTypes::xsFloat->xdtTypeMatches(t)) {
         if (num->isInf() || num->isNaN()) {
            return ValidationError::createError(NumericToDecimalCaster<false>::errorMessage()
                                                .arg(formatType(context->namePool(), DerivedInteger<type>::itemType()))
                                                .arg(formatType(context->namePool(), t))
                                                .arg(formatData(num->stringValue())),
                                                ReportContext::FOCA0002);
         }
      }

      return toItem(DerivedInteger<type>::fromValue(context->namePool(), from.template as<Numeric>()->toInteger()));
   }
};
}

QT_END_NAMESPACE

#endif
