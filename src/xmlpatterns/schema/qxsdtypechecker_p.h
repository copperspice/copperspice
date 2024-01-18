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

#ifndef QXsdTypeChecker_P_H
#define QXsdTypeChecker_P_H

#include <QSourceLocation>
#include <qschematype_p.h>
#include <qsourcelocationreflection_p.h>
#include <qxsdschemacontext_p.h>

class QXmlQuery;

namespace QPatternist {

class XsdSchemaSourceLocationReflection : public SourceLocationReflection
{
 public:
   XsdSchemaSourceLocationReflection(const QSourceLocation &location);

   const SourceLocationReflection *actualReflection() const override;
   QSourceLocation sourceLocation() const override;

 private:
   const QSourceLocation m_sourceLocation;
};

/**
 * @short The class that provides methods for checking a string against a type.
 *
 * The class provides functionality for type-aware string handling.
 */
class XsdTypeChecker
{
 public:
   /**
    * Creates a new type checker.
    *
    * @param context The schema context that is used for error reporting.
    * @param namespaceBindings The namespace bindings that shall be used to check against xs:QName based types.
    * @param location The source location that is used for error reporting.
    */
   XsdTypeChecker(const XsdSchemaContext::Ptr &context, const QVector<QXmlName> &namespaceBindings,
                  const QSourceLocation &location);

   /**
    * Destroys the type checker.
    */
   ~XsdTypeChecker();


   static XsdFacet::Hash mergedFacetsForType(const SchemaType::Ptr &type, const XsdSchemaContext::Ptr &context);

   static QString normalizedValue(const QString &value, const XsdFacet::Hash &facets);

   bool isValidString(const QString &normalizedString, const AnySimpleType::Ptr &type, QString &errorMsg,
                      AnySimpleType::Ptr *boundType = nullptr) const;

   bool valuesAreEqual(const QString &value, const QString &otherValue, const AnySimpleType::Ptr &type) const;

 private:
   XsdTypeChecker(const XsdTypeChecker &) = delete;
   XsdTypeChecker &operator=(const XsdTypeChecker &) = delete;

   bool checkConstrainingFacets(const AtomicValue::Ptr &value, const QString &lexicalValue,
                                const AnySimpleType::Ptr &type, QString &errorMsg) const;

   bool checkConstrainingFacetsString(const QString &value, const XsdFacet::Hash &facets,
                                      const AnySimpleType::Ptr &type, QString &errorMsg) const;

   bool checkConstrainingFacetsSignedInteger(long long value, const QString &lexicalValue,
         const XsdFacet::Hash &facets, QString &errorMsg) const;

   bool checkConstrainingFacetsUnsignedInteger(unsigned long long value, const QString &lexicalValue,
         const XsdFacet::Hash &facets, QString &errorMsg) const;
   bool checkConstrainingFacetsDouble(double value, const QString &lexicalValue, const XsdFacet::Hash &facets,
                                      QString &errorMsg) const;

   bool checkConstrainingFacetsDecimal(const AtomicValue::Ptr &value, const QString &lexicalValue,
                                       const XsdFacet::Hash &facets, QString &errorMsg) const;

   bool checkConstrainingFacetsDateTime(const QDateTime &value, const QString &lexicalValue,
                                        const XsdFacet::Hash &facets, const AnySimpleType::Ptr &type, QString &errorMsg) const;

   bool checkConstrainingFacetsDuration(const AtomicValue::Ptr &value, const QString &lexicalValue,
                                        const XsdFacet::Hash &facets, QString &errorMsg) const;

   bool checkConstrainingFacetsBoolean(bool value, const QString &lexicalValue, const XsdFacet::Hash &facets,
                                       QString &errorMsg) const;

   bool checkConstrainingFacetsBinary(const QByteArray &value, const XsdFacet::Hash &facets,
                                      const AnySimpleType::Ptr &type, QString &errorMsg) const;

   bool checkConstrainingFacetsQName(const QXmlName &, const QString &lexicalValue, const XsdFacet::Hash &facets,
                                     QString &errorMsg) const;

   bool checkConstrainingFacetsNotation(const QXmlName &value, const XsdFacet::Hash &facets, QString &errorMsg) const;
   bool checkConstrainingFacetsList(const QStringList &values, const QString &lexicalValue,
                                    const AnySimpleType::Ptr &itemType, const XsdFacet::Hash &facets, QString &errorMsg) const;

   bool checkConstrainingFacetsUnion(const QString &value, const QString &lexicalValue,
                                     const XsdSimpleType::Ptr &simpleType,
                                     const XsdFacet::Hash &facets, QString &errorMsg) const;

   AtomicValue::Ptr fromLexical(const QString &value, const SchemaType::Ptr &type, const ReportContext::Ptr &context,
                                const SourceLocationReflection *const reflection) const;

   QXmlName convertToQName(const QString &name) const;

   XsdSchemaContext::Ptr m_context;
   XsdSchema::Ptr m_schema;
   const NamePool::Ptr m_namePool;
   QVector<QXmlName> m_namespaceBindings;
   SourceLocationReflection  *m_reflection;
};
}

#endif
