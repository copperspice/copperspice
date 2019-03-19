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

#include <qglobal.h>

#if !defined(QT_NO_RAWFONT)

#include <qrawfont.h>
#include <qrawfont_p.h>

#include <QtCore/qendian.h>

QT_BEGIN_NAMESPACE

QRawFont::QRawFont()
   : d(new QRawFontPrivate)
{
}

QRawFont::QRawFont(const QString &fileName, qreal pixelSize, QFont::HintingPreference hintingPreference)
   : d(new QRawFontPrivate)
{
   loadFromFile(fileName, pixelSize, hintingPreference);
}

/*!
   Constructs a QRawFont representing the font contained in the supplied
   \a fontData for the size (in pixels) given by \a pixelSize, and using the
   hinting preference specified by \a hintingPreference.

   \note The data must contain a TrueType or OpenType font.
*/
QRawFont::QRawFont(const QByteArray &fontData, qreal pixelSize, QFont::HintingPreference hintingPreference)
   : d(new QRawFontPrivate)
{
   loadFromData(fontData, pixelSize, hintingPreference);
}

/*!
   Creates a QRawFont which is a copy of \a other.
*/
QRawFont::QRawFont(const QRawFont &other)
{
   d = other.d;
}

/*!
   Destroys the QRawFont
*/
QRawFont::~QRawFont()
{
}

/*!
  Assigns \a other to this QRawFont.
*/
QRawFont &QRawFont::operator=(const QRawFont &other)
{
   d = other.d;
   return *this;
}

/*!
   Returns true if the QRawFont is valid and false otherwise.
*/
bool QRawFont::isValid() const
{
   return d->isValid();
}

/*!
   Replaces the current QRawFont with the contents of the file referenced
   by \a fileName for the size (in pixels) given by \a pixelSize, and using the
   hinting preference specified by \a hintingPreference.

   The file must reference a TrueType or OpenType font.

   \sa loadFromData()
*/
void QRawFont::loadFromFile(const QString &fileName,
                            qreal pixelSize,
                            QFont::HintingPreference hintingPreference)
{
   QFile file(fileName);
   if (file.open(QIODevice::ReadOnly)) {
      loadFromData(file.readAll(), pixelSize, hintingPreference);
   }
}

/*!
   Replaces the current QRawFont with the font contained in the supplied
   \a fontData for the size (in pixels) given by \a pixelSize, and using the
   hinting preference specified by \a hintingPreference.

   The \a fontData must contain a TrueType or OpenType font.

   \sa loadFromFile()
*/
void QRawFont::loadFromData(const QByteArray &fontData,
                            qreal pixelSize,
                            QFont::HintingPreference hintingPreference)
{
   d.detach();
   d->cleanUp();
   d->hintingPreference = hintingPreference;
   d->thread = QThread::currentThread();
   d->platformLoadFromData(fontData, pixelSize, hintingPreference);
}

/*!
   This function returns a rasterized image of the glyph at the given
   \a glyphIndex in the underlying font, using the \a transform specified.
   If the QRawFont is not valid, this function will return an invalid QImage.

   If \a antialiasingType is set to QRawFont::SubPixelAntialiasing, then the resulting image will be
   in QImage::Format_RGB32 and the RGB values of each pixel will represent the subpixel opacities of
   the pixel in the rasterization of the glyph. Otherwise, the image will be in the format of
   QImage::Format_Indexed8 and each pixel will contain the opacity of the pixel in the
   rasterization.

   \sa pathForGlyph(), QPainter::drawGlyphRun()
*/
QImage QRawFont::alphaMapForGlyph(quint32 glyphIndex, AntialiasingType antialiasingType,
                                  const QTransform &transform) const
{
   if (!d->isValid()) {
      return QImage();
   }

   if (antialiasingType == SubPixelAntialiasing) {
      return d->fontEngine->alphaRGBMapForGlyph(glyphIndex, QFixed(), 0, transform);
   }

   return d->fontEngine->alphaMapForGlyph(glyphIndex, QFixed(), transform);
}

/*!
   This function returns the shape of the glyph at a given \a glyphIndex in the underlying font
   if the QRawFont is valid. Otherwise, it returns an empty QPainterPath.

   The returned glyph will always be unhinted.

   \sa alphaMapForGlyph(), QPainterPath::addText()
*/
QPainterPath QRawFont::pathForGlyph(quint32 glyphIndex) const
{
   if (!d->isValid()) {
      return QPainterPath();
   }

   QFixedPoint position;
   QPainterPath path;
   d->fontEngine->addGlyphsToPath(&glyphIndex, &position, 1, &path, 0);
   return path;
}

/*!
   Returns true if this QRawFont is equal to \a other. Otherwise, returns false.
*/
bool QRawFont::operator==(const QRawFont &other) const
{
   return d->fontEngine == other.d->fontEngine;
}

