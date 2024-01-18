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

#include "qatomicmathematicianlocators_p.h"
#include "qbuiltintypes_p.h"

#include "qbuiltinatomictypes_p.h"

using namespace QPatternist;

/* -------------------------------------------------------------- */
#define implAccept(className)                                                                       \
AtomicTypeVisitorResult::Ptr className##Type::accept(const AtomicTypeVisitor::Ptr &v,               \
                                                     const SourceLocationReflection *const r) const \
{                                                                                                   \
    return v->visit(this, r);                                                                       \
}                                                                                                   \
                                                                                                    \
AtomicTypeVisitorResult::Ptr                                                                        \
className##Type::accept(const ParameterizedAtomicTypeVisitor::Ptr &v,                               \
                        const qint16 op,                                                            \
                        const SourceLocationReflection *const r) const                              \
{                                                                                                   \
    return v->visit(this, op, r);                                                                   \
}

#define deployComp(className, qname, parent, comp, mather, caster)                          \
className##Type::className##Type() : BuiltinAtomicType(BuiltinTypes::parent,                \
                                                       comp,                                \
                                                       mather,                              \
                                                       caster)                              \
{                                                                                           \
}                                                                                           \
implAccept(className)

#define deployBase(className, qname, parent) deployComp(className, qname, parent,           \
                                                        AtomicComparatorLocator::Ptr(),     \
                                                        AtomicMathematicianLocator::Ptr(),  \
                                                        AtomicCasterLocator::Ptr())

