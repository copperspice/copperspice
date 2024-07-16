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

#include <qplatform_fontdatabase.h>

#include <qfontengine_p.h>
#include <qstandardpaths.h>
#include <qapplication.h>
#include <qscreen.h>
#include <qplatform_screen.h>
#include <qlibraryinfo.h>
#include <qdir.h>

#include <qfontengine_qpf2_p.h>

#include <algorithm>
#include <iterator>

void qt_registerFont(const QString &familyname, const QString &stylename,
   const QString &foundryname, int weight,
   QFont::Style style, int stretch, bool antialiased,
   bool scalable, int pixelSize, bool fixedPitch,
   const QSupportedWritingSystems &writingSystems, void *hanlde);

void qt_registerFontFamily(const QString &familyName);
void qt_registerAliasToFontFamily(const QString &familyName, const QString &alias);

void QPlatformFontDatabase::registerQPF2Font(const QByteArray &dataArray, void *handle)
{
   if (dataArray.size() == 0) {
      return;
   }

   const uchar *data = reinterpret_cast<const uchar *>(dataArray.constData());

   if (QFontEngineQPF2::verifyHeader(data, dataArray.size())) {
      QString fontName = QFontEngineQPF2::extractHeaderField(data, QFontEngineQPF2::Tag_FontName).toString();
      int pixelSize    = QFontEngineQPF2::extractHeaderField(data, QFontEngineQPF2::Tag_PixelSize).toInt();

      QVariant weight  = QFontEngineQPF2::extractHeaderField(data, QFontEngineQPF2::Tag_Weight);
      QVariant style   = QFontEngineQPF2::extractHeaderField(data, QFontEngineQPF2::Tag_Style);

      QByteArray writingSystemBits = QFontEngineQPF2::extractHeaderField(data,
            QFontEngineQPF2::Tag_WritingSystems).toByteArray();

      if (!fontName.isEmpty() && pixelSize) {
         QFont::Weight fontWeight = QFont::Normal;
         if (weight.type() == QVariant::Int || weight.type() == QVariant::UInt) {
            fontWeight = QFont::Weight(weight.toInt());
         }

         QFont::Style fontStyle = static_cast<QFont::Style>(style.toInt());

         QSupportedWritingSystems writingSystems;
         for (int i = 0; i < writingSystemBits.count(); ++i) {
            uchar currentByte = writingSystemBits.at(i);
            for (int j = 0; j < 8; ++j) {
               if (currentByte & 1) {
                  writingSystems.setSupported(QFontDatabase::WritingSystem(i * 8 + j));
               }
               currentByte >>= 1;
            }
         }
         QFont::Stretch stretch = QFont::Unstretched;
         registerFont(fontName, QString(), QString(), fontWeight, fontStyle, stretch, true, false,
            pixelSize, false, writingSystems, handle);
      }

   } else {
#if defined(CS_SHOW_DEBUG_GUI)
      qDebug() << "QPlatformFontDatabase::registerQPF2Font() Header verification of QPF2 font failed, it might be corrupt";
#endif
   }
}

void QPlatformFontDatabase::registerFont(const QString &familyname, const QString &stylename,
   const QString &foundryname, QFont::Weight weight, QFont::Style style, QFont::Stretch stretch,
   bool antialiased, bool scalable, int pixelSize, bool fixedPitch,
   const QSupportedWritingSystems &writingSystems, void *usrPtr)
{
   if (scalable) {
      pixelSize = 0;
   }

   qt_registerFont(familyname, stylename, foundryname, weight, style,
      stretch, antialiased, scalable, pixelSize,
      fixedPitch, writingSystems, usrPtr);
}

void QPlatformFontDatabase::registerFontFamily(const QString &familyName)
{
   qt_registerFontFamily(familyName);
}

class QWritingSystemsPrivate
{
 public:
   QWritingSystemsPrivate()
      : ref(1), vector(QFontDatabase::WritingSystemsCount, false) {
   }

   QWritingSystemsPrivate(const QWritingSystemsPrivate *other)
      : ref(1), vector(other->vector) {
   }

   QAtomicInt ref;
   QVector<bool> vector;
};

