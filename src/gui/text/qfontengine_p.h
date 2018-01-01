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

#ifndef QFONTENGINE_P_H
#define QFONTENGINE_P_H

#include <QtCore/qglobal.h>
#include <QtCore/qatomic.h>
#include <QtCore/qvarlengtharray.h>
#include <QtCore/QLinkedList>
#include <qtextengine_p.h>
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

#include <qfontengineglyphcache_p.h>

struct glyph_metrics_t;
typedef unsigned int glyph_t;

QT_BEGIN_NAMESPACE

class QChar;
class QPainterPath;
class QTextEngine;
struct QGlyphLayout;

#define MAKE_TAG(ch1, ch2, ch3, ch4) (\
    (((quint32)(ch1)) << 24) | \
    (((quint32)(ch2)) << 16) | \
    (((quint32)(ch3)) << 8) | \
    ((quint32)(ch4)) \
   )

class Q_GUI_EXPORT QFontEngine : public QObject
{
 public:
   enum Type {
      Box,
      Multi,

      // X11 types
      XLFD,

      // MS Windows types
      Win,

      // Apple Mac OS types
      Mac,

      // QWS types
      Freetype,
      QPF1,
      QPF2,
      Proxy,

      // S60 types
      S60FontEngine, // Cannot be simply called "S60". Reason is qt_s60Data.h

      DirectWrite,

      TestFontEngine = 0x1000
   };

   enum GlyphFormat {
      Format_None,
      Format_Render = Format_None,
      Format_Mono,
      Format_A8,
      Format_A32
   };

   QFontEngine();
   virtual ~QFontEngine();

   // all of these are in unscaled metrics if the engine supports uncsaled metrics,
   // otherwise in design metrics
   struct Properties {
      QByteArray postscriptName;
      QByteArray copyright;
      QRectF boundingBox;
      QFixed emSquare;
      QFixed ascent;
      QFixed descent;
      QFixed leading;
      QFixed italicAngle;
      QFixed capHeight;
      QFixed lineWidth;
   };
   virtual Properties properties() const;
   virtual void getUnscaledGlyph(glyph_t glyph, QPainterPath *path, glyph_metrics_t *metrics);
   QByteArray getSfntTable(uint /*tag*/) const;
   virtual bool getSfntTableData(uint /*tag*/, uchar * /*buffer*/, uint * /*length*/) const {
      return false;
   }

   struct FaceId {
      FaceId() : index(0), encoding(0) {}
      QByteArray filename;
      QByteArray uuid;
      int index;
      int encoding;
   };
   virtual FaceId faceId() const {
      return FaceId();
   }
   enum SynthesizedFlags {
      SynthesizedItalic = 0x1,
      SynthesizedBold = 0x2,
      SynthesizedStretch = 0x4
   };
   virtual int synthesized() const {
      return 0;
   }
   virtual bool supportsSubPixelPositions() const {
      return false;
   }

   virtual QFixed emSquareSize() const {
      return ascent();
   }

   /* returns 0 as glyph index for non existent glyphs */
   virtual bool stringToCMap(const QChar *str, int len, QGlyphLayout *glyphs, int *nglyphs,
                             QTextEngine::ShaperFlags flags) const = 0;

   /**
    * This is a callback from harfbuzz. The font engine uses the font-system in use to find out the
    * advances of each glyph and set it on the layout.
    */
   virtual void recalcAdvances(QGlyphLayout *, QTextEngine::ShaperFlags) const {}
   virtual void doKerning(QGlyphLayout *, QTextEngine::ShaperFlags) const;

#if !defined(Q_WS_X11) && !defined(Q_OS_WIN) && !defined(Q_OS_MAC) && !defined(Q_WS_QPA)
   virtual void draw(QPaintEngine *p, qreal x, qreal y, const QTextItemInt &si) = 0;
#endif

   virtual void addGlyphsToPath(glyph_t *glyphs, QFixedPoint *positions, int nglyphs,
                                QPainterPath *path, QTextItem::RenderFlags flags);

   void getGlyphPositions(const QGlyphLayout &glyphs, const QTransform &matrix, QTextItem::RenderFlags flags,
                          QVarLengthArray<glyph_t> &glyphs_out, QVarLengthArray<QFixedPoint> &positions);

   virtual void addOutlineToPath(qreal, qreal, const QGlyphLayout &, QPainterPath *, QTextItem::RenderFlags flags);
   void addBitmapFontToPath(qreal x, qreal y, const QGlyphLayout &, QPainterPath *, QTextItem::RenderFlags);
   /**
    * Create a qimage with the alpha values for the glyph.
    * Returns an image indexed_8 with index values ranging from 0=fully transparent to 255=opaque
    */
   virtual QImage alphaMapForGlyph(glyph_t);
   virtual QImage alphaMapForGlyph(glyph_t glyph, QFixed subPixelPosition);
   virtual QImage alphaMapForGlyph(glyph_t, const QTransform &t);
   virtual QImage alphaMapForGlyph(glyph_t, QFixed subPixelPosition, const QTransform &t);
   virtual QImage alphaRGBMapForGlyph(glyph_t, QFixed subPixelPosition, int margin, const QTransform &t);

