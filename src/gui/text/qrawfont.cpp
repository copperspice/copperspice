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

#include <qrawfont_p.h>

#include <qglobal.h>
#include <qendian.h>
#include <qrawfont.h>
#include <qplatform_fontdatabase.h>
#include <qplatform_integration.h>

#include <qapplication_p.h>

QRawFont::QRawFont()
   : m_fontPrivate(std::make_shared<QRawFontPrivate>())
{
}

QRawFont::QRawFont(const QString &fileName, qreal pixelSize, QFont::HintingPreference hintingPreference)
   : m_fontPrivate(std::make_shared<QRawFontPrivate>())
{
   loadFromFile(fileName, pixelSize, hintingPreference);
}

QRawFont::QRawFont(const QByteArray &fontData, qreal pixelSize, QFont::HintingPreference hintingPreference)
   : m_fontPrivate(std::make_shared<QRawFontPrivate>())
{
   loadFromData(fontData, pixelSize, hintingPreference);
}

QRawFont::QRawFont(const QRawFont &other)
{
   m_fontPrivate = other.m_fontPrivate;
}

QRawFont::~QRawFont()
{
}

QRawFont &QRawFont::operator=(const QRawFont &other)
{
   m_fontPrivate = other.m_fontPrivate;
   return *this;
}

bool QRawFont::isValid() const
{
   return m_fontPrivate->isValid();
}

void QRawFont::loadFromFile(const QString &fileName, qreal pixelSize, QFont::HintingPreference hintingPreference)
{
   QFile file(fileName);
   if (file.open(QIODevice::ReadOnly)) {
      loadFromData(file.readAll(), pixelSize, hintingPreference);
   }
}

void QRawFont::loadFromData(const QByteArray &fontData, qreal pixelSize, QFont::HintingPreference hintingPreference)
{
   if (m_fontPrivate.use_count() > 1) {
      // make a copy
      m_fontPrivate = std::make_shared<QRawFontPrivate>(*m_fontPrivate);
   }

   m_fontPrivate->cleanUp();
   m_fontPrivate->hintingPreference = hintingPreference;
   m_fontPrivate->loadFromData(fontData, pixelSize, hintingPreference);
}

QImage QRawFont::alphaMapForGlyph(quint32 glyphIndex, AntialiasingType antialiasingType, const QTransform &transform) const
{
   if (! m_fontPrivate->isValid()) {
      return QImage();
   }

   if (antialiasingType == SubPixelAntialiasing) {
      return m_fontPrivate->fontEngine->alphaRGBMapForGlyph(glyphIndex, QFixed(), transform);
   }

   return m_fontPrivate->fontEngine->alphaMapForGlyph(glyphIndex, QFixed(), transform);
}

QPainterPath QRawFont::pathForGlyph(quint32 glyphIndex) const
{
   if (!m_fontPrivate->isValid()) {
      return QPainterPath();
   }

   QFixedPoint position;
   QPainterPath path;
   m_fontPrivate->fontEngine->addGlyphsToPath(&glyphIndex, &position, 1, &path, Qt::EmptyFlag);
   return path;
}

bool QRawFont::operator==(const QRawFont &other) const
{
   return m_fontPrivate->fontEngine == other.m_fontPrivate->fontEngine;
}

qreal QRawFont::ascent() const
{
   return m_fontPrivate->isValid() ? m_fontPrivate->fontEngine->ascent().toReal() : 0.0;
}

qreal QRawFont::descent() const
{
   return m_fontPrivate->isValid() ? m_fontPrivate->fontEngine->descent().toReal() : 0.0;
}

qreal QRawFont::xHeight() const
{
   return m_fontPrivate->isValid() ? m_fontPrivate->fontEngine->xHeight().toReal() : 0.0;
}

qreal QRawFont::leading() const
{
   return m_fontPrivate->isValid() ? m_fontPrivate->fontEngine->leading().toReal() : 0.0;
}

qreal QRawFont::averageCharWidth() const
{
   return m_fontPrivate->isValid() ? m_fontPrivate->fontEngine->averageCharWidth().toReal() : 0.0;
}

qreal QRawFont::maxCharWidth() const
{
   return m_fontPrivate->isValid() ? m_fontPrivate->fontEngine->maxCharWidth() : 0.0;
}

qreal QRawFont::pixelSize() const
{
   return m_fontPrivate->isValid() ? m_fontPrivate->fontEngine->fontDef.pixelSize : 0.0;
}

qreal QRawFont::unitsPerEm() const
{
   return m_fontPrivate->isValid() ? m_fontPrivate->fontEngine->emSquareSize().toReal() : 0.0;
}

qreal QRawFont::lineThickness() const
{
   return m_fontPrivate->isValid() ? m_fontPrivate->fontEngine->lineThickness().toReal() : 0.0;
}
qreal QRawFont::underlinePosition() const
{
   return m_fontPrivate->isValid() ? m_fontPrivate->fontEngine->underlinePosition().toReal() : 0.0;
}
QString QRawFont::familyName() const
{
   return m_fontPrivate->isValid() ? m_fontPrivate->fontEngine->fontDef.family : QString();
}

