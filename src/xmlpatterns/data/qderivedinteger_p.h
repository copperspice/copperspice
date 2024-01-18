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

#ifndef QDerivedInteger_P_H
#define QDerivedInteger_P_H

#include <qbuiltintypes_p.h>
#include <qinteger_p.h>
#include <qpatternistlocale_p.h>
#include <qvalidationerror_p.h>

namespace QPatternist {

enum DerivedIntegerLimitsUsage {
   None            = 1,
   LimitUpwards    = 2,
   LimitDownwards  = 4,
   LimitBoth       = LimitUpwards | LimitDownwards
};

enum {
   IgnorableSignedValue = 0,
   IgnorableUnsignedValue = 0
};

template<TypeOfDerivedInteger DerivedType> class DerivedInteger;

template<TypeOfDerivedInteger DerivedType> class DerivedIntegerDetails;

template<>
class DerivedIntegerDetails<TypeByte>
{
 private:
   friend class DerivedInteger<TypeByte>;
   typedef qint8                           StorageType;
   typedef xsInteger                       TemporaryStorageType;
   static const StorageType                maxInclusive = 127;
   static const StorageType                minInclusive = -128;
   static const DerivedIntegerLimitsUsage  limitsUsage = LimitBoth;

   /**
    * Disable the default constructor.
    */
   DerivedIntegerDetails() {}

   DerivedIntegerDetails(const DerivedIntegerDetails &) = delete;
   DerivedIntegerDetails &operator=(const DerivedIntegerDetails &) = delete;
};

template<>
class DerivedIntegerDetails<TypeInt>
{
 private:
   friend class DerivedInteger<TypeInt>;
   typedef qint32                          StorageType;
   typedef xsInteger                       TemporaryStorageType;
   static const StorageType                maxInclusive = Q_INT64_C(2147483647);
   static const StorageType                minInclusive = Q_INT64_C(-2147483648);
   static const DerivedIntegerLimitsUsage  limitsUsage = LimitBoth;

   /**
    * Disable the default constructor.
    */
   DerivedIntegerDetails() {}

   DerivedIntegerDetails(const DerivedIntegerDetails &) = delete;
   DerivedIntegerDetails &operator=(const DerivedIntegerDetails &) = delete;
};

template<>
class DerivedIntegerDetails<TypeLong>
{
 private:
   friend class DerivedInteger<TypeLong>;
   typedef qint64                          StorageType;
   typedef StorageType                     TemporaryStorageType;
   static const StorageType                maxInclusive = Q_INT64_C(9223372036854775807);

   /**
    * This messy arithmetic expression ensures that we don't get a warning
    * on neither GCC nor MSVC.
    */
   static const StorageType                minInclusive = -(Q_INT64_C(9223372036854775807)) - 1;

   static const DerivedIntegerLimitsUsage  limitsUsage = LimitBoth;

   /**
    * Disable the default constructor.
    */
   DerivedIntegerDetails() {}

   DerivedIntegerDetails(const DerivedIntegerDetails &) = delete;
   DerivedIntegerDetails &operator=(const DerivedIntegerDetails &) = delete;
};

template<>
class DerivedIntegerDetails<TypeNegativeInteger>
{
 private:
   friend class DerivedInteger<TypeNegativeInteger>;
   typedef xsInteger                       StorageType;
   typedef StorageType                     TemporaryStorageType;
   static const StorageType                maxInclusive = -1;
   static const StorageType                minInclusive = IgnorableSignedValue;
   static const DerivedIntegerLimitsUsage  limitsUsage = LimitUpwards;

   /**
    * Disable the default constructor.
    */
   DerivedIntegerDetails() {}

   DerivedIntegerDetails(const DerivedIntegerDetails &) = delete;
   DerivedIntegerDetails &operator=(const DerivedIntegerDetails &) = delete;
};

template<>
class DerivedIntegerDetails<TypeNonNegativeInteger>
{
 private:
   friend class DerivedInteger<TypeNonNegativeInteger>;
   typedef xsInteger                       StorageType;
   typedef StorageType                     TemporaryStorageType;
   static const StorageType                maxInclusive = IgnorableSignedValue;
   static const StorageType                minInclusive = 0;
   static const DerivedIntegerLimitsUsage  limitsUsage = LimitDownwards;

   /**
    * Disable the default constructor.
    */
   DerivedIntegerDetails() {}

