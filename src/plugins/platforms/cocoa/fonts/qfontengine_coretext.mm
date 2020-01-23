/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#include <qfontengine_coretext_p.h>

#include <qplatform_fontdatabase.h>
#include <qendian.h>
#include <qsettings.h>
#include <qstring16.h>

#include <qimage_p.h>

#include <cmath>

#if defined(Q_OS_DARWIN)
#import <AppKit/AppKit.h>
#endif

#if defined(Q_OS_IOS)
#import <UIKit/UIKit.h>
#endif

// These are available cross platform, exported as kCTFontWeightXXX from CoreText.framework,
// but they are not documented and are not in public headers so are private API and exposed
// only through the NSFontWeightXXX and UIFontWeightXXX aliases in AppKit and UIKit (rdar://26109857)

#if defined(Q_OS_DARWIN)
#define kCTFontWeightUltraLight NSFontWeightUltraLight
#define kCTFontWeightThin NSFontWeightThin
#define kCTFontWeightLight NSFontWeightLight
#define kCTFontWeightRegular NSFontWeightRegular
#define kCTFontWeightMedium NSFontWeightMedium
#define kCTFontWeightSemibold NSFontWeightSemibold
#define kCTFontWeightBold NSFontWeightBold
#define kCTFontWeightHeavy NSFontWeightHeavy
#define kCTFontWeightBlack NSFontWeightBlack

#elif defined(Q_OS_IOS)
#define kCTFontWeightUltraLight UIFontWeightUltraLight
#define kCTFontWeightThin UIFontWeightThin
#define kCTFontWeightLight UIFontWeightLight
#define kCTFontWeightRegular UIFontWeightRegular
#define kCTFontWeightMedium UIFontWeightMedium
#define kCTFontWeightSemibold UIFontWeightSemibold
#define kCTFontWeightBold UIFontWeightBold
#define kCTFontWeightHeavy UIFontWeightHeavy
#define kCTFontWeightBlack UIFontWeightBlack

#endif

static float SYNTHETIC_ITALIC_SKEW = std::tan(14.f * std::acos(0.f) / 90.f);

bool QCoreTextFontEngine::ct_getSfntTable(void *user_data, uint tag, uchar *buffer, uint *length)
{
   CTFontRef ctfont = *(CTFontRef *)user_data;

   QCFType<CFDataRef> table = CTFontCopyTable(ctfont, tag, 0);
   if (!table) {
      return false;
   }

   CFIndex tableLength = CFDataGetLength(table);
   if (buffer && int(*length) >= tableLength) {
      CFDataGetBytes(table, CFRangeMake(0, tableLength), buffer);
   }
   *length = tableLength;
   Q_ASSERT(int(*length) > 0);

   return true;
}

QFont::Weight QCoreTextFontEngine::qtWeightFromCFWeight(float value)
{
   if (value >= kCTFontWeightBlack) {
      return QFont::Black;
   }

   if (value >= kCTFontWeightHeavy) {
      return QFont::ExtraBold;
   }

   if (value >= kCTFontWeightBold) {
      return QFont::Bold;
   }

   if (value >= kCTFontWeightSemibold) {
      return QFont::DemiBold;
   }

   if (value >= kCTFontWeightMedium) {
      return QFont::Medium;
   }

   if (value == kCTFontWeightRegular) {
      return QFont::Normal;
   }

   if (value <= kCTFontWeightUltraLight) {
      return QFont::Thin;
   }

   if (value <= kCTFontWeightThin) {
      return QFont::ExtraLight;
   }

   if (value <= kCTFontWeightLight) {
      return QFont::Light;
   }

   return QFont::Normal;
}

static void loadAdvancesForGlyphs(CTFontRef ctfont, QVarLengthArray<CGGlyph> &cgGlyphs, QGlyphLayout *glyphs, int len,
   QFontEngine::ShaperFlags flags, const QFontDef &fontDef)
{
   QVarLengthArray<CGSize> advances(len);
   CTFontGetAdvancesForGlyphs(ctfont, kCTFontOrientationHorizontal, cgGlyphs.data(), advances.data(), len);

   for (int i = 0; i < len; ++i) {
      if (glyphs->glyphs[i] & 0xff000000) {
         continue;
      }

      glyphs->advances[i] = QFixed::fromReal(advances[i].width);
   }

   if (fontDef.styleStrategy & QFont::ForceIntegerMetrics) {
      for (int i = 0; i < len; ++i) {
         glyphs->advances[i] = glyphs->advances[i].round();
      }
   }
}