QString QRawFont::styleName() const
{
   return m_fontPrivate->isValid() ? m_fontPrivate->fontEngine->fontDef.styleName : QString();
}

QFont::Style QRawFont::style() const
{
   return m_fontPrivate->isValid() ? QFont::Style(m_fontPrivate->fontEngine->fontDef.style) : QFont::StyleNormal;
}

int QRawFont::weight() const
{
   return m_fontPrivate->isValid() ? int(m_fontPrivate->fontEngine->fontDef.weight) : -1;
}

QVector<quint32> QRawFont::glyphIndexesForString(const QString &text) const
{
   QVector<quint32> glyphIndexes;

   if (! m_fontPrivate->isValid() || text.isEmpty()) {
      return glyphIndexes;
   }

   int len = text.length();
   glyphIndexes.resize(len);

   QGlyphLayout glyphs;
   glyphs.numGlyphs = len;
   glyphs.glyphs    = glyphIndexes.data();

   if (! m_fontPrivate->fontEngine->stringToCMap(text, &glyphs, &len, QFontEngine::GlyphIndicesOnly)) {
      // error, may want to error
   }

   glyphIndexes.resize(len);

   return glyphIndexes;
}

bool QRawFont::glyphIndexesForChars(QStringView str, quint32 *glyphIndexes, int *numGlyphs) const
{
   if (! m_fontPrivate->isValid() || str.isEmpty()) {
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

   return m_fontPrivate->fontEngine->stringToCMap(str, &glyphs, numGlyphs, QFontEngine::ShaperFlag::GlyphIndicesOnly);
}

bool QRawFont::advancesForGlyphIndexes(const quint32 *glyphIndexes, QPointF *advances, int numGlyphs, LayoutFlags layoutFlags) const
{
   Q_ASSERT(glyphIndexes && advances);
   if (! m_fontPrivate->isValid() || numGlyphs <= 0) {
      return false;
   }

   QVarLengthArray<QFixed> tmpAdvances(numGlyphs);

   QGlyphLayout glyphs;
   glyphs.glyphs    = const_cast<glyph_t *>(glyphIndexes);
   glyphs.numGlyphs = numGlyphs;
   glyphs.advances = tmpAdvances.data();

   bool design = layoutFlags & UseDesignMetrics;

   m_fontPrivate->fontEngine->recalcAdvances(&glyphs, design ? QFontEngine::DesignMetrics : QFontEngine::ShaperFlag(0));
   if (layoutFlags & KernedAdvances) {
      m_fontPrivate->fontEngine->doKerning(&glyphs, design ? QFontEngine::DesignMetrics : QFontEngine::ShaperFlag(0));
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
   return m_fontPrivate->isValid() ? m_fontPrivate->hintingPreference : QFont::PreferDefaultHinting;
}

QByteArray QRawFont::fontTable(const char *tagName) const
{
   if (!m_fontPrivate->isValid()) {
      return QByteArray();
   }

   return m_fontPrivate->fontEngine->getSfntTable(MAKE_TAG(tagName[0], tagName[1], tagName[2], tagName[3]));
}

QList<QFontDatabase::WritingSystem> QRawFont::supportedWritingSystems() const
{
   QList<QFontDatabase::WritingSystem> writingSystems;

   if (m_fontPrivate->isValid()) {
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
   return m_fontPrivate->isValid() && m_fontPrivate->fontEngine->canRender(make_view(QString(c)));
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
      rawFont.m_fontPrivate->setFontEngine(fe);
      rawFont.m_fontPrivate->hintingPreference = font.hintingPreference();
   }

   return rawFont;
}

void QRawFont::setPixelSize(qreal pixelSize)
{
   if (! m_fontPrivate->isValid() || qFuzzyCompare(m_fontPrivate->fontEngine->fontDef.pixelSize, pixelSize)) {
      return;
   }

   if (m_fontPrivate.use_count() > 1) {
      // make a copy
      m_fontPrivate = std::make_shared<QRawFontPrivate>(*m_fontPrivate);
   }

   m_fontPrivate->setFontEngine(m_fontPrivate->fontEngine->cloneWithSize(pixelSize));
}

void QRawFontPrivate::loadFromData(const QByteArray &fontData, qreal pixelSize, QFont::HintingPreference hintingPreference)
{
   Q_ASSERT(fontEngine == nullptr);

   QPlatformFontDatabase *pfdb = QGuiApplicationPrivate::platformIntegration()->fontDatabase();
   setFontEngine(pfdb->fontEngine(fontData, pixelSize, hintingPreference));
}

QRectF QRawFont::boundingRect(quint32 glyphIndex) const
{
   if (! m_fontPrivate->isValid()) {
      return QRectF();
   }

   glyph_metrics_t gm = m_fontPrivate->fontEngine->boundingBox(glyphIndex);
   return QRectF(gm.x.toReal(), gm.y.toReal(), gm.width.toReal(), gm.height.toReal());
}


