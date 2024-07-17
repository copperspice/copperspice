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

#include <qwin_fontdatabase_ft.h>

#include <qwin_fontdatabase.h>
#include <qwin_context.h>
#include <QDir>
#include <QDirIterator>
#include <QSettings>
#include <QRegularExpression>
#include <QApplication>
#include <QFontDatabase>

#include <qfontengine_ft_p.h>

#include <wchar.h>

static inline QFontDatabase::WritingSystem writingSystemFromCharSet(uchar charSet)
{
   switch (charSet) {
      case ANSI_CHARSET:
      case EASTEUROPE_CHARSET:
      case BALTIC_CHARSET:
      case TURKISH_CHARSET:
         return QFontDatabase::Latin;
      case GREEK_CHARSET:
         return QFontDatabase::Greek;
      case RUSSIAN_CHARSET:
         return QFontDatabase::Cyrillic;
      case HEBREW_CHARSET:
         return QFontDatabase::Hebrew;
      case ARABIC_CHARSET:
         return QFontDatabase::Arabic;
      case THAI_CHARSET:
         return QFontDatabase::Thai;
      case GB2312_CHARSET:
         return QFontDatabase::SimplifiedChinese;
      case CHINESEBIG5_CHARSET:
         return QFontDatabase::TraditionalChinese;
      case SHIFTJIS_CHARSET:
         return QFontDatabase::Japanese;
      case HANGUL_CHARSET:
      case JOHAB_CHARSET:
         return QFontDatabase::Korean;
      case VIETNAMESE_CHARSET:
         return QFontDatabase::Vietnamese;
      case SYMBOL_CHARSET:
         return QFontDatabase::Symbol;
      default:
         break;
   }

   return QFontDatabase::Any;
}

static FontFile *createFontFile(const QString &fileName, int index)
{
   FontFile *fontFile   = new FontFile;
   fontFile->fileName   = fileName;
   fontFile->indexValue = index;

   return fontFile;
}

extern bool localizedName(const QString &name);
extern QString getEnglishName(const QString &familyName);

namespace {
struct FontKey {
   QString fileName;
   QStringList fontNames;
};
} // namespace

typedef QVector<FontKey> FontKeys;

static FontKeys &fontKeys()
{
   static FontKeys result;
   if (result.isEmpty()) {
      const QSettings fontRegistry(QString("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts"),
         QSettings::NativeFormat);

      const QStringList allKeys = fontRegistry.allKeys();
      const QString trueType    = "(TrueType)";

      const QRegularExpression sizeListMatch("\\s(\\d+,)+\\d+");

      const int size = allKeys.size();
      result.reserve(size);

      for (int i = 0; i < size; ++i) {
         FontKey fontKey;
         const QString &registryFontKey = allKeys.at(i);
         fontKey.fileName = fontRegistry.value(registryFontKey).toString();

         QString realKey = registryFontKey;
         realKey.remove(trueType);
         realKey.remove(sizeListMatch);

         const QStringList fontNames = realKey.trimmed().split('&');

         for (const QString &fontName : fontNames) {
            fontKey.fontNames.append(fontName.trimmed());
         }

         result.append(fontKey);
      }
   }

   return result;
}

static const FontKey *findFontKey(const QString &name, int *indexIn = nullptr)
{
   const FontKeys &keys = fontKeys();

   for (auto it = keys.constBegin(), cend = keys.constEnd(); it != cend; ++it) {
      const int index = it->fontNames.indexOf(name);

      if (index >= 0) {
         if (indexIn) {
            *indexIn = index;
         }

         return &(*it);
      }
   }

   if (indexIn) {
      *indexIn = -1;
   }

   return nullptr;
}