/*!
    Constructs a new object to handle supported writing systems.
*/
QSupportedWritingSystems::QSupportedWritingSystems()
{
   d = new QWritingSystemsPrivate;
}

/*!
    Constructs a copy of the \a other writing systems object.
*/
QSupportedWritingSystems::QSupportedWritingSystems(const QSupportedWritingSystems &other)
{
   d = other.d;
   d->ref.ref();
}

/*!
    Constructs a copy of the \a other writing systems object.
*/
QSupportedWritingSystems &QSupportedWritingSystems::operator=(const QSupportedWritingSystems &other)
{
   if (d != other.d) {
      other.d->ref.ref();
      if (!d->ref.deref()) {
         delete d;
      }
      d = other.d;
   }
   return *this;
}

/*!
    Destroys the supported writing systems object.
*/
QSupportedWritingSystems::~QSupportedWritingSystems()
{
   if (!d->ref.deref()) {
      delete d;
   }
}

/*!
    \internal
*/
void QSupportedWritingSystems::detach()
{
   if (d->ref.load() != 1) {
      QWritingSystemsPrivate *newd = new QWritingSystemsPrivate(d);
      if (!d->ref.deref()) {
         delete d;
      }
      d = newd;
   }
}

/*!
    Sets or clears support for the specified \a writingSystem based on the
    value given by \a support.
*/
void QSupportedWritingSystems::setSupported(QFontDatabase::WritingSystem writingSystem, bool support)
{
   detach();
   d->vector[writingSystem] = support;
}

/*!
    Returns \c true if the writing system specified by \a writingSystem is
    supported; otherwise returns \c false.
*/
bool QSupportedWritingSystems::supported(QFontDatabase::WritingSystem writingSystem) const
{
   return d->vector.at(writingSystem);
}

/*!
    \internal
 */
QPlatformFontDatabase::~QPlatformFontDatabase()
{
}

void QPlatformFontDatabase::populateFontDatabase()
{
   QString fontpath = fontDir();

   if (! QFile::exists(fontpath)) {
      qWarning("QFontDatabase::populateFontDatabase() Unable to find font directory '%s'", csPrintable(QDir::toNativeSeparators(fontpath)));
      return;
   }

   QDir dir(fontpath);
   dir.setNameFilters(QStringList() << "*.qpf2");
   dir.refresh();

   for (int i = 0; i < int(dir.count()); ++i) {
      const QByteArray fileName = QFile::encodeName(dir.absoluteFilePath(dir[i]));
      QFile file(QString::fromUtf8(fileName));

      if (file.open(QFile::ReadOnly)) {
         const QByteArray fileData = file.readAll();

         QByteArray *fileDataPtr = new QByteArray(fileData);
         registerQPF2Font(fileData, fileDataPtr);
      }
   }
}

void QPlatformFontDatabase::populateFamily(const QString &familyName)
{
   (void) familyName;
}

void QPlatformFontDatabase::invalidate()
{
}

QFontEngineMulti *QPlatformFontDatabase::fontEngineMulti(QFontEngine *fontEngine, QChar::Script script)
{
   return new QFontEngineMulti(fontEngine, script);
}

QFontEngine *QPlatformFontDatabase::fontEngine(const QFontDef &fontDef, void *handle)
{
   QByteArray *fileDataPtr = static_cast<QByteArray *>(handle);
   QFontEngineQPF2 *engine = new QFontEngineQPF2(fontDef, *fileDataPtr);

   return engine;
}

QFontEngine *QPlatformFontDatabase::fontEngine(const QByteArray &fontData, qreal pixelSize,
   QFont::HintingPreference hintingPreference)
{
   (void) fontData;
   (void) pixelSize;
   (void) hintingPreference;

   qWarning("QPlatformFontDatabase::fontEngine() Plugin does not support font engines created directly from font data");
   return nullptr;
}

QStringList QPlatformFontDatabase::addApplicationFont(const QByteArray &fontData, const QString &fileName)
{
   (void) fontData;
   (void) fileName;

   qWarning("QPlatformFontDatabase::addApplicationFont() Plugin does not support application fonts");
   return QStringList();
}