   DerivedIntegerDetails(const DerivedIntegerDetails &) = delete;
   DerivedIntegerDetails &operator=(const DerivedIntegerDetails &) = delete;
};

template<>
class DerivedIntegerDetails<TypeNonPositiveInteger>
{
 private:
   friend class DerivedInteger<TypeNonPositiveInteger>;
   typedef xsInteger                       StorageType;
   typedef StorageType                     TemporaryStorageType;
   static const StorageType                maxInclusive = 0;
   static const StorageType                minInclusive = IgnorableSignedValue;
   static const DerivedIntegerLimitsUsage  limitsUsage = LimitUpwards;

   /**
    * Disable the default constructor.
    */
   DerivedIntegerDetails() {}

   DerivedIntegerDetails(const DerivedIntegerDetails &) = delete;
   DerivedIntegerDetails &operator=(const DerivedIntegerDetails &) = delete;
};

template<>
class DerivedIntegerDetails<TypePositiveInteger>
{
 private:
   friend class DerivedInteger<TypePositiveInteger>;
   typedef xsInteger                       StorageType;
   typedef StorageType                     TemporaryStorageType;
   static const StorageType                maxInclusive = IgnorableSignedValue;
   static const StorageType                minInclusive = 1;
   static const DerivedIntegerLimitsUsage  limitsUsage = LimitDownwards;

   /**
    * Disable the default constructor.
    */
   DerivedIntegerDetails() {}

   DerivedIntegerDetails(const DerivedIntegerDetails &) = delete;
   DerivedIntegerDetails &operator=(const DerivedIntegerDetails &) = delete;
};

template<>
class DerivedIntegerDetails<TypeShort>
{
 private:
   friend class DerivedInteger<TypeShort>;
   typedef qint16                          StorageType;
   typedef xsInteger                       TemporaryStorageType;
   static const StorageType                maxInclusive = 32767;
   static const StorageType                minInclusive = -32768;
   static const DerivedIntegerLimitsUsage  limitsUsage = LimitBoth;

   /**
    * Disable the default constructor.
    */
   DerivedIntegerDetails() {}

   DerivedIntegerDetails(const DerivedIntegerDetails &) = delete;
   DerivedIntegerDetails &operator=(const DerivedIntegerDetails &) = delete;
};

template<>
class DerivedIntegerDetails<TypeUnsignedByte>
{
 private:
   friend class DerivedInteger<TypeUnsignedByte>;
   typedef quint8                          StorageType;
   typedef qint64                          TemporaryStorageType;
   static const StorageType                maxInclusive = 255;
   static const StorageType                minInclusive = 0;
   static const DerivedIntegerLimitsUsage  limitsUsage = LimitBoth;

   /**
    * Disable the default constructor.
    */
   DerivedIntegerDetails() {}

   DerivedIntegerDetails(const DerivedIntegerDetails &) = delete;
   DerivedIntegerDetails &operator=(const DerivedIntegerDetails &) = delete;
};

template<>
class DerivedIntegerDetails<TypeUnsignedInt>
{
 private:
   friend class DerivedInteger<TypeUnsignedInt>;
   typedef quint32                         StorageType;
   typedef qint64                          TemporaryStorageType;
   static const StorageType                maxInclusive = Q_UINT64_C(4294967295);
   static const StorageType                minInclusive = 0;
   static const DerivedIntegerLimitsUsage  limitsUsage = LimitBoth;

   /**
    * Disable the default constructor.
    */
   DerivedIntegerDetails() {}

   DerivedIntegerDetails(const DerivedIntegerDetails &) = delete;
   DerivedIntegerDetails &operator=(const DerivedIntegerDetails &) = delete;
};

template<>
class DerivedIntegerDetails<TypeUnsignedLong>
{
 private:
   friend class DerivedInteger<TypeUnsignedLong>;
   typedef quint64                         StorageType;
   typedef StorageType                     TemporaryStorageType;
   static const StorageType                maxInclusive = Q_UINT64_C(18446744073709551615);
   static const StorageType                minInclusive = 0;
   static const DerivedIntegerLimitsUsage  limitsUsage = LimitBoth;

   /**
    * Disable the default constructor.
    */
   DerivedIntegerDetails() {}

   DerivedIntegerDetails(const DerivedIntegerDetails &) = delete;
   DerivedIntegerDetails &operator=(const DerivedIntegerDetails &) = delete;
};

template<>
class DerivedIntegerDetails<TypeUnsignedShort>
{
 private:
   friend class DerivedInteger<TypeUnsignedShort>;
   typedef quint16                         StorageType;
   typedef qint64                          TemporaryStorageType;
   static const StorageType                maxInclusive = 65535;
   static const StorageType                minInclusive = 0;
   static const DerivedIntegerLimitsUsage  limitsUsage = LimitBoth;

