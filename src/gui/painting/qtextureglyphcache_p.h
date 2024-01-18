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

#ifndef QTEXTUREGLYPHCACHE_P_H
#define QTEXTUREGLYPHCACHE_P_H

#include <qhash.h>
#include <qimage.h>
#include <qobject.h>
#include <qtransform.h>

#include <qfontengineglyphcache_p.h>

#ifndef QT_DEFAULT_TEXTURE_GLYPH_CACHE_WIDTH
#define QT_DEFAULT_TEXTURE_GLYPH_CACHE_WIDTH 256
#endif

struct glyph_metrics_t;
typedef unsigned int glyph_t;

class QTextItemInt;

class Q_GUI_EXPORT QTextureGlyphCache : public QFontEngineGlyphCache
{
 public:
   QTextureGlyphCache(QFontEngine::GlyphFormat format, const QTransform &matrix)
      : QFontEngineGlyphCache(format, matrix), m_current_fontengine(nullptr),
        m_w(0), m_h(0), m_cx(0), m_cy(0), m_currentRowHeight(0)
   { }

   virtual ~QTextureGlyphCache();

   struct GlyphAndSubPixelPosition {
      GlyphAndSubPixelPosition(glyph_t g, QFixed spp) : glyph(g), subPixelPosition(spp) {}

      bool operator==(const GlyphAndSubPixelPosition &other) const {
         return glyph == other.glyph && subPixelPosition == other.subPixelPosition;
      }

      glyph_t glyph;
      QFixed subPixelPosition;
   };

   struct Coord {
      int x;
      int y;
      int w;
      int h;

      int baseLineX;
      int baseLineY;

      bool isNull() const {
         return w == 0 || h == 0;
      }
   };

   bool populate(QFontEngine *fontEngine, int numGlyphs, const glyph_t *glyphs, const QFixedPoint *positions);

   bool hasPendingGlyphs() const {
      return !m_pendingGlyphs.isEmpty();
   };

   void fillInPendingGlyphs();

   virtual void createTextureData(int width, int height) = 0;
   virtual void resizeTextureData(int width, int height) = 0;

   virtual int glyphPadding() const {
      return 0;
   }

   virtual void fillTexture(const Coord &coord, glyph_t glyph, QFixed subPixelPosition) = 0;

   inline void createCache(int width, int height) {
      m_w = width;
      m_h = height;
      createTextureData(width, height);
   }

   inline void resizeCache(int width, int height) {
      resizeTextureData(width, height);
      m_w = width;
      m_h = height;
   }

   inline bool isNull() const {
      return m_h == 0;
   }

   QHash<GlyphAndSubPixelPosition, Coord> coords;

   virtual int maxTextureWidth() const {
      return QT_DEFAULT_TEXTURE_GLYPH_CACHE_WIDTH;
   }

   virtual int maxTextureHeight() const {
      return -1;
   }

   QImage textureMapForGlyph(glyph_t g, QFixed subPixelPosition) const;

 protected:
   int calculateSubPixelPositionCount(glyph_t) const;

   QFontEngine *m_current_fontengine;
   QHash<GlyphAndSubPixelPosition, Coord> m_pendingGlyphs;

   int m_w; // image width
   int m_h; // image height
   int m_cx; // current x
   int m_cy; // current y
   int m_currentRowHeight; // Height of last row
};

inline uint qHash(const QTextureGlyphCache::GlyphAndSubPixelPosition &g)
{
   return (g.glyph << 8)  | (g.subPixelPosition * 10).round().toInt();
}

class Q_GUI_EXPORT QImageTextureGlyphCache : public QTextureGlyphCache
{
 public:
   QImageTextureGlyphCache(QFontEngine::GlyphFormat format, const QTransform &matrix)
      : QTextureGlyphCache(format, matrix) { }

   ~QImageTextureGlyphCache();

   void createTextureData(int width, int height) override;
   void resizeTextureData(int width, int height) override;
   void fillTexture(const Coord &c, glyph_t glyph, QFixed subPixelPosition) override;

   inline const QImage &image() const {
      return m_image;
   }

 private:
   QImage m_image;
};

#endif
