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

#ifndef QXsdReference_P_H
#define QXsdReference_P_H

#include <qxsdterm_p.h>
#include <QSourceLocation>

namespace QPatternist {

class XsdReference : public XsdTerm
{
 public:
   typedef QExplicitlySharedDataPointer<XsdReference> Ptr;

   /**
    * Describes the type of the reference.
    */
   enum Type {
      Element,        ///< The reference points to an element.
      ModelGroup      ///< The reference points to a model group.
   };

   /**
    * Returns always @c true, used to avoid dynamic casts.
    */
   bool isReference() const override;

   /**
    * Sets the @p type of the reference.
    *
    * @see Type
    */
   void setType(Type type);

   /**
    * Returns the type of the reference.
    */
   Type type() const;

   /**
    * Sets the @p name of the referenced object.
    *
    * The name can either be a top-level element declaration
    * or a top-level group declaration.
    */
   void setReferenceName(const QXmlName &ame);

   /**
    * Returns the name of the referenced object.
    */
   QXmlName referenceName() const;

   /**
    * Sets the source @p location where the reference is located.
    */
   void setSourceLocation(const QSourceLocation &location);

   /**
    * Returns the source location where the reference is located.
    */
   QSourceLocation sourceLocation() const;

 private:
   Type            m_type;
   QXmlName        m_referenceName;
   QSourceLocation m_sourceLocation;
};
}

#endif
