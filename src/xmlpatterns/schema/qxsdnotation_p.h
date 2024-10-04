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

#ifndef QXsdNotation_P_H
#define QXsdNotation_P_H

#include <qlist.h>

#include <qanyuri_p.h>
#include <qderivedstring_p.h>
#include <qnamedschemacomponent_p.h>
#include <qxsdannotated_p.h>

namespace QPatternist {

class XsdNotation : public NamedSchemaComponent, public XsdAnnotated
{
 public:
   typedef QExplicitlySharedDataPointer<XsdNotation> Ptr;
   typedef QList<XsdNotation::Ptr> List;

   void setPublicId(const DerivedString<TypeToken>::Ptr &identifier);

   DerivedString<TypeToken>::Ptr publicId() const;

   void setSystemId(const AnyURI::Ptr &identifier);
   AnyURI::Ptr systemId() const;

 private:
   DerivedString<TypeToken>::Ptr m_publicId;
   AnyURI::Ptr m_systemId;
};

}

#endif
