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

#ifndef QTemplateMode_P_H
#define QTemplateMode_P_H

#include <QSharedData>
#include <QXmlName>

#include <qtemplatepattern_p.h>

namespace QPatternist {

class TemplateMode : public QSharedData
{
 public:
   typedef QExplicitlySharedDataPointer<TemplateMode> Ptr;

   inline TemplateMode(const QXmlName &modeName) : m_modeName(modeName) {
   }

   TemplatePattern::Vector templatePatterns;

   inline void addMode(const TemplateMode::Ptr &mode);
   inline const QXmlName &name() const;

   void finalize();

 private:
   const QXmlName m_modeName;

   TemplateMode(const TemplateMode &) = delete;
   TemplateMode &operator=(const TemplateMode &) = delete;

   /**
    * Operator for std::sort()
    */
   static inline bool lessThanByPriority(const TemplatePattern::Ptr &t1, const TemplatePattern::Ptr &t2);
};

const QXmlName &TemplateMode::name() const
{
   return m_modeName;
}

void TemplateMode::addMode(const TemplateMode::Ptr &mode)
{
   templatePatterns += mode->templatePatterns;
}

}

#endif
