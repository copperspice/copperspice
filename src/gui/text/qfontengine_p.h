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

#ifndef QFONTENGINE_P_H
#define QFONTENGINE_P_H

#include <qglobal.h>
#include <qatomic.h>

#include <qlinkedlist.h>
#include <qvarlengtharray.h>
#include <qstringfwd.h>
#include <qfontengine_faceid_p.h>
#include <qtextengine_p.h>
#include <qfont_p.h>

class QGlyph;
class QPainterPath;
class QFontEngineGlyphCache;

struct QGlyphLayout;

#define MAKE_TAG(ch1, ch2, ch3, ch4) (\
    (((quint32)(ch1)) << 24) | \
    (((quint32)(ch2)) << 16) | \
    (((quint32)(ch3)) << 8)  | \
    ((quint32)(ch4)) \
   )

typedef void (*qt_destroy_func_t) (void *user_data);
typedef bool (*qt_get_font_table_func_t) (void *user_data, uint tag, uchar *buffer, uint *length);

class Q_GUI_EXPORT QFontEngine
{

 public:
   using FaceId = QFontEngine_FaceId;

   enum Type {
      Box,
      Multi,

      // MS Windows types
      Win,

      // Apple Mac OS types
      Mac,

      // QWS types
      Freetype,
      QPF1,
      QPF2,
      Proxy,

      DirectWrite,
      TestFontEngine = 0x1000
   };

   enum GlyphFormat {
      Format_None,
      Format_Render = Format_None,
      Format_Mono,
      Format_A8,
      Format_A32,
      Format_ARGB
   };

   enum ShaperFlag {
      DesignMetrics    = 0x0002,
      GlyphIndicesOnly = 0x0004
   };
   using ShaperFlags = QFlags<ShaperFlag>;


   enum SynthesizedFlags {
      SynthesizedItalic  = 0x1,
      SynthesizedBold    = 0x2,
      SynthesizedStretch = 0x4
   };

   enum HintStyle {
      HintNone,
      HintLight,
      HintMedium,
      HintFull
   };

   enum SubpixelAntialiasingType {
      Subpixel_None,
      Subpixel_RGB,
      Subpixel_BGR,
      Subpixel_VRGB,
      Subpixel_VBGR
   };

   // all of these are in unscaled metrics if the engine supports uncsaled metrics,
   // otherwise in design metrics

   struct Properties {
      QString postscriptName;
      QString copyright;

      QRectF boundingBox;
      QFixed emSquare;
      QFixed ascent;
      QFixed descent;
      QFixed leading;
      QFixed italicAngle;
      QFixed capHeight;
      QFixed lineWidth;
   };

   virtual ~QFontEngine();

   inline Type type() const {
      return m_type;
   }

   virtual Properties properties() const;
   virtual void getUnscaledGlyph(glyph_t glyph, QPainterPath *path, glyph_metrics_t *metrics);

   QByteArray getSfntTable(uint tag) const;
   virtual bool getSfntTableData(uint tag, uchar *buffer, uint *length) const;

   virtual FaceId faceId() const {
      return FaceId();
   }

   virtual int synthesized() const {
      return 0;
   }

   virtual bool supportsSubPixelPositions() const {
      return false;
   }

   virtual QFixed subPixelPositionForX(QFixed x) const;

   virtual QFixed emSquareSize() const {
      return ascent();
   }

   virtual glyph_t glyphIndex(char32_t ch) const = 0;

   // returns 0 as glyph index for non existent glyphs
   virtual bool stringToCMap(QStringView str, QGlyphLayout *glyphs, int *nglyphs, QFontEngine::ShaperFlags flags) const = 0;

   virtual void recalcAdvances(QGlyphLayout *, ShaperFlags) const {}
   virtual void doKerning(QGlyphLayout *, ShaperFlags) const;

