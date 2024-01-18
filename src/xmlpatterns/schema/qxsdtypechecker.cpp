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

#include "qxsdtypechecker_p.h"

#include "qabstractdatetime_p.h"
#include "qbase64binary_p.h"
#include "qboolean_p.h"
#include "qdecimal_p.h"
#include "qderivedinteger_p.h"
#include "qduration_p.h"
#include "qgenericstaticcontext_p.h"
#include "qhexbinary_p.h"
#include "qnamespaceresolver_p.h"
#include "qpatternplatform_p.h"
#include "qqnamevalue_p.h"
#include "qvaluefactory_p.h"
#include "qxmlnamepool.h"
#include "qxsdschemahelper_p.h"
#include "qxsdschemamerger_p.h"
#include "qxsdstatemachine_p.h"
#include "qabstractfloat_p.h"

#include "qxsdschemadebugger_p.h"

using namespace QPatternist;

XsdSchemaSourceLocationReflection::XsdSchemaSourceLocationReflection(const QSourceLocation &location)
   : m_sourceLocation(location)
{
}

const SourceLocationReflection *XsdSchemaSourceLocationReflection::actualReflection() const
{
   return this;
}

QSourceLocation XsdSchemaSourceLocationReflection::sourceLocation() const
{
   return m_sourceLocation;
}


static AnySimpleType::Ptr comparableType(const AnySimpleType::Ptr &type)
{
   if (!type->isDefinedBySchema()) {
      return type;
   } else {
      const XsdSimpleType::Ptr simpleType(type);
      if (type->category() == SchemaType::SimpleTypeAtomic) {
         return simpleType->primitiveType();
      } else if (type->category() == SchemaType::SimpleTypeList) {
         return simpleType->itemType();
      } else if (type->category() == SchemaType::SimpleTypeUnion) {
         return simpleType->memberTypes().first();
      }
   }

   Q_ASSERT(false);
   return AnySimpleType::Ptr();
}

static int totalDigitsForSignedLongLong(long long value)
{
   QString number = QString::number(value);
   if (number.startsWith(QLatin1Char('-'))) {
      number = number.mid(1);
   }

   return number.length();
}

static int totalDigitsForUnsignedLongLong(unsigned long long value)
{
   const QString number = QString::number(value);
   return number.length();
}

static int totalDigitsForDecimal(const QString &lexicalValue)
{
   const QLatin1Char zeroChar('0');
   const int length = lexicalValue.length() - 1;

   // strip leading zeros
   int pos = 0;
   while (lexicalValue.at(pos) == zeroChar && (pos != length)) {
      pos++;
   }

   QString value = lexicalValue.mid(pos);

   // if contains '.' strip trailing zeros
   if (value.contains(QLatin1Char('.'))) {
      pos = value.length() - 1;
      while (value.at(pos) == zeroChar) {
         pos--;
      }

      value = value.left(pos + 1);
   }

   // check number of digits of remaining string
   int totalDigits = 0;
   for (int i = 0; i < value.count(); ++i)
      if (value.at(i).isDigit()) {
         ++totalDigits;
      }

   if (totalDigits == 0) {
      totalDigits = 1;
   }

   return totalDigits;
}

static int fractionDigitsForDecimal(const QString &lexicalValue)
{
   // we use the lexical value here, as the conversion to double might strip
   // away decimal positions

   QString trimmedValue(lexicalValue.trimmed());
   const int pos = trimmedValue.indexOf(QLatin1Char('.'));
   if (pos == -1) { // no '.' -> 0 fraction digits
      return 0;
   } else {
      return (trimmedValue.length() - pos - 1);
   }
}

XsdTypeChecker::XsdTypeChecker(const XsdSchemaContext::Ptr &context, const QVector<QXmlName> &namespaceBindings,
                               const QSourceLocation &location)
   : m_context(context)
   , m_namePool(m_context->namePool())
   , m_namespaceBindings(namespaceBindings)
   , m_reflection(new XsdSchemaSourceLocationReflection(location))
{
}

XsdTypeChecker::~XsdTypeChecker()
{
   delete m_reflection;
}

QString XsdTypeChecker::normalizedValue(const QString &value, const XsdFacet::Hash &facets)
{
   if (!facets.contains(XsdFacet::WhiteSpace)) {
      return value;
   }

   const XsdFacet::Ptr whiteSpaceFacet = facets.value(XsdFacet::WhiteSpace);

   const DerivedString<TypeString>::Ptr facetValue = whiteSpaceFacet->value();
   const QString stringValue = facetValue->stringValue();
   if (stringValue == XsdSchemaToken::toString(XsdSchemaToken::Preserve)) {
      return value;
   } else if (stringValue == XsdSchemaToken::toString(XsdSchemaToken::Replace)) {
      QString newValue(value);
      newValue.replace(QLatin1Char('\t'), QLatin1Char(' '));
      newValue.replace(QLatin1Char('\n'), QLatin1Char(' '));
      newValue.replace(QLatin1Char('\r'), QLatin1Char(' '));

      return newValue;
   } else if (stringValue == XsdSchemaToken::toString(XsdSchemaToken::Collapse)) {
      return value.simplified();
   }

   return value;
}

XsdFacet::Hash XsdTypeChecker::mergedFacetsForType(const SchemaType::Ptr &type, const XsdSchemaContext::Ptr &context)
{
   if (!type) {
      return XsdFacet::Hash();
   }

   const XsdFacet::Hash baseFacets = mergedFacetsForType(type->wxsSuperType(), context);
   const XsdFacet::Hash facets = context->facetsForType(type);

   XsdFacet::Hash result = baseFacets;
   XsdFacet::HashIterator it(facets);
   while (it.hasNext()) {
      it.next();

      result.insert(it.key(), it.value());
   }

   return result;
}