void QPlatformFontDatabase::releaseHandle(void *handle)
{
   QByteArray *fileDataPtr = static_cast<QByteArray *>(handle);
   delete fileDataPtr;
}

QString QPlatformFontDatabase::fontDir() const
{
   QString fontpath = QString::fromUtf8(qgetenv("QT_FONTDIR"));

   if (fontpath.isEmpty()) {
      QStringList list = QStandardPaths::standardLocations(QStandardPaths::FontsLocation);

      if (! list.isEmpty()) {
         fontpath = list[0] + "/fonts";
      }
   }

   return fontpath;
}

bool QPlatformFontDatabase::isPrivateFontFamily(const QString &family) const
{
   (void) family;

   return false;
}

QFont QPlatformFontDatabase::defaultFont() const
{
   return QFont("Helvetica");
}

QString qt_resolveFontFamilyAlias(const QString &alias);

QString QPlatformFontDatabase::resolveFontFamilyAlias(const QString &family) const
{
   return qt_resolveFontFamilyAlias(family);
}

bool QPlatformFontDatabase::fontsAlwaysScalable() const
{
   return false;
}

QList<int> QPlatformFontDatabase::standardSizes() const
{
   QList<int> retval;

   static const quint8 standard[] =
         { 6, 7, 8, 9, 10, 11, 12, 14, 16, 18, 20, 22, 24, 26, 28, 36, 48, 72 };

   static constexpr const int num_standards = int(sizeof standard / sizeof * standard);

   std::copy(standard, standard + num_standards, std::back_inserter(retval));

   return retval;
}

QFontEngine::SubpixelAntialiasingType QPlatformFontDatabase::subpixelAntialiasingTypeHint() const
{
   static int type = -1;

   if (type == -1) {
      if (QScreen *screen = QApplication::primaryScreen()) {
         type = screen->handle()->subpixelAntialiasingTypeHint();
      }
   }

   return static_cast<QFontEngine::SubpixelAntialiasingType>(type);
}

// ### copied to tools/makeqpf/qpf2.cpp

// see the Unicode subset bitfields in the MSDN docs
static const quint8 requiredUnicodeBits[QFontDatabase::WritingSystemsCount][2] = {
   { 127, 127 }, // Any
   { 0, 127 },   // Latin
   { 7, 127 },   // Greek
   { 9, 127 },   // Cyrillic
   { 10, 127 },  // Armenian
   { 11, 127 },  // Hebrew
   { 13, 127 },  // Arabic
   { 71, 127 },  // Syriac
   { 72, 127 },  // Thaana
   { 15, 127 },  // Devanagari
   { 16, 127 },  // Bengali
   { 17, 127 },  // Gurmukhi
   { 18, 127 },  // Gujarati
   { 19, 127 },  // Oriya
   { 20, 127 },  // Tamil
   { 21, 127 },  // Telugu
   { 22, 127 },  // Kannada
   { 23, 127 },  // Malayalam
   { 73, 127 },  // Sinhala
   { 24, 127 },  // Thai
   { 25, 127 },  // Lao
   { 70, 127 },  // Tibetan
   { 74, 127 },  // Myanmar
   { 26, 127 },  // Georgian
   { 80, 127 },  // Khmer
   { 126, 127 }, // SimplifiedChinese
   { 126, 127 }, // TraditionalChinese
   { 126, 127 }, // Japanese
   { 56, 127 },  // Korean
   { 0, 127 },   // Vietnamese (same as latin1)
   { 126, 127 }, // Other
   { 78, 127 },  // Ogham
   { 79, 127 },  // Runic
   { 14, 127 },  // Nko
};

enum {
   Latin1CsbBit = 0,
   CentralEuropeCsbBit = 1,
   TurkishCsbBit = 4,
   BalticCsbBit = 7,
   CyrillicCsbBit = 2,
   GreekCsbBit = 3,
   HebrewCsbBit = 5,
   ArabicCsbBit = 6,
   VietnameseCsbBit = 8,
   SimplifiedChineseCsbBit = 18,
   TraditionalChineseCsbBit = 20,
   ThaiCsbBit = 16,
   JapaneseCsbBit = 17,
   KoreanCsbBit = 19,
   KoreanJohabCsbBit = 21,
   SymbolCsbBit = 31
};

