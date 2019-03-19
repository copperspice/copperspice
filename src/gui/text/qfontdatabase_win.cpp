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

#include <qt_windows.h>
#include <qmath.h>
#include <qapplication_p.h>
#include <qfont_p.h>
#include <qfontengine_p.h>
#include <qpaintdevice.h>
#include <qsystemlibrary_p.h>
#include <qabstractfileengine.h>
#include <qendian.h>

#if !defined(QT_NO_DIRECTWRITE)
#  include <qsettings.h>
#  include <qfontenginedirectwrite_p.h>
#endif

QT_BEGIN_NAMESPACE

extern HDC   shared_dc();                // common dc for all fonts

#ifdef MAKE_TAG
#undef MAKE_TAG
#endif

// GetFontData expects the tags in little endian ;(
#define MAKE_TAG(ch1, ch2, ch3, ch4) (\
    (((quint32)(ch4)) << 24) | \
    (((quint32)(ch3)) << 16) | \
    (((quint32)(ch2)) << 8) | \
    ((quint32)(ch1)) \
   )

static HFONT stock_sysfont  = 0;

static bool localizedName(const QString &name)
{
   for (QChar c : name) {
      if (c >= 0x100) {
         return true;
      }
   }

   return false;
}

static inline quint16 getUShort(const unsigned char *p)
{
   quint16 val;
   val = *p++ << 8;
   val |= *p;

   return val;
}

static QString getEnglishName(const uchar *table, quint32 bytes)
{
   QString i18n_name;

   enum {
      NameRecordSize = 12,
      FamilyId = 1,
      MS_LangIdEnglish = 0x009
   };

   // get the name table
   quint16 count;
   quint16 string_offset;
   const unsigned char *names;

   int microsoft_id = -1;
   int apple_id = -1;
   int unicode_id = -1;

   if (getUShort(table) != 0) {
      goto error;
   }

   count = getUShort(table + 2);
   string_offset = getUShort(table + 4);
   names = table + 6;

   if (string_offset >= bytes || 6 + count * NameRecordSize > string_offset) {
      goto error;
   }

   for (int i = 0; i < count; ++i) {
      // search for the correct name entry

      quint16 platform_id = getUShort(names + i * NameRecordSize);
      quint16 encoding_id = getUShort(names + 2 + i * NameRecordSize);
      quint16 language_id = getUShort(names + 4 + i * NameRecordSize);
      quint16 name_id = getUShort(names + 6 + i * NameRecordSize);

      if (name_id != FamilyId) {
         continue;
      }

      enum {
         PlatformId_Unicode = 0,
         PlatformId_Apple = 1,
         PlatformId_Microsoft = 3
      };

      quint16 length = getUShort(names + 8 + i * NameRecordSize);
      quint16 offset = getUShort(names + 10 + i * NameRecordSize);

      if (DWORD(string_offset + offset + length) >= bytes) {
         continue;
      }

      if ((platform_id == PlatformId_Microsoft
            && (encoding_id == 0 || encoding_id == 1))
            && (language_id & 0x3ff) == MS_LangIdEnglish
            && microsoft_id == -1) {
         microsoft_id = i;
      }
      // not sure if encoding id 4 for Unicode is utf16 or ucs4...
      else if (platform_id == PlatformId_Unicode && encoding_id < 4 && unicode_id == -1) {
         unicode_id = i;
      } else if (platform_id == PlatformId_Apple && encoding_id == 0 && language_id == 0) {
         apple_id = i;
      }
   }
   {
      bool unicode = false;
      int id = -1;

      if (microsoft_id != -1) {
         id = microsoft_id;
         unicode = true;

      } else if (apple_id != -1) {
         id = apple_id;
         unicode = false;

      } else if (unicode_id != -1) {
         id = unicode_id;
         unicode = true;
      }

      if (id != -1) {
         quint16 length = getUShort(names + 8 + id * NameRecordSize);
         quint16 offset = getUShort(names + 10 + id * NameRecordSize);

         if (unicode) {
            // utf16

            length /= 2;

            std::wstring tmp;
            const unsigned char *string = table + string_offset + offset;

            for (int i = 0; i < length; ++i) {
               tmp.push_back(getUShort(string + 2 * i));
            }

            i18n_name = QString::fromStdWString(tmp);

         } else {
            // Apple Roman

            const unsigned char *string = table + string_offset + offset;
            i18n_name = QString::fromLatin1( reinterpret_cast<const char *>(string), length);

         }
      }
   }

error:
   return i18n_name;
}

static QString getEnglishName(const QString &familyName)
{
   QString i18n_name;

   HDC hdc = GetDC( 0 );
   LOGFONT lf;

   QString16 tmp = familyName.toUtf16();

   memset(&lf, 0, sizeof(LOGFONT));
   memcpy(lf.lfFaceName, tmp.constData(), qMin(LF_FACESIZE, tmp.size_storage()) * sizeof(wchar_t));

   lf.lfCharSet = DEFAULT_CHARSET;
   HFONT hfont = CreateFontIndirect(&lf);

   if (! hfont) {
      ReleaseDC(0, hdc);
      return QString();
   }

   HGDIOBJ oldobj = SelectObject( hdc, hfont );

   const DWORD name_tag = MAKE_TAG( 'n', 'a', 'm', 'e' );

   // get the name table
   unsigned char *table = 0;

   DWORD bytes = GetFontData( hdc, name_tag, 0, 0, 0 );
   if ( bytes == GDI_ERROR ) {
      // ### Unused variable
      /* int err = GetLastError(); */
      goto error;
   }

   table = new unsigned char[bytes];
   GetFontData(hdc, name_tag, 0, table, bytes);
   if ( bytes == GDI_ERROR ) {
      goto error;
   }

   i18n_name = getEnglishName(table, bytes);

error:
   delete [] table;
   SelectObject( hdc, oldobj );
   DeleteObject( hfont );
   ReleaseDC( 0, hdc );

   return i18n_name;
}