static float getTraitValue(CFDictionaryRef allTraits, CFStringRef trait)
{
   if (CFDictionaryContainsKey(allTraits, trait)) {
      CFNumberRef traitNum = (CFNumberRef) CFDictionaryGetValue(allTraits, trait);
      float v = 0;

      CFNumberGetValue(traitNum, kCFNumberFloatType, &v);
      return v;
   }

   return 0;
}

int QCoreTextFontEngine::antialiasingThreshold = 0;
QFontEngine::GlyphFormat QCoreTextFontEngine::defaultGlyphFormat = QFontEngine::Format_A32;

CGAffineTransform qt_transform_from_fontdef(const QFontDef &fontDef)
{
   CGAffineTransform transform = CGAffineTransformIdentity;
   if (fontDef.stretch != 100) {
      transform = CGAffineTransformMakeScale(float(fontDef.stretch) / float(100), 1);
   }
   return transform;
}

QCoreTextFontEngine::QCoreTextFontEngine(CTFontRef font, const QFontDef &def)
   : QFontEngine(Mac)
{
   fontDef = def;
   transform = qt_transform_from_fontdef(fontDef);
   ctfont = font;
   CFRetain(ctfont);
   cgFont = CTFontCopyGraphicsFont(font, NULL);
   init();
}

QCoreTextFontEngine::QCoreTextFontEngine(CGFontRef font, const QFontDef &def)
   : QFontEngine(Mac)
{
   fontDef   = def;
   transform = qt_transform_from_fontdef(fontDef);
   cgFont    = font;

   // Keep reference count balanced
   CFRetain(cgFont);
   ctfont = CTFontCreateWithGraphicsFont(font, fontDef.pixelSize, &transform, NULL);
   init();
}

QCoreTextFontEngine::~QCoreTextFontEngine()
{
   CFRelease(cgFont);
   CFRelease(ctfont);
}

void QCoreTextFontEngine::init()
{
   Q_ASSERT(ctfont != NULL);
   Q_ASSERT(cgFont != NULL);

   face_id.index    = 0;
   QCFString name   = CTFontCopyName(ctfont, kCTFontUniqueNameKey);
   face_id.filename = QCFString::toQString(name);

   QCFString family = CTFontCopyFamilyName(ctfont);
   fontDef.family = family;

   QCFString styleName = (CFStringRef) CTFontCopyAttribute(ctfont, kCTFontStyleNameAttribute);
   fontDef.styleName = styleName;

   synthesisFlags = 0;
   CTFontSymbolicTraits traits = CTFontGetSymbolicTraits(ctfont);

   if (traits & kCTFontColorGlyphsTrait) {
      glyphFormat = QFontEngine::Format_ARGB;
   } else {
      glyphFormat = defaultGlyphFormat;
   }

   if (traits & kCTFontItalicTrait) {
      fontDef.style = QFont::StyleItalic;
   }

   CFDictionaryRef allTraits = CTFontCopyTraits(ctfont);
   fontDef.weight = QCoreTextFontEngine::qtWeightFromCFWeight(getTraitValue(allTraits, kCTFontWeightTrait));
   int slant = static_cast<int>(getTraitValue(allTraits, kCTFontSlantTrait) * 500 + 500);
   if (slant > 500 && !(traits & kCTFontItalicTrait)) {
      fontDef.style = QFont::StyleOblique;
   }
   CFRelease(allTraits);

   if (fontDef.weight >= QFont::Bold && !(traits & kCTFontBoldTrait)) {
      synthesisFlags |= SynthesizedBold;
   }

   // we probably do not need to synthesis italic for oblique font
   if (fontDef.style != QFont::StyleNormal && !(traits & kCTFontItalicTrait)) {
      synthesisFlags |= SynthesizedItalic;
   }

   avgCharWidth = 0;
   QByteArray os2Table = getSfntTable(MAKE_TAG('O', 'S', '/', '2'));
   unsigned emSize = CTFontGetUnitsPerEm(ctfont);

   if (os2Table.size() >= 10) {
      fsType = qFromBigEndian<quint16>(reinterpret_cast<const uchar *>(os2Table.constData() + 8));

      // qAbs is a workaround for weird fonts like Lucida Grande
      qint16 width = qAbs(qFromBigEndian<qint16>(reinterpret_cast<const uchar *>(os2Table.constData() + 2)));
      avgCharWidth = QFixed::fromReal(width * fontDef.pixelSize / emSize);

   } else {
      avgCharWidth = QFontEngine::averageCharWidth();
   }

   cache_cost = (CTFontGetAscent(ctfont) + CTFontGetDescent(ctfont)) * avgCharWidth.toInt() * 2000;

   // possible issue: hb_coretext requires both CTFont and CGFont but user_data is only void*
   Q_ASSERT((void *)(&ctfont + 1) == (void *)&cgFont);


   // faceData is a data member in QFontEngine
   faceData.user_data           = &ctfont;
   faceData.font_table_func_ptr = ct_getSfntTable;
}

