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

#ifndef QFONTENGINEGLYPHCACHE_P_H
#define QFONTENGINEGLYPHCACHE_P_H

#include <qglobal.h>
#include <qatomic.h>
#include <qvarlengtharray.h>

#include <qfont_p.h>
#include <qfontengine_p.h>

class Q_GUI_EXPORT QFontEngineGlyphCache: public QSharedData
{
 public:
   QFontEngineGlyphCache(QFontEngine::GlyphFormat format, const QTransform &matrix)
      : m_format(format), m_transform(matrix) {
      Q_ASSERT(m_format != QFontEngine::Format_None);
   }

   virtual ~QFontEngineGlyphCache();

   QFontEngine::GlyphFormat glyphFormat() const {
      return m_format;
   }

   const QTransform &transform() const {
      return m_transform;
   }

   QFontEngine::GlyphFormat m_format;
   QTransform m_transform;
};

typedef QHash<void *, QList<QFontEngineGlyphCache *>> GlyphPointerHash;
typedef QHash<int, QList<QFontEngineGlyphCache *>> GlyphIntHash;

#endif