extern QFont::Weight weightFromInteger(int weight); // qfontdatabase.cpp

static
void addFontToDatabase(QString familyName, const QString &scriptName, TEXTMETRIC *textmetric,
                  const FONTSIGNATURE *signature, int type)
{
   const int script = -1;
   const QString foundryName;

   Q_UNUSED(script);

   bool italic = false;
   int weight;
   bool fixed;
   bool ttf;
   bool scalable;
   int size;

   NEWTEXTMETRIC *tm = (NEWTEXTMETRIC *)textmetric;
   fixed = !(tm->tmPitchAndFamily & TMPF_FIXED_PITCH);
   ttf = (tm->tmPitchAndFamily & TMPF_TRUETYPE);
   scalable = tm->tmPitchAndFamily & (TMPF_VECTOR | TMPF_TRUETYPE);
   size = scalable ? SMOOTH_SCALABLE : tm->tmHeight;
   italic = tm->tmItalic;
   weight = tm->tmWeight;

   // the "@family" fonts are just the same as "family". Ignore them.
   if (familyName[0] != QLatin1Char('@') && !familyName.startsWith(QLatin1String("WST_"))) {
      QtFontStyle::Key styleKey;
      styleKey.style = italic ? QFont::StyleItalic : QFont::StyleNormal;
      styleKey.weight = weightFromInteger(weight);

      QtFontFamily *family = privateDb()->family(familyName, true);

      if (ttf && localizedName(familyName) && family->english_name.isEmpty()) {
         family->english_name = getEnglishName(familyName);
      }

      QtFontFoundry *foundry = family->foundry(foundryName, true);
      QtFontStyle *style = foundry->style(styleKey, QString(), true);
      style->smoothScalable = scalable;
      style->pixelSize( size, TRUE);

      // add fonts windows can generate for us:
      if (styleKey.weight <= QFont::DemiBold) {
         QtFontStyle::Key key(styleKey);
         key.weight = QFont::Bold;
         QtFontStyle *style = foundry->style(key, QString(), true);
         style->smoothScalable = scalable;
         style->pixelSize( size, TRUE);
      }
      if (styleKey.style != QFont::StyleItalic) {
         QtFontStyle::Key key(styleKey);
         key.style = QFont::StyleItalic;
         QtFontStyle *style = foundry->style(key, QString(), true);
         style->smoothScalable = scalable;
         style->pixelSize( size, TRUE);
      }
      if (styleKey.weight <= QFont::DemiBold && styleKey.style != QFont::StyleItalic) {
         QtFontStyle::Key key(styleKey);
         key.weight = QFont::Bold;
         key.style = QFont::StyleItalic;
         QtFontStyle *style = foundry->style(key, QString(), true);
         style->smoothScalable = scalable;
         style->pixelSize( size, TRUE);
      }

      family->fixedPitch = fixed;

      if (!family->writingSystemCheck && type & TRUETYPE_FONTTYPE) {
         quint32 unicodeRange[4] = {
            signature->fsUsb[0], signature->fsUsb[1],
            signature->fsUsb[2], signature->fsUsb[3]
         };

         quint32 codePageRange[2] = {
            signature->fsCsb[0], signature->fsCsb[1]
         };
         QList<QFontDatabase::WritingSystem> systems = qt_determine_writing_systems_from_truetype_bits(unicodeRange,
               codePageRange);

         for (int i = 0; i < systems.count(); ++i) {
            QFontDatabase::WritingSystem writingSystem = systems.at(i);

            // ### Hack to work around problem with Thai text on Windows 7. Segoe UI contains
            // the symbol for Baht, and Windows thus reports that it supports the Thai script.
            // Since it's the default UI font on this platform, most widgets will be unable to
            // display Thai text by default. As a temporary work around, we special case Segoe UI
            // and remove the Thai script from its list of supported writing systems.
            if (writingSystem != QFontDatabase::Thai || familyName != QLatin1String("Segoe UI")) {
               family->writingSystems[writingSystem] = QtFontFamily::Supported;
            }
         }
      } else if (!family->writingSystemCheck) {
         //qDebug("family='%s' script=%s", family->name.latin1(), script.latin1());
         if (scriptName == QLatin1String("Western")
               || scriptName == QLatin1String("Baltic")
               || scriptName == QLatin1String("Central European")
               || scriptName == QLatin1String("Turkish")
               || scriptName == QLatin1String("Vietnamese")) {
            family->writingSystems[QFontDatabase::Latin] = QtFontFamily::Supported;
         } else if (scriptName == QLatin1String("Thai")) {
            family->writingSystems[QFontDatabase::Thai] = QtFontFamily::Supported;
         } else if (scriptName == QLatin1String("Symbol")
                    || scriptName == QLatin1String("Other")) {
            family->writingSystems[QFontDatabase::Symbol] = QtFontFamily::Supported;
         } else if (scriptName == QLatin1String("OEM/Dos")) {
            family->writingSystems[QFontDatabase::Latin] = QtFontFamily::Supported;
         } else if (scriptName == QLatin1String("CHINESE_GB2312")) {
            family->writingSystems[QFontDatabase::SimplifiedChinese] = QtFontFamily::Supported;
         } else if (scriptName == QLatin1String("CHINESE_BIG5")) {
            family->writingSystems[QFontDatabase::TraditionalChinese] = QtFontFamily::Supported;
         } else if (scriptName == QLatin1String("Cyrillic")) {
            family->writingSystems[QFontDatabase::Cyrillic] = QtFontFamily::Supported;
         } else if (scriptName == QLatin1String("Hangul")) {
            family->writingSystems[QFontDatabase::Korean] = QtFontFamily::Supported;
         } else if (scriptName == QLatin1String("Hebrew")) {
            family->writingSystems[QFontDatabase::Hebrew] = QtFontFamily::Supported;
         } else if (scriptName == QLatin1String("Greek")) {
            family->writingSystems[QFontDatabase::Greek] = QtFontFamily::Supported;
         } else if (scriptName == QLatin1String("Japanese")) {
            family->writingSystems[QFontDatabase::Japanese] = QtFontFamily::Supported;
         } else if (scriptName == QLatin1String("Arabic")) {
            family->writingSystems[QFontDatabase::Arabic] = QtFontFamily::Supported;
         }
      }
   }
}

