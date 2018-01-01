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

#ifndef QFONTENGINEDIRECTWRITE_P_H
#define QFONTENGINEDIRECTWRITE_P_H

#ifndef QT_NO_DIRECTWRITE

#include <qfontengine_p.h>

struct IDWriteFont ;
struct IDWriteFontFace ;
struct IDWriteFactory ;
struct IDWriteBitmapRenderTarget ;
struct IDWriteGdiInterop ;

QT_BEGIN_NAMESPACE

class QFontEngineDirectWrite : public QFontEngine
{
   GUI_CS_OBJECT(QFontEngineDirectWrite)

 public:
   explicit QFontEngineDirectWrite(IDWriteFactory *directWriteFactory,
         IDWriteFontFace *directWriteFontFace, qreal pixelSize);

   ~QFontEngineDirectWrite();

   QFixed lineThickness() const;
   bool getSfntTableData(uint tag, uchar *buffer, uint *length) const;
   QFixed emSquareSize() const;

   bool stringToCMap(const QChar *str, int len, QGlyphLayout *glyphs, int *nglyphs, QTextEngine::ShaperFlags flags) const;
   void recalcAdvances(QGlyphLayout *glyphs, QTextEngine::ShaperFlags) const;

   void addGlyphsToPath(glyph_t *glyphs, QFixedPoint *positions, int nglyphs, QPainterPath *path, 
         QTextItem::RenderFlags flags);

   glyph_metrics_t boundingBox(const QGlyphLayout &glyphs);
   glyph_metrics_t boundingBox(glyph_t g);
   glyph_metrics_t alphaMapBoundingBox(glyph_t glyph, QFixed subPixelPosition, const QTransform &matrix, GlyphFormat format);

   QFixed ascent() const;
   QFixed descent() const;
   QFixed leading() const;
   QFixed xHeight() const;
   qreal maxCharWidth() const;

   const char *name() const;

   bool supportsSubPixelPositions() const;

   QImage alphaMapForGlyph(glyph_t, QFixed subPixelPosition, const QTransform &t);
   QImage alphaRGBMapForGlyph(glyph_t t, QFixed subPixelPosition, int margin, const QTransform &xform);

   QFontEngine *cloneWithSize(qreal pixelSize) const;

   bool canRender(const QChar *string, int len);
   Type type() const;

 private:
   friend class QRawFontPrivate;

   QImage imageForGlyph(glyph_t t, QFixed subPixelPosition, int margin, const QTransform &xform);
   void collectMetrics();

   IDWriteFontFace *m_directWriteFontFace;
   IDWriteFactory *m_directWriteFactory;
   IDWriteBitmapRenderTarget *m_directWriteBitmapRenderTarget;

   QFixed m_lineThickness;
   int m_unitsPerEm;
   QFixed m_ascent;
   QFixed m_descent;
   QFixed m_xHeight;
   QFixed m_lineGap;
   FaceId m_faceId;
};

QT_END_NAMESPACE

#endif // QT_NO_DIRECTWRITE

#endif // QFONTENGINEDIRECTWRITE_H
