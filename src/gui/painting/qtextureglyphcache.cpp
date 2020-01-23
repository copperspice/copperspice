/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#include <qmath.h>
#include <qtextureglyphcache_p.h>
#include <qnativeimage_p.h>
#include <qnumeric_p.h>
#include <qfontengine_p.h>

// #define CACHE_DEBUG

QTextureGlyphCache::~QTextureGlyphCache()
{
}

int QTextureGlyphCache::calculateSubPixelPositionCount(glyph_t glyph) const
{
   // Test 12 different subpixel positions since it factors into 3*4 so it gives
   // the coverage we need.

   const int NumSubpixelPositions = 12;

   QImage images[NumSubpixelPositions];
   int numImages = 0;
   for (int i = 0; i < NumSubpixelPositions; ++i) {
      QImage img = textureMapForGlyph(glyph, QFixed::fromReal(i / 12.0));

      if (numImages == 0) {
         QPainterPath path;
         QFixedPoint point;
         m_current_fontengine->addGlyphsToPath(&glyph, &point, 1, &path, QTextItem::RenderFlags());

         // Glyph is space, return 0 to indicate that we need to keep trying
         if (path.isEmpty()) {
            break;
         }

         images[numImages++] = std::move(img);

      } else {
         bool found = false;
         for (int j = 0; j < numImages; ++j) {
            if (images[j] == img) {
               found = true;
               break;
            }
         }
         if (!found) {
            images[numImages++] = std::move(img);
         }
      }
   }

   return numImages;
}

bool QTextureGlyphCache::populate(QFontEngine *fontEngine, int numGlyphs, const glyph_t *glyphs,
   const QFixedPoint *positions)
{
   m_current_fontengine     = fontEngine;
   const int padding        = glyphPadding();
   const int paddingDoubled = padding * 2;

   bool supportsSubPixelPositions = fontEngine->supportsSubPixelPositions();

   if (fontEngine->m_subPixelPositionCount == 0) {
      if (! supportsSubPixelPositions) {
         fontEngine->m_subPixelPositionCount = 1;

      } else {
         int i = 0;

         while (fontEngine->m_subPixelPositionCount == 0 && i < numGlyphs) {
            fontEngine->m_subPixelPositionCount = calculateSubPixelPositionCount(glyphs[i++]);
         }
      }
   }

   if (m_cx == 0 && m_cy == 0) {
      m_cx = padding;
      m_cy = padding;
   }

   QHash<GlyphAndSubPixelPosition, Coord> listItemCoordinates;
   int rowHeight = 0;

   // check each glyph for its metrics and get the required rowHeight.
   for (int i = 0; i < numGlyphs; ++i) {
      const glyph_t glyph = glyphs[i];

      QFixed subPixelPosition;

      if (supportsSubPixelPositions) {
         QFixed x = positions != 0 ? positions[i].x : QFixed();
         subPixelPosition = fontEngine->subPixelPositionForX(x);
      }

      if (coords.contains(GlyphAndSubPixelPosition(glyph, subPixelPosition))) {
         continue;
      }

      if (listItemCoordinates.contains(GlyphAndSubPixelPosition(glyph, subPixelPosition))) {
         continue;
      }

      glyph_metrics_t metrics = fontEngine->alphaMapBoundingBox(glyph, subPixelPosition, m_transform, m_format);

      GlyphAndSubPixelPosition key(glyph, subPixelPosition);
      int glyph_width = metrics.width.ceil().toInt();
      int glyph_height = metrics.height.ceil().toInt();

      if (glyph_height == 0 || glyph_width == 0) {
         // Avoid multiple calls to boundingBox() for non-printable characters
         Coord c = { 0, 0, 0, 0, 0, 0 };
         coords.insert(key, c);
         continue;
      }

      // align to 8-bit boundary
      if (m_format == QFontEngine::Format_Mono) {
         glyph_width = (glyph_width + 7) & ~7;
      }

      Coord c = { 0, 0, // will be filled in later
               glyph_width,
               glyph_height, // texture coords
               metrics.x.truncate(),
               -metrics.y.truncate()
            }; // baseline for horizontal scripts

      listItemCoordinates.insert(key, c);
      rowHeight = qMax(rowHeight, glyph_height);
   }

   if (listItemCoordinates.isEmpty()) {
      return true;
   }

   rowHeight += paddingDoubled;

   if (m_w == 0) {
      if (fontEngine->maxCharWidth() <= QT_DEFAULT_TEXTURE_GLYPH_CACHE_WIDTH) {
         m_w = QT_DEFAULT_TEXTURE_GLYPH_CACHE_WIDTH;
      } else {
         m_w = qNextPowerOfTwo(qCeil(fontEngine->maxCharWidth()) - 1);
      }
   }

   // now actually use the coords and paint the wanted glyps into cache.
   QHash<GlyphAndSubPixelPosition, Coord>::iterator iter = listItemCoordinates.begin();
   int requiredWidth = m_w;

   while (iter != listItemCoordinates.end()) {
      Coord c = iter.value();

      m_currentRowHeight = qMax(m_currentRowHeight, c.h);

      if (m_cx + c.w + padding > requiredWidth) {
         int new_width = requiredWidth * 2;

         while (new_width < m_cx + c.w + padding) {
            new_width *= 2;
         }

         if (new_width <= maxTextureWidth()) {
            requiredWidth = new_width;
         } else {
            // no room on the current line, start new glyph strip
            m_cx = padding;
            m_cy += m_currentRowHeight + paddingDoubled;
            m_currentRowHeight = c.h; // New row
         }
      }

      if (maxTextureHeight() > 0 && m_cy + c.h + padding > maxTextureHeight()) {
         // can not make a cache of the required size, so we bail out
         return false;
      }

      c.x = m_cx;
      c.y = m_cy;

      coords.insert(iter.key(), c);
      m_pendingGlyphs.insert(iter.key(), c);

      m_cx += c.w + paddingDoubled;
      ++iter;
   }

   return true;

}