glyph_t QCoreTextFontEngine::glyphIndex(char32_t ch) const
{
   QString16 str = QString16(ch);

   CGGlyph glyphIndices[2];
   CTFontGetGlyphsForCharacters(ctfont, (const UniChar *)str.constData(), glyphIndices, str.size_storage());

   return glyphIndices[0];
}

bool QCoreTextFontEngine::stringToCMap(QStringView strView, QGlyphLayout *glyphs, int *nglyphs, QFontEngine::ShaperFlags flags) const
{
   Q_ASSERT(glyphs->numGlyphs >= *nglyphs);

   QString16 str = QString16(strView.constBegin(), strView.constEnd());
   auto len = str.size_storage();

   if (*nglyphs < len) {
      *nglyphs = len;
      return false;
   }

   QVarLengthArray<CGGlyph> cg_glyphs(len);
   CTFontGetGlyphsForCharacters(ctfont, (const UniChar *)str.constData(), cg_glyphs.data(), len);

   int position = 0;

   for (QString16::size_type i = 0; i < len; ++i) {
      glyphs->glyphs[position] = cg_glyphs[i];

      if (position < i) {
         cg_glyphs[position] = cg_glyphs[i];
      }

      ++position;

      // for a code point outside the BMP, skip the lower part of the surrogate pair
      // and go directly to the next code point without increasing glyph_pos

      const char16_t *ch = str.constData();

      if ( (ch[i] >= 0xD800 && ch[i] <= 0xDBFF) && (i + 1 < len) && (ch[i + 1] >= 0xDC00 && ch[i + 1] <= 0xDFFF) ) {
         // test hi then lo surrogate
         ++i;
      }
   }

   *nglyphs = position;
   glyphs->numGlyphs = position;

   if (flags & GlyphIndicesOnly) {
      return true;
   }

   QVarLengthArray<CGSize> cg_advances(glyphs->numGlyphs);

   CTFontGetAdvancesForGlyphs(ctfont, kCTFontOrientationHorizontal, cg_glyphs.data(), cg_advances.data(), glyphs->numGlyphs);

   for (int i = 0; i < glyphs->numGlyphs; ++i) {
      if (glyphs->glyphs[i] & 0xff000000) {
         continue;
      }

      glyphs->advances[i] = QFixed::fromReal(cg_advances[i].width);
   }

   if (fontDef.styleStrategy & QFont::ForceIntegerMetrics) {
      for (int i = 0; i < glyphs->numGlyphs; ++i) {
         glyphs->advances[i] = glyphs->advances[i].round();
      }
   }

   return true;
}

glyph_metrics_t QCoreTextFontEngine::boundingBox(const QGlyphLayout &glyphs)
{
   QFixed w;
   bool round = fontDef.styleStrategy & QFont::ForceIntegerMetrics;

   for (int i = 0; i < glyphs.numGlyphs; ++i) {
      w += round ? glyphs.effectiveAdvance(i).round() : glyphs.effectiveAdvance(i);
   }

   return glyph_metrics_t(0, -(ascent()), w - lastRightBearing(glyphs, round), ascent() + descent(), w, 0);
}

