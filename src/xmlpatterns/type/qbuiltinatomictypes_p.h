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

#ifndef QBuiltinAtomicTypes_P_H
#define QBuiltinAtomicTypes_P_H

#include <qatomiccasterlocators_p.h>
#include <qatomiccomparatorlocators_p.h>
#include <qbuiltinatomictype_p.h>

QT_BEGIN_NAMESPACE

namespace QPatternist {

class AnyAtomicType : public BuiltinAtomicType
{
 public:
   typedef QExplicitlySharedDataPointer<AnyAtomicType> Ptr;

   virtual AtomicTypeVisitorResult::Ptr accept(const AtomicTypeVisitor::Ptr &visitor,
         const SourceLocationReflection *const reflection) const;
   virtual AtomicTypeVisitorResult::Ptr accept(const ParameterizedAtomicTypeVisitor::Ptr &visitor,
         const qint16 op,
         const SourceLocationReflection *const reflection) const;
   virtual QXmlName name(const NamePool::Ptr &np) const;
   virtual QString displayName(const NamePool::Ptr &np) const;

   /**
    * Overridden to return <tt>item()</tt>.
    *
    * @returns BuiltinTypes::item
    */
   virtual ItemType::Ptr xdtSuperType() const;

   /**
    * Overridden to return @c xs:anySimpleType.
    *
    * @returns BuiltinTypes::xsAnySimpleType
    */
   virtual SchemaType::Ptr wxsSuperType() const;

   /**
    * Overridden to return @c true, @c xs:anyAtomicType is abstract.
    *
    * @returns always @c true
    */
   virtual bool isAbstract() const;

 protected:
   friend class BuiltinTypes;
   AnyAtomicType();
};

class UntypedAtomicType : public BuiltinAtomicType
{
 public:
   typedef QExplicitlySharedDataPointer<UntypedAtomicType> Ptr;

   virtual AtomicTypeVisitorResult::Ptr accept(const AtomicTypeVisitor::Ptr &visitor,
         const SourceLocationReflection *const reflection) const;
   virtual AtomicTypeVisitorResult::Ptr accept(const ParameterizedAtomicTypeVisitor::Ptr &visitor,
         const qint16 op,
         const SourceLocationReflection *const reflection) const;
   virtual QXmlName name(const NamePool::Ptr &np) const;
   virtual QString displayName(const NamePool::Ptr &np) const;

 protected:
   friend class BuiltinTypes;
   UntypedAtomicType();
};

class DateTimeType : public BuiltinAtomicType
{
 public:
   typedef QExplicitlySharedDataPointer<DateTimeType> Ptr;

   virtual AtomicTypeVisitorResult::Ptr accept(const AtomicTypeVisitor::Ptr &visitor,
         const SourceLocationReflection *const reflection) const;
   virtual AtomicTypeVisitorResult::Ptr accept(const ParameterizedAtomicTypeVisitor::Ptr &visitor,
         const qint16 op,
         const SourceLocationReflection *const reflection) const;

   virtual QXmlName name(const NamePool::Ptr &np) const;
   virtual QString displayName(const NamePool::Ptr &np) const;
 protected:
   friend class BuiltinTypes;
   DateTimeType();
};

class DateType : public BuiltinAtomicType
{
 public:
   typedef QExplicitlySharedDataPointer<DateType> Ptr;

   virtual AtomicTypeVisitorResult::Ptr accept(const AtomicTypeVisitor::Ptr &visitor,
         const SourceLocationReflection *const reflection) const;
   virtual AtomicTypeVisitorResult::Ptr accept(const ParameterizedAtomicTypeVisitor::Ptr &visitor,
         const qint16 op,
         const SourceLocationReflection *const reflection) const;
   virtual QXmlName name(const NamePool::Ptr &np) const;
   virtual QString displayName(const NamePool::Ptr &np) const;

