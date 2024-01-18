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

#include "qxsdparticle_p.h"

using namespace QPatternist;

XsdParticle::XsdParticle()
   : m_minimumOccurs(1)
   , m_maximumOccurs(1)
   , m_maximumOccursUnbounded(false)
{
}

void XsdParticle::setMinimumOccurs(unsigned int occurs)
{
   m_minimumOccurs = occurs;
}

unsigned int XsdParticle::minimumOccurs() const
{
   return m_minimumOccurs;
}

void XsdParticle::setMaximumOccurs(unsigned int occurs)
{
   m_maximumOccurs = occurs;
}

unsigned int XsdParticle::maximumOccurs() const
{
   return m_maximumOccurs;
}

void XsdParticle::setMaximumOccursUnbounded(bool unbounded)
{
   m_maximumOccursUnbounded = unbounded;
}

bool XsdParticle::maximumOccursUnbounded() const
{
   return m_maximumOccursUnbounded;
}

void XsdParticle::setTerm(const XsdTerm::Ptr &term)
{
   m_term = term;
}

XsdTerm::Ptr XsdParticle::term() const
{
   return m_term;
}