   /**
    * Disable the default constructor.
    */
   DerivedIntegerDetails() {}

   DerivedIntegerDetails(const DerivedIntegerDetails &) = delete;
   DerivedIntegerDetails &operator=(const DerivedIntegerDetails &) = delete;
};

template<TypeOfDerivedInteger DerivedType>
class DerivedInteger : public Numeric
{
 private:
   typedef typename DerivedIntegerDetails<DerivedType>::StorageType StorageType;
   typedef typename DerivedIntegerDetails<DerivedType>::TemporaryStorageType TemporaryStorageType;

   static const StorageType                maxInclusive        = DerivedIntegerDetails<DerivedType>::maxInclusive;
   static const StorageType                minInclusive        = DerivedIntegerDetails<DerivedType>::minInclusive;
   static const DerivedIntegerLimitsUsage  limitsUsage         = DerivedIntegerDetails<DerivedType>::limitsUsage;

   const StorageType m_value;

   inline DerivedInteger(const StorageType num) : m_value(num) {
   }

   /**
    * By refactoring out the simple comparison below into a template
    * function, we avoid the warning "warning: comparison of unsigned expression < 0 is always false" with gcc
    * when the class is instantiated with TypeUnsignedLong. The warning is
    * a false positive since we check wehther LimitUpwards is set before
    * instantiating.
    *
    * This template function exists for no other reason. */
   template<typename A, typename B>
   static bool lessThan(const A &a, const B &b) {
      return a < b;
   }

   /**
    * This function exists for the same reason that lessThan() do.
    */
   template<typename A, typename B>
   static bool largerOrEqual(const A &a, const B &b) {
      return qint64(a) >= b;
   }

 public:

   static ItemType::Ptr itemType() {
      switch (DerivedType) {
         case TypeByte:
            return BuiltinTypes::xsByte;
         case TypeInt:
            return BuiltinTypes::xsInt;
         case TypeLong:
            return BuiltinTypes::xsLong;
         case TypeNegativeInteger:
            return BuiltinTypes::xsNegativeInteger;
         case TypeNonNegativeInteger:
            return BuiltinTypes::xsNonNegativeInteger;
         case TypeNonPositiveInteger:
            return BuiltinTypes::xsNonPositiveInteger;
         case TypePositiveInteger:
            return BuiltinTypes::xsPositiveInteger;
         case TypeShort:
            return BuiltinTypes::xsShort;
         case TypeUnsignedByte:
            return BuiltinTypes::xsUnsignedByte;
         case TypeUnsignedInt:
            return BuiltinTypes::xsUnsignedInt;
         case TypeUnsignedLong:
            return BuiltinTypes::xsUnsignedLong;
         case TypeUnsignedShort:
            return BuiltinTypes::xsUnsignedShort;
      }

      Q_ASSERT(false);
      return ItemType::Ptr();
   }

   static AtomicValue::Ptr fromValue(const NamePool::Ptr &np, const TemporaryStorageType num) {
      /* If we use minInclusive when calling lessThan(), we for some
       * reason get a linker error with GCC. Using this temporary
       * variable solves it. */
      const StorageType minimum = minInclusive;

      if ((limitsUsage & LimitUpwards) && num > maxInclusive) {

         return ValidationError::createError(QtXmlPatterns::tr("Value %1 of type %2 exceeds maximum (%3).")
                                             .formatArg(QPatternist::formatData(static_cast<xsInteger>(num)))
                                             .formatArg(formatType(np, itemType()))
                                             .formatArg(QPatternist::formatData(static_cast<xsInteger>(maxInclusive))));

      } else if ((limitsUsage & LimitDownwards) && lessThan(num, minimum)) {

         return ValidationError::createError(QtXmlPatterns::tr("Value %1 of type %2 is below minimum (%3).")
                                             .formatArg(QPatternist::formatData(static_cast<xsInteger>(num)))
                                             .formatArg(formatType(np, itemType()))
                                             .formatArg(QPatternist::formatData(static_cast<xsInteger>(minInclusive))));
      } else {
         return AtomicValue::Ptr(new DerivedInteger(num));
      }
   }

   static AtomicValue::Ptr fromValueUnchecked(const TemporaryStorageType num) {
      return AtomicValue::Ptr(new DerivedInteger(num));
   }