void QTextureGlyphCache::fillInPendingGlyphs()
{
   if (!hasPendingGlyphs()) {
      return;
   }

   int requiredHeight = m_h;
   int requiredWidth  = m_w; // Use a minimum size to avoid a lot of initial reallocations

   {
      QHash<GlyphAndSubPixelPosition, Coord>::iterator iter = m_pendingGlyphs.begin();

      while (iter != m_pendingGlyphs.end()) {
         Coord c = iter.value();
         requiredHeight = qMax(requiredHeight, c.y + c.h);
         requiredWidth  = qMax(requiredWidth, c.x + c.w);
         ++iter;
      }
   }

   if (isNull() || requiredHeight > m_h || requiredWidth > m_w) {
      if (isNull()) {
         createCache(qNextPowerOfTwo(requiredWidth - 1), qNextPowerOfTwo(requiredHeight - 1));
      } else {
         resizeCache(qNextPowerOfTwo(requiredWidth - 1), qNextPowerOfTwo(requiredHeight - 1));
      }
   }


   {
      QHash<GlyphAndSubPixelPosition, Coord>::iterator iter = m_pendingGlyphs.begin();

      while (iter != m_pendingGlyphs.end()) {
         GlyphAndSubPixelPosition key = iter.key();
         fillTexture(iter.value(), key.glyph, key.subPixelPosition);

         ++iter;
      }
   }

   m_pendingGlyphs.clear();
}

QImage QTextureGlyphCache::textureMapForGlyph(glyph_t g, QFixed subPixelPosition) const
{
   switch (m_format) {
      case QFontEngine::Format_A32:
         return m_current_fontengine->alphaRGBMapForGlyph(g, subPixelPosition, m_transform);

      case QFontEngine::Format_ARGB:
         return m_current_fontengine->bitmapForGlyph(g, subPixelPosition, m_transform);

      default:
         return m_current_fontengine->alphaMapForGlyph(g, subPixelPosition, m_transform);
   }
}
QImageTextureGlyphCache::~QImageTextureGlyphCache()
{
}