glyph_metrics_t QCoreTextFontEngine::boundingBox(glyph_t glyph)
{
   glyph_metrics_t ret;
   CGGlyph g = glyph;

   CGRect rect = CTFontGetBoundingRectsForGlyphs(ctfont, kCTFontOrientationHorizontal, &g, 0, 1);

   if (synthesisFlags & QFontEngine::SynthesizedItalic) {
      rect.size.width += rect.size.height * SYNTHETIC_ITALIC_SKEW;
   }

   ret.width = QFixed::fromReal(rect.size.width);
   ret.height = QFixed::fromReal(rect.size.height);
   ret.x = QFixed::fromReal(rect.origin.x);
   ret.y = -QFixed::fromReal(rect.origin.y) - ret.height;
   CGSize advances[1];

   CTFontGetAdvancesForGlyphs(ctfont, kCTFontOrientationHorizontal, &g, advances, 1);
   ret.xoff = QFixed::fromReal(advances[0].width);
   ret.yoff = QFixed::fromReal(advances[0].height);

   if (fontDef.styleStrategy & QFont::ForceIntegerMetrics) {
      ret.xoff = ret.xoff.round();
      ret.yoff = ret.yoff.round();
   }

   return ret;
}

QFixed QCoreTextFontEngine::ascent() const
{
   return (fontDef.styleStrategy & QFont::ForceIntegerMetrics)
      ? QFixed::fromReal(CTFontGetAscent(ctfont)).round()
      : QFixed::fromReal(CTFontGetAscent(ctfont));
}
QFixed QCoreTextFontEngine::descent() const
{
   QFixed d = QFixed::fromReal(CTFontGetDescent(ctfont));
   if (fontDef.styleStrategy & QFont::ForceIntegerMetrics) {
      d = d.round();
   }

   return d;
}

QFixed QCoreTextFontEngine::leading() const
{
   return (fontDef.styleStrategy & QFont::ForceIntegerMetrics)
      ? QFixed::fromReal(CTFontGetLeading(ctfont)).round()
      : QFixed::fromReal(CTFontGetLeading(ctfont));
}

QFixed QCoreTextFontEngine::xHeight() const
{
   return (fontDef.styleStrategy & QFont::ForceIntegerMetrics)
      ? QFixed::fromReal(CTFontGetXHeight(ctfont)).round()
      : QFixed::fromReal(CTFontGetXHeight(ctfont));
}

QFixed QCoreTextFontEngine::averageCharWidth() const
{
   return (fontDef.styleStrategy & QFont::ForceIntegerMetrics)
      ? avgCharWidth.round() : avgCharWidth;
}

qreal QCoreTextFontEngine::maxCharWidth() const
{
   // ### FIXME: 'W' might not be the widest character, but this is better than nothing
   const glyph_t glyph = glyphIndex('W');
   glyph_metrics_t bb = const_cast<QCoreTextFontEngine *>(this)->boundingBox(glyph);
   return bb.xoff.toReal();
}

void QCoreTextFontEngine::draw(CGContextRef ctx, qreal x, qreal y, const QTextItemInt &ti, int paintDeviceHeight)
{
   QVarLengthArray<QFixedPoint> positions;
   QVarLengthArray<glyph_t> glyphs;
   QTransform matrix;

   matrix.translate(x, y);

   getGlyphPositions(ti.glyphs, matrix, ti.flags, glyphs, positions);
   if (glyphs.size() == 0) {
      return;
   }

   CGContextSetFontSize(ctx, fontDef.pixelSize);

   CGAffineTransform oldTextMatrix = CGContextGetTextMatrix(ctx);
   CGAffineTransform cgMatrix      = CGAffineTransformMake(1, 0, 0, -1, 0, -paintDeviceHeight);

   CGAffineTransformConcat(cgMatrix, oldTextMatrix);

   if (synthesisFlags & QFontEngine::SynthesizedItalic) {
      cgMatrix = CGAffineTransformConcat(cgMatrix, CGAffineTransformMake(1, 0, -SYNTHETIC_ITALIC_SKEW, 1, 0, 0));
   }

   cgMatrix = CGAffineTransformConcat(cgMatrix, transform);

   CGContextSetTextMatrix(ctx, cgMatrix);
   CGContextSetTextDrawingMode(ctx, kCGTextFill);

   // referenced Cairo

   QVarLengthArray<CGPoint> cg_positions(glyphs.size());
   QVarLengthArray<CGGlyph> cg_glyphs(glyphs.size());

   QFixedPoint origin = positions[0];

   for (int i = 0; i < glyphs.size(); ++i) {
      QFixedPoint distance = positions[i] - origin;
      cg_positions[i] = CGPointMake(distance.x.toReal(), distance.y.toReal());
      cg_glyphs[i]    = glyphs[i];
   }

   CTFontDrawGlyphs(ctfont, cg_glyphs.data(), cg_positions.data(), glyphs.size(), ctx);

   if (synthesisFlags & QFontEngine::SynthesizedBold) {
      CGContextSetTextPosition(ctx, positions[0].x.toReal() + 0.5 * lineThickness().toReal(), positions[0].y.toReal());
      CTFontDrawGlyphs(ctfont, cg_glyphs.data(), cg_positions.data(), glyphs.size(), ctx);
   }

   CGContextSetTextMatrix(ctx, oldTextMatrix);
}

