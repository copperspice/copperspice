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

#ifndef QGLGradientCache_P_H
#define QGLGradientCache_P_H

#include <QMultiHash>
#include <QObject>
#include <QtOpenGL/QtOpenGL>
#include <qgl_p.h>
#include <QtCore/qmutex.h>

QT_BEGIN_NAMESPACE

class QGL2GradientCache
{
   struct CacheInfo {
      inline CacheInfo(QGradientStops s, qreal op, QGradient::InterpolationMode mode) :
         stops(s), opacity(op), interpolationMode(mode) {}

      GLuint texId;
      QGradientStops stops;
      qreal opacity;
      QGradient::InterpolationMode interpolationMode;
   };

   typedef QMultiHash<quint64, CacheInfo> QGLGradientColorTableHash;

 public:
   static QGL2GradientCache *cacheForContext(const QGLContext *context);

   QGL2GradientCache(const QGLContext *) {}
   ~QGL2GradientCache() {
      cleanCache();
   }

   GLuint getBuffer(const QGradient &gradient, qreal opacity);
   inline int paletteSize() const {
      return 1024;
   }

 private:
   inline int maxCacheSize() const {
      return 60;
   }
   inline void generateGradientColorTable(const QGradient &gradient,
                                          uint *colorTable,
                                          int size, qreal opacity) const;
   GLuint addCacheElement(quint64 hash_val, const QGradient &gradient, qreal opacity);
   void cleanCache();

   QGLGradientColorTableHash cache;
   QMutex m_mutex;
};

QT_END_NAMESPACE

#endif