void QImageTextureGlyphCache::resizeTextureData(int width, int height)
{
   m_image = m_image.copy(0, 0, width, height);
}

void QImageTextureGlyphCache::createTextureData(int width, int height)
{
   switch (m_format) {
      case QFontEngine::Format_Mono:
         m_image = QImage(width, height, QImage::Format_Mono);
         break;

      case QFontEngine::Format_A8:
         m_image = QImage(width, height, QImage::Format_Alpha8);
         break;

      case QFontEngine::Format_A32:
         m_image = QImage(width, height, QImage::Format_RGB32);
         break;

      case QFontEngine::Format_ARGB:
         m_image = QImage(width, height, QImage::Format_ARGB32_Premultiplied);
         break;

      default:
         // may want to throw here
         break;
   }
}

void QImageTextureGlyphCache::fillTexture(const Coord &c, glyph_t g, QFixed subPixelPosition)
{
   QImage mask = textureMapForGlyph(g, subPixelPosition);

   if (m_format == QFontEngine::Format_A32 || m_format == QFontEngine::Format_ARGB) {

      QImage ref(m_image.bits() + (c.x * 4 + c.y * m_image.bytesPerLine()),
         qMax(mask.width(), c.w), qMax(mask.height(), c.h), m_image.bytesPerLine(),
         m_image.format());

      QPainter p(&ref);
      p.setCompositionMode(QPainter::CompositionMode_Source);
      p.fillRect(0, 0, c.w, c.h, QColor(0, 0, 0, 0)); // TODO optimize this
      p.drawImage(0, 0, mask);
      p.end();

   } else if (m_format == QFontEngine::Format_Mono) {

      if (mask.depth() > 1) {
         // TODO optimize this
         mask = mask.alphaChannel();
         mask.invertPixels();
         mask = mask.convertToFormat(QImage::Format_Mono);
      }

      int mw = qMin(mask.width(), c.w);
      int mh = qMin(mask.height(), c.h);
      uchar *d = m_image.bits();
      int dbpl = m_image.bytesPerLine();

      for (int y = 0; y < c.h; ++y) {
         uchar *dest = d + (c.y + y) * dbpl + c.x / 8;

         if (y < mh) {
            const uchar *src = mask.constScanLine(y);
            for (int x = 0; x < c.w / 8; ++x) {
               if (x < (mw + 7) / 8) {
                  dest[x] = src[x];
               } else {
                  dest[x] = 0;
               }
            }
         } else {
            for (int x = 0; x < c.w / 8; ++x) {
               dest[x] = 0;
            }
         }
      }
   } else { // A8
      int mw = qMin(mask.width(), c.w);
      int mh = qMin(mask.height(), c.h);
      uchar *d = m_image.bits();
      int dbpl = m_image.bytesPerLine();

      if (mask.depth() == 1) {
         for (int y = 0; y < c.h; ++y) {
            uchar *dest = d + (c.y + y) * dbpl + c.x;
            if (y < mh) {
               const uchar *src = mask.constScanLine(y);
               for (int x = 0; x < c.w; ++x) {
                  if (x < mw) {
                     dest[x] = (src[x >> 3] & (1 << (7 - (x & 7)))) > 0 ? 255 : 0;
                  }
               }
            }
         }
      } else if (mask.depth() == 8) {
         for (int y = 0; y < c.h; ++y) {
            uchar *dest = d + (c.y + y) * dbpl + c.x;
            if (y < mh) {
               const uchar *src = mask.constScanLine(y);
               for (int x = 0; x < c.w; ++x) {
                  if (x < mw) {
                     dest[x] = src[x];
                  }
               }
            }
         }
      }
   }
}

