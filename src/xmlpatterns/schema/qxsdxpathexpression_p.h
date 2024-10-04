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

   void setNamespaceBindings(const QList<QXmlName> &bindings);
   QList<QXmlName> namespaceBindings() const;

   void setDefaultNamespace(const AnyURI::Ptr &defaultNamespace);
   AnyURI::Ptr defaultNamespace() const;

   void setBaseURI(const AnyURI::Ptr &uri);
   AnyURI::Ptr baseURI() const;

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