static bool addFontToDatabase(const QString &faceName, const QString &fullName, uchar charSet, const TEXTMETRIC *textmetric,
   const FONTSIGNATURE *signature, int type, bool registerAlias)
{
   // the "@family" fonts are just the same as "family". Ignore them.
   if (faceName.isEmpty() || faceName.at(0) == QChar('@') || faceName.startsWith("WST_")) {
      return false;
   }

   static const int SMOOTH_SCALABLE = 0xffff;
   const QString foundryName; // No such concept

   const bool fixed = !(textmetric->tmPitchAndFamily & TMPF_FIXED_PITCH);
   const bool ttf = (textmetric->tmPitchAndFamily & TMPF_TRUETYPE);
   const bool scalable = textmetric->tmPitchAndFamily & (TMPF_VECTOR | TMPF_TRUETYPE);
   const int size = scalable ? SMOOTH_SCALABLE : textmetric->tmHeight;

   const QFont::Style style = textmetric->tmItalic ? QFont::StyleItalic : QFont::StyleNormal;
   const bool antialias = false;
   const QFont::Weight weight = QPlatformFontDatabase::weightFromInteger(textmetric->tmWeight);
   const QFont::Stretch stretch = QFont::Unstretched;

#if defined(CS_SHOW_DEBUG_PLATFORM)
   QString message;
   QTextStream str(&message);

   str << "addFontToDatabase() " << faceName << "::" << fullName << ' ' << charSet << " TTF = " << ttf;

   if (type & DEVICE_FONTTYPE) {
      str << " DEVICE";
   }

   if (type & RASTER_FONTTYPE) {
      str << " RASTER";
   }

   if (type & TRUETYPE_FONTTYPE) {
      str << " TRUETYPE";
   }

   str << " scalable = " << scalable << " Size = " << size
      << " Style = " << style << " Weight = " << weight << " stretch = " << stretch;

   qDebug() << message;
#endif

   QString englishName;
   if (registerAlias & ttf && localizedName(faceName)) {
      englishName = getEnglishName(faceName);
   }

   QSupportedWritingSystems writingSystems;
   if (type & TRUETYPE_FONTTYPE) {
      Q_ASSERT(signature);
      quint32 unicodeRange[4] = {
         signature->fsUsb[0], signature->fsUsb[1],
         signature->fsUsb[2], signature->fsUsb[3]
      };
      quint32 codePageRange[2] = {
         signature->fsCsb[0], signature->fsCsb[1]
      };
      writingSystems = QPlatformFontDatabase::writingSystemsFromTrueTypeBits(unicodeRange, codePageRange);
      // ### Hack to work around problem with Thai text on Windows 7. Segoe UI contains
      // the symbol for Baht, and Windows thus reports that it supports the Thai script.
      // Since it's the default UI font on this platform, most widgets will be unable to
      // display Thai text by default. As a temporary work around, we special case Segoe UI
      // and remove the Thai script from its list of supported writing systems.
      if (writingSystems.supported(QFontDatabase::Thai) &&
         faceName == QString("Segoe UI")) {
         writingSystems.setSupported(QFontDatabase::Thai, false);
      }

   } else {
      const QFontDatabase::WritingSystem ws = writingSystemFromCharSet(charSet);
      if (ws != QFontDatabase::Any) {
         writingSystems.setSupported(ws);
      }
   }

   int index = 0;

   const FontKey *key = findFontKey(faceName, &index);
   if (!key) {
      key = findFontKey(fullName, &index);
      if (!key && !registerAlias && englishName.isEmpty() && localizedName(faceName)) {
         englishName = getEnglishName(faceName);
      }
      if (!key && !englishName.isEmpty()) {
         key = findFontKey(englishName, &index);
      }
      if (!key) {
         return false;
      }
   }
   QString value = key->fileName;


   if (value.isEmpty()) {
      return false;
   }

   if (!QDir::isAbsolutePath(value))
      value.prepend(QFile::decodeName(qgetenv("windir") + "\\Fonts\\"));

   QPlatformFontDatabase::registerFont(faceName, QString(), foundryName, weight, style, stretch,
      antialias, scalable, size, fixed, writingSystems, createFontFile(value, index));

   // add fonts windows can generate for us:
   if (weight <= QFont::DemiBold)
      QPlatformFontDatabase::registerFont(faceName, QString(), foundryName, QFont::Bold, style, stretch,
         antialias, scalable, size, fixed, writingSystems, createFontFile(value, index));

   if (style != QFont::StyleItalic)
      QPlatformFontDatabase::registerFont(faceName, QString(), foundryName, weight, QFont::StyleItalic, stretch,
         antialias, scalable, size, fixed, writingSystems, createFontFile(value, index));

   if (weight <= QFont::DemiBold && style != QFont::StyleItalic)
      QPlatformFontDatabase::registerFont(faceName, QString(), foundryName, QFont::Bold, QFont::StyleItalic, stretch,
         antialias, scalable, size, fixed, writingSystems, createFontFile(value, index));

   if (! englishName.isEmpty()) {
      QPlatformFontDatabase::registerAliasToFontFamily(faceName, englishName);
   }

   return true;
}