   virtual void addGlyphsToPath(glyph_t *glyphs, QFixedPoint *positions, int nglyphs,
      QPainterPath *path, QTextItem::RenderFlags flags);

   void getGlyphPositions(const QGlyphLayout &glyphs, const QTransform &matrix, QTextItem::RenderFlags flags,
      QVarLengthArray<glyph_t> &glyphs_out, QVarLengthArray<QFixedPoint> &positions);

   virtual void addOutlineToPath(qreal, qreal, const QGlyphLayout &, QPainterPath *, QTextItem::RenderFlags flags);
   void addBitmapFontToPath(qreal x, qreal y, const QGlyphLayout &, QPainterPath *, QTextItem::RenderFlags);

   // Create a qimage with the alpha values for the glyph.
   // Returns an image indexed_8 with index values ranging from 0=fully transparent to 255=opaque
   virtual QImage alphaMapForGlyph(glyph_t);
   virtual QImage alphaMapForGlyph(glyph_t glyph, QFixed subPixelPosition);
   virtual QImage alphaMapForGlyph(glyph_t, const QTransform &t);
   virtual QImage alphaMapForGlyph(glyph_t, QFixed subPixelPosition, const QTransform &t);
   virtual QImage alphaRGBMapForGlyph(glyph_t glyph, QFixed subPixelPosition, const QTransform &t);

   virtual QImage bitmapForGlyph(glyph_t, QFixed subPixelPosition, const QTransform &t);

   virtual QImage *lockedAlphaMapForGlyph(glyph_t glyph, QFixed subPixelPosition, GlyphFormat neededFormat,
      const QTransform &t = QTransform(), QPoint *offset = nullptr);

   virtual void unlockAlphaMapForGlyph();
   virtual bool hasInternalCaching() const {
      return false;
   }

   virtual glyph_metrics_t alphaMapBoundingBox(glyph_t glyph, QFixed subPixelPosition,
                  const QTransform &matrix, GlyphFormat format) {
      (void) subPixelPosition;
      (void) format;

      return boundingBox(glyph, matrix);
   }

   virtual void removeGlyphFromCache(glyph_t);

   virtual glyph_metrics_t boundingBox(const QGlyphLayout &glyphs) = 0;
   virtual glyph_metrics_t boundingBox(glyph_t glyph) = 0;
   virtual glyph_metrics_t boundingBox(glyph_t glyph, const QTransform &matrix);
   glyph_metrics_t tightBoundingBox(const QGlyphLayout &glyphs);

   virtual QFixed ascent()  const = 0;
   virtual QFixed descent() const = 0;
   virtual QFixed leading() const = 0;
   virtual QFixed xHeight() const;

   virtual QFixed averageCharWidth() const;

   virtual QFixed lineThickness() const;
   virtual QFixed underlinePosition() const;

   virtual qreal maxCharWidth() const = 0;
   virtual qreal minLeftBearing() const;
   virtual qreal minRightBearing() const;

   virtual void getGlyphBearings(glyph_t glyph, qreal *leftBearing = nullptr, qreal *rightBearing = nullptr);

   virtual bool canRender(QStringView str) const;
   virtual const QString &fontEngineName() const = 0;

   virtual bool supportsTransformation(const QTransform &transform) const;

   virtual int glyphCount() const;
   virtual int glyphMargin(GlyphFormat format) {
      return format == Format_A32 ? 2 : 0;
   }

   virtual QFontEngine *cloneWithSize(qreal pixelSize) const {
      (void) pixelSize;

      return nullptr;
   }

   virtual Qt::HANDLE handle() const;

   bool supportsScript(QChar::Script script) const;

   virtual int getPointInOutline(glyph_t glyph, int flags, quint32 point, QFixed  *xpos, QFixed  *ypos, quint32 *nPoints);

   void clearGlyphCache(const void *key);
   void setGlyphCache(const void *key, QFontEngineGlyphCache *cache);
   QFontEngineGlyphCache *glyphCache(const void *key, GlyphFormat format, const QTransform &transform) const;