 protected:
   friend class BuiltinTypes;
   DateType();
};

class SchemaTimeType : public BuiltinAtomicType
{
 public:
   typedef QExplicitlySharedDataPointer<SchemaTimeType> Ptr;

   virtual AtomicTypeVisitorResult::Ptr accept(const AtomicTypeVisitor::Ptr &visitor,
         const SourceLocationReflection *const reflection) const;
   virtual AtomicTypeVisitorResult::Ptr accept(const ParameterizedAtomicTypeVisitor::Ptr &visitor,
         const qint16 op,
         const SourceLocationReflection *const reflection) const;

   virtual QXmlName name(const NamePool::Ptr &np) const;
   virtual QString displayName(const NamePool::Ptr &np) const;

 protected:
   friend class BuiltinTypes;
   SchemaTimeType();
};

class DurationType : public BuiltinAtomicType
{
 public:
   typedef QExplicitlySharedDataPointer<DurationType> Ptr;

   virtual AtomicTypeVisitorResult::Ptr accept(const AtomicTypeVisitor::Ptr &visitor,
         const SourceLocationReflection *const reflection) const;
   virtual AtomicTypeVisitorResult::Ptr accept(const ParameterizedAtomicTypeVisitor::Ptr &visitor,
         const qint16 op,
         const SourceLocationReflection *const reflection) const;
   virtual QXmlName name(const NamePool::Ptr &np) const;
   virtual QString displayName(const NamePool::Ptr &np) const;

 protected:
   friend class BuiltinTypes;
   DurationType();
};

class YearMonthDurationType : public BuiltinAtomicType
{
 public:
   typedef QExplicitlySharedDataPointer<YearMonthDurationType> Ptr;

   virtual AtomicTypeVisitorResult::Ptr accept(const AtomicTypeVisitor::Ptr &visitor,
         const SourceLocationReflection *const reflection) const;
   virtual AtomicTypeVisitorResult::Ptr accept(const ParameterizedAtomicTypeVisitor::Ptr &visitor,
         const qint16 op,
         const SourceLocationReflection *const reflection) const;
   virtual QXmlName name(const NamePool::Ptr &np) const;
   virtual QString displayName(const NamePool::Ptr &np) const;

 protected:
   friend class BuiltinTypes;
   YearMonthDurationType();
};

class DayTimeDurationType : public BuiltinAtomicType
{
 public:
   typedef QExplicitlySharedDataPointer<DayTimeDurationType> Ptr;

   virtual AtomicTypeVisitorResult::Ptr accept(const AtomicTypeVisitor::Ptr &visitor,
         const SourceLocationReflection *const reflection) const;
   virtual AtomicTypeVisitorResult::Ptr accept(const ParameterizedAtomicTypeVisitor::Ptr &visitor,
         const qint16 op,
         const SourceLocationReflection *const reflection) const;
   virtual QXmlName name(const NamePool::Ptr &np) const;
   virtual QString displayName(const NamePool::Ptr &np) const;

 protected:
   friend class BuiltinTypes;
   DayTimeDurationType();
};


class DoubleType : public BuiltinAtomicType
{
 public:
   typedef QExplicitlySharedDataPointer<DoubleType> Ptr;

   virtual AtomicTypeVisitorResult::Ptr accept(const AtomicTypeVisitor::Ptr &visitor,
         const SourceLocationReflection *const reflection) const;
   virtual AtomicTypeVisitorResult::Ptr accept(const ParameterizedAtomicTypeVisitor::Ptr &visitor,
         const qint16 op,
         const SourceLocationReflection *const reflection) const;
   virtual QXmlName name(const NamePool::Ptr &np) const;
   virtual QString displayName(const NamePool::Ptr &np) const;

 protected:
   friend class BuiltinTypes;
   DoubleType();
};

class FloatType : public BuiltinAtomicType
{
 public:
   typedef QExplicitlySharedDataPointer<FloatType> Ptr;

