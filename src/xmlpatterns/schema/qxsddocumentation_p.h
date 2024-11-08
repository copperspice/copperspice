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

#ifndef QXsdDocumentation_P_H
#define QXsdDocumentation_P_H

#include <qanytype_p.h>
#include <qanyuri_p.h>
#include <qderivedstring_p.h>
#include <qnamedschemacomponent_p.h>

namespace QPatternist {

class XsdDocumentation : public NamedSchemaComponent
{
 public:
   typedef QExplicitlySharedDataPointer<XsdDocumentation> Ptr;
   typedef QList<XsdDocumentation::Ptr> List;

   XsdDocumentation();

   ~XsdDocumentation();

   void setSource(const AnyURI::Ptr &source);
   AnyURI::Ptr source() const;

   void setLanguage(const DerivedString<TypeLanguage>::Ptr &language);
   DerivedString<TypeLanguage>::Ptr language() const;

   void setContent(const QString &content);
   QString content() const;

 private:
   AnyURI::Ptr m_source;
   DerivedString<TypeLanguage>::Ptr m_language;
   QString m_content;
};

}

#endif