bool XsdTypeChecker::isValidString(const QString &normalizedString, const AnySimpleType::Ptr &type, QString &errorMsg,
                                   AnySimpleType::Ptr *boundType) const
{
   if (type->name(m_namePool) == BuiltinTypes::xsAnySimpleType->name(m_namePool)) {
      if (boundType) {
         *boundType = type;
      }

      return true;
   }

   if (!type->isDefinedBySchema()) {
      // special QName check
      if (BuiltinTypes::xsQName->wxsTypeMatches(type)) {
         if (!XPathHelper::isQName(normalizedString)) {
            errorMsg = QtXmlPatterns::tr("%1 is not valid according to %2.").formatArg(formatData(normalizedString)).formatArg(formatType(
                          m_namePool, type));
            return false;
         }
      }

      const AtomicValue::Ptr value = fromLexical(normalizedString, type, m_context, m_reflection);
      if (value->hasError()) {
         errorMsg = QtXmlPatterns::tr("%1 is not valid according to %2.").formatArg(formatData(normalizedString)).formatArg(formatType(
                       m_namePool, type));
         return false;
      }

      if (!checkConstrainingFacets(value, normalizedString, type, errorMsg)) {
         return false;
      }

      if (boundType) {
         *boundType = type;
      }

   } else {
      const XsdSimpleType::Ptr simpleType(type);

      if (simpleType->category() == XsdSimpleType::SimpleTypeAtomic) {
         AnySimpleType::Ptr targetType = simpleType->primitiveType();
         if (!simpleType->wxsSuperType()->isDefinedBySchema()) {
            targetType = simpleType->wxsSuperType();
         }

         const AtomicValue::Ptr value = fromLexical(normalizedString, targetType, m_context, m_reflection);
         if (value->hasError()) {
            errorMsg = QtXmlPatterns::tr("%1 is not valid according to %2.").formatArg(formatData(normalizedString)).formatArg(formatType(
                          m_namePool, targetType));
            return false;
         }

         if (!checkConstrainingFacets(value, normalizedString, type, errorMsg)) {
            return false;
         }

         if (boundType) {
            *boundType = type;
         }

      } else if (simpleType->category() == XsdSimpleType::SimpleTypeList) {
         QStringList entries = normalizedString.split(QLatin1Char(' '), QStringParser::SkipEmptyParts);
         for (int i = 0; i < entries.count(); ++i) {
            entries[i] = normalizedValue(entries.at(i), mergedFacetsForType(simpleType->itemType(), m_context));
         }

         if (!checkConstrainingFacetsList(entries, normalizedString, simpleType->itemType(), mergedFacetsForType(simpleType,
                                          m_context), errorMsg)) {
            return false;
         }

         for (int i = 0; i < entries.count(); ++i) {
            if (!isValidString(entries.at(i), simpleType->itemType(), errorMsg)) {
               return false;
            }
         }

         if (boundType) {
            *boundType = simpleType->itemType();
         }

      } else if (simpleType->category() == XsdSimpleType::SimpleTypeUnion) {
         if (!checkConstrainingFacetsUnion(normalizedString, normalizedString, simpleType, mergedFacetsForType(simpleType,
                                           m_context), errorMsg)) {
            return false;
         }

         const AnySimpleType::List memberTypes = simpleType->memberTypes();

         bool foundValidType = false;
         for (int i = 0; i < memberTypes.count(); ++i) {
            const XsdFacet::Hash mergedFacets = mergedFacetsForType(memberTypes.at(i), m_context);
            if (isValidString(normalizedValue(normalizedString, mergedFacets), memberTypes.at(i), errorMsg)) {
               foundValidType = true;

               if (boundType) {
                  *boundType = memberTypes.at(i);
               }

               break;
            }
         }

         if (!foundValidType) {
            return false;
         }
      }
   }

   return true;
}

bool XsdTypeChecker::valuesAreEqual(const QString &value, const QString &otherValue,
                                    const AnySimpleType::Ptr &type) const
{
   const AnySimpleType::Ptr targetType = comparableType(type);

   // if the type is xs:anySimpleType we just do string comparison...
   if (targetType->name(m_namePool) == BuiltinTypes::xsAnySimpleType->name(m_namePool)) {
      return (value == otherValue);
   }

   if (BuiltinTypes::xsQName->wxsTypeMatches(type)) {
      const QXmlName valueName = convertToQName(value);
      const QXmlName otherValueName = convertToQName(otherValue);

      if (valueName == otherValueName) {
         return true;
      }
   }

   if (type->category() == SchemaType::SimpleTypeAtomic) {
      // ... otherwise we use the casting platform for value comparison
      const DerivedString<TypeString>::Ptr valueStr = DerivedString<TypeString>::fromLexical(m_namePool, value);
      const DerivedString<TypeString>::Ptr otherValueStr = DerivedString<TypeString>::fromLexical(m_namePool, otherValue);

      return XsdSchemaHelper::constructAndCompare(valueStr, AtomicComparator::OperatorEqual, otherValueStr, targetType,
             m_context, m_reflection);
   } else if (type->category() == SchemaType::SimpleTypeList) {
      const QStringList values = value.split(QLatin1Char(' '), QStringParser::SkipEmptyParts);
      const QStringList otherValues = otherValue.split(QLatin1Char(' '), QStringParser::SkipEmptyParts);
      if (values.count() != otherValues.count()) {
         return false;
      }

      for (int i = 0; i < values.count(); ++i) {
         if (!valuesAreEqual(values.at(i), otherValues.at(i), XsdSimpleType::Ptr(type)->itemType())) {
            return false;
         }
      }

      return true;
   } else if (type->category() == SchemaType::SimpleTypeUnion) {
      const AnySimpleType::List memberTypes = XsdSimpleType::Ptr(type)->memberTypes();
      for (int i = 0; i < memberTypes.count(); ++i) {
         if (valuesAreEqual(value, otherValue, memberTypes.at(i))) {
            return true;
         }
      }

      return false;
   }

   return false;
}

bool XsdTypeChecker::checkConstrainingFacets(const AtomicValue::Ptr &value, const QString &lexicalValue,
      const AnySimpleType::Ptr &type, QString &errorMsg) const
{
   const XsdFacet::Hash facets = mergedFacetsForType(type, m_context);

   if (BuiltinTypes::xsString->wxsTypeMatches(type) ||
         BuiltinTypes::xsUntypedAtomic->wxsTypeMatches(type)) {
      return checkConstrainingFacetsString(value->stringValue(), facets, BuiltinTypes::xsString, errorMsg);
   } else if (BuiltinTypes::xsAnyURI->wxsTypeMatches(type)) {
      return checkConstrainingFacetsString(value->stringValue(), facets, BuiltinTypes::xsAnyURI, errorMsg);
   } else if (BuiltinTypes::xsNOTATION->wxsTypeMatches(type)) {
      return checkConstrainingFacetsNotation(value->as<QNameValue>()->qName(), facets, errorMsg);
   } else if (BuiltinTypes::xsUnsignedByte->wxsTypeMatches(type) ||
              BuiltinTypes::xsUnsignedInt->wxsTypeMatches(type) ||
              BuiltinTypes::xsUnsignedLong->wxsTypeMatches(type) ||
              BuiltinTypes::xsUnsignedShort->wxsTypeMatches(type)) {
      return checkConstrainingFacetsUnsignedInteger(value->as<Numeric>()->toUnsignedInteger(), lexicalValue, facets,
             errorMsg);
   } else if (BuiltinTypes::xsInteger->wxsTypeMatches(type)) {
      return checkConstrainingFacetsSignedInteger(value->as<Numeric>()->toInteger(), lexicalValue, facets, errorMsg);
   } else if (BuiltinTypes::xsFloat->wxsTypeMatches(type) ||
              BuiltinTypes::xsDouble->wxsTypeMatches(type)) {
      return checkConstrainingFacetsDouble(value->as<Numeric>()->toDouble(), lexicalValue, facets, errorMsg);
   } else if (BuiltinTypes::xsDecimal->wxsTypeMatches(type)) {
      return checkConstrainingFacetsDecimal(value, lexicalValue, facets, errorMsg);
   } else if (BuiltinTypes::xsDateTime->wxsTypeMatches(type)) {
      return checkConstrainingFacetsDateTime(value->as<AbstractDateTime>()->toDateTime(), lexicalValue, facets,
                                             BuiltinTypes::xsDateTime, errorMsg);
   } else if (BuiltinTypes::xsDate->wxsTypeMatches(type)) {
      return checkConstrainingFacetsDateTime(value->as<AbstractDateTime>()->toDateTime(), lexicalValue, facets,
                                             BuiltinTypes::xsDate, errorMsg);
   } else if (BuiltinTypes::xsGYear->wxsTypeMatches(type)) {
      return checkConstrainingFacetsDateTime(value->as<AbstractDateTime>()->toDateTime(), lexicalValue, facets,
                                             BuiltinTypes::xsGYear, errorMsg);
   } else if (BuiltinTypes::xsGYearMonth->wxsTypeMatches(type)) {
      return checkConstrainingFacetsDateTime(value->as<AbstractDateTime>()->toDateTime(), lexicalValue, facets,
                                             BuiltinTypes::xsGYearMonth, errorMsg);
   } else if (BuiltinTypes::xsGMonth->wxsTypeMatches(type)) {
      return checkConstrainingFacetsDateTime(value->as<AbstractDateTime>()->toDateTime(), lexicalValue, facets,
                                             BuiltinTypes::xsGMonth, errorMsg);

   } else if (BuiltinTypes::xsGMonthDay->wxsTypeMatches(type)) {
      return checkConstrainingFacetsDateTime(value->as<AbstractDateTime>()->toDateTime(), lexicalValue, facets,
                                             BuiltinTypes::xsGMonthDay, errorMsg);

   } else if (BuiltinTypes::xsGDay->wxsTypeMatches(type)) {
      return checkConstrainingFacetsDateTime(value->as<AbstractDateTime>()->toDateTime(), lexicalValue, facets,
                                             BuiltinTypes::xsGDay, errorMsg);

   } else if (BuiltinTypes::xsTime->wxsTypeMatches(type)) {
      return checkConstrainingFacetsDateTime(value->as<AbstractDateTime>()->toDateTime(), lexicalValue, facets,
                                             BuiltinTypes::xsTime, errorMsg);

   } else if (BuiltinTypes::xsDuration->wxsTypeMatches(type)) {
      return checkConstrainingFacetsDuration(value, lexicalValue, facets, errorMsg);

   } else if (BuiltinTypes::xsBoolean->wxsTypeMatches(type)) {
      return checkConstrainingFacetsBoolean(value->as<Boolean>()->value(), lexicalValue, facets, errorMsg);

   } else if (BuiltinTypes::xsHexBinary->wxsTypeMatches(type)) {
      return checkConstrainingFacetsBinary(value->as<Base64Binary>()->asByteArray(), facets, BuiltinTypes::xsHexBinary,
                                           errorMsg);

   } else if (BuiltinTypes::xsBase64Binary->wxsTypeMatches(type)) {
      return checkConstrainingFacetsBinary(value->as<Base64Binary>()->asByteArray(), facets, BuiltinTypes::xsBase64Binary,
                                           errorMsg);

   } else if (BuiltinTypes::xsQName->wxsTypeMatches(type)) {
      return checkConstrainingFacetsQName(value->as<QNameValue>()->qName(), lexicalValue, facets, errorMsg);
   }

   return true;
}

