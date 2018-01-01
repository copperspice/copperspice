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

#ifndef QFONTENGINE_X11_P_H
#define QFONTENGINE_X11_P_H

#include <qt_x11_p.h>
#include <qfontengine_ft_p.h>

QT_BEGIN_NAMESPACE

class QFreetypeFace;

class QFontEngineMultiXLFD : public QFontEngineMulti
{
 public:
   QFontEngineMultiXLFD(const QFontDef &r, const QList<int> &l, int s);
   ~QFontEngineMultiXLFD();

   void loadEngine(int at) override;

 private:
   QList<int> encodings;
   int screen;
   QFontDef request;
};

/**
 * \internal The font engine for X Logical Font Description (XLFD) fonts, which is for X11 systems without freetype.
 */
class QFontEngineXLFD : public QFontEngine
{
 public:
   QFontEngineXLFD(XFontStruct *f, const QByteArray &name, int mib);
   ~QFontEngineXLFD();

   QFontEngine::FaceId faceId() const override;
   QFontEngine::Properties properties() const override;
   void getUnscaledGlyph(glyph_t glyph, QPainterPath *path, glyph_metrics_t *metrics) override;
   bool getSfntTableData(uint tag, uchar *buffer, uint *length) const override;
   int synthesized() const override;

   bool stringToCMap(const QChar *str, int len, QGlyphLayout *glyphs, int *nglyphs,
                             QTextEngine::ShaperFlags flags) const override;
   void recalcAdvances(QGlyphLayout *, QTextEngine::ShaperFlags) const override;

   glyph_metrics_t boundingBox(const QGlyphLayout &glyphs) override;
   glyph_metrics_t boundingBox(glyph_t glyph) override;

   void addOutlineToPath(qreal x, qreal y, const QGlyphLayout &glyphs, QPainterPath *path, QTextItem::RenderFlags) override;
   QFixed ascent() const override;
   QFixed descent() const override;
   QFixed leading() const override;
   qreal maxCharWidth() const override;
   qreal minLeftBearing() const override;
   qreal minRightBearing() const override;
   QImage alphaMapForGlyph(glyph_t) override;

   Type type() const override {
      return QFontEngine::XLFD;
   }

   bool canRender(const QChar *string, int len) override;
   const char *name() const override;

   inline XFontStruct *fontStruct() const {
      return _fs;
   }

#ifndef QT_NO_FREETYPE
   FT_Face non_locked_face() const;
   glyph_t glyphIndexToFreetypeGlyphIndex(glyph_t g) const;
#endif
   uint toUnicode(glyph_t g) const;

 private:
   QBitmap bitmapForGlyphs(const QGlyphLayout &glyphs, const glyph_metrics_t &metrics, QTextItem::RenderFlags flags = 0);

   XFontStruct *_fs;
   QByteArray _name;
   QTextCodec *_codec;
   int _cmap;
   int lbearing, rbearing;
   mutable QFontEngine::FaceId face_id;
   mutable QFreetypeFace *freetype;
   mutable int synth;
};

#ifndef QT_NO_FONTCONFIG

class Q_GUI_EXPORT QFontEngineMultiFT : public QFontEngineMulti
{
 public:
   QFontEngineMultiFT(QFontEngine *fe, FcPattern *firstEnginePattern, FcPattern *p, int s, const QFontDef &request);
   ~QFontEngineMultiFT();

   void loadEngine(int at) override;

 private:
   QFontDef request;
   FcPattern *pattern;
   FcPattern *firstEnginePattern;
   FcFontSet *fontSet;
   int screen;
   int firstFontIndex; // first font in fontset
};

class Q_GUI_EXPORT QFontEngineX11FT : public QFontEngineFT
{
 public:
   explicit QFontEngineX11FT(const QFontDef &fontDef) : QFontEngineFT(fontDef) {}
   explicit QFontEngineX11FT(FcPattern *pattern, const QFontDef &fd, int screen);
   ~QFontEngineX11FT();

   QFontEngine *cloneWithSize(qreal pixelSize) const override;

#ifndef QT_NO_XRENDER
   int xglyph_format;
#endif

 protected:
   bool uploadGlyphToServer(QGlyphSet *set, uint glyphid, Glyph *g, GlyphInfo *info, int glyphDataSize) const override;
   unsigned long allocateServerGlyphSet() override;
   void freeServerGlyphSet(unsigned long id) override;
};

#endif // QT_NO_FONTCONFIG

QT_END_NAMESPACE

#endif // QFONTENGINE_X11_P_H