/*!
    \fn bool QRawFont::operator!=(const QRawFont &other) const

    Returns true if this QRawFont is not equal to \a other. Otherwise, returns false.
*/

/*!
   Returns the ascent of this QRawFont in pixel units.

   \sa QFontMetricsF::ascent()
*/
qreal QRawFont::ascent() const
{
   return d->isValid() ? d->fontEngine->ascent().toReal() : 0.0;
}

/*!
   Returns the descent of this QRawFont in pixel units.

   \sa QFontMetricsF::descent()
*/
qreal QRawFont::descent() const
{
   return d->isValid() ? d->fontEngine->descent().toReal() : 0.0;
}

/*!
   Returns the xHeight of this QRawFont in pixel units.

   \sa QFontMetricsF::xHeight()
*/
qreal QRawFont::xHeight() const
{
   return d->isValid() ? d->fontEngine->xHeight().toReal() : 0.0;
}

/*!
   Returns the leading of this QRawFont in pixel units.

   \sa QFontMetricsF::leading()
*/
qreal QRawFont::leading() const
{
   return d->isValid() ? d->fontEngine->leading().toReal() : 0.0;
}

/*!
   Returns the average character width of this QRawFont in pixel units.

   \sa QFontMetricsF::averageCharWidth()
*/
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
   if (! d->isValid()) {
      return QVector<quint32>();
   }

   int len = text.length();
   QVarLengthGlyphLayoutArray glyphs(len);

   if (!glyphIndexesForChars(text, glyphs.glyphs, &len)) {
      glyphs.resize(len);

      if (! glyphIndexesForChars(text, glyphs.glyphs, &len)) {
         Q_ASSERT_X(false, Q_FUNC_INFO, "stringToCMap should not fail twice");
         return QVector<quint32>();
      }
   }

   QVector<quint32> glyphIndexes;

   for (int i = 0; i < len; ++i) {
      glyphIndexes.append(glyphs.glyphs[i]);
   }

   return glyphIndexes;
}

bool QRawFont::glyphIndexesForChars(QStringView str, quint32 *glyphIndexes, int *numGlyphs) const
{
   if (! d->isValid()) {
      return false;
   }

   QGlyphLayout glyphs;
   glyphs.glyphs = glyphIndexes;

   return d->fontEngine->stringToCMap(str, &glyphs, numGlyphs, QTextEngine::GlyphIndicesOnly);
}

QVector<QPointF> QRawFont::advancesForGlyphIndexes(const QVector<quint32> &glyphIndexes) const
{
   if (!d->isValid()) {
      return QVector<QPointF>();
   }

   int numGlyphs = glyphIndexes.size();
   QVarLengthGlyphLayoutArray glyphs(numGlyphs);
   memcpy(glyphs.glyphs, glyphIndexes.data(), numGlyphs * sizeof(quint32));

   d->fontEngine->recalcAdvances(&glyphs, 0);

   QVector<QPointF> advances;

   for (int i = 0; i < numGlyphs; ++i) {
      advances.append(QPointF(glyphs.advances_x[i].toReal(), glyphs.advances_y[i].toReal()));
   }

   return advances;
}

/*!
   Returns the QRawFont's advances for each of the \a glyphIndexes in pixel units. The advances
   give the distance from the position of a given glyph to where the next glyph should be drawn
   to make it appear as if the two glyphs are unspaced. The glyph indexes are given with the
   array \a glyphIndexes while the results are returned through \a advances, both of them must
   have \a numGlyphs elements.

   \sa QTextLine::horizontalAdvance(), QFontMetricsF::width()
*/
bool QRawFont::advancesForGlyphIndexes(const quint32 *glyphIndexes, QPointF *advances, int numGlyphs) const
{
   if (!d->isValid()) {
      return false;
   }

   QGlyphLayout glyphs;
   glyphs.glyphs = const_cast<HB_Glyph *>(glyphIndexes);
   glyphs.numGlyphs = numGlyphs;
   QVarLengthArray<QFixed> advances_x(numGlyphs);
   QVarLengthArray<QFixed> advances_y(numGlyphs);
   glyphs.advances_x = advances_x.data();
   glyphs.advances_y = advances_y.data();

   d->fontEngine->recalcAdvances(&glyphs, 0);

   for (int i = 0; i < numGlyphs; ++i) {
      advances[i] = QPointF(glyphs.advances_x[i].toReal(), glyphs.advances_y[i].toReal());
   }

   return true;
}

/*!
   Returns the hinting preference used to construct this QRawFont.

   \sa QFont::hintingPreference()
*/
QFont::HintingPreference QRawFont::hintingPreference() const
{
   return d->isValid() ? d->hintingPreference : QFont::PreferDefaultHinting;
}

