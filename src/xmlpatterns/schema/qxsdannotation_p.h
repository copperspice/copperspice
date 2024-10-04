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

#ifndef QXsdAnnotation_P_H
#define QXsdAnnotation_P_H

#include <qderivedstring_p.h>
#include <qxsdapplicationinformation_p.h>
#include <qxsddocumentation_p.h>

namespace QPatternist {

class XsdAnnotation : public NamedSchemaComponent
{
 public:
   typedef QExplicitlySharedDataPointer<XsdAnnotation> Ptr;
   typedef QList<XsdAnnotation::Ptr> List;

   void setId(const DerivedString<TypeID>::Ptr &id);
   DerivedString<TypeID>::Ptr id() const;

   void addApplicationInformation(const XsdApplicationInformation::Ptr &information);
   XsdApplicationInformation::List applicationInformation() const;

   void addDocumentation(const XsdDocumentation::Ptr &documentation);
   XsdDocumentation::List documentation() const;

 private:
   DerivedString<TypeID>::Ptr      m_id;
   XsdApplicationInformation::List m_applicationInformation;
   XsdDocumentation::List          m_documentations;
};

}

#endif
