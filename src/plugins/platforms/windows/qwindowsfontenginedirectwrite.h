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

#ifndef QWINDOWSFONTENGINEDIRECTWRITE_H
#define QWINDOWSFONTENGINEDIRECTWRITE_H

#include <qglobal.h>

#ifndef QT_NO_DIRECTWRITE

#include <qfontengine_p.h>
#include <QSharedPointer>

struct IDWriteFont;
struct IDWriteFontFace;
struct IDWriteFactory;
struct IDWriteBitmapRenderTarget;
struct IDWriteGdiInterop;

class QWindowsFontEngineData;

class QWindowsFontEngineDirectWrite : public QFontEngine
{
 public:
   explicit QWindowsFontEngineDirectWrite(IDWriteFontFace *directWriteFontFace,
      qreal pixelSize, const QSharedPointer<QWindowsFontEngineData> &d);

   ~QWindowsFontEngineDirectWrite();

   void initFontInfo(const QFontDef &request, int dpi);

   QFixed lineThickness() const override;
   QFixed underlinePosition() const override;
   bool getSfntTableData(uint tag, uchar *buffer, uint *length) const override;
   QFixed emSquareSize() const override;

   glyph_t glyphIndex(char32_t ch) const override;
   bool stringToCMap(QStringView view, QGlyphLayout *glyphs, int *nglyphs, ShaperFlags flags) const override;

   void recalcAdvances(QGlyphLayout *glyphs, ShaperFlags) const override;
   void addGlyphsToPath(glyph_t *glyphs, QFixedPoint *positions, int nglyphs, QPainterPath *path, QTextItem::RenderFlags flags) override;

   glyph_metrics_t boundingBox(const QGlyphLayout &glyphs) override;
   glyph_metrics_t boundingBox(glyph_t g) override;
   glyph_metrics_t alphaMapBoundingBox(glyph_t glyph, QFixed, const QTransform &matrix, GlyphFormat) override;

   QFixed ascent() const override;
   QFixed descent() const override;
   QFixed leading() const override;
   QFixed xHeight() const override;
   qreal maxCharWidth() const override;

   bool supportsSubPixelPositions() const override;

   QImage alphaMapForGlyph(glyph_t glyph, QFixed subPixelPosition) override;
   QImage alphaMapForGlyph(glyph_t glyph, QFixed subPixelPosition, const QTransform &t) override;
   QImage alphaRGBMapForGlyph(glyph_t t, QFixed subPixelPosition, const QTransform &xform) override;

   QFontEngine *cloneWithSize(qreal pixelSize) const override;
   Qt::HANDLE handle() const override;

   const QSharedPointer<QWindowsFontEngineData> &fontEngineData() const {
      return m_fontEngineData;
   }

   static QString fontNameSubstitute(const QString &familyName);

   IDWriteFontFace *directWriteFontFace() const {
      return m_directWriteFontFace;
   }

 private:
   QImage imageForGlyph(glyph_t t, QFixed subPixelPosition, int margin, const QTransform &xform);
   void collectMetrics();

   const QSharedPointer<QWindowsFontEngineData> m_fontEngineData;

   IDWriteFontFace *m_directWriteFontFace;
   IDWriteBitmapRenderTarget *m_directWriteBitmapRenderTarget;

   QFixed m_lineThickness;
   QFixed m_underlinePosition;
   int m_unitsPerEm;
   QFixed m_ascent;
   QFixed m_descent;
   QFixed m_xHeight;
   QFixed m_lineGap;
   FaceId m_faceId;
};

#endif // QT_NO_DIRECTWRITE

#endif
