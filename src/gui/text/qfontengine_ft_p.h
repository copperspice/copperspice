/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#ifndef QT_NO_FREETYPE

#include <ft2build.h>
#include FT_FREETYPE_H

#if defined(Q_WS_X11)
#include <qt_x11_p.h>
#endif

#include <unistd.h>
#include <fontconfig/fontconfig.h>
#include <qmutex.h>
#include <harfbuzz-shaper.h>

QT_BEGIN_NAMESPACE

class QFontEngineFTRawFont;

/*
 * This struct represents one font file on disk (like Arial.ttf) and is shared between all the font engines
 * that show this font file (at different pixel sizes).
 */
struct QFreetypeFace {
   void computeSize(const QFontDef &fontDef, int *xsize, int *ysize, bool *outline_drawing);
   QFontEngine::Properties properties() const;
   bool getSfntTable(uint tag, uchar *buffer, uint *length) const;

   static QFreetypeFace *getFace(const QFontEngine::FaceId &face_id, const QByteArray &fontData = QByteArray());
   void release(const QFontEngine::FaceId &face_id);

   // locks the struct for usage. Any read/write operations require locking.
   void lock() {
      _lock.lock();
   }

   void unlock() {
      _lock.unlock();
   }

   FT_Face face;
   HB_Face hbFace;

   FcCharSet *charset;

   int xsize; // 26.6
   int ysize; // 26.6
   FT_Matrix matrix;
   FT_CharMap unicode_map;
   FT_CharMap symbol_map;

   enum { cmapCacheSize = 0x200 };
   glyph_t cmapCache[cmapCacheSize];

   int fsType() const;

   HB_Error getPointInOutline(HB_Glyph glyph, int flags, hb_uint32 point, HB_Fixed *xpos, HB_Fixed *ypos,
                              hb_uint32 *nPoints);

   static void addGlyphToPath(FT_Face face, FT_GlyphSlot g, const QFixedPoint &point, QPainterPath *path, FT_Fixed x_scale,
                              FT_Fixed y_scale);
   static void addBitmapToPath(FT_GlyphSlot slot, const QFixedPoint &point, QPainterPath *path, bool = false);

 private:
   friend class QFontEngineFT;
   friend class QScopedPointerDeleter<QFreetypeFace>;
   QFreetypeFace() : _lock(QMutex::Recursive) {}
   ~QFreetypeFace() {}
   QAtomicInt ref;
   QMutex _lock;
   QByteArray fontData;
};

class Q_GUI_EXPORT QFontEngineFT : public QFontEngine
{
 public:

   /* we don't cache glyphs that are too large anyway, so we can make this struct rather small */
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
      unsigned int uploadedToServer : 1;
   };

   enum SubpixelAntialiasingType {
      Subpixel_None,
      Subpixel_RGB,
      Subpixel_BGR,
      Subpixel_VRGB,
      Subpixel_VBGR
   };

#if defined(Q_WS_X11) && !defined(QT_NO_XRENDER)
   typedef XGlyphInfo GlyphInfo;
#else
   struct GlyphInfo {
      unsigned short  width;
      unsigned short  height;
      short           x;
      short           y;
      short           xOff;
      short           yOff;
   };
