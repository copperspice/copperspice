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

#ifndef QXsdApplicationInformation_P_H
#define QXsdApplicationInformation_P_H

#include <qanytype_p.h>
#include <qanyuri_p.h>
#include <qnamedschemacomponent_p.h>

namespace QPatternist {

class XsdApplicationInformation : public NamedSchemaComponent
{
 public:
   typedef QExplicitlySharedDataPointer<XsdApplicationInformation> Ptr;
   typedef QList<XsdApplicationInformation::Ptr> List;

   void setSource(const AnyURI::Ptr &source);
   AnyURI::Ptr source() const;

   void setContent(const QString &content);
   QString content() const;

 private:
   AnyURI::Ptr  m_source;
   QString      m_content;
};

}

#endif
