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

#ifndef QWINDOWSFONTENGINE_H
#define QWINDOWSFONTENGINE_H

#include <qfontengine_p.h>

#include <qimage.h>
#include <qsharedpointer.h>
#include <qwin_additional.h>

class QWindowsNativeImage;
class QWindowsFontEngineData;

class QWindowsFontEngine : public QFontEngine
{
 public:
   QWindowsFontEngine(const QString &name, LOGFONT lf,
      const QSharedPointer<QWindowsFontEngineData> &fontEngineData);

   ~QWindowsFontEngine();

   const QString &fontEngineName() const override {
      static QString retval = "windows";
      return retval;
   }

   void initFontInfo(const QFontDef &request, int dpi);

   QFixed lineThickness() const override;
   Properties properties() const override;
   void getUnscaledGlyph(glyph_t glyph, QPainterPath *path, glyph_metrics_t *metrics) override;
   FaceId faceId() const override;
   bool getSfntTableData(uint tag, uchar *buffer, uint *length) const override;
   int synthesized() const override;
   QFixed emSquareSize() const override;

   glyph_t glyphIndex(char32_t ch) const override;
   bool stringToCMap(QStringView view, QGlyphLayout *glyphs, int *nglyphs, ShaperFlags flags) const override;

   void recalcAdvances(QGlyphLayout *glyphs, ShaperFlags) const override;

   void addOutlineToPath(qreal x, qreal y, const QGlyphLayout &glyphs, QPainterPath *path, QTextItem::RenderFlags flags) override;
   virtual void addGlyphsToPath(glyph_t *glyphs, QFixedPoint *positions, int nglyphs,
      QPainterPath *path, QTextItem::RenderFlags flags) override;

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

   QImage alphaMapForGlyph(glyph_t t) override {
      return alphaMapForGlyph(t, QTransform());
   }
   QImage alphaMapForGlyph(glyph_t, const QTransform &xform) override;
   QImage alphaRGBMapForGlyph(glyph_t t, QFixed subPixelPosition, const QTransform &xform) override;
   glyph_metrics_t alphaMapBoundingBox(glyph_t glyph, QFixed, const QTransform &matrix, GlyphFormat) override;

   QFontEngine *cloneWithSize(qreal pixelSize) const override;
   Qt::HANDLE handle() const override;
   bool supportsTransformation(const QTransform &transform) const override;

#ifndef Q_CC_MINGW
   void getGlyphBearings(glyph_t glyph, qreal *leftBearing = 0, qreal *rightBearing = 0) override;
#endif

   bool hasUnreliableGlyphOutline() const override;

   int getGlyphIndexes(QStringView strView, QGlyphLayout *glyphs) const;
   void getCMap();

   bool getOutlineMetrics(glyph_t glyph, const QTransform &t, glyph_metrics_t *metrics) const;

   const QSharedPointer<QWindowsFontEngineData> &fontEngineData() const {
      return m_fontEngineData;
   }

   void setUniqueFamilyName(const QString &newName) {
      uniqueFamilyName = newName;
   }

 private:
   QWindowsNativeImage *drawGDIGlyph(HFONT font, glyph_t, int margin, const QTransform &xform,
      QImage::Format mask_format);
   bool hasCFFTable() const;
   bool hasCMapTable() const;
   bool hasGlyfTable() const;
   bool hasEbdtTable() const;

   const QSharedPointer<QWindowsFontEngineData> m_fontEngineData;

   const QString     _name;
   QString     uniqueFamilyName;
   HFONT       hfont;
   const LOGFONT     m_logfont;
   uint        ttf        : 1;
   uint        hasOutline : 1;
   uint        hasUnreliableOutline : 1;
   uint        cffTable   : 1;

   TEXTMETRIC  tm;
   const unsigned char *cmap;
   int cmapSize;

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

   // emerald (multi)
   friend class QWindowsMultiFontEngine;
};

// emerald (multi)
class QWindowsMultiFontEngine : public QFontEngineMulti
{
 public:
   explicit QWindowsMultiFontEngine(QFontEngine *fe, int script);

   QFontEngine *loadEngine(int at) override;
};

CS_DECLARE_METATYPE(LOGFONT)

#endif