QSupportedWritingSystems QPlatformFontDatabase::writingSystemsFromTrueTypeBits(quint32 unicodeRange[4], quint32 codePageRange[2])
{
   QSupportedWritingSystems writingSystems;

   bool hasScript = false;
   for (int i = 0; i < QFontDatabase::WritingSystemsCount; ++i) {
      int bit = requiredUnicodeBits[i][0];
      int index = bit / 32;
      int flag = 1 << (bit & 31);
      if (bit != 126 && (unicodeRange[index] & flag)) {
         bit = requiredUnicodeBits[i][1];
         index = bit / 32;

         flag = 1 << (bit & 31);

         if (bit == 127 || (unicodeRange[index] & flag)) {
            writingSystems.setSupported(QFontDatabase::WritingSystem(i));
            hasScript = true;
         }
      }
   }

   if (codePageRange[0] & ((1 << Latin1CsbBit) | (1 << CentralEuropeCsbBit) | (1 << TurkishCsbBit) | (1 << BalticCsbBit))) {
      writingSystems.setSupported(QFontDatabase::Latin);
      hasScript = true;
   }

   if (codePageRange[0] & (1 << CyrillicCsbBit)) {
      writingSystems.setSupported(QFontDatabase::Cyrillic);
      hasScript = true;
   }

   if (codePageRange[0] & (1 << GreekCsbBit)) {
      writingSystems.setSupported(QFontDatabase::Greek);
      hasScript = true;
   }

   if (codePageRange[0] & (1 << HebrewCsbBit)) {
      writingSystems.setSupported(QFontDatabase::Hebrew);
      hasScript = true;
   }

   if (codePageRange[0] & (1 << ArabicCsbBit)) {
      writingSystems.setSupported(QFontDatabase::Arabic);
      hasScript = true;
   }

   if (codePageRange[0] & (1 << ThaiCsbBit)) {
      writingSystems.setSupported(QFontDatabase::Thai);
      hasScript = true;
   }

   if (codePageRange[0] & (1 << VietnameseCsbBit)) {
      writingSystems.setSupported(QFontDatabase::Vietnamese);
      hasScript = true;
   }

   if (codePageRange[0] & (1 << SimplifiedChineseCsbBit)) {
      writingSystems.setSupported(QFontDatabase::SimplifiedChinese);
      hasScript = true;
   }

   if (codePageRange[0] & (1 << TraditionalChineseCsbBit)) {
      writingSystems.setSupported(QFontDatabase::TraditionalChinese);
      hasScript = true;
   }

   if (codePageRange[0] & (1 << JapaneseCsbBit)) {
      writingSystems.setSupported(QFontDatabase::Japanese);
      hasScript = true;
   }

   if (codePageRange[0] & ((1 << KoreanCsbBit) | (1 << KoreanJohabCsbBit))) {
      writingSystems.setSupported(QFontDatabase::Korean);
      hasScript = true;
   }

   if (codePageRange[0] & (1U << SymbolCsbBit)) {
      writingSystems = QSupportedWritingSystems();
      hasScript = false;
   }

   if (! hasScript) {
      writingSystems.setSupported(QFontDatabase::Symbol);
   }

   return writingSystems;
}

// convert 0 ~ 1000 integer to QFont::Weight
QFont::Weight QPlatformFontDatabase::weightFromInteger(int weight)
{
   if (weight < 150) {
      return QFont::Thin;
   }
   if (weight < 250) {
      return QFont::ExtraLight;
   }
   if (weight < 350) {
      return QFont::Light;
   }
   if (weight < 450) {
      return QFont::Normal;
   }
   if (weight < 550) {
      return QFont::Medium;
   }
   if (weight < 650) {
      return QFont::DemiBold;
   }
   if (weight < 750) {
      return QFont::Bold;
   }
   if (weight < 850) {
      return QFont::ExtraBold;
   }
   return QFont::Black;
}

void QPlatformFontDatabase::registerAliasToFontFamily(const QString &familyName, const QString &alias)
{
   qt_registerAliasToFontFamily(familyName, alias);
}