   virtual AtomicTypeVisitorResult::Ptr accept(const AtomicTypeVisitor::Ptr &visitor,
         const SourceLocationReflection *const reflection) const;
   virtual AtomicTypeVisitorResult::Ptr accept(const ParameterizedAtomicTypeVisitor::Ptr &visitor,
         const qint16 op,
         const SourceLocationReflection *const reflection) const;
   virtual QXmlName name(const NamePool::Ptr &np) const;
   virtual QString displayName(const NamePool::Ptr &np) const;

 protected:
   FloatType();
   friend class BuiltinTypes;
};


class DecimalType : public BuiltinAtomicType
{
 public:
   typedef QExplicitlySharedDataPointer<DecimalType> Ptr;

   virtual AtomicTypeVisitorResult::Ptr accept(const AtomicTypeVisitor::Ptr &visitor,
         const SourceLocationReflection *const reflection) const;
   virtual AtomicTypeVisitorResult::Ptr accept(const ParameterizedAtomicTypeVisitor::Ptr &visitor,
         const qint16 op,
         const SourceLocationReflection *const reflection) const;
   virtual QXmlName name(const NamePool::Ptr &np) const;
   virtual QString displayName(const NamePool::Ptr &np) const;

 protected:
   friend class BuiltinTypes;
   DecimalType();
};

class IntegerType : public BuiltinAtomicType
{
 public:
   typedef QExplicitlySharedDataPointer<IntegerType> Ptr;

   virtual AtomicTypeVisitorResult::Ptr accept(const AtomicTypeVisitor::Ptr &visitor,
         const SourceLocationReflection *const reflection) const;
   virtual AtomicTypeVisitorResult::Ptr accept(const ParameterizedAtomicTypeVisitor::Ptr &visitor,
         const qint16 op,
         const SourceLocationReflection *const reflection) const;
   virtual QXmlName name(const NamePool::Ptr &np) const;
   virtual QString displayName(const NamePool::Ptr &np) const;

 protected:
   friend class BuiltinTypes;
   IntegerType(const AtomicType::Ptr &parentType,
               const AtomicCasterLocator::Ptr &casterLocator);
};

template<TypeOfDerivedInteger derivedType>
class DerivedIntegerType : public IntegerType
{
 public:
   using IntegerType::accept;

   virtual AtomicTypeVisitorResult::Ptr accept(const AtomicTypeVisitor::Ptr &v,
         const SourceLocationReflection *const r) const {
      return v->visit(this, r);
   }

   virtual QXmlName name(const NamePool::Ptr &np) const {
      switch (derivedType) {
         case TypeByte:
            return np->allocateQName(StandardNamespaces::xs, QLatin1String("byte"));
         case TypeInt:
            return np->allocateQName(StandardNamespaces::xs, QLatin1String("int"));
         case TypeLong:
            return np->allocateQName(StandardNamespaces::xs, QLatin1String("long"));
         case TypeNegativeInteger:
            return np->allocateQName(StandardNamespaces::xs, QLatin1String("negativeInteger"));
         case TypeNonNegativeInteger:
            return np->allocateQName(StandardNamespaces::xs, QLatin1String("nonNegativeInteger"));
         case TypeNonPositiveInteger:
            return np->allocateQName(StandardNamespaces::xs, QLatin1String("nonPositiveInteger"));
         case TypePositiveInteger:
            return np->allocateQName(StandardNamespaces::xs, QLatin1String("positiveInteger"));
         case TypeShort:
            return np->allocateQName(StandardNamespaces::xs, QLatin1String("short"));
         case TypeUnsignedByte:
            return np->allocateQName(StandardNamespaces::xs, QLatin1String("unsignedByte"));
         case TypeUnsignedInt:
            return np->allocateQName(StandardNamespaces::xs, QLatin1String("unsignedInt"));
         case TypeUnsignedLong:
            return np->allocateQName(StandardNamespaces::xs, QLatin1String("unsignedLong"));
         case TypeUnsignedShort:
            return np->allocateQName(StandardNamespaces::xs, QLatin1String("unsignedShort"));
      }

      Q_ASSERT_X(false, "DerivedIntegerType::name()", "Invalid value in instantiation.");
      return QXmlName();
   }