struct ConvertPathInfo {
   ConvertPathInfo(QPainterPath *newPath, const QPointF &newPos) : path(newPath), pos(newPos) {}
   QPainterPath *path;
   QPointF pos;
};

static void convertCGPathToQPainterPath(void *info, const CGPathElement *element)
{
   ConvertPathInfo *myInfo = static_cast<ConvertPathInfo *>(info);
   switch (element->type) {
      case kCGPathElementMoveToPoint:
         myInfo->path->moveTo(element->points[0].x + myInfo->pos.x(),
            element->points[0].y + myInfo->pos.y());
         break;

      case kCGPathElementAddLineToPoint:
         myInfo->path->lineTo(element->points[0].x + myInfo->pos.x(),
            element->points[0].y + myInfo->pos.y());
         break;

      case kCGPathElementAddQuadCurveToPoint:
         myInfo->path->quadTo(element->points[0].x + myInfo->pos.x(),
            element->points[0].y + myInfo->pos.y(),
            element->points[1].x + myInfo->pos.x(),
            element->points[1].y + myInfo->pos.y());
         break;

      case kCGPathElementAddCurveToPoint:
         myInfo->path->cubicTo(element->points[0].x + myInfo->pos.x(),
            element->points[0].y + myInfo->pos.y(),
            element->points[1].x + myInfo->pos.x(),
            element->points[1].y + myInfo->pos.y(),
            element->points[2].x + myInfo->pos.x(),
            element->points[2].y + myInfo->pos.y());
         break;
      case kCGPathElementCloseSubpath:
         myInfo->path->closeSubpath();
         break;
      default:
         qDebug() << "Unhandled path transform type: " << element->type;
   }

}

void QCoreTextFontEngine::addGlyphsToPath(glyph_t *glyphs, QFixedPoint *positions, int nGlyphs,
   QPainterPath *path, QTextItem::RenderFlags)
{
   if (glyphFormat == QFontEngine::Format_ARGB) {
      return;   // We can't convert color-glyphs to path
   }

   CGAffineTransform cgMatrix = CGAffineTransformIdentity;
   cgMatrix = CGAffineTransformScale(cgMatrix, 1, -1);

   if (synthesisFlags & QFontEngine::SynthesizedItalic) {
      cgMatrix = CGAffineTransformConcat(cgMatrix, CGAffineTransformMake(1, 0, -SYNTHETIC_ITALIC_SKEW, 1, 0, 0));
   }

   for (int i = 0; i < nGlyphs; ++i) {
      QCFType<CGPathRef> cgpath = CTFontCreatePathForGlyph(ctfont, glyphs[i], &cgMatrix);
      ConvertPathInfo info(path, positions[i].toPointF());
      CGPathApply(cgpath, &info, convertCGPathToQPainterPath);
   }
}

static void qcoretextfontengine_scaleMetrics(glyph_metrics_t &br, const QTransform &matrix)
{
   if (matrix.isScaling()) {
      qreal hscale = matrix.m11();
      qreal vscale = matrix.m22();
      br.width  = QFixed::fromReal(br.width.toReal() * hscale);
      br.height = QFixed::fromReal(br.height.toReal() * vscale);
      br.x      = QFixed::fromReal(br.x.toReal() * hscale);
      br.y      = QFixed::fromReal(br.y.toReal() * vscale);
   }
}

