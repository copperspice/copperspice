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

#ifndef QFONTENGINEGLYPHCACHE_P_H
#define QFONTENGINEGLYPHCACHE_P_H

#include <QtCore/qglobal.h>
#include <QtCore/qatomic.h>
#include <QtCore/qvarlengtharray.h>
#include <qfont_p.h>

#ifdef Q_OS_WIN
#   include <QtCore/qt_windows.h>
#endif

#ifdef Q_OS_MAC
#   include <qt_mac_p.h>
#   include <QtCore/qmap.h>
#   include <QtCore/qcache.h>
#   include <qcore_mac_p.h>
#endif

QT_BEGIN_NAMESPACE

class QFontEngineGlyphCache: public QSharedData
{
 public:
   enum Type {
      Raster_RGBMask,
      Raster_A8,
      Raster_Mono,
      Raster_ARGB
   };

   QFontEngineGlyphCache(const QTransform &matrix, Type type) : m_transform(matrix), m_type(type) { }

   virtual ~QFontEngineGlyphCache() { }

   Type cacheType() const {
      return m_type;
   }

   QTransform m_transform;
   QFontEngineGlyphCache::Type m_type;
};
typedef QHash<void *, QList<QFontEngineGlyphCache *> > GlyphPointerHash;
typedef QHash<int, QList<QFontEngineGlyphCache *> > GlyphIntHash;

QT_END_NAMESPACE

#endif