   virtual QString displayName(const NamePool::Ptr &np) const {
      return np->displayName(name(np));
   }

 protected:
   friend class BuiltinTypes;

   DerivedIntegerType(const AtomicType::Ptr &parentType,
                      const AtomicCasterLocator::Ptr &casterLoc) : IntegerType(parentType, casterLoc) {
   }

};

class GYearMonthType : public BuiltinAtomicType
{
 public:
   typedef QExplicitlySharedDataPointer<GYearMonthType> Ptr;

   virtual AtomicTypeVisitorResult::Ptr accept(const AtomicTypeVisitor::Ptr &visitor,
         const SourceLocationReflection *const reflection) const;
   virtual AtomicTypeVisitorResult::Ptr accept(const ParameterizedAtomicTypeVisitor::Ptr &visitor,
         const qint16 op,
         const SourceLocationReflection *const reflection) const;
   virtual QXmlName name(const NamePool::Ptr &np) const;
   virtual QString displayName(const NamePool::Ptr &np) const;

 protected:
   friend class BuiltinTypes;
   GYearMonthType();
};

class GYearType : public BuiltinAtomicType
{
 public:
   typedef QExplicitlySharedDataPointer<GYearType> Ptr;

   virtual AtomicTypeVisitorResult::Ptr accept(const AtomicTypeVisitor::Ptr &visitor,
         const SourceLocationReflection *const reflection) const;
   virtual AtomicTypeVisitorResult::Ptr accept(const ParameterizedAtomicTypeVisitor::Ptr &visitor,
         const qint16 op,
         const SourceLocationReflection *const reflection) const;
   virtual QXmlName name(const NamePool::Ptr &np) const;
   virtual QString displayName(const NamePool::Ptr &np) const;

 protected:
   friend class BuiltinTypes;
   GYearType();
};

class GMonthDayType : public BuiltinAtomicType
{
 public:
   typedef QExplicitlySharedDataPointer<GMonthDayType> Ptr;

   virtual AtomicTypeVisitorResult::Ptr accept(const AtomicTypeVisitor::Ptr &visitor,
         const SourceLocationReflection *const reflection) const;
   virtual AtomicTypeVisitorResult::Ptr accept(const ParameterizedAtomicTypeVisitor::Ptr &visitor,
         const qint16 op,
         const SourceLocationReflection *const reflection) const;
   virtual QXmlName name(const NamePool::Ptr &np) const;
   virtual QString displayName(const NamePool::Ptr &np) const;

 protected:
   friend class BuiltinTypes;
   GMonthDayType();
};

class GDayType : public BuiltinAtomicType
{
 public:
   typedef QExplicitlySharedDataPointer<GDayType> Ptr;

   virtual AtomicTypeVisitorResult::Ptr accept(const AtomicTypeVisitor::Ptr &visitor,
         const SourceLocationReflection *const reflection) const;
   virtual AtomicTypeVisitorResult::Ptr accept(const ParameterizedAtomicTypeVisitor::Ptr &visitor,
         const qint16 op,
         const SourceLocationReflection *const reflection) const;
   virtual QXmlName name(const NamePool::Ptr &np) const;
   virtual QString displayName(const NamePool::Ptr &np) const;

 protected:
   friend class BuiltinTypes;
   GDayType();
};

class GMonthType : public BuiltinAtomicType
{
 public:
   typedef QExplicitlySharedDataPointer<GMonthType> Ptr;