glyph_metrics_t QCoreTextFontEngine::alphaMapBoundingBox(glyph_t glyph, QFixed subPixelPosition, const QTransform &matrix,
   GlyphFormat format)
{
   if (matrix.type() > QTransform::TxScale) {
      return QFontEngine::alphaMapBoundingBox(glyph, subPixelPosition, matrix, format);
   }

   glyph_metrics_t br = boundingBox(glyph);
   qcoretextfontengine_scaleMetrics(br, matrix);

   // Normalize width and height
   if (br.width < 0) {
      br.width = -br.width;
   }
   if (br.height < 0) {
      br.height = -br.height;
   }

   if (format == QFontEngine::Format_A8 || format == QFontEngine::Format_A32) {
      // Drawing a glyph at x-position 0 with anti-aliasing enabled
      // will potentially fill the pixel to the left of 0, as the
      // coordinates are not aligned to the center of pixels. To
      // prevent clipping of this pixel we need to shift the glyph
      // in the bitmap one pixel to the right. The shift needs to
      // be reflected in the glyph metrics as well, so that the final
      // position of the glyph is correct, which is why doing the
      // shift in imageForGlyph() is not enough.
      br.x -= 1;

      // As we've shifted the glyph one pixel to the right, we need
      // to expand the width of the alpha map bounding box as well.
      br.width += 1;

      // But we have the same anti-aliasing problem on the right
      // hand side of the glyph, eg. if the width of the glyph
      // results in the bounding rect landing between two pixels.
      // We pad the bounding rect again to account for the possible
      // anti-aliased drawing.
      br.width += 1;

      // We also shift the glyph to right right based on the subpixel
      // position, so we pad the bounding box to take account for the
      // subpixel positions that may result in the glyph being drawn
      // one pixel to the right of the 0-subpixel position.
      br.width += 1;

      // The same same logic as for the x-position needs to be applied
      // to the y-position, except we don't need to compensate for
      // the subpixel positioning.
      br.y -= 1;
      br.height += 2;
   }

   return br;
}

QImage QCoreTextFontEngine::imageForGlyph(glyph_t glyph, QFixed subPixelPosition, bool aa, const QTransform &matrix)
{
   glyph_metrics_t br = alphaMapBoundingBox(glyph, subPixelPosition, matrix, glyphFormat);

   bool isColorGlyph = glyphFormat == QFontEngine::Format_ARGB;
   QImage::Format imageFormat = isColorGlyph ? QImage::Format_ARGB32_Premultiplied : QImage::Format_RGB32;
   QImage im(br.width.ceil().toInt(), br.height.ceil().toInt(), imageFormat);
   im.fill(0);

   if (! im.width() || ! im.height()) {
      return im;
   }

#ifndef Q_OS_IOS
   CGColorSpaceRef colorspace = CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB);
#else
   CGColorSpaceRef colorspace = CGColorSpaceCreateDeviceRGB();
#endif

   uint cgflags = isColorGlyph ? kCGImageAlphaPremultipliedFirst : kCGImageAlphaNoneSkipFirst;

#ifdef kCGBitmapByteOrder32Host          // only needed because CGImage.h added symbols in the minor version
   cgflags |= kCGBitmapByteOrder32Host;
#endif

   CGContextRef ctx = CGBitmapContextCreate(im.bits(), im.width(), im.height(), 8, im.bytesPerLine(), colorspace, cgflags);

   Q_ASSERT(ctx);

   CGContextSetFontSize(ctx, fontDef.pixelSize);
   const bool antialias = (aa || fontDef.pointSize > antialiasingThreshold) && !(fontDef.styleStrategy & QFont::NoAntialias);

   CGContextSetShouldAntialias(ctx, antialias);
   const bool smoothing = antialias && !(fontDef.styleStrategy & QFont::NoSubpixelAntialias);

   CGContextSetShouldSmoothFonts(ctx, smoothing);

   CGAffineTransform cgMatrix = CGAffineTransformIdentity;

   if (synthesisFlags & QFontEngine::SynthesizedItalic) {
      cgMatrix = CGAffineTransformConcat(cgMatrix, CGAffineTransformMake(1, 0, SYNTHETIC_ITALIC_SKEW, 1, 0, 0));
   }

   if (! isColorGlyph) {
      // CTFontDrawGlyphs incorporates the font's matrix already
      cgMatrix = CGAffineTransformConcat(cgMatrix, transform);
   }

   if (matrix.isScaling()) {
      cgMatrix = CGAffineTransformConcat(cgMatrix, CGAffineTransformMakeScale(matrix.m11(), matrix.m22()));
   }

   CGGlyph cg_glyph = glyph;
   qreal pos_x = -br.x.truncate() + subPixelPosition.toReal();
   qreal pos_y = im.height() + br.y.toReal();

   if (! isColorGlyph) {
      CGContextSetTextMatrix(ctx, cgMatrix);
      CGContextSetRGBFillColor(ctx, 1, 1, 1, 1);
      CGContextSetTextDrawingMode(ctx, kCGTextFill);
      CGContextSetFont(ctx, cgFont);
      CGContextSetTextPosition(ctx, pos_x, pos_y);

      CTFontDrawGlyphs(ctfont, &cg_glyph, &CGPointZero, 1, ctx);

      if (synthesisFlags & QFontEngine::SynthesizedBold) {
         CGContextSetTextPosition(ctx, pos_x + 0.5 * lineThickness().toReal(), pos_y);
         CTFontDrawGlyphs(ctfont, &cg_glyph, &CGPointZero, 1, ctx);
      }

   } else {
      // CGContextSetTextMatrix does not work with color glyphs, so we use
      // the CTM instead. This means we must translate the CTM as well, to
      // set the glyph position, instead of using CGContextSetTextPosition.
      CGContextTranslateCTM(ctx, pos_x, pos_y);
      CGContextConcatCTM(ctx, cgMatrix);

      // CGContextShowGlyphsWithAdvances does not support the 'sbix' color-bitmap
      // glyphs in the Apple Color Emoji font, so we use CTFontDrawGlyphs instead.
      CTFontDrawGlyphs(ctfont, &cg_glyph, &CGPointZero, 1, ctx);
   }

   CGContextRelease(ctx);
   CGColorSpaceRelease(colorspace);

   return im;
}

