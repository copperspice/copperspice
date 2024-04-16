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

#ifndef QGLGradientCache_P_H
#define QGLGradientCache_P_H

#include <qgl_p.h>

#include <qobject.h>
#include <qmultihash.h>
#include <qmutex.h>

class QGL2GradientCache : public QOpenGLSharedResource
{
   struct CacheInfo {
      CacheInfo(QVector<QPair<qreal, QColor>> s, qreal op, QGradient::InterpolationMode mode)
         : stops(s), opacity(op), interpolationMode(mode)
      { }

      GLuint texId;
      QVector<QPair<qreal, QColor>> stops;
      qreal opacity;
      QGradient::InterpolationMode interpolationMode;
   };

   typedef QMultiHash<quint64, CacheInfo> QGLGradientColorTableHash;

 public:
   QGL2GradientCache(QOpenGLContext *);
   ~QGL2GradientCache();

   static QGL2GradientCache *cacheForContext(const QGLContext *context);

   GLuint getBuffer(const QGradient &gradient, qreal opacity);
   int paletteSize() const {
      return 1024;
   }

   void invalidateResource() override;
   void freeResource(QOpenGLContext *ctx) override;

 private:
   inline int maxCacheSize() const {
      return 60;
   }

   inline void generateGradientColorTable(const QGradient &gradient, uint *colorTable,
         int size, qreal opacity) const;

   GLuint addCacheElement(quint64 hash_val, const QGradient &gradient, qreal opacity);
   void cleanCache();

   QGLGradientColorTableHash cache;
   QMutex m_mutex;
};



#endif