bool XsdTypeChecker::checkConstrainingFacetsString(const QString &value, const XsdFacet::Hash &facets,
      const AnySimpleType::Ptr &type, QString &errorMsg) const
{
   if (facets.contains(XsdFacet::Length)) {
      const XsdFacet::Ptr facet = facets.value(XsdFacet::Length);
      const DerivedInteger<TypeNonNegativeInteger>::Ptr length = facet->value();

      if (length->toInteger() != value.length()) {
         errorMsg = QtXmlPatterns::tr("String content does not match the length facet.");
         return false;
      }
   }

   if (facets.contains(XsdFacet::MinimumLength)) {
      const XsdFacet::Ptr facet = facets.value(XsdFacet::MinimumLength);
      const DerivedInteger<TypeNonNegativeInteger>::Ptr length = facet->value();

      if (length->toInteger() > value.length()) {
         errorMsg = QtXmlPatterns::tr("String content does not match the minLength facet.");
         return false;
      }
   }

   if (facets.contains(XsdFacet::MaximumLength)) {
      const XsdFacet::Ptr facet = facets.value(XsdFacet::MaximumLength);
      const DerivedInteger<TypeNonNegativeInteger>::Ptr length = facet->value();

      if (length->toInteger() < value.length()) {
         errorMsg = QtXmlPatterns::tr("String content does not match the maxLength facet.");
         return false;
      }
   }

   if (facets.contains(XsdFacet::Pattern)) {
      const XsdFacet::Ptr facet = facets.value(XsdFacet::Pattern);
      const AtomicValue::List multiValue = facet->multiValue();
      bool found = false;

      for (int j = 0; j < multiValue.count(); ++j) {
         const QString pattern = multiValue.at(j)->as<DerivedString<TypeString> >()->stringValue();
         const QRegularExpression exp = PatternPlatform::parsePattern(pattern, QPatternOption::ExactMatchOption, m_context, m_reflection);

         if (exp.match(value).hasMatch()) {
            found = true;
            break;
         }
      }

      if (!found) {
         errorMsg = QtXmlPatterns::tr("String content does not match pattern facet.");
         return false;
      }
   }

   if (facets.contains(XsdFacet::Enumeration)) {
      const XsdFacet::Ptr facet = facets.value(XsdFacet::Enumeration);
      const DerivedString<TypeString>::Ptr valueStr = DerivedString<TypeString>::fromLexical(m_namePool, value);

      const AtomicValue::List multiValue = facet->multiValue();
      bool found = false;
      for (int j = 0; j < multiValue.count(); ++j) {
         if (XsdSchemaHelper::constructAndCompare(valueStr, AtomicComparator::OperatorEqual, multiValue.at(j), type, m_context,
               m_reflection)) {
            found = true;
            break;
         }
      }

      if (!found) {
         errorMsg = QtXmlPatterns::tr("String content is not listed in the enumeration facet.");
         return false;
      }
   }

   if (facets.contains(XsdFacet::Assertion)) {
      //TODO: implement
   }

   return true;
}

bool XsdTypeChecker::checkConstrainingFacetsSignedInteger(long long value, const QString &lexicalValue,
      const XsdFacet::Hash &facets, QString &errorMsg) const
{
   if (facets.contains(XsdFacet::MaximumInclusive)) {
      const XsdFacet::Ptr facet = facets.value(XsdFacet::MaximumInclusive);
      const Numeric::Ptr facetValue = ValueFactory::fromLexical(
                  facet->value()->as<DerivedString<TypeString> >()->stringValue(), BuiltinTypes::xsLong, m_context, m_reflection);

      if (facetValue->toInteger() < value) {
         errorMsg = QtXmlPatterns::tr("Signed integer content does not match the maxInclusive facet.");
         return false;
      }
   }

   if (facets.contains(XsdFacet::MaximumExclusive)) {
      const XsdFacet::Ptr facet = facets.value(XsdFacet::MaximumExclusive);
      const Numeric::Ptr facetValue = ValueFactory::fromLexical(
                  facet->value()->as<DerivedString<TypeString> >()->stringValue(), BuiltinTypes::xsLong, m_context, m_reflection);

      if (facetValue->toInteger() <= value) {
         errorMsg = QtXmlPatterns::tr("Signed integer content does not match the maxExclusive facet.");
         return false;
      }
   }
   if (facets.contains(XsdFacet::MinimumInclusive)) {
      const XsdFacet::Ptr facet = facets.value(XsdFacet::MinimumInclusive);
      const Numeric::Ptr facetValue = ValueFactory::fromLexical(
                  facet->value()->as<DerivedString<TypeString> >()->stringValue(), BuiltinTypes::xsLong, m_context, m_reflection);

      if (facetValue->toInteger() > value) {
         errorMsg = QtXmlPatterns::tr("Signed integer content does not match the minInclusive facet.");
         return false;
      }
   }

   if (facets.contains(XsdFacet::MinimumExclusive)) {
      const XsdFacet::Ptr facet = facets.value(XsdFacet::MinimumExclusive);
      const Numeric::Ptr facetValue = ValueFactory::fromLexical(
                  facet->value()->as<DerivedString<TypeString> >()->stringValue(), BuiltinTypes::xsLong, m_context, m_reflection);

      if (facetValue->toInteger() >= value) {
         errorMsg = QtXmlPatterns::tr("Signed integer content does not match the minExclusive facet.");
         return false;
      }
   }

   if (facets.contains(XsdFacet::Enumeration)) {
      const XsdFacet::Ptr facet = facets.value(XsdFacet::Enumeration);
      const DerivedString<TypeString>::Ptr valueStr = DerivedString<TypeString>::fromLexical(m_namePool,
            QString::number(value));

      const AtomicValue::List multiValue = facet->multiValue();
      bool found = false;

      for (int j = 0; j < multiValue.count(); ++j) {
         if (XsdSchemaHelper::constructAndCompare(valueStr, AtomicComparator::OperatorEqual, multiValue.at(j),
               BuiltinTypes::xsLong, m_context, m_reflection)) {
            found = true;
            break;
         }
      }

      if (!found) {
         errorMsg = QtXmlPatterns::tr("Signed integer content is not listed in the enumeration facet.");
         return false;
      }
   }
   if (facets.contains(XsdFacet::Pattern)) {
      const XsdFacet::Ptr facet = facets.value(XsdFacet::Pattern);
      const AtomicValue::List multiValue = facet->multiValue();
      bool found = false;
      for (int j = 0; j < multiValue.count(); ++j) {
         const QString pattern = multiValue.at(j)->as<DerivedString<TypeString> >()->stringValue();
         const QRegularExpression exp = PatternPlatform::parsePattern(pattern, QPatternOption::ExactMatchOption, m_context, m_reflection);

         if (exp.match(lexicalValue).hasMatch()) {
            found = true;
            break;
         }
      }

      if (!found) {
         errorMsg = QtXmlPatterns::tr("Signed integer content does not match pattern facet.");
         return false;
      }
   }
   if (facets.contains(XsdFacet::TotalDigits)) {
      const XsdFacet::Ptr facet = facets.value(XsdFacet::TotalDigits);
      const DerivedInteger<TypePositiveInteger>::Ptr facetValue = facet->value();

      if (totalDigitsForSignedLongLong(value) > facetValue->toInteger()) {
         errorMsg = QtXmlPatterns::tr("Signed integer content does not match in the totalDigits facet.");
         return false;
      }
   }
   if (facets.contains(XsdFacet::Assertion)) {
      //TODO: implement
   }

   return true;
}