static int CALLBACK storeFont(ENUMLOGFONTEX *f, NEWTEXTMETRICEX *textmetric, int type, LPARAM /*p*/)
{
   QString familyName = QString::fromStdWString(std::wstring(f->elfLogFont.lfFaceName));
   QString script     = QString::fromStdWString(std::wstring(f->elfScript));

   FONTSIGNATURE signature = textmetric->ntmFontSig;

   // NEWTEXTMETRICEX is a NEWTEXTMETRIC, which according to the documentation is
   // identical to a TEXTMETRIC except for the last four members, which we don't use anyway

   addFontToDatabase(familyName, script, (TEXTMETRIC *)textmetric, &signature, type);

   // keep on enumerating
   return 1;
}

static void populate_database(const QString &fam)
{
   QFontDatabasePrivate *d = privateDb();

   if (! d) {
      return;
   }

   QtFontFamily *family = 0;

   if (! fam.isEmpty()) {
      family = d->family(fam);

      if (family && family->loaded) {
         return;
      }

   } else if (d->count) {
      return;
   }

   HDC dummy = GetDC(0);

   LOGFONT lf;
   lf.lfCharSet = DEFAULT_CHARSET;

   if (fam.isEmpty()) {
      lf.lfFaceName[0] = 0;

   } else {
      QString16 tmp = fam.toUtf16();
      memcpy(lf.lfFaceName, tmp.constData(), sizeof(wchar_t) * qMin(tmp.size_storage() + 1, 32));  // 32 = Windows hard-coded
   }

   lf.lfPitchAndFamily = 0;

   EnumFontFamiliesEx(dummy, &lf, (FONTENUMPROC)storeFont, (LPARAM)privateDb(), 0);
   ReleaseDC(0, dummy);

   for (int i = 0; i < d->applicationFonts.count(); ++i) {
      QFontDatabasePrivate::ApplicationFont fnt = d->applicationFonts.at(i);

      if (!fnt.memoryFont) {
         continue;
      }

      for (int j = 0; j < fnt.families.count(); ++j) {
         const QString familyName = fnt.families.at(j);
         HDC hdc = GetDC(0);
         LOGFONT lf;

         QString16 tmp = familyName.toUtf16();

         memset(&lf, 0, sizeof(LOGFONT));
         memcpy(lf.lfFaceName, tmp.constData(), sizeof(wchar_t) * qMin(LF_FACESIZE, tmp.size_storage()));

         lf.lfCharSet = DEFAULT_CHARSET;
         HFONT hfont = CreateFontIndirect(&lf);
         HGDIOBJ oldobj = SelectObject(hdc, hfont);

         TEXTMETRIC textMetrics;
         GetTextMetrics(hdc, &textMetrics);

         addFontToDatabase(familyName, QString(), &textMetrics, &fnt.signatures.at(j), TRUETYPE_FONTTYPE);

         SelectObject(hdc, oldobj);
         DeleteObject(hfont);
         ReleaseDC(0, hdc);
      }
   }

   if (!fam.isEmpty()) {
      family = d->family(fam);
      if (family) {
         if (!family->writingSystemCheck) {
         }
         family->loaded = true;
      }
   }
}

static void initializeDb()
{
   QFontDatabasePrivate *db = privateDb();
   if (!db || db->count) {
      return;
   }

   populate_database(QString());

#ifdef QFONTDATABASE_DEBUG
   // print the database
   for (int f = 0; f < db->count; f++) {
      QtFontFamily *family = db->families[f];
      qDebug("    %s: %p", qPrintable(family->name), family);
      populate_database(family->name);
   }
#endif // QFONTDATABASE_DEBUG

}

static inline void load(const QString &family = QString(), int = -1)
{
   populate_database(family);
}

// --------------------------------------------------------------------------------------
// font loader
// --------------------------------------------------------------------------------------

