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

#ifndef QFONTENGINE_WIN_P_H
#define QFONTENGINE_WIN_P_H

QT_BEGIN_NAMESPACE

class QNativeImage;

class QFontEngineWin : public QFontEngine
{
 public:
   QFontEngineWin(const QString &name, HFONT, bool, LOGFONT);
   ~QFontEngineWin();

   QFixed lineThickness() const override;
   Properties properties() const override;
   void getUnscaledGlyph(glyph_t glyph, QPainterPath *path, glyph_metrics_t *metrics) override;
   FaceId faceId() const override;
   bool getSfntTableData(uint tag, uchar *buffer, uint *length) const override;
   int synthesized() const override;
   QFixed emSquareSize() const override;

   bool stringToCMap(const QChar *str, int len, QGlyphLayout *glyphs, int *nglyphs,
         QTextEngine::ShaperFlags flags) const override;

   void recalcAdvances(QGlyphLayout *glyphs, QTextEngine::ShaperFlags) const override;

   void addOutlineToPath(qreal x, qreal y, const QGlyphLayout &glyphs, QPainterPath *path,
         QTextItem::RenderFlags flags) override;

   void addGlyphsToPath(glyph_t *glyphs, QFixedPoint *positions, int nglyphs, QPainterPath *path,
         QTextItem::RenderFlags flags) override;

   HGDIOBJ selectDesignFont() const;

   glyph_metrics_t boundingBox(const QGlyphLayout &glyphs) override;
   glyph_metrics_t boundingBox(glyph_t g) override {
      return boundingBox(g, QTransform());
   }

   glyph_metrics_t boundingBox(glyph_t g, const QTransform &t) override;

   QFixed ascent() const override;
   QFixed descent() const override;
   QFixed leading() const override;
   QFixed xHeight() const override;
   QFixed averageCharWidth() const override;
   qreal maxCharWidth() const override;
   qreal minLeftBearing() const override;
   qreal minRightBearing() const override;

   const char *name() const override;

   bool canRender(const QChar *string, int len) override;

   Type type() const override;

   QImage alphaMapForGlyph(glyph_t t) override {
      return alphaMapForGlyph(t, QTransform());
   }

   QImage alphaMapForGlyph(glyph_t, const QTransform &xform) override;
   QImage alphaRGBMapForGlyph(glyph_t t, QFixed subPixelPosition, int margin, const QTransform &xform) override;

   QFontEngine *cloneWithSize(qreal pixelSize) const override;

#ifndef Q_CC_MINGW
   virtual void getGlyphBearings(glyph_t glyph, qreal *leftBearing = 0, qreal *rightBearing = 0);
#endif

   int getGlyphIndexes(const QChar *ch, int numChars, QGlyphLayout *glyphs, bool mirrored) const;
   void getCMap();

   bool getOutlineMetrics(glyph_t glyph, const QTransform &t, glyph_metrics_t *metrics) const;

   QString     _name;
   QString     uniqueFamilyName;
   HFONT       hfont;
   LOGFONT     logfont;
   uint        stockFont  : 1;
   uint        ttf        : 1;
   uint        hasOutline : 1;
   uint        cffTable   : 1;
   TEXTMETRIC  tm;
   int         lw;
   const unsigned char *cmap;
   QByteArray cmapTable;
   mutable qreal lbearing;
   mutable qreal rbearing;
   QFixed designToDevice;
   int unitsPerEm;
   QFixed x_height;
   FaceId _faceId;

   mutable int synthesized_flags;
   mutable QFixed lineWidth;
   mutable unsigned char *widthCache;
   mutable uint widthCacheSize;
   mutable QFixed *designAdvances;
   mutable int designAdvancesSize;

 private:
   bool hasCFFTable() const;
   bool hasCMapTable() const;
   QNativeImage *drawGDIGlyph(HFONT font, glyph_t, int margin, const QTransform &xform, QImage::Format mask_format);

};

class QFontEngineMultiWin : public QFontEngineMulti
{
 public:
   QFontEngineMultiWin(QFontEngine *first, const QStringList &fallbacks);
   void loadEngine(int at) override;

   QStringList fallbacks;
};

QT_END_NAMESPACE

#endif // QFONTENGINE_WIN_P_H