bool XsdTypeChecker::checkConstrainingFacetsUnsignedInteger(unsigned long long value, const QString &lexicalValue,
      const XsdFacet::Hash &facets, QString &errorMsg) const
{
   if (facets.contains(XsdFacet::MaximumInclusive)) {
      const XsdFacet::Ptr facet = facets.value(XsdFacet::MaximumInclusive);
      const Numeric::Ptr facetValue = ValueFactory::fromLexical(
                                         facet->value()->as<DerivedString<TypeString> >()->stringValue(), BuiltinTypes::xsUnsignedLong, m_context, m_reflection);
      if (facetValue->toUnsignedInteger() < value) {
         errorMsg = QtXmlPatterns::tr("Unsigned integer content does not match the maxInclusive facet.");
         return false;
      }
   }
   if (facets.contains(XsdFacet::MaximumExclusive)) {
      const XsdFacet::Ptr facet = facets.value(XsdFacet::MaximumExclusive);
      const Numeric::Ptr facetValue = ValueFactory::fromLexical(
                                         facet->value()->as<DerivedString<TypeString> >()->stringValue(), BuiltinTypes::xsUnsignedLong, m_context, m_reflection);
      if (facetValue->toUnsignedInteger() <= value) {
         errorMsg = QtXmlPatterns::tr("Unsigned integer content does not match the maxExclusive facet.");
         return false;
      }
   }
   if (facets.contains(XsdFacet::MinimumInclusive)) {
      const XsdFacet::Ptr facet = facets.value(XsdFacet::MinimumInclusive);
      const Numeric::Ptr facetValue = ValueFactory::fromLexical(
                                         facet->value()->as<DerivedString<TypeString> >()->stringValue(), BuiltinTypes::xsUnsignedLong, m_context, m_reflection);
      if (facetValue->toUnsignedInteger() > value) {
         errorMsg = QtXmlPatterns::tr("Unsigned integer content does not match the minInclusive facet.");
         return false;
      }
   }
   if (facets.contains(XsdFacet::MinimumExclusive)) {
      const XsdFacet::Ptr facet = facets.value(XsdFacet::MinimumExclusive);
      const Numeric::Ptr facetValue = ValueFactory::fromLexical(
                                         facet->value()->as<DerivedString<TypeString> >()->stringValue(), BuiltinTypes::xsUnsignedLong, m_context, m_reflection);
      if (facetValue->toUnsignedInteger() >= value) {
         errorMsg = QtXmlPatterns::tr("Unsigned integer content does not match the minExclusive facet.");
         return false;
      }
   }
   if (facets.contains(XsdFacet::Enumeration)) {
      const XsdFacet::Ptr facet = facets.value(XsdFacet::Enumeration);
      const DerivedString<TypeString>::Ptr valueStr = DerivedString<TypeString>::fromLexical(m_namePool,
            QString::number(value));

      const AtomicValue::List multiValue = facet->multiValue();
      bool found = false;

      for (int j = 0; j < multiValue.count(); ++j) {
         if (XsdSchemaHelper::constructAndCompare(valueStr, AtomicComparator::OperatorEqual, multiValue.at(j),
               BuiltinTypes::xsUnsignedLong, m_context, m_reflection)) {
            found = true;
            break;
         }
      }

      if (!found) {
         errorMsg = QtXmlPatterns::tr("Unsigned integer content is not listed in the enumeration facet.");
         return false;
      }
   }
   if (facets.contains(XsdFacet::Pattern)) {
      const XsdFacet::Ptr facet = facets.value(XsdFacet::Pattern);
      const AtomicValue::List multiValue = facet->multiValue();
      bool found = false;

      for (int j = 0; j < multiValue.count(); ++j) {
         const QString pattern = multiValue.at(j)->as<DerivedString<TypeString> >()->stringValue();
         const QRegularExpression exp = PatternPlatform::parsePattern(pattern, QPatternOption::ExactMatchOption, m_context, m_reflection);

         if (exp.match(lexicalValue).hasMatch()) {
            found = true;
            break;
         }
      }

      if (!found) {
         errorMsg = QtXmlPatterns::tr("Unsigned integer content does not match pattern facet.");
         return false;
      }
   }
   if (facets.contains(XsdFacet::TotalDigits)) {
      const XsdFacet::Ptr facet = facets.value(XsdFacet::TotalDigits);
      const DerivedInteger<TypePositiveInteger>::Ptr facetValue = facet->value();

      if (totalDigitsForUnsignedLongLong(value) > facetValue->toInteger()) {
         errorMsg = QtXmlPatterns::tr("Unsigned integer content does not match in the totalDigits facet.");
         return false;
      }
   }
   if (facets.contains(XsdFacet::Assertion)) {
      //TODO: implement
   }

   return true;
}

