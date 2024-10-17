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

#ifndef QXsdAttributeReference_P_H
#define QXsdAttributeReference_P_H

#include <qxsdattributeuse_p.h>
#include <QSourceLocation>

namespace QPatternist {

class XsdAttributeReference : public XsdAttributeUse
{
 public:
   typedef QExplicitlySharedDataPointer<XsdAttributeReference> Ptr;

   enum Type {
      AttributeUse,      // reference points to an attribute use
      AttributeGroup     // reference points to an attribute group
   };

   bool isAttributeUse() const override;
   bool isReference() const override;

   void setType(Type type);
   Type type() const;

   void setReferenceName(const QXmlName &name);
   QXmlName referenceName() const;

   void setSourceLocation(const QSourceLocation &location);
   QSourceLocation sourceLocation() const;

 private:
   Type            m_type;
   QXmlName        m_referenceName;
   QSourceLocation m_sourceLocation;
};

}

#endif