#define deployFull(className, qname, parent)                                                \
deployComp(className, qname, parent,                                                        \
           AtomicComparatorLocator::Ptr(new className##ComparatorLocator()),                \
           AtomicMathematicianLocator::Ptr(),                                               \
           AtomicCasterLocator::Ptr(new To##className##CasterLocator()))

#define deployMathComp(className, qname, parent)                                            \
deployComp(className, qname, parent,                                                        \
           AtomicComparatorLocator::Ptr(new className##ComparatorLocator()),                \
           AtomicMathematicianLocator::Ptr(new className##MathematicianLocator()),          \
           AtomicCasterLocator::Ptr(new To##className##CasterLocator()))
/* -------------------------------------------------------------- */

/* -------------------------------------------------------------- */
/* xs:anyURI & xs:untypedAtomic are much treated like strings. This ensures
 * they get the correct operators and automatically takes care of type promotion. */
deployComp(UntypedAtomic,       xsUntypedAtomic,       xsAnyAtomicType,
           AtomicComparatorLocator::Ptr(new StringComparatorLocator()),
           AtomicMathematicianLocator::Ptr(),
           AtomicCasterLocator::Ptr(new ToUntypedAtomicCasterLocator()))
deployComp(AnyURI,              xsAnyURI,            xsAnyAtomicType,
           AtomicComparatorLocator::Ptr(new StringComparatorLocator()),
           AtomicMathematicianLocator::Ptr(),
           AtomicCasterLocator::Ptr(new ToAnyURICasterLocator()))

deployBase(NOTATION,            xsNOTATION,                 xsAnyAtomicType)

deployMathComp(Float,               xsFloat,                numeric)
deployMathComp(Double,              xsDouble,               numeric)
deployMathComp(Decimal,             xsDecimal,              numeric)
deployMathComp(DayTimeDuration,     xsDayTimeDuration,      xsDuration)
deployMathComp(YearMonthDuration,   xsYearMonthDuration,    xsDuration)
deployMathComp(Date,                xsDate,                 xsAnyAtomicType)
deployMathComp(DateTime,            xsDateTime,             xsAnyAtomicType)
deployMathComp(SchemaTime,          xsTime,                 xsAnyAtomicType)

deployFull(Base64Binary,        xsBase64Binary,             xsAnyAtomicType)
deployFull(Boolean,             xsBoolean,                  xsAnyAtomicType)
deployFull(Duration,            xsDuration,                 xsAnyAtomicType)
deployFull(GDay,                xsGDay,                     xsAnyAtomicType)
deployFull(GMonth,              xsGMonth,                   xsAnyAtomicType)
deployFull(GMonthDay,           xsGMonthDay,                xsAnyAtomicType)
deployFull(GYear,               xsGYear,                    xsAnyAtomicType)
deployFull(GYearMonth,          xsGYearMonth,               xsAnyAtomicType)
deployFull(HexBinary,           xsHexBinary,                xsAnyAtomicType)
deployFull(QName,               xsQName,                    xsAnyAtomicType)
/* --------------------------------------------------------------- */

/* --------------------------------------------------------------- */
StringType::StringType(const AtomicType::Ptr &pType,
                       const AtomicCasterLocator::Ptr &casterLoc)
   : BuiltinAtomicType(pType,
                       AtomicComparatorLocator::Ptr(new StringComparatorLocator()),
                       AtomicMathematicianLocator::Ptr(),
                       casterLoc)
{
}
implAccept(String)
/* --------------------------------------------------------------- */

/* --------------------------------------------------------------- */
IntegerType::IntegerType(const AtomicType::Ptr &pType,
                         const AtomicCasterLocator::Ptr &casterLoc)
   : BuiltinAtomicType(pType,
                       AtomicComparatorLocator::Ptr(new IntegerComparatorLocator()),
                       AtomicMathematicianLocator::Ptr(new IntegerMathematicianLocator()),
                       casterLoc)
{
}
implAccept(Integer)
/* --------------------------------------------------------------- */

/* ---------------------- Special Overrides ---------------------- */
AnyAtomicType::AnyAtomicType() : BuiltinAtomicType(AtomicType::Ptr(),
         AtomicComparatorLocator::Ptr(),
         AtomicMathematicianLocator::Ptr(),
         AtomicCasterLocator::Ptr())
{
}
implAccept(AnyAtomic)

ItemType::Ptr AnyAtomicType::xdtSuperType() const
{
   return BuiltinTypes::item;
}

SchemaType::Ptr AnyAtomicType::wxsSuperType() const
{
   return BuiltinTypes::xsAnySimpleType;
}

bool AnyAtomicType::isAbstract() const
{
   return true;
}

bool NOTATIONType::isAbstract() const
{
   return true;
}

#define implementName(className, typeName)                          \
QXmlName className##Type::name(const NamePool::Ptr &np) const          \
{                                                                   \
    return np->allocateQName(StandardNamespaces::xs, typeName);     \
}                                                                   \
                                                                    \
QString className##Type::displayName(const NamePool::Ptr &np) const \
{                                                                   \
    return np->displayName(name(np));                               \
}

implementName(AnyAtomic,            QLatin1String("anyAtomicType"))
implementName(AnyURI,               QLatin1String("anyURI"))
implementName(Base64Binary,         QLatin1String("base64Binary"))
implementName(Boolean,              QLatin1String("boolean"))
implementName(Date,                 QLatin1String("date"))
implementName(DateTime,             QLatin1String("dateTime"))
implementName(DayTimeDuration,      QLatin1String("dayTimeDuration"))
implementName(Decimal,              QLatin1String("decimal"))
implementName(Double,               QLatin1String("double"))
implementName(Duration,             QLatin1String("duration"))
implementName(Float,                QLatin1String("float"))
implementName(GDay,                 QLatin1String("gDay"))
implementName(GMonthDay,            QLatin1String("gMonthDay"))
implementName(GMonth,               QLatin1String("gMonth"))
implementName(GYearMonth,           QLatin1String("gYearMonth"))
implementName(GYear,                QLatin1String("gYear"))
implementName(HexBinary,            QLatin1String("hexBinary"))
implementName(Integer,              QLatin1String("integer"))
implementName(NOTATION,             QLatin1String("NOTATION"))
implementName(QName,                QLatin1String("QName"))
implementName(String,               QLatin1String("string"))
implementName(SchemaTime,                 QLatin1String("time"))
implementName(UntypedAtomic,        QLatin1String("untypedAtomic"))
implementName(YearMonthDuration,    QLatin1String("yearMonthDuration"))
/* --------------------------------------------------------------- */

#undef implAccept
#undef implementName
#undef deployComp
#undef deployBase
#undef deployFull
#undef deployMathComp