bool XsdTypeChecker::checkConstrainingFacetsDouble(double value, const QString &lexicalValue,
      const XsdFacet::Hash &facets, QString &errorMsg) const
{
   if (facets.contains(XsdFacet::MaximumInclusive)) {
      const XsdFacet::Ptr facet = facets.value(XsdFacet::MaximumInclusive);
      const Numeric::Ptr facetValue = ValueFactory::fromLexical(
                                         facet->value()->as<DerivedString<TypeString> >()->stringValue(), BuiltinTypes::xsDouble, m_context, m_reflection);
      if (facetValue->toDouble() < value) {
         errorMsg = QtXmlPatterns::tr("Double content does not match the maxInclusive facet.");
         return false;
      }
   }
   if (facets.contains(XsdFacet::MaximumExclusive)) {
      const XsdFacet::Ptr facet = facets.value(XsdFacet::MaximumExclusive);
      const Numeric::Ptr facetValue = ValueFactory::fromLexical(
                                         facet->value()->as<DerivedString<TypeString> >()->stringValue(), BuiltinTypes::xsDouble, m_context, m_reflection);
      if (facetValue->toDouble() <= value) {
         errorMsg = QtXmlPatterns::tr("Double content does not match the maxExclusive facet.");
         return false;
      }
   }
   if (facets.contains(XsdFacet::MinimumInclusive)) {
      const XsdFacet::Ptr facet = facets.value(XsdFacet::MinimumInclusive);
      const Numeric::Ptr facetValue = ValueFactory::fromLexical(
                                         facet->value()->as<DerivedString<TypeString> >()->stringValue(), BuiltinTypes::xsDouble, m_context, m_reflection);
      if (facetValue->toDouble() > value) {
         errorMsg = QtXmlPatterns::tr("Double content does not match the minInclusive facet.");
         return false;
      }
   }
   if (facets.contains(XsdFacet::MinimumExclusive)) {
      const XsdFacet::Ptr facet = facets.value(XsdFacet::MinimumExclusive);
      const Numeric::Ptr facetValue = ValueFactory::fromLexical(
                                         facet->value()->as<DerivedString<TypeString> >()->stringValue(), BuiltinTypes::xsDouble, m_context, m_reflection);
      if (facetValue->toDouble() >= value) {
         errorMsg = QtXmlPatterns::tr("Double content does not match the minExclusive facet.");
         return false;
      }
   }
   if (facets.contains(XsdFacet::Enumeration)) {
      const XsdFacet::Ptr facet = facets.value(XsdFacet::Enumeration);
      const Numeric::Ptr valuePtr = Double::fromValue(value);
      const DerivedString<TypeString>::Ptr valueStr = DerivedString<TypeString>::fromLexical(m_namePool,
            valuePtr->stringValue());

      const AtomicValue::List multiValue = facet->multiValue();
      bool found = false;
      for (int j = 0; j < multiValue.count(); ++j) {
         if (XsdSchemaHelper::constructAndCompare(valueStr, AtomicComparator::OperatorEqual, multiValue.at(j),
               BuiltinTypes::xsDouble, m_context, m_reflection)) {
            found = true;
            break;
         }

         // Handle case when both facet and value are NaN separately as equals for NaN returns always false.
         const Numeric::Ptr facetValue = ValueFactory::fromLexical(multiValue.at(
                                            j)->as<DerivedString<TypeString> >()->stringValue(), BuiltinTypes::xsDouble, m_context, m_reflection);
         if (facetValue->isNaN() && valuePtr->isNaN()) {
            found = true;
            break;
         }
      }

      if (!found) {
         errorMsg = QtXmlPatterns::tr("Double content is not listed in the enumeration facet.");
         return false;
      }
   }
   if (facets.contains(XsdFacet::Pattern)) {
      const XsdFacet::Ptr facet = facets.value(XsdFacet::Pattern);
      const AtomicValue::List multiValue = facet->multiValue();
      bool found = false;

      for (int j = 0; j < multiValue.count(); ++j) {
         const QString pattern = multiValue.at(j)->as<DerivedString<TypeString> >()->stringValue();
         const QRegularExpression exp = PatternPlatform::parsePattern(pattern, QPatternOption::ExactMatchOption, m_context, m_reflection);

         if (exp.match(lexicalValue).hasMatch()) {
            found = true;
            break;
         }
      }

      if (!found) {
         errorMsg = QtXmlPatterns::tr("Double content does not match pattern facet.");
         return false;
      }
   }
   if (facets.contains(XsdFacet::Assertion)) {
      //TODO: implement
   }

   return true;
}

bool XsdTypeChecker::checkConstrainingFacetsDecimal(const AtomicValue::Ptr &value, const QString &lexicalValue,
      const XsdFacet::Hash &facets, QString &errorMsg) const
{
   if (facets.contains(XsdFacet::FractionDigits)) {
      const XsdFacet::Ptr facet = facets.value(XsdFacet::FractionDigits);
      const DerivedInteger<TypePositiveInteger>::Ptr facetValue = facet->value();

      if (fractionDigitsForDecimal(lexicalValue) > facetValue->toInteger()) {
         errorMsg = QtXmlPatterns::tr("Decimal content does not match in the fractionDigits facet.");
         return false;
      }
   }
   if (facets.contains(XsdFacet::TotalDigits)) {
      const XsdFacet::Ptr facet = facets.value(XsdFacet::TotalDigits);
      const DerivedInteger<TypePositiveInteger>::Ptr facetValue = facet->value();

      if (totalDigitsForDecimal(lexicalValue) > facetValue->toInteger()) {
         errorMsg = QtXmlPatterns::tr("Decimal content does not match in the totalDigits facet.");
         return false;
      }
   }

   return checkConstrainingFacetsDouble(value->as<Decimal>()->toDouble(), lexicalValue, facets, errorMsg);
}

bool XsdTypeChecker::checkConstrainingFacetsDateTime(const QDateTime &value, const QString &lexicalValue,
      const XsdFacet::Hash &facets, const AnySimpleType::Ptr &type, QString &errorMsg) const
{
   if (facets.contains(XsdFacet::MaximumInclusive)) {
      const XsdFacet::Ptr facet = facets.value(XsdFacet::MaximumInclusive);
      const AbstractDateTime::Ptr facetValue = ValueFactory::fromLexical(
               facet->value()->as<DerivedString<TypeString> >()->stringValue(), type, m_context, m_reflection);
      if (facetValue->toDateTime() < value) {
         errorMsg = QtXmlPatterns::tr("Date time content does not match the maxInclusive facet.");
         return false;
      }
   }
   if (facets.contains(XsdFacet::MaximumExclusive)) {
      const XsdFacet::Ptr facet = facets.value(XsdFacet::MaximumExclusive);
      const AbstractDateTime::Ptr facetValue = ValueFactory::fromLexical(
               facet->value()->as<DerivedString<TypeString> >()->stringValue(), type, m_context, m_reflection);
      if (facetValue->toDateTime() <= value) {
         errorMsg = QtXmlPatterns::tr("Date time content does not match the maxExclusive facet.");
         return false;
      }
   }
   if (facets.contains(XsdFacet::MinimumInclusive)) {
      const XsdFacet::Ptr facet = facets.value(XsdFacet::MinimumInclusive);
      const AbstractDateTime::Ptr facetValue = ValueFactory::fromLexical(
               facet->value()->as<DerivedString<TypeString> >()->stringValue(), type, m_context, m_reflection);
      if (facetValue->toDateTime() > value) {
         errorMsg = QtXmlPatterns::tr("Date time content does not match the minInclusive facet.");
         return false;
      }
   }
   if (facets.contains(XsdFacet::MinimumExclusive)) {
      const XsdFacet::Ptr facet = facets.value(XsdFacet::MinimumExclusive);
      const AbstractDateTime::Ptr facetValue = ValueFactory::fromLexical(
               facet->value()->as<DerivedString<TypeString> >()->stringValue(), type, m_context, m_reflection);
      if (facetValue->toDateTime() >= value) {
         errorMsg = QtXmlPatterns::tr("Date time content does not match the minExclusive facet.");
         return false;
      }
   }
   if (facets.contains(XsdFacet::Enumeration)) {
      const XsdFacet::Ptr facet = facets.value(XsdFacet::Enumeration);

      const AtomicValue::List multiValue = facet->multiValue();
      bool found = false;
      for (int j = 0; j < multiValue.count(); ++j) {
         const AbstractDateTime::Ptr facetValue = ValueFactory::fromLexical(multiValue.at(
                  j)->as<DerivedString<TypeString> >()->stringValue(), type, m_context, m_reflection);
         if (facetValue->toDateTime() == value) {
            found = true;
            break;
         }
      }

      if (!found) {
         errorMsg = QtXmlPatterns::tr("Date time content is not listed in the enumeration facet.");
         return false;
      }
   }
   if (facets.contains(XsdFacet::Pattern)) {
      const XsdFacet::Ptr facet = facets.value(XsdFacet::Pattern);
      const AtomicValue::List multiValue = facet->multiValue();
      bool found = false;

      for (int j = 0; j < multiValue.count(); ++j) {
         const QString pattern = multiValue.at(j)->as<DerivedString<TypeString> >()->stringValue();
         const QRegularExpression exp = PatternPlatform::parsePattern(pattern, QPatternOption::ExactMatchOption, m_context, m_reflection);

         if (exp.match(lexicalValue).hasMatch()) {
            found = true;
            break;
         }
      }

      if (!found) {
         errorMsg = QtXmlPatterns::tr("Date time content does not match pattern facet.");
         return false;
      }
   }

   return true;
}