static int QT_WIN_CALLBACK storeFont(const LOGFONT *logFont, const TEXTMETRIC *textmetric,  DWORD type, LPARAM)
{
   const ENUMLOGFONTEX *f = reinterpret_cast<const ENUMLOGFONTEX *>(logFont);

   const QString faceName = QString::fromStdWString(std::wstring(f->elfLogFont.lfFaceName));
   const QString fullName = QString::fromStdWString(std::wstring(f->elfFullName));

   const uchar charSet = f->elfLogFont.lfCharSet;

   // NEWTEXTMETRICEX (passed for TT fonts) is a NEWTEXTMETRIC, which according
   // to the documentation is identical to a TEXTMETRIC except for the last four
   // members, which we don't use anyway

   const FONTSIGNATURE *signature = nullptr;

   if (type & TRUETYPE_FONTTYPE) {
      signature = &reinterpret_cast<const NEWTEXTMETRICEX *>(textmetric)->ntmFontSig;
   }

   addFontToDatabase(faceName, fullName, charSet, textmetric, signature, type, false);

   // keep on enumerating
   return 1;
}

void QWindowsFontDatabaseFT::populateFamily(const QString &familyName)
{
#if defined(CS_SHOW_DEBUG_PLATFORM)
   qDebug() << "QWindowsFontDatabaseFT::populateFamily() Family Name = " << familyName;
#endif

   if (familyName.size() >= LF_FACESIZE) {
      qWarning() << "QWindowsFontDatabaseFT::populateFamily() Unable to enumerate family name = " << familyName;
      return;
   }

   HDC dummy = GetDC(nullptr);
   LOGFONT lf;
   lf.lfCharSet = DEFAULT_CHARSET;

   QString16 tmp = familyName.toUtf16();
   memcpy(lf.lfFaceName, tmp.constData(), tmp.size_storage() * 2);

   lf.lfFaceName[tmp.size_storage()] = 0;
   lf.lfPitchAndFamily = 0;

   EnumFontFamiliesEx(dummy, &lf, storeFont, 0, 0);
   ReleaseDC(nullptr, dummy);
}

namespace {
// Context for enumerating system fonts, records whether the default font has been
// encountered, which is normally not enumerated.

struct PopulateFamiliesContext {
   PopulateFamiliesContext(const QString &f) : systemDefaultFont(f), seenSystemDefaultFont(false) {}

   QString systemDefaultFont;
   bool seenSystemDefaultFont;
};
} // namespace


// Delayed population of font families

