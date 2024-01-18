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

#ifndef QFONTENGINE_CORETEXT_P_H
#define QFONTENGINE_CORETEXT_P_H

#include <qfontengine_p.h>
#include <qcore_mac_p.h>

#include <ApplicationServices/ApplicationServices.h>

class QCoreTextFontEngine : public QFontEngine
{
 public:
   QCoreTextFontEngine(CTFontRef font, const QFontDef &def);
   QCoreTextFontEngine(CGFontRef font, const QFontDef &def);
   ~QCoreTextFontEngine();

   glyph_t glyphIndex(char32_t ch) const override;

   bool stringToCMap(QStringView str, QGlyphLayout *glyphs, int *nglyphs, QFontEngine::ShaperFlags flags) const override;
   void recalcAdvances(QGlyphLayout *, ShaperFlags) const override;

   glyph_metrics_t boundingBox(const QGlyphLayout &glyphs) override;
   glyph_metrics_t boundingBox(glyph_t glyph) override;

   QFixed ascent()  const override;
   QFixed descent() const override;
   QFixed leading() const override;
   QFixed xHeight() const override;

   QFixed averageCharWidth() const override;
   qreal maxCharWidth() const override;

   void addGlyphsToPath(glyph_t *glyphs, QFixedPoint *positions, int numGlyphs, QPainterPath *path, QTextItem::RenderFlags) override;

   bool canRender(QStringView str) const override;

   const QString &fontEngineName() const override {
      static QString retval = "CoreText";
      return retval;
   }

   int synthesized() const override {
      return synthesisFlags;
   }

   bool supportsSubPixelPositions() const override {
      return true;
   }

   void draw(CGContextRef ctx, qreal x, qreal y, const QTextItemInt &ti, int paintDeviceHeight);

   FaceId faceId() const override;
   bool getSfntTableData(uint tag, uchar *buffer, uint *length) const override;
   void getUnscaledGlyph(glyph_t glyph, QPainterPath *path, glyph_metrics_t *metrics) override;
   QImage alphaMapForGlyph(glyph_t, QFixed subPixelPosition) override;
   QImage alphaMapForGlyph(glyph_t glyph, QFixed subPixelPosition, const QTransform &t) override;
   QImage alphaRGBMapForGlyph(glyph_t, QFixed subPixelPosition, const QTransform &t) override;
   glyph_metrics_t alphaMapBoundingBox(glyph_t glyph, QFixed, const QTransform &matrix, GlyphFormat) override;
   QImage bitmapForGlyph(glyph_t, QFixed subPixelPosition, const QTransform &t) override;
   QFixed emSquareSize() const override;

   bool supportsTransformation(const QTransform &transform) const override;

   QFontEngine *cloneWithSize(qreal pixelSize) const override;
   Qt::HANDLE handle() const override;

   int glyphMargin(QFontEngine::GlyphFormat format) override {
      (void) format;
      return 0;
   }

   QFontEngine::Properties properties() const override;

   static bool ct_getSfntTable(void *user_data, uint tag, uchar *buffer, uint *length);
   static QFont::Weight qtWeightFromCFWeight(float value);

   static int antialiasingThreshold;
   static QFontEngine::GlyphFormat defaultGlyphFormat;

 private:
   void init();
   QImage imageForGlyph(glyph_t glyph, QFixed subPixelPosition, bool colorful, const QTransform &m);
   CTFontRef ctfont;
   CGFontRef cgFont;
   int synthesisFlags;
   CGAffineTransform transform;
   QFixed avgCharWidth;
   QFontEngine::FaceId face_id;
};

CGAffineTransform qt_transform_from_fontdef(const QFontDef &fontDef);

#endif