   static const uchar *getCMap(const uchar *table, uint tableSize, bool *isSymbolFont, int *cmapSize);
   static quint32 getTrueTypeGlyphIndex(const uchar *cmap, int cmapSize, char32_t ch);

   static QString convertToPostscriptFontFamilyName(const QString &fontFamily);

   virtual bool hasUnreliableGlyphOutline() const;

   virtual void setDefaultHintStyle(HintStyle) {
   }

   inline QVariant userData() const {
      return m_userData;
   }

   // harfbuzz
   std::shared_ptr<hb_face_t> harfbuzzFace() const;

   mutable std::shared_ptr<hb_font_t> m_hb_font;
   mutable std::shared_ptr<hb_face_t> m_hb_face;

   struct FaceData {
      void *user_data;
      cs_fontTable_func_ptr  m_fontTable_funcPtr;
   } faceData;

   QAtomicInt m_refCount;
   QFontDef fontDef;

   uint cache_cost;       // amount of memory used in kb by the font

   uint fsType : 16;
   bool symbol;
   bool isSmoothlyScalable;

   struct KernPair {
      uint left_right;
      QFixed adjust;

      inline bool operator<(const KernPair &other) const {
         return left_right < other.left_right;
      }
   };

   QVector<KernPair> kerning_pairs;
   void loadKerningPairs(QFixed scalingFactor);

   GlyphFormat glyphFormat;
   QImage currentlyLockedAlphaMap;
   int m_subPixelPositionCount;             // number of positions within a single pixel for this cache

 protected:
   explicit QFontEngine(Type type);

   QFixed lastRightBearing(const QGlyphLayout &glyphs, bool round = false);

   inline void setUserData(const QVariant &userData) {
      m_userData = userData;
   }

 private:
   const Type m_type;
   QVariant m_userData;

   mutable qreal m_minLeftBearing;
   mutable qreal m_minRightBearing;

   struct GlyphCacheEntry {
      GlyphCacheEntry();
      GlyphCacheEntry(const GlyphCacheEntry &);

      ~GlyphCacheEntry();

      GlyphCacheEntry &operator=(const GlyphCacheEntry &);

      QExplicitlySharedDataPointer<QFontEngineGlyphCache> cache;

      bool operator==(const GlyphCacheEntry &other) const {
         return cache == other.cache;
      }
   };

   using GlyphCaches = QLinkedList<GlyphCacheEntry>;
   mutable QHash<const void *, GlyphCaches> m_glyphCaches;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QFontEngine::ShaperFlags)

inline bool operator ==(const QFontEngine::FaceId &f1, const QFontEngine::FaceId &f2)
{
   return f1.index == f2.index && f1.encoding == f2.encoding && f1.filename == f2.filename && f1.uuid == f2.uuid;
}

inline uint qHash(const QFontEngine::FaceId &f, uint seed = 0) noexcept(noexcept(qHash(f.filename)))
{
   seed = qHash(f.filename, seed);
   seed = qHash(f.uuid,     seed);
   seed = qHash(f.index,    seed);
   seed = qHash(f.encoding, seed);

   return seed;
}

class QFontEngineBox : public QFontEngine
{
 public:
   QFontEngineBox(int size);
   ~QFontEngineBox();

   glyph_t glyphIndex(char32_t ch) const override;

   bool stringToCMap(QStringView str, QGlyphLayout *glyphs, int *nglyphs, QFontEngine::ShaperFlags flags) const override;
   void recalcAdvances(QGlyphLayout *, QFontEngine::ShaperFlags) const override;

   void draw(QPaintEngine *p, qreal x, qreal y, const QTextItemInt &si);

   void addOutlineToPath(qreal x, qreal y, const QGlyphLayout &glyphs, QPainterPath *path, QTextItem::RenderFlags flags) override;