   virtual AtomicTypeVisitorResult::Ptr accept(const AtomicTypeVisitor::Ptr &visitor,
         const SourceLocationReflection *const reflection) const;
   virtual AtomicTypeVisitorResult::Ptr accept(const ParameterizedAtomicTypeVisitor::Ptr &visitor,
         const qint16 op,
         const SourceLocationReflection *const reflection) const;
   virtual QXmlName name(const NamePool::Ptr &np) const;
   virtual QString displayName(const NamePool::Ptr &np) const;

 protected:
   friend class BuiltinTypes;
   GMonthType();
};

class BooleanType : public BuiltinAtomicType
{
 public:
   typedef QExplicitlySharedDataPointer<BooleanType> Ptr;

   virtual AtomicTypeVisitorResult::Ptr accept(const AtomicTypeVisitor::Ptr &visitor,
         const SourceLocationReflection *const reflection) const;
   virtual AtomicTypeVisitorResult::Ptr accept(const ParameterizedAtomicTypeVisitor::Ptr &visitor,
         const qint16 op,
         const SourceLocationReflection *const reflection) const;
   virtual QXmlName name(const NamePool::Ptr &np) const;
   virtual QString displayName(const NamePool::Ptr &np) const;

 protected:
   friend class BuiltinTypes;
   BooleanType();
};

class Base64BinaryType : public BuiltinAtomicType
{
 public:
   typedef QExplicitlySharedDataPointer<Base64BinaryType> Ptr;

   virtual AtomicTypeVisitorResult::Ptr accept(const AtomicTypeVisitor::Ptr &visitor,
         const SourceLocationReflection *const reflection) const;
   virtual AtomicTypeVisitorResult::Ptr accept(const ParameterizedAtomicTypeVisitor::Ptr &visitor,
         const qint16 op,
         const SourceLocationReflection *const reflection) const;
   virtual QXmlName name(const NamePool::Ptr &np) const;
   virtual QString displayName(const NamePool::Ptr &np) const;

 protected:
   friend class BuiltinTypes;
   Base64BinaryType();
};

class HexBinaryType : public BuiltinAtomicType
{
 public:
   typedef QExplicitlySharedDataPointer<HexBinaryType> Ptr;

   virtual AtomicTypeVisitorResult::Ptr accept(const AtomicTypeVisitor::Ptr &visitor,
         const SourceLocationReflection *const reflection) const;
   virtual AtomicTypeVisitorResult::Ptr accept(const ParameterizedAtomicTypeVisitor::Ptr &visitor,
         const qint16 op,
         const SourceLocationReflection *const reflection) const;
   virtual QXmlName name(const NamePool::Ptr &np) const;
   virtual QString displayName(const NamePool::Ptr &np) const;

 protected:
   friend class BuiltinTypes;
   HexBinaryType();
};

class AnyURIType : public BuiltinAtomicType
{
 public:
   typedef QExplicitlySharedDataPointer<AnyURIType> Ptr;

   virtual AtomicTypeVisitorResult::Ptr accept(const AtomicTypeVisitor::Ptr &visitor,
         const SourceLocationReflection *const reflection) const;
   virtual AtomicTypeVisitorResult::Ptr accept(const ParameterizedAtomicTypeVisitor::Ptr &visitor,
         const qint16 op,
         const SourceLocationReflection *const reflection) const;
   virtual QXmlName name(const NamePool::Ptr &np) const;
   virtual QString displayName(const NamePool::Ptr &np) const;

 protected:
   friend class BuiltinTypes;
   AnyURIType();
};

class QNameType : public BuiltinAtomicType
{
 public:
   typedef QExplicitlySharedDataPointer<QNameType> Ptr;

   virtual AtomicTypeVisitorResult::Ptr accept(const AtomicTypeVisitor::Ptr &visitor,
         const SourceLocationReflection *const reflection) const;
   virtual AtomicTypeVisitorResult::Ptr accept(const ParameterizedAtomicTypeVisitor::Ptr &visitor,
         const qint16 op,
         const SourceLocationReflection *const reflection) const;
   virtual QXmlName name(const NamePool::Ptr &np) const;
   virtual QString displayName(const NamePool::Ptr &np) const;