   virtual glyph_metrics_t alphaMapBoundingBox(glyph_t glyph, QFixed /*subPixelPosition*/, const QTransform &matrix,
         GlyphFormat /*format*/) {
      return boundingBox(glyph, matrix);
   }

   virtual void removeGlyphFromCache(glyph_t);

   virtual glyph_metrics_t boundingBox(const QGlyphLayout &glyphs) = 0;
   virtual glyph_metrics_t boundingBox(glyph_t glyph) = 0;
   virtual glyph_metrics_t boundingBox(glyph_t glyph, const QTransform &matrix);
   glyph_metrics_t tightBoundingBox(const QGlyphLayout &glyphs);

   virtual QFixed ascent() const = 0;
   virtual QFixed descent() const = 0;
   virtual QFixed leading() const = 0;
   virtual QFixed xHeight() const;
   virtual QFixed averageCharWidth() const;

   virtual QFixed lineThickness() const;
   virtual QFixed underlinePosition() const;

   virtual qreal maxCharWidth() const = 0;
   virtual qreal minLeftBearing() const {
      return qreal();
   }
   virtual qreal minRightBearing() const {
      return qreal();
   }

   virtual void getGlyphBearings(glyph_t glyph, qreal *leftBearing = 0, qreal *rightBearing = 0);

   virtual const char *name() const = 0;

   virtual bool canRender(const QChar *string, int len) = 0;

   virtual Type type() const = 0;

   virtual int glyphCount() const;

   virtual QFontEngine *cloneWithSize(qreal /*pixelSize*/) const {
      return 0;
   }

   HB_Font harfbuzzFont() const;
   HB_Face harfbuzzFace() const;

   virtual HB_Error getPointInOutline(HB_Glyph glyph, int flags, hb_uint32 point, HB_Fixed *xpos, HB_Fixed *ypos,
                                      hb_uint32 *nPoints);

   void setGlyphCache(void *key, QFontEngineGlyphCache *data);
   QFontEngineGlyphCache *glyphCache(void *key, QFontEngineGlyphCache::Type type, const QTransform &transform) const;

   static const uchar *getCMap(const uchar *table, uint tableSize, bool *isSymbolFont, int *cmapSize);
   static quint32 getTrueTypeGlyphIndex(const uchar *cmap, uint unicode);

   static QByteArray convertToPostscriptFontFamilyName(const QByteArray &fontFamily);

   QAtomicInt ref;
   QFontDef fontDef;
   uint cache_cost; // amount of mem used in kb by the font
   int cache_count;
   uint fsType : 16;
   bool symbol;
   mutable HB_FontRec hbFont;
   mutable HB_Face hbFace;

#if defined(Q_OS_WIN) || defined(Q_WS_X11) || defined(Q_WS_QWS) || defined(Q_WS_QPA)
   struct KernPair {
      uint left_right;
      QFixed adjust;

      inline bool operator<(const KernPair &other) const {
         return left_right < other.left_right;
      }
   };
   QVector<KernPair> kerning_pairs;
   void loadKerningPairs(QFixed scalingFactor);
#endif

   int glyphFormat;

 protected:
   static const QVector<QRgb> &grayPalette();
   QFixed lastRightBearing(const QGlyphLayout &glyphs, bool round = false);

 private:
   struct GlyphCacheEntry {
      void *context;
      QExplicitlySharedDataPointer<QFontEngineGlyphCache> cache;

      bool operator==(const GlyphCacheEntry &other) const {
         return (context == other.context) && (cache == other.cache);
      }
   };

   mutable QLinkedList<GlyphCacheEntry> m_glyphCaches;
};

inline bool operator ==(const QFontEngine::FaceId &f1, const QFontEngine::FaceId &f2)
{
   return (f1.index == f2.index) && (f1.encoding == f2.encoding) && (f1.filename == f2.filename);
}

inline uint qHash(const QFontEngine::FaceId &f)
{
   return qHash((f.index << 16) + f.encoding) + qHash(f.filename + f.uuid);
}


class QGlyph;

#if defined(Q_WS_QWS)

#ifndef QT_NO_QWS_QPF

class QFontEngineQPF1Data;

class QFontEngineQPF1 : public QFontEngine
{
 public:
   QFontEngineQPF1(const QFontDef &, const QString &fn);
   ~QFontEngineQPF1();

