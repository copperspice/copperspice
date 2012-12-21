/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QFONTENGINE_WIN_P_H
#define QFONTENGINE_WIN_P_H

#include <QtCore/qconfig.h>

QT_BEGIN_NAMESPACE

class QNativeImage;

class QFontEngineWin : public QFontEngine
{
 public:
   QFontEngineWin(const QString &name, HFONT, bool, LOGFONT);
   ~QFontEngineWin();

   virtual QFixed lineThickness() const;
   virtual Properties properties() const;
   virtual void getUnscaledGlyph(glyph_t glyph, QPainterPath *path, glyph_metrics_t *metrics);
   virtual FaceId faceId() const;
   virtual bool getSfntTableData(uint tag, uchar *buffer, uint *length) const;
   virtual int synthesized() const;
   virtual QFixed emSquareSize() const;

   virtual bool stringToCMap(const QChar *str, int len, QGlyphLayout *glyphs, int *nglyphs,
         QTextEngine::ShaperFlags flags) const;

   virtual void recalcAdvances(QGlyphLayout *glyphs, QTextEngine::ShaperFlags) const;

   virtual void addOutlineToPath(qreal x, qreal y, const QGlyphLayout &glyphs, QPainterPath *path,
         QTextItem::RenderFlags flags);

   virtual void addGlyphsToPath(glyph_t *glyphs, QFixedPoint *positions, int nglyphs, QPainterPath *path, 
         QTextItem::RenderFlags flags);

   HGDIOBJ selectDesignFont() const;

   virtual glyph_metrics_t boundingBox(const QGlyphLayout &glyphs);
   virtual glyph_metrics_t boundingBox(glyph_t g) {
      return boundingBox(g, QTransform());
   }
   virtual glyph_metrics_t boundingBox(glyph_t g, const QTransform &t);


   virtual QFixed ascent() const;
   virtual QFixed descent() const;
   virtual QFixed leading() const;
   virtual QFixed xHeight() const;
   virtual QFixed averageCharWidth() const;
   virtual qreal maxCharWidth() const;
   virtual qreal minLeftBearing() const;
   virtual qreal minRightBearing() const;

   virtual const char *name() const;

   bool canRender(const QChar *string, int len);

   Type type() const;

   virtual QImage alphaMapForGlyph(glyph_t t) {
      return alphaMapForGlyph(t, QTransform());
   }
   virtual QImage alphaMapForGlyph(glyph_t, const QTransform &xform);
   virtual QImage alphaRGBMapForGlyph(glyph_t t, QFixed subPixelPosition, int margin, const QTransform &xform);

   virtual QFontEngine *cloneWithSize(qreal pixelSize) const;

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
   QNativeImage *drawGDIGlyph(HFONT font, glyph_t, int margin, const QTransform &xform, QImage::Format mask_format);

};

class QFontEngineMultiWin : public QFontEngineMulti
{
 public:
   QFontEngineMultiWin(QFontEngine *first, const QStringList &fallbacks);
   void loadEngine(int at);

   QStringList fallbacks;
};

QT_END_NAMESPACE

#endif // QFONTENGINE_WIN_P_H