 protected:
   friend class BuiltinTypes;
   QNameType();
};

class StringType : public BuiltinAtomicType
{
 public:
   typedef QExplicitlySharedDataPointer<StringType> Ptr;

   virtual AtomicTypeVisitorResult::Ptr accept(const AtomicTypeVisitor::Ptr &visitor,
         const SourceLocationReflection *const reflection) const;
   virtual AtomicTypeVisitorResult::Ptr accept(const ParameterizedAtomicTypeVisitor::Ptr &visitor,
         const qint16 op,
         const SourceLocationReflection *const reflection) const;
   virtual QXmlName name(const NamePool::Ptr &np) const;
   virtual QString displayName(const NamePool::Ptr &np) const;

 protected:
   friend class BuiltinTypes;
   StringType(const AtomicType::Ptr &parentType,
              const AtomicCasterLocator::Ptr &casterLoc);
};

template<TypeOfDerivedString derivedType>
class DerivedStringType : public StringType
{
 public:
   using StringType::accept;

   virtual AtomicTypeVisitorResult::Ptr accept(const AtomicTypeVisitor::Ptr &v,
         const SourceLocationReflection *const r) const {
      return v->visit(this, r);
   }

   virtual QXmlName name(const NamePool::Ptr &np) const {
      switch (derivedType) {
         case TypeString:
            return np->allocateQName(StandardNamespaces::xs, QLatin1String("string"));
         case TypeNormalizedString:
            return np->allocateQName(StandardNamespaces::xs, QLatin1String("normalizedString"));
         case TypeToken:
            return np->allocateQName(StandardNamespaces::xs, QLatin1String("token"));
         case TypeLanguage:
            return np->allocateQName(StandardNamespaces::xs, QLatin1String("language"));
         case TypeNMTOKEN:
            return np->allocateQName(StandardNamespaces::xs, QLatin1String("NMTOKEN"));
         case TypeName:
            return np->allocateQName(StandardNamespaces::xs, QLatin1String("Name"));
         case TypeNCName:
            return np->allocateQName(StandardNamespaces::xs, QLatin1String("NCName"));
         case TypeID:
            return np->allocateQName(StandardNamespaces::xs, QLatin1String("ID"));
         case TypeIDREF:
            return np->allocateQName(StandardNamespaces::xs, QLatin1String("IDREF"));
         case TypeENTITY:
            return np->allocateQName(StandardNamespaces::xs, QLatin1String("ENTITY"));
      }

      Q_ASSERT_X(false, "DerivedStringType::name()", "Invalid value in instantiation.");
      return QXmlName();
   }

   virtual QString displayName(const NamePool::Ptr &np) const {
      return np->displayName(name(np));
   }

 protected:
   friend class BuiltinTypes;

   DerivedStringType(const AtomicType::Ptr &parentType,
                     const AtomicCasterLocator::Ptr &casterLoc) : StringType(parentType, casterLoc) {
   }

};


class NOTATIONType : public BuiltinAtomicType
{
 public:
   typedef QExplicitlySharedDataPointer<NOTATIONType> Ptr;

   virtual AtomicTypeVisitorResult::Ptr accept(const AtomicTypeVisitor::Ptr &visitor,
         const SourceLocationReflection *const reflection) const;
   virtual AtomicTypeVisitorResult::Ptr accept(const ParameterizedAtomicTypeVisitor::Ptr &visitor,
         const qint16 op,
         const SourceLocationReflection *const reflection) const;
   virtual QXmlName name(const NamePool::Ptr &np) const;
   virtual QString displayName(const NamePool::Ptr &np) const;

   /**
    * Overridden to return @c true, xs:NOTATION is abstract.
    *
    * @returns always @c true
    */
   virtual bool isAbstract() const;

 protected:
   friend class BuiltinTypes;
   NOTATIONType();
};
}

QT_END_NAMESPACE

#endif