static void initFontInfo(QFontEngineWin *fe, const QFontDef &request, HDC fontHdc, int dpi)
{
   fe->fontDef = request;                                // most settings are equal

   HDC dc = ((request.styleStrategy & QFont::PreferDevice) && fontHdc) ? fontHdc : shared_dc();

   SelectObject(dc, fe->hfont);

   std::wstring n(64, L'\0');
   GetTextFace(dc, 64, &n[0]);

   fe->fontDef.family = QString::fromStdWString(n);
   fe->fontDef.fixedPitch = !(fe->tm.tmPitchAndFamily & TMPF_FIXED_PITCH);

   if (fe->fontDef.pointSize < 0) {
      fe->fontDef.pointSize = fe->fontDef.pixelSize * 72. / dpi;

   } else if (fe->fontDef.pixelSize == -1) {
      fe->fontDef.pixelSize = qRound(fe->fontDef.pointSize * dpi / 72.);
   }
}

#if !defined(QT_NO_DIRECTWRITE)
static void initFontInfo(QFontEngineDirectWrite *fe, const QFontDef &request, int dpi, IDWriteFont *font)
{
   fe->fontDef = request;

   IDWriteFontFamily *fontFamily = NULL;
   HRESULT hr = font->GetFontFamily(&fontFamily);

   IDWriteLocalizedStrings *familyNames = NULL;
   if (SUCCEEDED(hr)) {
      hr = fontFamily->GetFamilyNames(&familyNames);
   }

   UINT32 index = 0;
   BOOL exists = false;

   wchar_t localeName[LOCALE_NAME_MAX_LENGTH];

   if (SUCCEEDED(hr)) {

      HMODULE handle = GetModuleHandle("KERNEL32.DLL");

      int (*ptr)(LPWSTR, int);
      ptr = static_cast<int (*)(LPWSTR, int)> (GetProcAddress(handle, "GetUserDefaultLocaleName");

      if (ptr)  {
      // running on Vista or greater
      int defaultLocaleSuccess = ptr(localeName, LOCALE_NAME_MAX_LENGTH);

         if (defaultLocaleSuccess) {
            hr = familyNames->FindLocaleName(localeName, &index, &exists);
         }
      }

      if (SUCCEEDED(hr) && ! exists) {
      hr = familyNames->FindLocaleName(L"en-us", &index, &exists);
      }
   }

   if (! exists) {
      index = 0;
   }

   UINT32 length = 0;
   if (SUCCEEDED(hr)) {
      hr = familyNames->GetStringLength(index, &length);
   }

   wchar_t *name = new (std::nothrow) wchar_t[length + 1];
   if (name == NULL) {
      hr = E_OUTOFMEMORY;
   }

   // Get the family name
   if (SUCCEEDED(hr)) {
      hr = familyNames->GetString(index, name, length + 1);
   }

   if (SUCCEEDED(hr)) {
      fe->fontDef.family = QString::fromWCharArray(name);
   }

   delete[] name;
   if (familyNames != NULL) {
      familyNames->Release();
   }

   if (FAILED(hr)) {
      qErrnoWarning(hr, "initFontInfo: Failed to get family name");
   }

   if (fe->fontDef.pointSize < 0) {
      fe->fontDef.pointSize = fe->fontDef.pixelSize * 72. / dpi;
   } else if (fe->fontDef.pixelSize == -1) {
      fe->fontDef.pixelSize = qRound(fe->fontDef.pointSize * dpi / 72.);
   }
}
#endif

static const QStringList other_tryFonts = {
   "Arial",
   "MS UI Gothic",
   "Gulim",
   "SimSun",
   "PMingLiU",
   "Arial Unicode MS"
};

static const QStringList jp_tryFonts = {
   "MS UI Gothic",
   "Arial",
   "Gulim",
   "SimSun",
   "PMingLiU",
   "Arial Unicode MS"
};

static const QStringList ch_CN_tryFonts = {
   "SimSun",
   "Arial",
   "PMingLiU",
   "Gulim",
   "MS UI Gothic",
   "Arial Unicode MS"
};

static const QStringList ch_TW_tryFonts = {
   "PMingLiU",
   "Arial",
   "SimSun",
   "Gulim",
   "MS UI Gothic",
   "Arial Unicode MS"
};

static const QStringList kr_tryFonts = {
   "Gulim",
   "Arial",
   "PMingLiU",
   "SimSun",
   "MS UI Gothic",
   "Arial Unicode MS"
};

static const QStringList *tryFonts = nullptr;

#if ! defined(QT_NO_DIRECTWRITE)

static QString fontNameSubstitute(const QString &familyName)
{
   QString key("HKEY_LOCAL_MACHINE\\Software\\Microsoft\\Windows NT\\CurrentVersion\\" "FontSubstitutes");
   return QSettings(key, QSettings::NativeFormat).value(familyName, familyName).toString();
}
#endif

static inline HFONT systemFont()
{
   if (stock_sysfont == 0) {
      stock_sysfont = (HFONT)GetStockObject(SYSTEM_FONT);
   }

   return stock_sysfont;
}

#if ! defined(DEFAULT_GUI_FONT)
#define DEFAULT_GUI_FONT 17
#endif

static QFontEngine *loadEngine(int script, const QFontDef &request, HDC fontHdc, int dpi, bool rawMode,
                  const QtFontDesc *desc, const QStringList &family_list)
{
   LOGFONT lf;
   memset(&lf, 0, sizeof(LOGFONT));

   bool useDevice = (request.styleStrategy & QFont::PreferDevice) && fontHdc;

   HDC hdc = shared_dc();
   QString font_name = desc != 0 ? desc->family->name : request.family;

   if (useDevice) {
      hdc = fontHdc;
      font_name = request.family;
   }

   bool stockFont = false;
   bool preferClearTypeAA = false;

   HFONT hfont = 0;

#if ! defined(QT_NO_DIRECTWRITE)
   bool useDirectWrite = (request.hintingPreference == QFont::PreferNoHinting)
                         || (request.hintingPreference == QFont::PreferVerticalHinting);

   IDWriteFont *directWriteFont = 0;
#else
   bool useDirectWrite = false;
#endif

   if (rawMode) {                        // will choose a stock font
      int f, deffnt = SYSTEM_FONT;
      QString fam = desc != 0 ? desc->family->name.toLower() : request.family.toLower();

      if (fam == QLatin1String("default")) {
         f = deffnt;

      } else if (fam == QLatin1String("system")) {
         f = SYSTEM_FONT;

      } else if (fam == QLatin1String("system_fixed")) {
         f = SYSTEM_FIXED_FONT;

      } else if (fam == QLatin1String("ansi_fixed")) {
         f = ANSI_FIXED_FONT;

      } else if (fam == QLatin1String("ansi_var")) {
         f = ANSI_VAR_FONT;

      } else if (fam == QLatin1String("device_default")) {
         f = DEVICE_DEFAULT_FONT;

      } else if (fam == QLatin1String("oem_fixed")) {
         f = OEM_FIXED_FONT;

      } else if (fam[0] == QLatin1Char('#')) {
         f = fam.right(fam.length() - 1).toInteger<int>();

      } else {
         f = deffnt;
      }

      hfont = (HFONT)GetStockObject(f);
      if (! hfont) {
         qErrnoWarning("QFontEngine::loadEngine: GetStockObject failed");
         hfont = systemFont();
      }
      stockFont = true;

   } else {

      int hint = FF_DONTCARE;

      switch (request.styleHint) {
         case QFont::Helvetica:
            hint = FF_SWISS;
            break;

         case QFont::Times:
            hint = FF_ROMAN;
            break;

         case QFont::Courier:
            hint = FF_MODERN;
            break;

         case QFont::OldEnglish:
            hint = FF_DECORATIVE;
            break;
         case QFont::System:
            hint = FF_MODERN;
            break;
         default:
            break;
      }

      lf.lfHeight       = -qRound(request.pixelSize);
      lf.lfWidth        = 0;
      lf.lfEscapement   = 0;
      lf.lfOrientation  = 0;

      if (desc == 0 || desc->style->key.weight == 50) {
         lf.lfWeight = FW_DONTCARE;

      } else {
         lf.lfWeight = (desc->style->key.weight * 900) / 99;
      }

      lf.lfItalic   = (desc != 0 && desc->style->key.style != QFont::StyleNormal);
      lf.lfCharSet  = DEFAULT_CHARSET;

      int strat = OUT_DEFAULT_PRECIS;

      if (request.styleStrategy & QFont::PreferBitmap) {
         strat = OUT_RASTER_PRECIS;

      } else if (request.styleStrategy & QFont::PreferDevice) {
         strat = OUT_DEVICE_PRECIS;

      } else if (request.styleStrategy & QFont::PreferOutline) {
         strat = OUT_OUTLINE_PRECIS;

      } else if (request.styleStrategy & QFont::ForceOutline) {
         strat = OUT_TT_ONLY_PRECIS;

      }

      lf.lfOutPrecision   = strat;

      int qual = DEFAULT_QUALITY;

      if (request.styleStrategy & QFont::PreferMatch) {
         qual = DRAFT_QUALITY;
      }

      else if (request.styleStrategy & QFont::PreferQuality) {
         qual = PROOF_QUALITY;
      }

      if (request.styleStrategy & QFont::PreferAntialias) {
         if (QSysInfo::WindowsVersion >= QSysInfo::WV_XP) {
            qual = CLEARTYPE_QUALITY;
            preferClearTypeAA = true;
         } else {
            qual = ANTIALIASED_QUALITY;
         }

      } else if (request.styleStrategy & QFont::NoAntialias) {
         qual = NONANTIALIASED_QUALITY;
      }

      lf.lfQuality = qual;

      lf.lfClipPrecision  = CLIP_DEFAULT_PRECIS;
      lf.lfPitchAndFamily = DEFAULT_PITCH | hint;

      QString fam = font_name;

      if (fam.isEmpty()) {
         fam = "MS Sans Serif";
      }

      if ( (fam == "MS Sans Serif") && (request.style == QFont::StyleItalic || (-lf.lfHeight > 18 && -lf.lfHeight != 24))) {
         fam = "Arial";    // MS Sans Serif has bearing problems in italic, and does not scale
      }

      if (fam == "Courier" && ! (request.styleStrategy & QFont::PreferBitmap)) {
         fam = "Courier New";
      }

      QString16 tmp = fam.toUtf16();
      memcpy(lf.lfFaceName, tmp.constData(), sizeof(wchar_t) * qMin(tmp.size_storage() + 1, 32));  // 32 = Windows hard-coded

      hfont = CreateFontIndirect(&lf);
      if (! hfont) {
         qErrnoWarning("QFontEngine::loadEngine: CreateFontIndirect failed");
      }

      stockFont = (hfont == 0);
      bool ttf = false;
      int avWidth = 0;
      BOOL res;
      HGDIOBJ oldObj = SelectObject(hdc, hfont);

      TEXTMETRIC tm;
      res = GetTextMetrics(hdc, &tm);

      avWidth = tm.tmAveCharWidth;
      ttf     = tm.tmPitchAndFamily & TMPF_TRUETYPE;
      SelectObject(hdc, oldObj);

      if (! ttf || !useDirectWrite) {
         useDirectWrite = false;

         if (hfont && (!ttf || request.stretch != 100)) {
            DeleteObject(hfont);

            if (! res) {
               qErrnoWarning("QFontEngine::loadEngine: GetTextMetrics failed");
            }

            lf.lfWidth = avWidth * request.stretch / 100;
            hfont = CreateFontIndirect(&lf);

            if (! hfont) {
               qErrnoWarning("QFontEngine::loadEngine: CreateFontIndirect with stretch failed");
            }
         }

         if (hfont == 0) {
            hfont = (HFONT)GetStockObject(ANSI_VAR_FONT);
            stockFont = true;
         }
      }

#if ! defined(QT_NO_DIRECTWRITE)
      else {
         // Default to false for DirectWrite (and re-enable once/if everything turns out okay)
         useDirectWrite = false;

         QFontDatabasePrivate *db = privateDb();

         if (db->directWriteFactory == 0) {
            HRESULT hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory),
                            reinterpret_cast<IUnknown **>(&db->directWriteFactory));

            if (FAILED(hr)) {
               qErrnoWarning("QFontEngine::loadEngine: DWriteCreateFactory failed");

            } else {
               hr = db->directWriteFactory->GetGdiInterop(&db->directWriteGdiInterop);

               if (FAILED(hr)) {
                  qErrnoWarning("QFontEngine::loadEngine: GetGdiInterop failed");
               }
            }
         }

         if (db->directWriteGdiInterop != 0) {
            QString nameSubstitute = fontNameSubstitute(QString::fromWCharArray(lf.lfFaceName));

            QString16 tmp = nameSubstitute.toUtf16();
            memcpy(lf.lfFaceName, tmp.constData(), sizeof(wchar_t) * qMin(tmp.size_storage() + 1, LF_FACESIZE));

            HRESULT hr = db->directWriteGdiInterop->CreateFontFromLOGFONT(&lf, &directWriteFont);

            if (FAILED(hr)) {

#ifndef QT_NO_DEBUG
               qErrnoWarning("QFontEngine::loadEngine: CreateFontFromLOGFONT failed for %ls (0x%lx)", lf.lfFaceName, hr);
#endif

            } else {
               DeleteObject(hfont);
               useDirectWrite = true;
            }
         }
      }
#endif

   }

   QFontEngine *fe = 0;

   if (! useDirectWrite)  {
      QFontEngineWin *few = new QFontEngineWin(font_name, hfont, stockFont, lf);

      if (preferClearTypeAA) {
         few->glyphFormat = QFontEngineGlyphCache::Raster_RGBMask;
      }

      // Also check for OpenType tables when using complex scripts
      // ### TODO: This only works for scripts that require OpenType. More generally
      // for scripts that do not require OpenType we should just look at the list of
      // supported writing systems in the font's OS/2 table.

      if (scriptRequiresOpenType(script)) {
         HB_Face hbFace = few->harfbuzzFace();

         if (! hbFace || ! hbFace->supported_scripts[script]) {
            FM_DEBUG("  OpenType support missing for script\n");
            delete few;
            return 0;
         }
      }

      initFontInfo(few, request, fontHdc, dpi);
      fe = few;
   }

