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

#ifndef QXsdModelGroup_P_H
#define QXsdModelGroup_P_H

#include <qxsdparticle_p.h>
#include <qxsdterm_p.h>

template<typename N>
class QList;

namespace QPatternist {

class XsdModelGroup : public XsdTerm
{
 public:
   typedef QExplicitlySharedDataPointer<XsdModelGroup> Ptr;
   typedef QList<XsdModelGroup::Ptr> List;

   enum ModelCompositor {
      SequenceCompositor,     // model group is a sequence.
      ChoiceCompositor,       // model group is a choice.
      AllCompositor           // model group contains elements only.
   };

   XsdModelGroup();

   bool isModelGroup() const override;

   void setCompositor(ModelCompositor compositor);
   ModelCompositor compositor() const;

   void setParticles(const XsdParticle::List &particles);
   XsdParticle::List particles() const;

 private:
   ModelCompositor   m_compositor;
   XsdParticle::List m_particles;
};

}

#endif
