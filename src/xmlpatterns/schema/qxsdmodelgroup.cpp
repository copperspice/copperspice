/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#include "qxsdmodelgroup_p.h"

QT_BEGIN_NAMESPACE

using namespace QPatternist;

XsdModelGroup::XsdModelGroup()
   : m_compositor(SequenceCompositor)
{
}

bool XsdModelGroup::isModelGroup() const
{
   return true;
}

void XsdModelGroup::setCompositor(ModelCompositor compositor)
{
   m_compositor = compositor;
}

XsdModelGroup::ModelCompositor XsdModelGroup::compositor() const
{
   return m_compositor;
}

void XsdModelGroup::setParticles(const XsdParticle::List &particles)
{
   m_particles = particles;
}

XsdParticle::List XsdModelGroup::particles() const
{
   return m_particles;
}

QT_END_NAMESPACE