   /**
    * Constructs an instance from the lexical
    * representation @p strNumeric.
    */
   static AtomicValue::Ptr fromLexical(const NamePool::Ptr &np, const QString &strNumeric) {
      bool conversionOk = false;
      TemporaryStorageType num;

      /* Depending on the type, we need to call different conversion functions on QString. */
      switch (DerivedType) {
         case TypeUnsignedLong: {

            /* flag '-' as invalid, so remove it before. */
            if (strNumeric.contains(QLatin1Char('-'))) {
               num = QString(strNumeric).remove('-').toInteger<quint64>(&conversionOk);

               if (num != 0) {
                  conversionOk = false;
               }
            } else {
               num = strNumeric.toInteger<quint64>(&conversionOk);
            }

            break;
         }

         default: {
            num = strNumeric.toInteger<qint64>(&conversionOk);
            break;
         }
      }

      if (conversionOk) {
         return fromValue(np, num);
      } else {
         return ValidationError::createError();
      }
   }

   inline StorageType storedValue() const {
      return m_value;
   }

   /**
    * Determines the Effective %Boolean Value of this number.
    *
    * @returns @c false if the number is 0, otherwise @c true.
    */
   bool evaluateEBV(const QExplicitlySharedDataPointer<DynamicContext> &) const override {
      return m_value != 0;
   }

   QString stringValue() const override {
      return QString::number(m_value);
   }

   ItemType::Ptr type() const override {
      return itemType();
   }

   xsDouble toDouble() const override {
      return static_cast<xsDouble>(m_value);
   }

   xsInteger toInteger() const override {
      return m_value;
   }

   xsFloat toFloat() const override {
      return static_cast<xsFloat>(m_value);
   }

   xsDecimal toDecimal() const override {
      return static_cast<xsDecimal>(m_value);
   }

   Numeric::Ptr round() const override {
      /* xs:integerS never have a mantissa. */
      return Numeric::Ptr(static_cast<Numeric *>(const_cast<AtomicValue *>(Integer::fromValue(m_value).asAtomicValue())));
   }

   Numeric::Ptr roundHalfToEven(const xsInteger) const override {
      return Numeric::Ptr(static_cast<Numeric *>(const_cast<AtomicValue *>(Integer::fromValue(m_value).asAtomicValue())));
   }

   Numeric::Ptr floor() const override {
      return Numeric::Ptr(static_cast<Numeric *>(const_cast<AtomicValue *>(Integer::fromValue(m_value).asAtomicValue())));
   }

   Numeric::Ptr ceiling() const override{
      return Numeric::Ptr(static_cast<Numeric *>(const_cast<AtomicValue *>(Integer::fromValue(m_value).asAtomicValue())));
   }

   Numeric::Ptr abs() const override {
      /* We unconditionally create an Integer even if we're a positive
       * value, because one part of this is the type change to
       * xs:integer.
       *
       * We've manually inlined qAbs() and invoke xsInteger's
       * constructor. The reason being that we other gets truncation down
       * to StorageType. See for instance XQTS test case absint1args-1. */
      return Numeric::Ptr(static_cast<Numeric *>(const_cast<AtomicValue *>(Integer::fromValue(largerOrEqual(m_value,
                          0) ? xsInteger(m_value) : -xsInteger(m_value)).asAtomicValue())));
   }

   /**
    * @returns always @c false, @c xs:DerivedInteger doesn't have
    * not-a-number in its value space.
    */
   bool isNaN() const override {
      return false;
   }

   /**
    * @returns always @c false, @c xs:DerivedInteger doesn't have
    * infinity in its value space.
    */
   bool isInf() const override {
      return false;
   }

   Item toNegated() const override {
      return Integer::fromValue(-xsInteger(m_value));
   }

   bool isSigned() const override {
      switch (DerivedType) {
         case TypeByte:
         case TypeInt:
         case TypeLong:
         case TypeNegativeInteger:
         case TypeNonNegativeInteger:
         case TypeNonPositiveInteger:
         case TypePositiveInteger:
         case TypeShort:
            return true;

         case TypeUnsignedByte:
         case TypeUnsignedInt:
         case TypeUnsignedLong:
         case TypeUnsignedShort:
            return false;
      }
      return false;
   }

   quint64 toUnsignedInteger() const override {
      switch (DerivedType) {

         case TypeByte:
         case TypeInt:
         case TypeLong:
         case TypeNegativeInteger:
         case TypeNonNegativeInteger:
         case TypeNonPositiveInteger:
         case TypePositiveInteger:

         case TypeShort:
            Q_ASSERT_X(false, Q_FUNC_INFO, "This function should never be called.");
            break;

         case TypeUnsignedByte:
         case TypeUnsignedInt:
         case TypeUnsignedLong:
         case TypeUnsignedShort:
            return m_value;
      }
      return 0;
   }
};

}  // namespace

#endif
