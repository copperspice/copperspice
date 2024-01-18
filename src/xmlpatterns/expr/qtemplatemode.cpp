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

#include <algorithm>

#include <qtemplatemode_p.h>

using namespace QPatternist;

bool TemplateMode::lessThanByPriority(const TemplatePattern::Ptr &t1,
                                      const TemplatePattern::Ptr &t2)
{
   return t1->priority() > t2->priority();
}

void TemplateMode::finalize()
{
   std::sort(templatePatterns.begin(), templatePatterns.end(), lessThanByPriority);

   /* Now we have a list of patterns sorted by priority. */
}