#if ! defined(QT_NO_DIRECTWRITE)
   else {
      QFontDatabasePrivate *db = privateDb();

      IDWriteFontFace *directWriteFontFace = NULL;
      HRESULT hr = directWriteFont->CreateFontFace(&directWriteFontFace);

      if (SUCCEEDED(hr)) {
         QFontEngineDirectWrite *fedw = new QFontEngineDirectWrite(db->directWriteFactory,
               directWriteFontFace, request.pixelSize);

         initFontInfo(fedw, request, dpi, directWriteFont);

         fe = fedw;

      } else {
         qErrnoWarning(hr, "QFontEngine::loadEngine: CreateFontFace failed");
      }
   }

   if (directWriteFont != 0) {
      directWriteFont->Release();
   }
#endif

   if (script == QChar::Script_Common && ! (request.styleStrategy & QFont::NoFontMerging)
         && desc != 0 && !(desc->family->writingSystems[QFontDatabase::Symbol] & QtFontFamily::Supported)) {

      if (tryFonts == nullptr) {
         LANGID lid = GetUserDefaultLangID();

         switch ( lid & 0xff ) {
            case LANG_CHINESE:
               // Chinese (Taiwan)

               if ( lid == 0x0804 ) {
                  // Taiwan
                  tryFonts = &ch_TW_tryFonts;
               } else {
                  tryFonts = &ch_CN_tryFonts;
               }
               break;

            case LANG_JAPANESE:
               tryFonts = &jp_tryFonts;
               break;

            case LANG_KOREAN:
               tryFonts = &kr_tryFonts;
               break;

            default:
               tryFonts = &other_tryFonts;
               break;
         }
      }

      QStringList fm   = QFontDatabase().families();
      QStringList list = family_list;

      for (const QString &item : *tryFonts)  {
         if (fm.contains(item)) {
            list << item;
         }
      }

      QFontEngine *mfe = new QFontEngineMultiWin(fe, list);

      mfe->fontDef = fe->fontDef;
      fe = mfe;
   }

   return fe;
}