bool XsdTypeChecker::checkConstrainingFacetsDuration(const AtomicValue::Ptr &, const QString &lexicalValue,
      const XsdFacet::Hash &facets, QString &errorMsg) const
{
   if (facets.contains(XsdFacet::MaximumInclusive)) {
      const XsdFacet::Ptr facet = facets.value(XsdFacet::MaximumInclusive);
      const DerivedString<TypeString>::Ptr value = DerivedString<TypeString>::fromLexical(m_namePool, lexicalValue);

      if (XsdSchemaHelper::constructAndCompare(facets.value(XsdFacet::MaximumInclusive)->value(),
            AtomicComparator::OperatorLessThan, value, BuiltinTypes::xsDuration, m_context, m_reflection)) {
         errorMsg = QtXmlPatterns::tr("Duration content does not match the maxInclusive facet.");
         return false;
      }
   }
   if (facets.contains(XsdFacet::MaximumExclusive)) {
      const XsdFacet::Ptr facet = facets.value(XsdFacet::MaximumExclusive);
      const DerivedString<TypeString>::Ptr value = DerivedString<TypeString>::fromLexical(m_namePool, lexicalValue);

      if (XsdSchemaHelper::constructAndCompare(facets.value(XsdFacet::MaximumExclusive)->value(),
            AtomicComparator::OperatorLessOrEqual, value, BuiltinTypes::xsDuration, m_context, m_reflection)) {
         errorMsg = QtXmlPatterns::tr("Duration content does not match the maxExclusive facet.");
         return false;
      }
   }
   if (facets.contains(XsdFacet::MinimumInclusive)) {
      const XsdFacet::Ptr facet = facets.value(XsdFacet::MinimumInclusive);
      const DerivedString<TypeString>::Ptr value = DerivedString<TypeString>::fromLexical(m_namePool, lexicalValue);

      if (XsdSchemaHelper::constructAndCompare(facets.value(XsdFacet::MinimumInclusive)->value(),
            AtomicComparator::OperatorGreaterThan, value, BuiltinTypes::xsDuration, m_context, m_reflection)) {
         errorMsg = QtXmlPatterns::tr("Duration content does not match the minInclusive facet.");
         return false;
      }
   }
   if (facets.contains(XsdFacet::MinimumExclusive)) {
      const XsdFacet::Ptr facet = facets.value(XsdFacet::MinimumExclusive);
      const DerivedString<TypeString>::Ptr value = DerivedString<TypeString>::fromLexical(m_namePool, lexicalValue);

      if (XsdSchemaHelper::constructAndCompare(facets.value(XsdFacet::MinimumExclusive)->value(),
            AtomicComparator::OperatorGreaterOrEqual, value, BuiltinTypes::xsDuration, m_context, m_reflection)) {
         errorMsg = QtXmlPatterns::tr("Duration content does not match the minExclusive facet.");
         return false;
      }
   }
   if (facets.contains(XsdFacet::Enumeration)) {
      const XsdFacet::Ptr facet = facets.value(XsdFacet::Enumeration);
      const DerivedString<TypeString>::Ptr value = DerivedString<TypeString>::fromLexical(m_namePool, lexicalValue);

      const AtomicValue::List multiValue = facet->multiValue();
      bool found = false;
      for (int j = 0; j < multiValue.count(); ++j) {
         if (XsdSchemaHelper::constructAndCompare(multiValue.at(j), AtomicComparator::OperatorEqual, value,
               BuiltinTypes::xsDuration, m_context, m_reflection)) {
            found = true;
            break;
         }
      }

      if (!found) {
         errorMsg = QtXmlPatterns::tr("Duration content is not listed in the enumeration facet.");
         return false;
      }
   }
   if (facets.contains(XsdFacet::Pattern)) {
      const XsdFacet::Ptr facet = facets.value(XsdFacet::Pattern);
      const AtomicValue::List multiValue = facet->multiValue();
      bool found = false;

      for (int j = 0; j < multiValue.count(); ++j) {
         const QString pattern = multiValue.at(j)->as<DerivedString<TypeString> >()->stringValue();
         const QRegularExpression exp = PatternPlatform::parsePattern(pattern, QPatternOption::ExactMatchOption, m_context, m_reflection);

         if (exp.match(lexicalValue).hasMatch()) {
            found = true;
            break;
         }
      }

      if (!found) {
         errorMsg = QtXmlPatterns::tr("Duration content does not match pattern facet.");
         return false;
      }
   }
   if (facets.contains(XsdFacet::Assertion)) {
      //TODO: implement
   }

   return true;
}

bool XsdTypeChecker::checkConstrainingFacetsBoolean(bool, const QString &lexicalValue, const XsdFacet::Hash &facets,
      QString &errorMsg) const
{
   if (facets.contains(XsdFacet::Pattern)) {
      const XsdFacet::Ptr facet = facets.value(XsdFacet::Pattern);
      const AtomicValue::List multiValue = facet->multiValue();
      bool found = false;

      for (int j = 0; j < multiValue.count(); ++j) {
         const QString pattern = multiValue.at(j)->as<DerivedString<TypeString> >()->stringValue();
         const QRegularExpression exp = PatternPlatform::parsePattern(pattern, QPatternOption::ExactMatchOption, m_context, m_reflection);

         if (exp.match(lexicalValue).hasMatch()) {
            found = true;
            break;
         }
      }

      if (!found) {
         errorMsg = QtXmlPatterns::tr("Boolean content does not match pattern facet.");
         return false;
      }
   }
   if (facets.contains(XsdFacet::Assertion)) {
      //TODO: implement
   }

   return true;
}

