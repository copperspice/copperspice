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

#ifndef QXsdParticle_P_H
#define QXsdParticle_P_H

#include <qlist.h>

#include <qnamedschemacomponent_p.h>
#include <qxsdterm_p.h>

namespace QPatternist {

class XsdParticle : public NamedSchemaComponent
{
 public:
   typedef QExplicitlySharedDataPointer<XsdParticle> Ptr;
   typedef QList<XsdParticle::Ptr> List;

   XsdParticle();

   void setMinimumOccurs(unsigned int occurrence);
   unsigned int minimumOccurs() const;

   void setMaximumOccurs(unsigned int occurrence);
   unsigned int maximumOccurs() const;

   void setMaximumOccursUnbounded(bool unbounded);
   bool maximumOccursUnbounded() const;

   void setTerm(const XsdTerm::Ptr &term);
   XsdTerm::Ptr term() const;

 private:
   unsigned int m_minimumOccurs;
   unsigned int m_maximumOccurs;
   bool         m_maximumOccursUnbounded;
   XsdTerm::Ptr m_term;
};

}

#endif