QFontEngine *qt_load_font_engine_win(const QFontDef &request)
{
   // From qfont.cpp
   extern int qt_defaultDpi();

   QFontCache::Key key(request, QChar::Script_Common);
   QFontEngine *fe = QFontCache::instance()->findEngine(key);

   if (fe != 0) {
      return fe;
   } else {
      return loadEngine(QChar::Script_Common, request, 0, qt_defaultDpi(), false, 0, QStringList());
   }
}

const char *styleHint(const QFontDef &request)
{
   const char *stylehint = 0;

   switch (request.styleHint) {
      case QFont::SansSerif:
         stylehint = "Arial";
         break;
      case QFont::Serif:
         stylehint = "Times New Roman";
         break;
      case QFont::TypeWriter:
         stylehint = "Courier New";
         break;
      default:
         if (request.fixedPitch) {
            stylehint = "Courier New";
         }
         break;
   }
   return stylehint;
}

static QFontEngine *loadWin(const QFontPrivate *d, int script, const QFontDef &req)
{
   // list of families to try
   QStringList family_list = familyList(req);

   const char *stylehint = styleHint(d->request);
   if (stylehint) {
      family_list << QString::fromLatin1(stylehint);
   }

   // append the default fallback font for the specified script
   // family_list << ... ; ###########

   // add the default family
   QString defaultFamily = QApplication::font().family();
   if (! family_list.contains(defaultFamily)) {
      family_list << defaultFamily;
   }

   // add QFont::defaultFamily() to the list, for compatibility with
   // previous versions
   family_list << QApplication::font().defaultFamily();

   // null family means find the first font matching the specified script
   family_list << QString();

   QtFontDesc desc;
   QFontEngine *fe = 0;
   QList<int> blacklistedFamilies;

   while (!fe) {
      for (int i = 0; i < family_list.size(); ++i) {
         QString family, foundry;
         parseFontName(family_list.at(i), foundry, family);
         FM_DEBUG("loadWin: >>>>>>>>>>>>>>trying to match '%s'", family.toLatin1().data());
         QT_PREPEND_NAMESPACE(match)(script, req, family, foundry, -1, &desc, blacklistedFamilies);
         if (desc.family) {
            break;
         }
      }
      if (!desc.family) {
         break;
      }
      fe = loadEngine(script, req, d->hdc, d->dpi, d->rawMode, &desc, family_list);
      if (!fe) {
         blacklistedFamilies.append(desc.familyIndex);
      }
   }
   return fe;
}

