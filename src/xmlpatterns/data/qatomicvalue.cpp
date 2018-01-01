/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#include <QVariant>

#include "qabstractdatetime_p.h"
#include "qabstractfloat_p.h"
#include "qatomicstring_p.h"
#include "qatomictype_p.h"
#include "qboolean_p.h"
#include "qbuiltintypes_p.h"
#include "qdate_p.h"
#include "qschemadatetime_p.h"
#include "qderivedinteger_p.h"
#include "qdynamiccontext_p.h"
#include "qgenericsequencetype_p.h"
#include "qhexbinary_p.h"
#include "qinteger_p.h"
#include "qpatternistlocale_p.h"
#include "qqnamevalue_p.h"
#include "qschematime_p.h"
#include "qvalidationerror_p.h"

#include "qitem_p.h"

QT_BEGIN_NAMESPACE

/**
 * @file
 * @short Contains the implementation for AtomicValue. The definition is in qitem_p.h.
 */

using namespace QPatternist;

AtomicValue::~AtomicValue()
{
}

bool AtomicValue::evaluateEBV(const QExplicitlySharedDataPointer<DynamicContext> &context) const
{
   context->error(QtXmlPatterns::tr("A value of type %1 cannot have an "
                                    "Effective Boolean Value.")
                  .arg(formatType(context->namePool(), type())),
                  ReportContext::FORG0006,
                  QSourceLocation());
   return false; /* Silence GCC warning. */
}

bool AtomicValue::hasError() const
{
   return false;
}

QVariant AtomicValue::toQt(const AtomicValue *const value)
{
   Q_ASSERT_X(value, Q_FUNC_INFO,
              "Internal error, a null pointer cannot be passed.");

   const ItemType::Ptr t(value->type());

   if (BuiltinTypes::xsString->xdtTypeMatches(t)
         || BuiltinTypes::xsUntypedAtomic->xdtTypeMatches(t)
         || BuiltinTypes::xsAnyURI->xdtTypeMatches(t)) {
      return value->stringValue();
   }
   /* Note, this occurs before the xsInteger test, since xs:unsignedLong
    * is a subtype of it. */
   else if (*BuiltinTypes::xsUnsignedLong == *t) {
      return QVariant(value->as<DerivedInteger<TypeUnsignedLong> >()->storedValue());
   } else if (BuiltinTypes::xsInteger->xdtTypeMatches(t)) {
      return QVariant(value->as<Numeric>()->toInteger());
   } else if (BuiltinTypes::xsFloat->xdtTypeMatches(t)
              || BuiltinTypes::xsDouble->xdtTypeMatches(t)
              || BuiltinTypes::xsDecimal->xdtTypeMatches(t)) {
      return QVariant(value->as<Numeric>()->toDouble());
   }
   /* We currently does not support xs:time. */
   else if (BuiltinTypes::xsDateTime->xdtTypeMatches(t)) {
      return QVariant(value->as<AbstractDateTime>()->toDateTime());
   } else if (BuiltinTypes::xsDate->xdtTypeMatches(t)) {
      return QVariant(value->as<AbstractDateTime>()->toDateTime().toUTC().date());
   } else if (BuiltinTypes::xsBoolean->xdtTypeMatches(t)) {
      return QVariant(value->as<Boolean>()->value());
   } else if (BuiltinTypes::xsBase64Binary->xdtTypeMatches(t)
              || BuiltinTypes::xsHexBinary->xdtTypeMatches(t)) {
      return QVariant(value->as<Base64Binary>()->asByteArray());
   } else if (BuiltinTypes::xsQName->xdtTypeMatches(t)) {
      return QVariant::fromValue(value->as<QNameValue>()->qName());
   } else {
      /* A type we don't support in Qt. Includes xs:time currently. */
      return QVariant();
   }
}

Item AtomicValue::toXDM(const QVariant &value)
{
   Q_ASSERT_X(value.isValid(), Q_FUNC_INFO,
              "QVariants sent to Patternist must be valid.");

   switch (value.userType()) {
      case QVariant::Char:
      /* Fallthrough. A single codepoint is a string in XQuery. */
      case QVariant::String:
         return AtomicString::fromValue(value.toString());
      case QVariant::Url: {
         /* QUrl doesn't follow the spec properly, so we
          * have to let it be an xs:string. Calling QVariant::toString()
          * on a QVariant that contains a QUrl returns, surprisingly,
          * an empty string. */
         return AtomicString::fromValue(value.toUrl().toString());
      }
      case QVariant::ByteArray:
         return HexBinary::fromValue(value.toByteArray());
      case QVariant::Int:
      /* Fallthrough. */
      case QVariant::LongLong:
      /* Fallthrough. */
      case QVariant::UInt:
         return Integer::fromValue(value.toLongLong());
      case QVariant::ULongLong:
         return DerivedInteger<TypeUnsignedLong>::fromValueUnchecked(value.toULongLong());
      case QVariant::Bool:
         return Boolean::fromValue(value.toBool());
      case QVariant::Time:
         return SchemaTime::fromDateTime(value.toDateTime());
      case QVariant::Date:
         return Date::fromDateTime(QDateTime(value.toDate(), QTime(), Qt::UTC));
      case QVariant::DateTime:
         return DateTime::fromDateTime(value.toDateTime());
      case QMetaType::Float:
         return Item(Double::fromValue(value.toFloat()));
      case QVariant::Double:
         return Item(Double::fromValue(value.toDouble()));

      default: {
         if (value.userType() == qMetaTypeId<float>()) {
            return Item(Float::fromValue(value.value<float>()));

         } else {
            Q_ASSERT_X(false, Q_FUNC_INFO, csPrintable(QString::fromLatin1("QVariants of type %1 are not supported "
                  "in Patternist").arg(QLatin1String(value.typeName()))));

            return AtomicValue::Ptr();
         }
      }
   }
}

ItemType::Ptr AtomicValue::qtToXDMType(const QXmlItem &item)
{
   Q_ASSERT(!item.isNull());

   if (item.isNull()) {
      return ItemType::Ptr();
   }

   if (item.isNode()) {
      return BuiltinTypes::node;
   }

   Q_ASSERT(item.isAtomicValue());
   const QVariant v(item.toAtomicValue());

   switch (int(v.type())) {
      case QVariant::Char:
      /* Fallthrough. */
      case QVariant::String:
      /* Fallthrough. */
      case QVariant::Url:
         return BuiltinTypes::xsString;
      case QVariant::Bool:
         return BuiltinTypes::xsBoolean;
      case QVariant::ByteArray:
         return BuiltinTypes::xsBase64Binary;
      case QVariant::Int:
      /* Fallthrough. */
      case QVariant::LongLong:
         return BuiltinTypes::xsInteger;
      case QVariant::ULongLong:
         return BuiltinTypes::xsUnsignedLong;
      case QVariant::Date:
         return BuiltinTypes::xsDate;
      case QVariant::DateTime:
      /* Fallthrough. */
      case QVariant::Time:
         return BuiltinTypes::xsDateTime;
      case QMetaType::Float:
         return BuiltinTypes::xsFloat;
      case QVariant::Double:
         return BuiltinTypes::xsDouble;
      default:
         return ItemType::Ptr();
   }
}

QT_END_NAMESPACE