   glyph_metrics_t boundingBox(const QGlyphLayout &glyphs) override;
   glyph_metrics_t boundingBox(glyph_t glyph) override;

   QFontEngine *cloneWithSize(qreal pixelSize) const override;

   QFixed ascent() const override;
   QFixed descent() const override;
   QFixed leading() const override;

   qreal maxCharWidth() const override;

   qreal minLeftBearing() const override {
      return 0;
   }

   qreal minRightBearing() const override {
      return 0;
   }

   QImage alphaMapForGlyph(glyph_t) override;

   bool canRender(QStringView str) const override;

   const QString &fontEngineName() const override {
      static QString retval = "box";
      return retval;
   }

   int size() const {
      return _size;
   }

 protected:
   explicit QFontEngineBox(Type type, int size);

 private:
   friend class QFontPrivate;
   int _size;
};

class Q_GUI_EXPORT QFontEngineMulti : public QFontEngine
{
 public:
   explicit QFontEngineMulti(QFontEngine *engine, int script, const QStringList &fallbackFamilies = QStringList());

   ~QFontEngineMulti();

   glyph_t glyphIndex(char32_t ch) const override;
   bool stringToCMap(QStringView str, QGlyphLayout *glyphs, int *nglyphs, QFontEngine::ShaperFlags flags) const override;

   glyph_metrics_t boundingBox(const QGlyphLayout &glyphs) override;
   glyph_metrics_t boundingBox(glyph_t glyph) override;

   void recalcAdvances(QGlyphLayout *, QFontEngine::ShaperFlags) const override;
   void doKerning(QGlyphLayout *, QFontEngine::ShaperFlags) const override;
   void addOutlineToPath(qreal, qreal, const QGlyphLayout &, QPainterPath *, QTextItem::RenderFlags flags) override;
   void getGlyphBearings(glyph_t glyph, qreal *leftBearing = nullptr, qreal *rightBearing = nullptr) override;

   QFixed ascent() const override;
   QFixed descent() const override;
   QFixed leading() const override;
   QFixed xHeight() const override;

   QFixed averageCharWidth() const override;

   QImage alphaMapForGlyph(glyph_t) override;
   QImage alphaMapForGlyph(glyph_t glyph, QFixed subPixelPosition) override;
   QImage alphaMapForGlyph(glyph_t, const QTransform &t) override;
   QImage alphaMapForGlyph(glyph_t, QFixed subPixelPosition, const QTransform &t) override;
   QImage alphaRGBMapForGlyph(glyph_t, QFixed subPixelPosition, const QTransform &t) override;

   QFixed lineThickness() const override;
   QFixed underlinePosition() const override;
   qreal maxCharWidth() const override;
   qreal minLeftBearing() const override;
   qreal minRightBearing() const override;

   bool canRender(QStringView str) const override;

   inline int fallbackFamilyCount() const {
      return m_fallbackFamilies.size();
   }

   inline QString fallbackFamilyAt(int at) const {
      return m_fallbackFamilies.at(at);
   }

   void setFallbackFamiliesList(const QStringList &fallbackFamilies);

   const QString &fontEngineName() const override {
      static QString retval("Multi");
      return retval;
   }

   QFontEngine *engine(int at) const {
      Q_ASSERT(at < m_engines.size());
      return m_engines.at(at);
   }

   void ensureEngineAt(int at);
   static QFontEngine *createMultiFontEngine(QFontEngine *fe, int script);

 protected:
   virtual void ensureFallbackFamiliesQueried();
   virtual bool shouldLoadFontEngineForCharacter(int at, char32_t ch) const;
   virtual QFontEngine *loadEngine(int at);

 private:
   QVector<QFontEngine *> m_engines;
   QStringList m_fallbackFamilies;

   const int m_script;
   bool m_fallbackFamiliesQueried;
};

class QTestFontEngine : public QFontEngineBox
{
 public:
   QTestFontEngine(int size);
};


#endif
