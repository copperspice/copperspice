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

#ifndef QXsdXPathExpression_P_H
#define QXsdXPathExpression_P_H

#include <qlist.h>

#include <qanyuri_p.h>
#include <qnamedschemacomponent_p.h>
#include <qxsdannotated_p.h>

namespace QPatternist {

class XsdXPathExpression : public NamedSchemaComponent, public XsdAnnotated
{
 public:
   typedef QExplicitlySharedDataPointer<XsdXPathExpression> Ptr;
   typedef QList<XsdXPathExpression::Ptr> List;

   /**
    * Sets the list of namespace @p bindings of the XPath expression.
    *
    * @see <a href="http://www.w3.org/TR/xmlschema11-1/#x-namespace_bindings">Namespace Bindings Definition</a>
    *
    * @note We can't use a QSet<QXmlName> here, as the hash method does not take the prefix
    *       in account, so we loose entries.
    */
   void setNamespaceBindings(const QList<QXmlName> &bindings);

   /**
    * Returns the list of namespace bindings of the XPath expression.
    */
   QList<QXmlName> namespaceBindings() const;

   /**
    * Sets the default namespace of the XPath expression.
    *
    * @see <a href="http://www.w3.org/TR/xmlschema11-1/#x-default_namespace">Default Namespace Definition</a>
    */
   void setDefaultNamespace(const AnyURI::Ptr &defaultNamespace);

   /**
    * Returns the default namespace of the XPath expression.
    */
   AnyURI::Ptr defaultNamespace() const;

   /**
    * Sets the base @p uri of the XPath expression.
    *
    * @see <a href="http://www.w3.org/TR/xmlschema11-1/#x-base_URI">Base URI Definition</a>
    */
   void setBaseURI(const AnyURI::Ptr &uri);

   /**
    * Returns the base uri of the XPath expression.
    */
   AnyURI::Ptr baseURI() const;

   /**
    * Sets the @p expression string of the XPath expression.
    *
    * @see <a href="http://www.w3.org/TR/xmlschema11-1/#x-expression">Expression Definition</a>
    */
   void setExpression(const QString &expression);


   QString expression() const;

 private:
   QList<QXmlName> m_namespaceBindings;
   AnyURI::Ptr     m_defaultNamespace;
   AnyURI::Ptr     m_baseURI;
   QString         m_expression;
};

}

#endif
