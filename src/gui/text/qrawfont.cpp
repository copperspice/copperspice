/***********************************************************************
*
* Copyright (c) 2012-2021 Barbara Geller
* Copyright (c) 2012-2021 Ansel Sermersheim
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

#include <qglobal.h>

#include <qendian.h>
#include <qrawfont.h>
#include <qplatform_fontdatabase.h>

#include <qapplication_p.h>
#include <qplatform_integration.h>

#include <qrawfont_p.h>

QRawFont::QRawFont()
   : d(new QRawFontPrivate)
{
}

QRawFont::QRawFont(const QString &fileName, qreal pixelSize, QFont::HintingPreference hintingPreference)
   : d(new QRawFontPrivate)
{
   loadFromFile(fileName, pixelSize, hintingPreference);
}

QRawFont::QRawFont(const QByteArray &fontData, qreal pixelSize, QFont::HintingPreference hintingPreference)
   : d(new QRawFontPrivate)
{
   loadFromData(fontData, pixelSize, hintingPreference);
}

QRawFont::QRawFont(const QRawFont &other)
{
   d = other.d;
}

QRawFont::~QRawFont()
{
}

QRawFont &QRawFont::operator=(const QRawFont &other)
{
   d = other.d;
   return *this;
}

bool QRawFont::isValid() const
{
   return d->isValid();
}

void QRawFont::loadFromFile(const QString &fileName, qreal pixelSize,
   QFont::HintingPreference hintingPreference)
{
   QFile file(fileName);
   if (file.open(QIODevice::ReadOnly)) {
      loadFromData(file.readAll(), pixelSize, hintingPreference);
   }
}

void QRawFont::loadFromData(const QByteArray &fontData, qreal pixelSize, QFont::HintingPreference hintingPreference)
{
   if (d.use_count() > 1) {
      // make a copy
      d = std::make_shared<QRawFontPrivate>(*d);
   }
   d->cleanUp();
   d->hintingPreference = hintingPreference;
   d->loadFromData(fontData, pixelSize, hintingPreference);
}

QImage QRawFont::alphaMapForGlyph(quint32 glyphIndex, AntialiasingType antialiasingType, const QTransform &transform) const
{
   if (!d->isValid()) {
      return QImage();
   }

   if (antialiasingType == SubPixelAntialiasing) {
      return d->fontEngine->alphaRGBMapForGlyph(glyphIndex, QFixed(), transform);
   }

   return d->fontEngine->alphaMapForGlyph(glyphIndex, QFixed(), transform);
}

QPainterPath QRawFont::pathForGlyph(quint32 glyphIndex) const
{
   if (!d->isValid()) {
      return QPainterPath();
   }

   QFixedPoint position;
   QPainterPath path;
   d->fontEngine->addGlyphsToPath(&glyphIndex, &position, 1, &path, Qt::EmptyFlag);
   return path;
}

bool QRawFont::operator==(const QRawFont &other) const
{
   return d->fontEngine == other.d->fontEngine;
}

qreal QRawFont::ascent() const
{
   return d->isValid() ? d->fontEngine->ascent().toReal() : 0.0;
}

qreal QRawFont::descent() const
{
   return d->isValid() ? d->fontEngine->descent().toReal() : 0.0;
}

qreal QRawFont::xHeight() const
{
   return d->isValid() ? d->fontEngine->xHeight().toReal() : 0.0;
}

qreal QRawFont::leading() const
{
   return d->isValid() ? d->fontEngine->leading().toReal() : 0.0;
}

qreal QRawFont::averageCharWidth() const
{
   return d->isValid() ? d->fontEngine->averageCharWidth().toReal() : 0.0;
}

qreal QRawFont::maxCharWidth() const
{
   return d->isValid() ? d->fontEngine->maxCharWidth() : 0.0;
}

qreal QRawFont::pixelSize() const
{
   return d->isValid() ? d->fontEngine->fontDef.pixelSize : 0.0;
}

qreal QRawFont::unitsPerEm() const
{
   return d->isValid() ? d->fontEngine->emSquareSize().toReal() : 0.0;
}

qreal QRawFont::lineThickness() const
{
   return d->isValid() ? d->fontEngine->lineThickness().toReal() : 0.0;
}
qreal QRawFont::underlinePosition() const
{
   return d->isValid() ? d->fontEngine->underlinePosition().toReal() : 0.0;
}
QString QRawFont::familyName() const
{
   return d->isValid() ? d->fontEngine->fontDef.family : QString();
}

QString QRawFont::styleName() const
{
   return d->isValid() ? d->fontEngine->fontDef.styleName : QString();
}

QFont::Style QRawFont::style() const
{
   return d->isValid() ? QFont::Style(d->fontEngine->fontDef.style) : QFont::StyleNormal;
}

int QRawFont::weight() const
{
   return d->isValid() ? int(d->fontEngine->fontDef.weight) : -1;
}

QVector<quint32> QRawFont::glyphIndexesForString(const QString &text) const
{
   QVector<quint32> glyphIndexes;

   if (! d->isValid() || text.isEmpty()) {
      return glyphIndexes;
   }

   int len = text.length();
   glyphIndexes.resize(len);

   QGlyphLayout glyphs;
   glyphs.numGlyphs = len;
   glyphs.glyphs    = glyphIndexes.data();

   if (! d->fontEngine->stringToCMap(text, &glyphs, &len, QFontEngine::GlyphIndicesOnly)) {
      // error, may want to error
   }

   glyphIndexes.resize(len);

   return glyphIndexes;
}

bool QRawFont::glyphIndexesForChars(QStringView str, quint32 *glyphIndexes, int *numGlyphs) const
{
   if (! d->isValid() || str.isEmpty()) {
      *numGlyphs = 0;
      return false;
   }

   if (*numGlyphs <= 0 || ! glyphIndexes) {
      *numGlyphs = str.length();
      return false;
   }
   QGlyphLayout glyphs;
   glyphs.numGlyphs = *numGlyphs;
   glyphs.glyphs    = glyphIndexes;

   return d->fontEngine->stringToCMap(str, &glyphs, numGlyphs, QFontEngine::ShaperFlag::GlyphIndicesOnly);
}

bool QRawFont::advancesForGlyphIndexes(const quint32 *glyphIndexes, QPointF *advances, int numGlyphs, LayoutFlags layoutFlags) const
{
   Q_ASSERT(glyphIndexes && advances);
   if (! d->isValid() || numGlyphs <= 0) {
      return false;
   }

   QVarLengthArray<QFixed> tmpAdvances(numGlyphs);

   QGlyphLayout glyphs;
   glyphs.glyphs    = const_cast<glyph_t *>(glyphIndexes);
   glyphs.numGlyphs = numGlyphs;
   glyphs.advances = tmpAdvances.data();

   bool design = layoutFlags & UseDesignMetrics;

   d->fontEngine->recalcAdvances(&glyphs, design ? QFontEngine::DesignMetrics : QFontEngine::ShaperFlag(0));
   if (layoutFlags & KernedAdvances) {
      d->fontEngine->doKerning(&glyphs, design ? QFontEngine::DesignMetrics : QFontEngine::ShaperFlag(0));
   }

   for (int i = 0; i < numGlyphs; ++i) {
      advances[i] = QPointF(tmpAdvances[i].toReal(), 0.0);
   }

   return true;
}


bool QRawFont::advancesForGlyphIndexes(const quint32 *glyphIndexes, QPointF *advances, int numGlyphs) const
{
   return QRawFont::advancesForGlyphIndexes(glyphIndexes, advances, numGlyphs, SeparateAdvances);
}

QFont::HintingPreference QRawFont::hintingPreference() const
{
   return d->isValid() ? d->hintingPreference : QFont::PreferDefaultHinting;
}

QByteArray QRawFont::fontTable(const char *tagName) const
{
   if (!d->isValid()) {
      return QByteArray();
   }

   return d->fontEngine->getSfntTable(MAKE_TAG(tagName[0], tagName[1], tagName[2], tagName[3]));
}

QList<QFontDatabase::WritingSystem> QRawFont::supportedWritingSystems() const
{
   QList<QFontDatabase::WritingSystem> writingSystems;

   if (d->isValid()) {
      QByteArray os2Table = fontTable("OS/2");

      if (os2Table.size() > 86) {
         const uchar *const data = reinterpret_cast<const uchar *>(os2Table.constData());
         const uchar *const bigEndianUnicodeRanges  = data + 42;
         const uchar *const bigEndianCodepageRanges = data + 78;

         quint32 unicodeRanges[4];
         quint32 codepageRanges[2];

         for (size_t i = 0; i < sizeof unicodeRanges / sizeof * unicodeRanges; ++i) {
            unicodeRanges[i] = qFromBigEndian<quint32>(bigEndianUnicodeRanges + i * sizeof(quint32));
         }

         for (size_t i = 0; i < sizeof codepageRanges / sizeof * codepageRanges; ++i) {
            codepageRanges[i] = qFromBigEndian<quint32>(bigEndianCodepageRanges + i * sizeof(quint32));
         }

         QSupportedWritingSystems ws = QPlatformFontDatabase::writingSystemsFromTrueTypeBits(unicodeRanges, codepageRanges);

         for (int i = 0; i < QFontDatabase::WritingSystemsCount; ++i) {
            if (ws.supported(QFontDatabase::WritingSystem(i))) {
               writingSystems.append(QFontDatabase::WritingSystem(i));
            }
         }
      }
   }

   return writingSystems;
}

bool QRawFont::supportsCharacter(QChar character) const
{
   return supportsCharacter(character.unicode());
}

bool QRawFont::supportsCharacter(quint32 ucs4) const
{
   QChar c = QChar(char32_t(ucs4));
   return d->isValid() && d->fontEngine->canRender(make_view(QString(c)));
}

// qfontdatabase.cpp
extern int qt_script_for_writing_system(QFontDatabase::WritingSystem writingSystem);


QRawFont QRawFont::fromFont(const QFont &font, QFontDatabase::WritingSystem writingSystem)
{
   QRawFont rawFont;

   QFontPrivate *font_d = QFontPrivate::get(font);
   int script = qt_script_for_writing_system(writingSystem);
   QFontEngine *fe = font_d->engineForScript(script);

   if (fe != nullptr && fe->type() == QFontEngine::Multi) {
      QFontEngineMulti *multiEngine = static_cast<QFontEngineMulti *>(fe);
      fe = multiEngine->engine(0);
      Q_ASSERT(fe);
   }

   if (fe != nullptr) {
      rawFont.d->setFontEngine(fe);
      rawFont.d->hintingPreference = font.hintingPreference();
   }

   return rawFont;
}

void QRawFont::setPixelSize(qreal pixelSize)
{
   if (! d->isValid() || qFuzzyCompare(d->fontEngine->fontDef.pixelSize, pixelSize)) {
      return;
   }

   if (d.use_count() > 1) {
      // make a copy
      d = std::make_shared<QRawFontPrivate>(*d);
   }

   d->setFontEngine(d->fontEngine->cloneWithSize(pixelSize));
}

void QRawFontPrivate::loadFromData(const QByteArray &fontData, qreal pixelSize, QFont::HintingPreference hintingPreference)
{
   Q_ASSERT(fontEngine == nullptr);

   QPlatformFontDatabase *pfdb = QGuiApplicationPrivate::platformIntegration()->fontDatabase();
   setFontEngine(pfdb->fontEngine(fontData, pixelSize, hintingPreference));
}

QRectF QRawFont::boundingRect(quint32 glyphIndex) const
{
   if (!d->isValid()) {
      return QRectF();
   }

   glyph_metrics_t gm = d->fontEngine->boundingBox(glyphIndex);
   return QRectF(gm.x.toReal(), gm.y.toReal(), gm.width.toReal(), gm.height.toReal());
}