QImage QCoreTextFontEngine::alphaMapForGlyph(glyph_t glyph, QFixed subPixelPosition)
{
   return alphaMapForGlyph(glyph, subPixelPosition, QTransform());
}

QImage QCoreTextFontEngine::alphaMapForGlyph(glyph_t glyph, QFixed subPixelPosition, const QTransform &x)
{
   if (x.type() > QTransform::TxScale) {
      return QFontEngine::alphaMapForGlyph(glyph, subPixelPosition, x);
   }

   QImage im = imageForGlyph(glyph, subPixelPosition, false, x);

   QImage alphaMap(im.width(), im.height(), QImage::Format_Alpha8);

   for (int y = 0; y < im.height(); ++y) {
      uint *src = (uint *) im.scanLine(y);
      uchar *dst = alphaMap.scanLine(y);
      for (int x = 0; x < im.width(); ++x) {
         *dst = qGray(*src);
         ++dst;
         ++src;
      }
   }

   return alphaMap;
}

QImage QCoreTextFontEngine::alphaRGBMapForGlyph(glyph_t glyph, QFixed subPixelPosition, const QTransform &x)
{
   if (x.type() > QTransform::TxScale) {
      return QFontEngine::alphaRGBMapForGlyph(glyph, subPixelPosition, x);
   }

   QImage im = imageForGlyph(glyph, subPixelPosition, true, x);
   qGamma_correct_back_to_linear_cs(&im);
   return im;
}

QImage QCoreTextFontEngine::bitmapForGlyph(glyph_t glyph, QFixed subPixelPosition, const QTransform &t)
{
   if (t.type() > QTransform::TxScale) {
      return QFontEngine::bitmapForGlyph(glyph, subPixelPosition, t);
   }

   return imageForGlyph(glyph, subPixelPosition, true, t);
}

void QCoreTextFontEngine::recalcAdvances(QGlyphLayout *glyphs, QFontEngine::ShaperFlags flags) const
{
   int i, numGlyphs = glyphs->numGlyphs;
   QVarLengthArray<CGGlyph> cgGlyphs(numGlyphs);

   for (i = 0; i < numGlyphs; ++i) {
      if (glyphs->glyphs[i] & 0xff000000) {
         cgGlyphs[i] = 0;
      } else {
         cgGlyphs[i] = glyphs->glyphs[i];
      }
   }

   loadAdvancesForGlyphs(ctfont, cgGlyphs, glyphs, numGlyphs, flags, fontDef);
}

QFontEngine::FaceId QCoreTextFontEngine::faceId() const
{
   return face_id;
}

bool QCoreTextFontEngine::canRender(QStringView strView) const
{
   QString16 string = QString16(strView.begin(), strView.end());
   auto len = string.size_storage();

   QVarLengthArray<CGGlyph> cgGlyphs(len);

   return CTFontGetGlyphsForCharacters(ctfont, (const UniChar *)string.constData(), cgGlyphs.data(), len);
}