#endif

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
      unsigned long id; // server sided id, GlyphSet for X11
      bool outline_drawing;

      void removeGlyphFromCache(glyph_t index, QFixed subPixelPosition);
      void clear();

      inline bool useFastGlyphData(glyph_t index, QFixed subPixelPosition) const {
         return (index < 256 && subPixelPosition == 0);
      }

      inline QFontEngineFT::Glyph *getGlyph(glyph_t index, QFixed subPixelPosition = 0) const;
      void setGlyph(glyph_t index, QFixed spp, Glyph *glyph);

    private:
      mutable QHash<GlyphAndSubPixelPosition, Glyph *> glyph_data; // maps from glyph index to glyph data
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
   qreal minLeftBearing() const override;
   qreal minRightBearing() const override;
   QFixed lineThickness() const override;
   QFixed underlinePosition() const override;

   void doKerning(QGlyphLayout *, QTextEngine::ShaperFlags) const override;

   bool canRender(QStringView str) override;

   const QString &fontEngineName() const override {
      static QString retval("freetype");
      return retval;
   }

   Type type() const override {
      return QFontEngine::Freetype;
   }

   void getUnscaledGlyph(glyph_t glyph, QPainterPath *path, glyph_metrics_t *metrics) override;
   void addGlyphsToPath(glyph_t *glyphs, QFixedPoint *positions, int nglyphs, QPainterPath *path, QTextItem::RenderFlags flags) override;
   void addOutlineToPath(qreal x, qreal y, const QGlyphLayout &glyphs, QPainterPath *path, QTextItem::RenderFlags flags) override;

   bool stringToCMap(QStringView str, QGlyphLayout *glyphs, int *nglyphs, QTextEngine::ShaperFlags flags) const override;

   glyph_metrics_t boundingBox(const QGlyphLayout &glyphs) override;
   glyph_metrics_t boundingBox(glyph_t glyph) override;
   glyph_metrics_t boundingBox(glyph_t glyph, const QTransform &matrix) override;

   void recalcAdvances(QGlyphLayout *glyphs, QTextEngine::ShaperFlags flags) const override;

   QImage alphaMapForGlyph(glyph_t g) override {
      return alphaMapForGlyph(g, 0);
   }

   QImage alphaMapForGlyph(glyph_t, QFixed) override;
   QImage alphaRGBMapForGlyph(glyph_t, QFixed subPixelPosition, int margin, const QTransform &t) override;

   glyph_metrics_t alphaMapBoundingBox(glyph_t glyph, QFixed subPixelPosition, const QTransform &matrix,
                  QFontEngine::GlyphFormat format) override;

   void removeGlyphFromCache(glyph_t glyph) override;
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
      return loadGlyph(&defaultGlyphSet, glyph, subPixelPosition, format, fetchMetricsOnly);
   }

   Glyph *loadGlyph(QGlyphSet *set, uint glyph, QFixed subPixelPosition, GlyphFormat = Format_None,
                  bool fetchMetricsOnly = false) const;

   QGlyphSet *defaultGlyphs() {
      return &defaultGlyphSet;
   }

   GlyphFormat defaultGlyphFormat() const {
      return defaultFormat;
   }

   Glyph *cachedGlyph(glyph_t g) const {
      return defaultGlyphSet.getGlyph(g, 0);
   }

   QGlyphSet *loadTransformedGlyphSet(const QTransform &matrix);
   QFixed subPixelPositionForX(QFixed x);

   bool loadGlyphs(QGlyphSet *gs, const glyph_t *glyphs, int num_glyphs, const QFixedPoint *positions,
                  GlyphFormat format = Format_Render);

#if defined(Q_WS_QWS)
   virtual void draw(QPaintEngine * /*p*/, qreal /*x*/, qreal /*y*/, const QTextItemInt & /*si*/) {}
#endif

   QFontEngineFT(const QFontDef &fd);
   virtual ~QFontEngineFT();

   bool init(FaceId faceId, bool antiaalias, GlyphFormat defaultFormat = Format_None,
                  const QByteArray &fontData = QByteArray());

   bool init(FaceId faceId, bool antialias, GlyphFormat format, QFreetypeFace *freetypeFace);

   HB_Error getPointInOutline(HB_Glyph glyph, int flags, hb_uint32 point, HB_Fixed *xpos,
                  HB_Fixed *ypos, hb_uint32 *nPoints) override;

   enum HintStyle {
      HintNone,
      HintLight,
      HintMedium,
      HintFull
   };

   void setDefaultHintStyle(HintStyle style);

   QFontEngine *cloneWithSize(qreal pixelSize) const override;
   bool initFromFontEngine(const QFontEngineFT *fontEngine);

   HintStyle defaultHintStyle() const {
      return default_hint_style;
   }
 protected:

   void freeGlyphSets();

   virtual bool uploadGlyphToServer(QGlyphSet *set, uint glyphid, Glyph *g, GlyphInfo *info, int glyphDataSize) const;
   virtual unsigned long allocateServerGlyphSet();
   virtual void freeServerGlyphSet(unsigned long id);

   QFreetypeFace *freetype;
   int default_load_flags;


   HintStyle default_hint_style;

   bool antialias;
   bool transform;
   bool embolden;
   SubpixelAntialiasingType subpixelType;
   int lcdFilterType;
   bool canUploadGlyphsToServer;
   bool embeddedbitmap;

 private:
   friend class QFontEngineFTRawFont;

   int loadFlags(QGlyphSet *set, GlyphFormat format, int flags, bool &hsubpixel, int &vfactor) const;

   GlyphFormat defaultFormat;
   FT_Matrix matrix;

   QList<QGlyphSet> transformedGlyphSets;
   mutable QGlyphSet defaultGlyphSet;

   QFontEngine::FaceId face_id;

   int xsize;
   int ysize;

   mutable QFixed lbearing;
   mutable QFixed rbearing;
   QFixed line_thickness;
   QFixed underline_position;

   FT_Size_Metrics metrics;
   mutable bool kerning_pairs_loaded;
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


QT_END_NAMESPACE

#endif // QT_NO_FREETYPE

#endif // QFONTENGINE_FT_P_H