   bool stringToCMap(const QChar *str, int len, QGlyphLayout *glyphs, int *nglyphs,
                  QTextEngine::ShaperFlags flags) const override;
   void recalcAdvances(QGlyphLayout *, QTextEngine::ShaperFlags) const override;

   void draw(QPaintEngine *p, qreal x, qreal y, const QTextItemInt &si) override;
   void addOutlineToPath(qreal x, qreal y, const QGlyphLayout &glyphs, QPainterPath *path,
                  QTextItem::RenderFlags flags) override;

   glyph_metrics_t boundingBox(const QGlyphLayout &glyphs) override;
   glyph_metrics_t boundingBox(glyph_t glyph) override;

   QFixed ascent() const override;
   QFixed descent() const override;
   QFixed leading() const override;
   qreal maxCharWidth() const override;
   qreal minLeftBearing() const override;
   qreal minRightBearing() const override;
   QFixed underlinePosition() const override;
   QFixed lineThickness() const override;

   Type type() const override;

   bool canRender(const QChar *string, int len) override;
   const char *name() const override {
      return 0;
   }
   QImage alphaMapForGlyph(glyph_t) override;

   QFontEngineQPF1Data *d;
};
#endif // QT_NO_QWS_QPF

#endif // QWS


class QFontEngineBox : public QFontEngine
{
 public:
   QFontEngineBox(int size);
   ~QFontEngineBox();

   bool stringToCMap(const QChar *str, int len, QGlyphLayout *glyphs, int *nglyphs,
                  QTextEngine::ShaperFlags flags) const override;

   void recalcAdvances(QGlyphLayout *, QTextEngine::ShaperFlags) const override;

#if !defined(Q_WS_X11) && !defined(Q_OS_WIN) && !defined(Q_OS_MAC)
   void draw(QPaintEngine *p, qreal x, qreal y, const QTextItemInt &si);
#endif

   void addOutlineToPath(qreal x, qreal y, const QGlyphLayout &glyphs, QPainterPath *path,
                  QTextItem::RenderFlags flags) override;

   glyph_metrics_t boundingBox(const QGlyphLayout &glyphs) override;
   glyph_metrics_t boundingBox(glyph_t glyph) override;

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

#ifdef Q_WS_X11
   int cmap() const;
#endif

   const char *name() const override;

   bool canRender(const QChar *string, int len) override;
   Type type() const override;

   int size() const {
      return _size;
   }

 private:
   friend class QFontPrivate;
   int _size;
};

class QFontEngineMulti : public QFontEngine
{
 public:
   explicit QFontEngineMulti(int engineCount);
   ~QFontEngineMulti();

   bool stringToCMap(const QChar *str, int len, QGlyphLayout *glyphs, int *nglyphs,
                  QTextEngine::ShaperFlags flags) const override;

   glyph_metrics_t boundingBox(const QGlyphLayout &glyphs) override;
   glyph_metrics_t boundingBox(glyph_t glyph) override;

   void recalcAdvances(QGlyphLayout *, QTextEngine::ShaperFlags) const override;
   void doKerning(QGlyphLayout *, QTextEngine::ShaperFlags) const override;
   void addOutlineToPath(qreal, qreal, const QGlyphLayout &, QPainterPath *, QTextItem::RenderFlags flags) override;
   void getGlyphBearings(glyph_t glyph, qreal *leftBearing = 0, qreal *rightBearing = 0) override;

   QFixed ascent() const override;
   QFixed descent() const override;
   QFixed leading() const override;
   QFixed xHeight() const override;
   QFixed averageCharWidth() const override;
   QImage alphaMapForGlyph(glyph_t) override;

   QFixed lineThickness() const override;
   QFixed underlinePosition() const override;
   qreal maxCharWidth() const override;
   qreal minLeftBearing() const override;
   qreal minRightBearing() const override;

   Type type() const override {
      return QFontEngine::Multi;
   }

   bool canRender(const QChar *string, int len) override;

   const char *name() const override {
      return "Multi";
   }

   QFontEngine *engine(int at) const {
      Q_ASSERT(at < engines.size());
      return engines.at(at);
   }

 protected:
   friend class QPSPrintEnginePrivate;
   friend class QPSPrintEngineFontMulti;
   friend class QRawFont;
   virtual void loadEngine(int at) = 0;
   QVector<QFontEngine *> engines;
};

class QTestFontEngine : public QFontEngineBox
{
 public:
   QTestFontEngine(int size) : QFontEngineBox(size) {}

   Type type() const  override {
      return TestFontEngine;
   }
};

QT_END_NAMESPACE

#ifdef Q_OS_WIN
#   include <qfontengine_win_p.h>
#endif

#endif // QFONTENGINE_P_H
