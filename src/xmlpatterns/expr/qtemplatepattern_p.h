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

#ifndef QTemplatePattern_P_H
#define QTemplatePattern_P_H

#include <qtemplate_p.h>

namespace QPatternist {

class TemplatePattern : public QSharedData
{
 public:
   typedef QExplicitlySharedDataPointer<TemplatePattern> Ptr;
   typedef QVector<Ptr> Vector;
   typedef int ID;

   inline TemplatePattern(const Expression::Ptr &matchPattern,
                          const PatternPriority pri,
                          const ID id,
                          const Template::Ptr templ);

   inline PatternPriority priority() const;
   inline const Expression::Ptr &matchPattern() const;
   inline void setMatchPattern(const Expression::Ptr &pattern);
   inline const Template::Ptr &templateTarget() const;
   inline ID id() const;

   /**
    * This ID is used to ensure that, as 6.4 Conflict Resolution for
    * Template Rules reads:
    *
    * "If the pattern contains multiple alternatives separated by |, then
    * the template rule is treated equivalently to a set of template
    * rules, one for each alternative. However, it is not an error if a
    * node matches more than one of the alternatives."
    *
    * For patterns separated by @c |, we have one Template instance for
    * each alternative, but they all have the same ID, hence if several
    * alternatives match, we don't flag it as an error if they have the
    * same ID.
    */
 private:
   Expression::Ptr m_matchPattern;
   PatternPriority m_priority;
   ID              m_id;
   Template::Ptr   m_templateTarget;

   TemplatePattern(const TemplatePattern &) = delete;
   TemplatePattern &operator=(const TemplatePattern &) = delete;
};

TemplatePattern::TemplatePattern(const Expression::Ptr &matchPattern,
                                 const PatternPriority pri,
                                 const ID id,
                                 const Template::Ptr templ) : m_matchPattern(matchPattern)
   , m_priority(pri)
   , m_id(id)
   , m_templateTarget(templ)

{
   Q_ASSERT(m_matchPattern);
   Q_ASSERT(m_templateTarget);
}

const Expression::Ptr &TemplatePattern::matchPattern() const
{
   return m_matchPattern;
}

void TemplatePattern::setMatchPattern(const Expression::Ptr &pattern)
{
   m_matchPattern = pattern;
}

PatternPriority TemplatePattern::priority() const
{
   return m_priority;
}

TemplatePattern::ID TemplatePattern::id() const
{
   return m_id;
}

const Template::Ptr &TemplatePattern::templateTarget() const
{
   return m_templateTarget;
}

}

#endif

