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

#ifndef QFONTENGINE_FT_P_H
#define QFONTENGINE_FT_P_H

#include <qfontengine_p.h>

#if defined(QT_USE_FREETYPE)

#include <ft2build.h>
#include FT_FREETYPE_H

#ifndef Q_OS_WIN
#include <unistd.h>
#endif

#include <qmutex.h>

class QFontEngineFTRawFont;
class QFontconfigDatabase;

/*
 * This struct represents one font file on disk (like Arial.ttf) and is shared between all the font engines
 * that show this font file (at different pixel sizes).
 */
class QFreetypeFace
{
 public:
   static constexpr const int CmapCacheSize = 0x200;

   void computeSize(const QFontDef &fontDef, int *xsize, int *ysize, bool *outline_drawing);
   QFontEngine::Properties properties() const;
   bool getSfntTable(uint tag, uchar *buffer, uint *length) const;

   static QFreetypeFace *getFace(const QFontEngine::FaceId &face_id, const QByteArray &fontData = QByteArray());
   void release(const QFontEngine::FaceId &face_id);

   // locks the struct for usage, any read/write operations require locking.
   void lock() {
      m_lock.lock();
   }

   void unlock() {
      m_lock.unlock();
   }

   FT_Face face;

   int xsize; // 26.6
   int ysize; // 26.6
   FT_Matrix matrix;
   FT_CharMap unicode_map;
   FT_CharMap symbol_map;

   glyph_t cmapCache[CmapCacheSize];

   int fsType() const;

   int getPointInOutline(glyph_t glyph, int flags, quint32 point, QFixed *xpos, QFixed *ypos, quint32 *nPoints);

   static void addGlyphToPath(FT_Face face, FT_GlyphSlot g, const QFixedPoint &point, QPainterPath *path,
                  FT_Fixed x_scale, FT_Fixed y_scale);

   static void addBitmapToPath(FT_GlyphSlot slot, const QFixedPoint &point, QPainterPath *path);

 private:
   QFreetypeFace()  = default;
   ~QFreetypeFace() = default;

   void cleanup();

   QAtomicInt ref;
   QRecursiveMutex m_lock;
   QByteArray fontData;

   // harfbuzz
   std::shared_ptr<hb_face_t> m_hb_FTFace;

   friend class QFontEngineFT;
   friend class QtFreetypeData;
};

class QFontEngineFT : public QFontEngine
{
 public:
   /* we do not cache glyphs that are too large anyway, so we can make this struct rather small */
   struct Glyph {
      ~Glyph();

      short linearAdvance;
      unsigned char width;
      unsigned char height;
      signed char x;
      signed char y;
      signed char advance;
      signed char format;
      uchar *data;
   };

   struct GlyphInfo {
      int             linearAdvance;
      unsigned short  width;
      unsigned short  height;
      short           x;
      short           y;
      short           xOff;
      short           yOff;
   };

   struct GlyphAndSubPixelPosition {
      GlyphAndSubPixelPosition(glyph_t g, QFixed spp) : glyph(g), subPixelPosition(spp) {}

      bool operator==(const GlyphAndSubPixelPosition &other) const {
         return glyph == other.glyph && subPixelPosition == other.subPixelPosition;
      }

      glyph_t glyph;
      QFixed subPixelPosition;
   };

   struct QGlyphSet {
      QGlyphSet();
      ~QGlyphSet();

      FT_Matrix transformationMatrix;

      bool outline_drawing;

      void removeGlyphFromCache(glyph_t index, QFixed subPixelPosition);
      void clear();

      inline bool useFastGlyphData(glyph_t index, QFixed subPixelPosition) const {
         return (index < 256 && subPixelPosition == 0);
      }

      inline QFontEngineFT::Glyph *getGlyph(glyph_t index, QFixed subPixelPosition = 0) const;

      void setGlyph(glyph_t index, QFixed spp, Glyph *glyph);

      inline bool isGlyphMissing(glyph_t index) const {
         return missing_glyphs.contains(index);
      }

      inline void setGlyphMissing(glyph_t index) const {
         missing_glyphs.insert(index);
      }

    private:
      mutable QHash<GlyphAndSubPixelPosition, Glyph *> glyph_data; // maps from glyph index to glyph data
      mutable QSet<glyph_t> missing_glyphs;
      mutable Glyph *fast_glyph_data[256]; // for fast lookup of glyphs < 256
      mutable int fast_glyph_count;
   };

   QFontEngine::FaceId faceId() const override;
   QFontEngine::Properties properties() const override;
   QFixed emSquareSize() const override;

   bool supportsSubPixelPositions() const override {
      return default_hint_style == HintLight || default_hint_style == HintNone;
   }

   bool getSfntTableData(uint tag, uchar *buffer, uint *length) const override;
   int synthesized() const override;

   QFixed ascent() const override;
   QFixed descent() const override;
   QFixed leading() const override;
   QFixed xHeight() const override;
   QFixed averageCharWidth() const override;

   qreal maxCharWidth() const override;

   QFixed lineThickness() const override;
   QFixed underlinePosition() const override;

   glyph_t glyphIndex(char32_t ch) const override;
   void doKerning(QGlyphLayout *, ShaperFlags) const override;

   void getUnscaledGlyph(glyph_t glyph, QPainterPath *path, glyph_metrics_t *metrics) override;

   const QString &fontEngineName() const override {
      static QString retval = "freetype";
      return retval;
   }

   bool supportsTransformation(const QTransform &transform) const override;

   void addGlyphsToPath(glyph_t *glyphs, QFixedPoint *positions, int nglyphs,
      QPainterPath *path, QTextItem::RenderFlags flags) override;