/*!
   Retrieves the sfnt table named \a tagName from the underlying physical font, or an empty
   byte array if no such table was found. The returned font table's byte order is Big Endian, like
   the sfnt format specifies. The \a tagName must be four characters long and should be formatted
   in the default endianness of the current platform.
*/
QByteArray QRawFont::fontTable(const char *tagName) const
{
   if (!d->isValid()) {
      return QByteArray();
   }

   const quint32 *tagId = reinterpret_cast<const quint32 *>(tagName);
   return d->fontEngine->getSfntTable(qToBigEndian(*tagId));
}

// From qfontdatabase.cpp
extern QList<QFontDatabase::WritingSystem> qt_determine_writing_systems_from_truetype_bits(quint32 unicodeRange[4],
      quint32 codePageRange[2]);

/*!
   Returns a list of writing systems supported by the font according to designer supplied
   information in the font file. Please note that this does not guarantee support for a
   specific unicode point in the font. You can use the supportsCharacter() to check support
   for a single, specific character.

   \note The list is determined based on the unicode ranges and codepage ranges set in the font's
   OS/2 table and requires such a table to be present in the underlying font file.

   \sa supportsCharacter()
*/
QList<QFontDatabase::WritingSystem> QRawFont::supportedWritingSystems() const
{
   if (d->isValid()) {
      QByteArray os2Table = fontTable("OS/2");
      if (os2Table.size() > 86) {
         char *data = os2Table.data();
         quint32 *bigEndianUnicodeRanges = reinterpret_cast<quint32 *>(data + 42);
         quint32 *bigEndianCodepageRanges = reinterpret_cast<quint32 *>(data + 78);

         quint32 unicodeRanges[4];
         quint32 codepageRanges[2];

         for (int i = 0; i < 4; ++i) {
            if (i < 2) {
               codepageRanges[i] = qFromBigEndian(bigEndianCodepageRanges[i]);
            }
            unicodeRanges[i] = qFromBigEndian(bigEndianUnicodeRanges[i]);
         }

         return qt_determine_writing_systems_from_truetype_bits(unicodeRanges, codepageRanges);
      }
   }

   return QList<QFontDatabase::WritingSystem>();
}

bool QRawFont::supportsCharacter(QChar character) const
{
   return d->isValid() && d->fontEngine->canRender(QString(character));
}

bool QRawFont::supportsCharacter(quint32 ucs4) const
{
   QChar c = QChar(char32_t(ucs4));
   return d->isValid() && d->fontEngine->canRender(QString(c));
}

// qfontdatabase.cpp
extern int qt_script_for_writing_system(QFontDatabase::WritingSystem writingSystem);


QRawFont QRawFont::fromFont(const QFont &font, QFontDatabase::WritingSystem writingSystem)
{
   QRawFont rawFont;

#if defined(Q_OS_MAC)
   QTextLayout layout(QFontDatabase::writingSystemSample(writingSystem), font);
   layout.beginLayout();

   QTextLine line = layout.createLine();
   layout.endLayout();

   QList<QGlyphRun> list = layout.glyphRuns();

   if (list.size()) {
      // Pick the one matches the family name we originally requested,
      // if none of them match, just pick the first one
      for (int i = 0; i < list.size(); i++) {
         rawFont = list.at(i).rawFont();
         if (rawFont.familyName() == font.family()) {
            return rawFont;
         }
      }
      return list.at(0).rawFont();
   }
#else
   QFontPrivate *font_d = QFontPrivate::get(font);
   int script = qt_script_for_writing_system(writingSystem);
   QFontEngine *fe = font_d->engineForScript(script);

   if (fe != 0 && fe->type() == QFontEngine::Multi) {
      QFontEngineMulti *multiEngine = static_cast<QFontEngineMulti *>(fe);
      fe = multiEngine->engine(0);
      if (fe == 0) {
         multiEngine->loadEngine(0);
         fe = multiEngine->engine(0);
      }
   }

   if (fe != 0) {
      rawFont.d.data()->fontEngine = fe;
      rawFont.d.data()->fontEngine->ref.ref();
      rawFont.d.data()->hintingPreference = font.hintingPreference();
   }
#endif
   return rawFont;
}

/*!
   Sets the pixel size with which this font should be rendered to \a pixelSize.
*/
void QRawFont::setPixelSize(qreal pixelSize)
{
   if (d->fontEngine == 0) {
      return;
   }

   d.detach();
   QFontEngine *oldFontEngine = d->fontEngine;

   d->fontEngine = d->fontEngine->cloneWithSize(pixelSize);
   if (d->fontEngine != 0) {
      d->fontEngine->ref.ref();
   }

   if (!oldFontEngine->ref.deref()) {
      delete oldFontEngine;
   }
}

/*!
    \internal
*/
void QRawFontPrivate::cleanUp()
{
   platformCleanUp();
   if (fontEngine != 0 && !fontEngine->ref.deref()) {
      delete fontEngine;
   }
   fontEngine = 0;

   hintingPreference = QFont::PreferDefaultHinting;
}

QT_END_NAMESPACE

#endif // QT_NO_RAWFONT