bool XsdTypeChecker::checkConstrainingFacetsBinary(const QByteArray &value, const XsdFacet::Hash &facets,
      const AnySimpleType::Ptr &type, QString &errorMsg) const
{
   if (facets.contains(XsdFacet::Length)) {
      const XsdFacet::Ptr facet = facets.value(XsdFacet::Length);
      const DerivedInteger<TypeNonNegativeInteger>::Ptr length = facet->value();
      if (length->toInteger() != value.length()) {
         errorMsg = QtXmlPatterns::tr("Binary content does not match the length facet.");
         return false;
      }
   }
   if (facets.contains(XsdFacet::MinimumLength)) {
      const XsdFacet::Ptr facet = facets.value(XsdFacet::MinimumLength);
      const DerivedInteger<TypeNonNegativeInteger>::Ptr length = facet->value();
      if (length->toInteger() > value.length()) {
         errorMsg = QtXmlPatterns::tr("Binary content does not match the minLength facet.");
         return false;
      }
   }
   if (facets.contains(XsdFacet::MaximumLength)) {
      const XsdFacet::Ptr facet = facets.value(XsdFacet::MaximumLength);
      const DerivedInteger<TypeNonNegativeInteger>::Ptr length = facet->value();
      if (length->toInteger() < value.length()) {
         errorMsg = QtXmlPatterns::tr("Binary content does not match the maxLength facet.");
         return false;
      }
   }
   if (facets.contains(XsdFacet::Enumeration)) {
      const XsdFacet::Ptr facet = facets.value(XsdFacet::Enumeration);
      const AtomicValue::List multiValue = facet->multiValue();
      bool found = false;
      for (int j = 0; j < multiValue.count(); ++j) {
         const Base64Binary::Ptr binary = ValueFactory::fromLexical(multiValue.at(
                                             j)->as<DerivedString<TypeString> >()->stringValue(), type, m_context, m_reflection);
         const QByteArray facetValue = binary->as<Base64Binary>()->asByteArray();
         if (value == facetValue) {
            found = true;
            break;
         }
      }

      if (!found) {
         errorMsg = QtXmlPatterns::tr("Binary content is not listed in the enumeration facet.");
         return false;
      }
   }
   if (facets.contains(XsdFacet::Pattern)) {
      //TODO: implement
   }
   if (facets.contains(XsdFacet::Assertion)) {
      //TODO: implement
   }

   return true;
}

bool XsdTypeChecker::checkConstrainingFacetsQName(const QXmlName &value, const QString &lexicalValue,
      const XsdFacet::Hash &facets, QString &errorMsg) const
{
   if (facets.contains(XsdFacet::Length)) {
      // always true
   }
   if (facets.contains(XsdFacet::MinimumLength)) {
      // always true
   }
   if (facets.contains(XsdFacet::MaximumLength)) {
      // always true
   }
   if (facets.contains(XsdFacet::Enumeration)) {
      if (! XPathHelper::isQName(lexicalValue)) {
         errorMsg = QtXmlPatterns::tr("Invalid QName content: %1.").formatArg(formatData(lexicalValue));
         return false;
      }

      const XsdFacet::Ptr facet = facets.value(XsdFacet::Enumeration);
      const AtomicValue::List multiValue = facet->multiValue();
      bool found = false;
      for (int j = 0; j < multiValue.count(); ++j) {
         const QXmlName facetValue = multiValue.at(j)->as<QNameValue>()->qName();

         if (value == facetValue) {
            found = true;
            break;
         }
      }

      if (! found) {
         errorMsg = QtXmlPatterns::tr("QName content is not listed in the enumeration facet.");
         return false;
      }
   }

   if (facets.contains(XsdFacet::Pattern)) {
      const XsdFacet::Ptr facet = facets.value(XsdFacet::Pattern);
      const AtomicValue::List multiValue = facet->multiValue();
      bool found = false;

      for (int j = 0; j < multiValue.count(); ++j) {
         const QString pattern = multiValue.at(j)->as<DerivedString<TypeString> >()->stringValue();
         const QRegularExpression exp = PatternPlatform::parsePattern(pattern, QPatternOption::ExactMatchOption, m_context, m_reflection);

         if (exp.match(lexicalValue).hasMatch()) {
            found = true;
            break;
         }
      }

      if (!found) {
         errorMsg = QtXmlPatterns::tr("QName content does not match pattern facet.");
         return false;
      }
   }
   if (facets.contains(XsdFacet::Assertion)) {
      //TODO: implement
   }

   return true;
}

bool XsdTypeChecker::checkConstrainingFacetsNotation(const QXmlName &value, const XsdFacet::Hash &facets,
      QString &errorMsg) const
{
   if (facets.contains(XsdFacet::Length)) {
      // deprecated by spec
   }
   if (facets.contains(XsdFacet::MinimumLength)) {
      // deprecated by spec
   }
   if (facets.contains(XsdFacet::MaximumLength)) {
      // deprecated by spec
   }
   if (facets.contains(XsdFacet::Enumeration)) {
      const XsdFacet::Ptr facet = facets.value(XsdFacet::Enumeration);
      const AtomicValue::List multiValue = facet->multiValue();
      bool found = false;
      for (int j = 0; j < multiValue.count(); ++j) {
         const QXmlName facetValue = multiValue.at(j)->as<QNameValue>()->qName();

         if (value == facetValue) {
            found = true;
            break;
         }
      }

      if (!found) {
         errorMsg = QtXmlPatterns::tr("Notation content is not listed in the enumeration facet.");
         return false;
      }
   }
   if (facets.contains(XsdFacet::Pattern)) {
      //TODO: implement
   }
   if (facets.contains(XsdFacet::Assertion)) {
      //TODO: implement
   }

   return true;
}