void QFontDatabase::load(const QFontPrivate *d, int script)
{
   // sanity checks
   if (!qApp) {
      qWarning("QFontDatabase::load: Must construct QApplication first");
   }
   Q_ASSERT(script >= 0 && script < QChar::ScriptCount);

   // normalize the request to get better caching
   QFontDef req = d->request;
   if (req.pixelSize <= 0) {
      req.pixelSize = floor((100.0 * req.pointSize * d->dpi) / 72. + 0.5) / 100;
   }
   if (req.pixelSize < 1) {
      req.pixelSize = 1;
   }
   if (req.weight == 0) {
      req.weight = QFont::Normal;
   }
   if (req.stretch == 0) {
      req.stretch = 100;
   }

   QFontCache::Key key(req, d->rawMode ? QChar::Script_Common : script, d->screen);
   if (!d->engineData) {
      getEngineData(d, key);
   }

   // the cached engineData could have already loaded the engine we want
   if (d->engineData->engines[script]) {
      return;
   }

   QFontEngine *fe = QFontCache::instance()->findEngine(key);

   // set it to the actual pointsize, so QFontInfo will do the right thing
   if (req.pointSize < 0) {
      req.pointSize = req.pixelSize * 72. / d->dpi;
   }

   if (!fe) {
      if (qt_enable_test_font && req.family == QLatin1String("__Qt__Box__Engine__")) {
         fe = new QTestFontEngine(req.pixelSize);
         fe->fontDef = req;
      } else {
         QMutexLocker locker(fontDatabaseMutex());
         if (!privateDb()->count) {
            initializeDb();
         }
         fe = loadWin(d, script, req);
      }
      if (!fe) {
         fe = new QFontEngineBox(req.pixelSize);
         fe->fontDef = QFontDef();
      }
   }
   d->engineData->engines[script] = fe;
   fe->ref.ref();
   QFontCache::instance()->insertEngine(key, fe);
}

#if !defined(FR_PRIVATE)
#define FR_PRIVATE 0x10
#endif

typedef int (WINAPI *PtrAddFontResourceExW)(LPCWSTR, DWORD, PVOID);
typedef HANDLE (WINAPI *PtrAddFontMemResourceEx)(PVOID, DWORD, PVOID, DWORD *);
typedef BOOL (WINAPI *PtrRemoveFontResourceExW)(LPCWSTR, DWORD, PVOID);
typedef BOOL (WINAPI *PtrRemoveFontMemResourceEx)(HANDLE);

static QList<quint32> getTrueTypeFontOffsets(const uchar *fontData)
{
   QList<quint32> offsets;
   const quint32 headerTag = *reinterpret_cast<const quint32 *>(fontData);
   if (headerTag != MAKE_TAG('t', 't', 'c', 'f')) {
      if (headerTag != MAKE_TAG(0, 1, 0, 0)
            && headerTag != MAKE_TAG('O', 'T', 'T', 'O')
            && headerTag != MAKE_TAG('t', 'r', 'u', 'e')
            && headerTag != MAKE_TAG('t', 'y', 'p', '1')) {
         return offsets;
      }
      offsets << 0;
      return offsets;
   }
   const quint32 numFonts = qFromBigEndian<quint32>(fontData + 8);
   for (uint i = 0; i < numFonts; ++i) {
      offsets << qFromBigEndian<quint32>(fontData + 12 + i * 4);
   }
   return offsets;
}