bool QCoreTextFontEngine::getSfntTableData(uint tag, uchar *buffer, uint *length) const
{
   return ct_getSfntTable((void *)&ctfont, tag, buffer, length);
}

void QCoreTextFontEngine::getUnscaledGlyph(glyph_t glyph, QPainterPath *path, glyph_metrics_t *metric)
{
   CGAffineTransform cgMatrix = CGAffineTransformIdentity;

   qreal emSquare = CTFontGetUnitsPerEm(ctfont);
   qreal scale = emSquare / CTFontGetSize(ctfont);
   cgMatrix = CGAffineTransformScale(cgMatrix, scale, -scale);

   QCFType<CGPathRef> cgpath = CTFontCreatePathForGlyph(ctfont, (CGGlyph) glyph, &cgMatrix);
   ConvertPathInfo info(path, QPointF(0, 0));
   CGPathApply(cgpath, &info, convertCGPathToQPainterPath);

   *metric = boundingBox(glyph);

   // scale the metrics too
   metric->width  = QFixed::fromReal(metric->width.toReal() * scale);
   metric->height = QFixed::fromReal(metric->height.toReal() * scale);
   metric->x      = QFixed::fromReal(metric->x.toReal() * scale);
   metric->y      = QFixed::fromReal(metric->y.toReal() * scale);
   metric->xoff   = QFixed::fromReal(metric->xoff.toReal() * scale);
   metric->yoff   = QFixed::fromReal(metric->yoff.toReal() * scale);
}

QFixed QCoreTextFontEngine::emSquareSize() const
{
   return QFixed(int(CTFontGetUnitsPerEm(ctfont)));
}

QFontEngine *QCoreTextFontEngine::cloneWithSize(qreal pixelSize) const
{
   QFontDef newFontDef = fontDef;
   newFontDef.pixelSize = pixelSize;
   newFontDef.pointSize = pixelSize * 72.0 / qt_defaultDpi();

   return new QCoreTextFontEngine(cgFont, newFontDef);
}

Qt::HANDLE QCoreTextFontEngine::handle() const
{
   return (Qt::HANDLE)ctfont;
}

bool QCoreTextFontEngine::supportsTransformation(const QTransform &transform) const
{
   if (transform.type() < QTransform::TxScale) {
      return true;
   } else if (transform.type() == QTransform::TxScale &&
      transform.m11() >= 0 && transform.m22() >= 0) {
      return true;
   } else {
      return false;
   }
}

QFontEngine::Properties QCoreTextFontEngine::properties() const
{
   Properties result;

   QCFString psName      = CTFontCopyPostScriptName(ctfont);
   QCFString copyright   = CTFontCopyName(ctfont, kCTFontCopyrightNameKey);

   result.postscriptName = QCFString::toQString(psName);
   result.copyright      = QCFString::toQString(copyright);

   qreal emSquare = CTFontGetUnitsPerEm(ctfont);
   qreal scale = emSquare / CTFontGetSize(ctfont);

   CGRect cgRect = CTFontGetBoundingBox(ctfont);
   result.boundingBox = QRectF(cgRect.origin.x * scale, -CTFontGetAscent(ctfont) * scale,
         cgRect.size.width * scale, cgRect.size.height * scale);

   result.emSquare    = emSquareSize();
   result.ascent      = QFixed::fromReal(CTFontGetAscent(ctfont) * scale);
   result.descent     = QFixed::fromReal(CTFontGetDescent(ctfont) * scale);
   result.leading     = QFixed::fromReal(CTFontGetLeading(ctfont) * scale);
   result.italicAngle = QFixed::fromReal(CTFontGetSlantAngle(ctfont));
   result.capHeight   = QFixed::fromReal(CTFontGetCapHeight(ctfont) * scale);
   result.lineWidth   = QFixed::fromReal(CTFontGetUnderlineThickness(ctfont) * scale);

   if (fontDef.styleStrategy & QFont::ForceIntegerMetrics) {
      result.ascent      = result.ascent.round();
      result.descent     = result.descent.round();
      result.leading     = result.leading.round();
      result.italicAngle = result.italicAngle.round();
      result.capHeight   = result.capHeight.round();
      result.lineWidth   = result.lineWidth.round();
   }

   return result;
}

