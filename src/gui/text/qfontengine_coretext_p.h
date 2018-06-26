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

#ifndef QFONTENGINE_CORETEXT_P_H
#define QFONTENGINE_CORETEXT_P_H

#include <qfontengine_p.h>

#if defined(Q_OS_IOS)
#include <CoreText/CoreText.h>
#include <CoreGraphics/CoreGraphics.h>
#include <qcore_mac_p.h>
#endif

QT_BEGIN_NAMESPACE

class QRawFontPrivate;
class QCoreTextFontEngineMulti;

class QCoreTextFontEngine : public QFontEngine
{
 public:
   QCoreTextFontEngine(CTFontRef font, const QFontDef &def);
   QCoreTextFontEngine(CGFontRef font, const QFontDef &def);
   ~QCoreTextFontEngine();

   bool stringToCMap(QStringView str, QGlyphLayout *glyphs, int *nglyphs, QTextEngine::ShaperFlags flags) const override;
   void recalcAdvances(QGlyphLayout *, QTextEngine::ShaperFlags) const override;

   glyph_metrics_t boundingBox(const QGlyphLayout &glyphs) override;
   glyph_metrics_t boundingBox(glyph_t glyph) override;

   QFixed ascent() const override;
   QFixed descent() const override;
   QFixed leading() const override;
   QFixed xHeight() const override;
   qreal maxCharWidth() const override;
   QFixed averageCharWidth() const override;

   void addGlyphsToPath(glyph_t *glyphs, QFixedPoint *positions, int numGlyphs, QPainterPath *path, QTextItem::RenderFlags) override;
   bool canRender(QStringView str) override;

   const QString &fontEngineName() const override {
      static QString retval("QCoreTextFontEngine");
      return retval;
   }

   Type type() const override {
      return QFontEngine::Mac;
   }

   int synthesized() const override {
      return synthesisFlags;
   }

   bool supportsSubPixelPositions() const override {
      return true;
   }

   void draw(CGContextRef ctx, qreal x, qreal y, const QTextItemInt &ti, int paintDeviceHeight);

   FaceId faceId() const override;
   bool getSfntTableData(uint /*tag*/, uchar * /*buffer*/, uint * /*length*/) const override;
   void getUnscaledGlyph(glyph_t glyph, QPainterPath *path, glyph_metrics_t *metrics) override;
   QImage alphaMapForGlyph(glyph_t, QFixed subPixelPosition) override;
   QImage alphaRGBMapForGlyph(glyph_t, QFixed subPixelPosition, int margin, const QTransform &t) override;
   qreal minRightBearing() const override;
   qreal minLeftBearing() const override;
   QFixed emSquareSize() const override;

   QFontEngine *cloneWithSize(qreal pixelSize) const override;

   int glyphMargin(QFontEngineGlyphCache::Type type) {
      Q_UNUSED(type);
      return 0;
   }

   static bool supportsColorGlyphs() {

#if defined(Q_OS_IOS)
      return true;

#elif defined(Q_OS_MAC)
      return true;

#else
      return false;

#endif
   }

   static int antialiasingThreshold;

   virtual QFontEngine::Properties properties() const override;

 private:
   friend class QRawFontPrivate;

   void init();
   QImage imageForGlyph(glyph_t glyph, QFixed subPixelPosition, int margin, bool colorful);
   CTFontRef ctfont;
   CGFontRef cgFont;
   int synthesisFlags;
   CGAffineTransform transform;
   QFixed avgCharWidth;
   friend class QCoreTextFontEngineMulti;
};

class QCoreTextFontEngineMulti : public QFontEngineMulti
{
 public:
   QCoreTextFontEngineMulti(const QCFString &name, const QFontDef &fontDef, bool kerning);
   QCoreTextFontEngineMulti(CTFontRef ctFontRef, const QFontDef &fontDef, bool kerning);
   ~QCoreTextFontEngineMulti();

   bool stringToCMap(QStringView str, QGlyphLayout *glyphs, int *nglyphs, QTextEngine::ShaperFlags flags) const override;

   bool stringToCMap(QStringView str, QGlyphLayout *glyphs, int *nglyphs, QTextEngine::ShaperFlags flags,
                     unsigned short *logClusters, const HB_CharAttributes *charAttributes, QScriptItem *si) const;

   const QString &fontEngineName() const override {
      static QString retval("CoreText");
      return retval;
   }

   inline CTFontRef macFontID() const {
      return ctfont;
   }

 protected:
   void loadEngine(int at) override;

 private:
   void init(bool kerning);
   inline const QCoreTextFontEngine *engineAt(int i) const {
      return static_cast<const QCoreTextFontEngine *>(engines.at(i));
   }

   uint fontIndexForFont(CTFontRef font) const;
   CTFontRef ctfont;
   mutable QCFType<CFMutableDictionaryRef> attributeDict;
   CGAffineTransform transform;
   friend class QFontDialogPrivate;
   bool transformAdvances;
};

CGAffineTransform qt_transform_from_fontdef(const QFontDef &fontDef);

QT_END_NAMESPACE

#endif // QFONTENGINE_CORETEXT_P_H