static void getFontTable(const uchar *fileBegin, const uchar *data, quint32 tag, const uchar **table, quint32 *length)
{
   const quint16 numTables = qFromBigEndian<quint16>(data + 4);
   for (uint i = 0; i < numTables; ++i) {
      const quint32 offset = 12 + 16 * i;
      if (*reinterpret_cast<const quint32 *>(data + offset) == tag) {
         *table = fileBegin + qFromBigEndian<quint32>(data + offset + 8);
         *length = qFromBigEndian<quint32>(data + offset + 12);
         return;
      }
   }
   *table = 0;
   *length = 0;
   return;
}

static void getFamiliesAndSignatures(const QByteArray &fontData, QFontDatabasePrivate::ApplicationFont *appFont)
{
   const uchar *data = reinterpret_cast<const uchar *>(fontData.constData());

   QList<quint32> offsets = getTrueTypeFontOffsets(data);
   if (offsets.isEmpty()) {
      return;
   }

   for (int i = 0; i < offsets.count(); ++i) {
      const uchar *font = data + offsets.at(i);
      const uchar *table;
      quint32 length;
      getFontTable(data, font, MAKE_TAG('n', 'a', 'm', 'e'), &table, &length);
      if (!table) {
         continue;
      }
      QString name = getEnglishName(table, length);
      if (name.isEmpty()) {
         continue;
      }

      appFont->families << name;
      FONTSIGNATURE signature;
      getFontTable(data, font, MAKE_TAG('O', 'S', '/', '2'), &table, &length);
      if (table && length >= 86) {
         // See also qfontdatabase_mac.cpp, offsets taken from OS/2 table in the TrueType spec
         signature.fsUsb[0] = qFromBigEndian<quint32>(table + 42);
         signature.fsUsb[1] = qFromBigEndian<quint32>(table + 46);
         signature.fsUsb[2] = qFromBigEndian<quint32>(table + 50);
         signature.fsUsb[3] = qFromBigEndian<quint32>(table + 54);

         signature.fsCsb[0] = qFromBigEndian<quint32>(table + 78);
         signature.fsCsb[1] = qFromBigEndian<quint32>(table + 82);
      } else {
         memset(&signature, 0, sizeof(signature));
      }
      appFont->signatures << signature;
   }
}

static void registerFont(QFontDatabasePrivate::ApplicationFont *fnt)
{
   if (!fnt->data.isEmpty()) {

      PtrAddFontMemResourceEx ptrAddFontMemResourceEx = (PtrAddFontMemResourceEx)QSystemLibrary::resolve(
               QLatin1String("gdi32"),
               "AddFontMemResourceEx");
      if (!ptrAddFontMemResourceEx) {
         return;
      }

      getFamiliesAndSignatures(fnt->data, fnt);
      if (fnt->families.isEmpty()) {
         return;
      }

      DWORD dummy = 0;
      HANDLE handle = ptrAddFontMemResourceEx((void *)fnt->data.constData(), fnt->data.size(), 0, &dummy);
      if (handle == 0) {
         return;
      }

      fnt->handle = handle;
      fnt->data = QByteArray();
      fnt->memoryFont = true;

   } else {
      QFile f(fnt->fileName);
      if (!f.open(QIODevice::ReadOnly)) {
         return;
      }

      QByteArray data = f.readAll();
      f.close();
      getFamiliesAndSignatures(data, fnt);

      PtrAddFontResourceExW ptrAddFontResourceExW = (PtrAddFontResourceExW)QSystemLibrary::resolve(QLatin1String("gdi32"),
            "AddFontResourceExW");

      std::wstring tmp = fnt->fileName.toStdWString();

      if (! ptrAddFontResourceExW || ptrAddFontResourceExW(&tmp[0], FR_PRIVATE, 0) == 0) {
         return;
      }

      fnt->memoryFont = false;
   }
}

bool QFontDatabase::removeApplicationFont(int handle)
{
   QMutexLocker locker(fontDatabaseMutex());

   QFontDatabasePrivate *db = privateDb();
   if (handle < 0 || handle >= db->applicationFonts.count()) {
      return false;
   }

   const QFontDatabasePrivate::ApplicationFont font = db->applicationFonts.at(handle);
   db->applicationFonts[handle] = QFontDatabasePrivate::ApplicationFont();

   if (font.memoryFont) {

      PtrRemoveFontMemResourceEx ptrRemoveFontMemResourceEx =
                  (PtrRemoveFontMemResourceEx)QSystemLibrary::resolve("gdi32", "RemoveFontMemResourceEx");

      if (! ptrRemoveFontMemResourceEx || ! ptrRemoveFontMemResourceEx(font.handle)) {
         return false;
      }

   } else {

      PtrRemoveFontResourceExW ptrRemoveFontResourceExW =
                  (PtrRemoveFontResourceExW)QSystemLibrary::resolve("gdi32", "RemoveFontResourceExW");

      std::wstring tmp = font.fileName.toStdWString();

      if (! ptrRemoveFontResourceExW || ! ptrRemoveFontResourceExW(&tmp[0], FR_PRIVATE, 0)) {
         return false;
      }
   }

   db->invalidate();

   return true;
}

bool QFontDatabase::removeAllApplicationFonts()
{
   QMutexLocker locker(fontDatabaseMutex());

   QFontDatabasePrivate *db = privateDb();

   for (int i = 0; i < db->applicationFonts.count(); ++i) {
      if (! removeApplicationFont(i)) {
         return false;
      }
   }

   return true;
}

bool QFontDatabase::supportsThreadedFontRendering()
{
   return true;
}

QT_END_NAMESPACE