static int QT_WIN_CALLBACK populateFontFamilies(const LOGFONT *logFont, const TEXTMETRIC *textmetric,
   DWORD, LPARAM lparam)
{
   const ENUMLOGFONTEX *f = reinterpret_cast<const ENUMLOGFONTEX *>(logFont);

   // the "@family" fonts are just the same as "family". Ignore them.
   const wchar_t *faceNameW = f->elfLogFont.lfFaceName;

   if (faceNameW[0] && faceNameW[0] != L'@' && wcsncmp(faceNameW, L"WST_", 4)) {
      // Register only font families for which a font file exists for delayed population

      const bool ttf = textmetric->tmPitchAndFamily & TMPF_TRUETYPE;
      const QString faceName = QString::fromStdWString(std::wstring(faceNameW));
      const FontKey *key = findFontKey(faceName);

      if (! key) {
         key = findFontKey(QString::fromStdWString(std::wstring(f->elfFullName)));
         if (!key && ttf && localizedName(faceName)) {
            key = findFontKey(getEnglishName(faceName));
         }
      }

      if (key) {
         QPlatformFontDatabase::registerFontFamily(faceName);
         PopulateFamiliesContext *context = reinterpret_cast<PopulateFamiliesContext *>(lparam);
         if (!context->seenSystemDefaultFont && faceName == context->systemDefaultFont) {
            context->seenSystemDefaultFont = true;
         }

         // Register current font's english name as alias
         if (ttf && localizedName(faceName)) {
            const QString englishName = getEnglishName(faceName);

            if (!englishName.isEmpty()) {
               QPlatformFontDatabase::registerAliasToFontFamily(faceName, englishName);
               // Check whether the system default font name is an alias of the current font family name,
               // as on Chinese Windows, where the system font "SimSun" is an alias to a font registered under a local name
               if (!context->seenSystemDefaultFont && englishName == context->systemDefaultFont) {
                  context->seenSystemDefaultFont = true;
               }
            }
         }
      }
   }
   return 1; // continue
}

void QWindowsFontDatabaseFT::populateFontDatabase()
{
   HDC dummy = GetDC(nullptr);

   LOGFONT lf;
   lf.lfCharSet = DEFAULT_CHARSET;
   lf.lfFaceName[0] = 0;
   lf.lfPitchAndFamily = 0;

   PopulateFamiliesContext context(QWindowsFontDatabase::systemDefaultFont().family());
   EnumFontFamiliesEx(dummy, &lf, populateFontFamilies, reinterpret_cast<LPARAM>(&context), 0);
   ReleaseDC(nullptr, dummy);

   // Work around EnumFontFamiliesEx() not listing the system font
   if (!context.seenSystemDefaultFont) {
      QPlatformFontDatabase::registerFontFamily(context.systemDefaultFont);
   }
}

QFontEngine *QWindowsFontDatabaseFT::fontEngine(const QFontDef &fontDef, void *handle)
{
   QFontEngine *fe = QBasicFontDatabase::fontEngine(fontDef, handle);

#if defined(CS_SHOW_DEBUG_PLATFORM)
   qDebug() << "QWindowsFontDatabaseFT::fontEngine() Font data = " << fontDef.family << fe << handle;
#endif

   return fe;
}

QFontEngine *QWindowsFontDatabaseFT::fontEngine(const QByteArray &fontData, qreal pixelSize,
      QFont::HintingPreference hintingPreference)
{
   QFontEngine *fe = QBasicFontDatabase::fontEngine(fontData, pixelSize, hintingPreference);

#if defined(CS_SHOW_DEBUG_PLATFORM)
   qDebug() << "QWindowsFontDatabaseFT::fontEngine() Font data = " << fontData << pixelSize << hintingPreference << fe;
#endif

   return fe;
}

QStringList QWindowsFontDatabaseFT::fallbacksForFamily(const QString &family, QFont::Style style, QFont::StyleHint styleHint,
   QChar::Script script) const
{
   QStringList result;

   result.append(QWindowsFontDatabase::familyForStyleHint(styleHint));
   result.append(QWindowsFontDatabase::extraTryFontsForFamily(family));
   result.append(QBasicFontDatabase::fallbacksForFamily(family, style, styleHint, script));

#if defined(CS_SHOW_DEBUG_PLATFORM)
   qDebug() << "QWindowsFontDatabaseFT::fallbacksForFamily() Font = "
         << family << style << styleHint << script << result;
#endif

   return result;
}

QString QWindowsFontDatabaseFT::fontDir() const
{
   const QString result = qgetenv("windir") + "/Fonts";
   return result;
}

QFont QWindowsFontDatabaseFT::defaultFont() const
{
   return QWindowsFontDatabase::systemDefaultFont();
}