   void addOutlineToPath(qreal x, qreal y, const QGlyphLayout &glyphs,
      QPainterPath *path, QTextItem::RenderFlags flags) override;

   bool stringToCMap(QStringView str, QGlyphLayout *glyphs, int *nglyphs, QFontEngine::ShaperFlags flags) const override;

   glyph_metrics_t boundingBox(const QGlyphLayout &glyphs) override;
   glyph_metrics_t boundingBox(glyph_t glyph) override;
   glyph_metrics_t boundingBox(glyph_t glyph, const QTransform &matrix) override;

   void recalcAdvances(QGlyphLayout *glyphs, QFontEngine::ShaperFlags flags) const override;

   QImage alphaMapForGlyph(glyph_t g) override {
      return alphaMapForGlyph(g, 0);
   }

   QImage alphaMapForGlyph(glyph_t, QFixed) override;

   QImage alphaMapForGlyph(glyph_t glyph, QFixed subPixelPosition, const QTransform &t) override;

   QImage alphaRGBMapForGlyph(glyph_t, QFixed subPixelPosition, const QTransform &t) override;

   glyph_metrics_t alphaMapBoundingBox(glyph_t glyph, QFixed subPixelPosition, const QTransform &matrix,
      QFontEngine::GlyphFormat format) override;

   QImage *lockedAlphaMapForGlyph(glyph_t glyph, QFixed subPixelPosition, GlyphFormat neededFormat, const QTransform &t,
      QPoint *offset) override;

   bool hasInternalCaching() const override {
      return cacheEnabled;
   }
   void unlockAlphaMapForGlyph() override;

   void removeGlyphFromCache(glyph_t glyph) override;
   int glyphMargin(QFontEngine::GlyphFormat format) override {
      (void) format;

      return 0;
   }

   int glyphCount() const override;

   enum Scaling {
      Scaled,
      Unscaled
   };

   FT_Face lockFace(Scaling scale = Scaled) const;
   void unlockFace() const;

   FT_Face non_locked_face() const;

   bool drawAntialiased() const {
      return antialias;
   }

   bool invalid() const {
      return xsize == 0 && ysize == 0;
   }

   bool isBitmapFont() const {
      return defaultFormat == Format_Mono;
   }

   Glyph *loadGlyph(uint glyph, QFixed subPixelPosition, GlyphFormat format = Format_None,
                  bool fetchMetricsOnly = false) const {
      return loadGlyph(cacheEnabled ? &defaultGlyphSet : nullptr, glyph, subPixelPosition, format, fetchMetricsOnly);
   }

   Glyph *loadGlyph(QGlyphSet *set, uint glyph, QFixed subPixelPosition, GlyphFormat = Format_None,
                  bool fetchMetricsOnly = false) const;

   Glyph *loadGlyphFor(glyph_t g, QFixed subPixelPosition, GlyphFormat format, const QTransform &t,
                  bool fetchBoundingBox = false);

   QGlyphSet *loadGlyphSet(const QTransform &matrix);

   QFontEngineFT(const QFontDef &fd);
   virtual ~QFontEngineFT();

   bool init(FaceId faceId, bool antiaalias, GlyphFormat defaultFormat = Format_None,
      const QByteArray &fontData = QByteArray());

   bool init(FaceId faceId, bool antialias, GlyphFormat format, QFreetypeFace *freetypeFace);

   int getPointInOutline(glyph_t glyph, int flags, quint32 point, QFixed *xpos, QFixed *ypos, quint32 *nPoints) override;

   void setQtDefaultHintStyle(QFont::HintingPreference hintingPreference);
   void setDefaultHintStyle(HintStyle style) override;

   QFontEngine *cloneWithSize(qreal pixelSize) const override;
   Qt::HANDLE handle() const override;

   bool initFromFontEngine(const QFontEngineFT *fontEngine);

   HintStyle defaultHintStyle() const {
      return default_hint_style;
   }

 protected:
   QFreetypeFace *freetype;
   mutable int default_load_flags;

   HintStyle default_hint_style;

   bool antialias;
   bool transform;
   bool embolden;
   bool obliquen;

   SubpixelAntialiasingType subpixelType;
   int lcdFilterType;

   bool embeddedbitmap;
   bool cacheEnabled;
   bool forceAutoHint;

 private:
   int loadFlags(QGlyphSet *set, GlyphFormat format, int flags, bool &hsubpixel, int &vfactor) const;
   bool shouldUseDesignMetrics(ShaperFlags flags) const;

   GlyphFormat defaultFormat;
   FT_Matrix matrix;

   QList<QGlyphSet> transformedGlyphSets;
   mutable QGlyphSet defaultGlyphSet;

   QFontEngine::FaceId face_id;

   int xsize;
   int ysize;

   QFixed line_thickness;
   QFixed underline_position;

   FT_Size_Metrics metrics;
   mutable bool kerning_pairs_loaded;

   friend class QFontEngineFTRawFont;
   friend class QFontconfigDatabase;
   friend class QBasicFontDatabase;
   friend class QCoreTextFontDatabase;
   friend class QFontEngineMultiFontConfig;
};

inline uint qHash(const QFontEngineFT::GlyphAndSubPixelPosition &g)
{
   return (g.glyph << 8)  | (g.subPixelPosition * 10).round().toInt();
}

inline QFontEngineFT::Glyph *QFontEngineFT::QGlyphSet::getGlyph(glyph_t index, QFixed subPixelPosition) const
{
   if (useFastGlyphData(index, subPixelPosition)) {
      return fast_glyph_data[index];
   }
   return glyph_data.value(GlyphAndSubPixelPosition(index, subPixelPosition));
}

extern FT_Library qt_getFreetype();

#endif // QT_USE_FREETYPE

#endif
