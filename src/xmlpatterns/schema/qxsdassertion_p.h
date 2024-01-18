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

#ifndef QXsdAssertion_P_H
#define QXsdAssertion_P_H

#include <qlist.h>

#include <qnamedschemacomponent_p.h>
#include <qxsdannotated_p.h>
#include <qxsdxpathexpression_p.h>

namespace QPatternist {

class XsdAssertion : public NamedSchemaComponent, public XsdAnnotated
{
 public:
   typedef QExplicitlySharedDataPointer<XsdAssertion> Ptr;
   typedef QList<XsdAssertion::Ptr> List;

   /**
    * Sets the @p test of the assertion.
    *
    * @see <a href="http://www.w3.org/TR/xmlschema11-1/#as-test">Test Definition</a>
    */
   void setTest(const XsdXPathExpression::Ptr &test);

   /**
    * Returns the test of the assertion.
    */
   XsdXPathExpression::Ptr test() const;

 private:
   XsdXPathExpression::Ptr m_test;
};

}

#endif