bool XsdTypeChecker::checkConstrainingFacetsList(const QStringList &values, const QString &lexicalValue,
      const AnySimpleType::Ptr &itemType, const XsdFacet::Hash &facets, QString &errorMsg) const
{
   if (facets.contains(XsdFacet::Length)) {
      const DerivedInteger<TypeNonNegativeInteger>::Ptr value = facets.value(XsdFacet::Length)->value();
      if (value->toInteger() != values.count()) {
         errorMsg = QtXmlPatterns::tr("List content does not match length facet.");
         return false;
      }
   }
   if (facets.contains(XsdFacet::MinimumLength)) {
      const DerivedInteger<TypeNonNegativeInteger>::Ptr value = facets.value(XsdFacet::MinimumLength)->value();
      if (value->toInteger() > values.count()) {
         errorMsg = QtXmlPatterns::tr("List content does not match minLength facet.");
         return false;
      }
   }
   if (facets.contains(XsdFacet::MaximumLength)) {
      const DerivedInteger<TypeNonNegativeInteger>::Ptr value = facets.value(XsdFacet::MaximumLength)->value();
      if (value->toInteger() < values.count()) {
         errorMsg = QtXmlPatterns::tr("List content does not match maxLength facet.");
         return false;
      }
   }
   if (facets.contains(XsdFacet::Enumeration)) {

      bool found = false;

      // we have to handle lists with QName derived items differently
      if (BuiltinTypes::xsQName->wxsTypeMatches(itemType) || BuiltinTypes::xsNOTATION->wxsTypeMatches(itemType)) {
         // first convert the string values from the instance document to a list of QXmlName
         QList<QXmlName> instanceValues;
         for (int i = 0; i < values.count(); ++i) {
            instanceValues.append(convertToQName(values.at(i)));
         }

         // fetch the values from the facet and create a list of QXmlNames for each of them
         const XsdFacet::Ptr facet = facets.value(XsdFacet::Enumeration);

         const AtomicValue::List multiValue = facet->multiValue();
         for (int i = 0; i < multiValue.count(); ++i) {
            const QStringList facetValueList = multiValue.at(i)->as<DerivedString<TypeString> >()->stringValue().split(
                                                  QLatin1Char(' '), QStringParser::SkipEmptyParts);

            // create the list of atomic string values
            QList<QXmlName> facetValues;
            for (int j = 0; j < facetValueList.count(); ++j) {
               facetValues.append(convertToQName(facetValueList.at(j)));
            }

            // check if both lists have the same length
            if (instanceValues.count() != facetValues.count()) {
               continue;
            }

            // check if both lists are equal, that means the contain equal items in the same order
            bool matchesAll = true;
            for (int j = 0; j < instanceValues.count(); ++j) {
               if (instanceValues.at(j) != facetValues.at(j)) {
                  matchesAll = false;
                  break;
               }
            }

            if (matchesAll) {
               found = true;
               break;
            }
         }
      } else {
         // first convert the string values from the instance document to atomic values of type string
         AtomicValue::List instanceValues;
         for (int i = 0; i < values.count(); ++i) {
            instanceValues.append(DerivedString<TypeString>::fromLexical(m_namePool, values.at(i)));
         }

         // fetch the values from the facet and create a list of atomic string values for each of them
         const XsdFacet::Ptr facet = facets.value(XsdFacet::Enumeration);

         const AnySimpleType::Ptr targetType = comparableType(itemType);

         const AtomicValue::List multiValue = facet->multiValue();
         for (int i = 0; i < multiValue.count(); ++i) {
            const QStringList facetValueList = multiValue.at(i)->as<DerivedString<TypeString> >()->stringValue().split(
                                                  QLatin1Char(' '), QStringParser::SkipEmptyParts);

            // create the list of atomic string values
            AtomicValue::List facetValues;
            for (int j = 0; j < facetValueList.count(); ++j) {
               facetValues.append(DerivedString<TypeString>::fromLexical(m_namePool, facetValueList.at(j)));
            }

            // check if both lists have the same length
            if (instanceValues.count() != facetValues.count()) {
               continue;
            }

            // check if both lists are equal, that means the contain equal items in the same order
            bool matchesAll = true;
            for (int j = 0; j < instanceValues.count(); ++j) {
               if (!XsdSchemaHelper::constructAndCompare(instanceValues.at(j), AtomicComparator::OperatorEqual, facetValues.at(j),
                     targetType, m_context, m_reflection)) {
                  matchesAll = false;
                  break;
               }
            }

            if (matchesAll) {
               found = true;
               break;
            }
         }
      }

      if (!found) {
         errorMsg = QtXmlPatterns::tr("List content is not listed in the enumeration facet.");
         return false;
      }
   }
   if (facets.contains(XsdFacet::Pattern)) {
      const XsdFacet::Ptr facet = facets.value(XsdFacet::Pattern);
      const AtomicValue::List multiValue = facet->multiValue();
      bool found = false;

      for (int j = 0; j < multiValue.count(); ++j) {
         const QString pattern = multiValue.at(j)->as<DerivedString<TypeString> >()->stringValue();
         const QRegularExpression exp = PatternPlatform::parsePattern(pattern, QPatternOption::ExactMatchOption, m_context, m_reflection);

         if (exp.match(lexicalValue).hasMatch()) {
            found = true;
            break;
         }
      }

      if (!found) {
         errorMsg = QtXmlPatterns::tr("List content does not match pattern facet.");
         return false;
      }
   }
   if (facets.contains(XsdFacet::Assertion)) {
      //TODO: implement
   }

   return true;
}

bool XsdTypeChecker::checkConstrainingFacetsUnion(const QString &value, const QString &lexicalValue,
      const XsdSimpleType::Ptr &simpleType, const XsdFacet::Hash &facets, QString &errorMsg) const
{
   if (facets.contains(XsdFacet::Enumeration)) {
      const AnySimpleType::List memberTypes = simpleType->memberTypes();

      const XsdFacet::Ptr facet = facets.value(XsdFacet::Enumeration);

      // convert the instance value into an atomic string value
      const DerivedString<TypeString>::Ptr valueString = DerivedString<TypeString>::fromLexical(m_namePool, value);

      // collect the facet values into a list of atomic string values
      const AtomicValue::List facetValues = facet->multiValue();

      // compare the instance value against the facetValues for each member type and
      // search for a match

      bool found = false;
      for (int i = 0; i < memberTypes.count(); ++i) {
         const AnySimpleType::Ptr targetType = comparableType(memberTypes.at(i));
         for (int j = 0; j < facetValues.count(); ++j) {
            if (XsdSchemaHelper::constructAndCompare(valueString, AtomicComparator::OperatorEqual, facetValues.at(j), targetType,
                  m_context, m_reflection)) {
               found = true;
               break;
            }
         }
      }

      if (!found) {
         errorMsg = QtXmlPatterns::tr("Union content is not listed in the enumeration facet.");
         return false;
      }
   }
   if (facets.contains(XsdFacet::Pattern)) {
      const XsdFacet::Ptr facet = facets.value(XsdFacet::Pattern);
      const AtomicValue::List multiValue = facet->multiValue();
      bool found = false;

      for (int j = 0; j < multiValue.count(); ++j) {
         const QString pattern = multiValue.at(j)->as<DerivedString<TypeString> >()->stringValue();
         const QRegularExpression exp = PatternPlatform::parsePattern(pattern, QPatternOption::ExactMatchOption, m_context, m_reflection);

         if (exp.match(lexicalValue).hasMatch()) {
            found = true;
            break;
         }
      }

      if (!found) {
         errorMsg = QtXmlPatterns::tr("Union content does not match pattern facet.");
         return false;
      }
   }
   if (facets.contains(XsdFacet::Assertion)) {
      //TODO: implement
   }

   return true;
}

AtomicValue::Ptr XsdTypeChecker::fromLexical(const QString &value, const SchemaType::Ptr &type,
      const ReportContext::Ptr &context, const SourceLocationReflection *const reflection) const
{
   if (type->name(m_namePool) == BuiltinTypes::xsNOTATION->name(m_namePool) ||
         type->name(m_namePool) == BuiltinTypes::xsQName->name(m_namePool)) {
      if (value.simplified().isEmpty()) {
         return ValidationError::createError(QtXmlPatterns::tr("Data of type %1 are not allowed to be empty.").formatArg(formatType(
                                                m_namePool, BuiltinTypes::xsNOTATION)));
      }

      const QXmlName valueName = convertToQName(value);
      return QNameValue::fromValue(m_namePool, valueName);
   } else {
      return ValueFactory::fromLexical(value, type, context, reflection);
   }
}

QXmlName XsdTypeChecker::convertToQName(const QString &name) const
{
   const int pos = name.indexOf(QLatin1Char(':'));

   QXmlName::PrefixCode prefixCode = 0;
   QXmlName::NamespaceCode namespaceCode;
   QXmlName::LocalNameCode localNameCode;
   if (pos != -1) {
      prefixCode = m_context->namePool()->allocatePrefix(name.left(pos));
      namespaceCode = StandardNamespaces::empty;
      for (int i = 0; i < m_namespaceBindings.count(); ++i) {
         if (m_namespaceBindings.at(i).prefix() == prefixCode) {
            namespaceCode = m_namespaceBindings.at(i).namespaceURI();
            break;
         }
      }
      localNameCode = m_context->namePool()->allocateLocalName(name.mid(pos + 1));
   } else {
      prefixCode = StandardPrefixes::empty;
      namespaceCode = StandardNamespaces::empty;
      for (int i = 0; i < m_namespaceBindings.count(); ++i) {
         if (m_namespaceBindings.at(i).prefix() == prefixCode) {
            namespaceCode = m_namespaceBindings.at(i).namespaceURI();
            break;
         }
      }
      localNameCode = m_context->namePool()->allocateLocalName(name);
   }

   return QXmlName(namespaceCode, localNameCode, prefixCode);
}
