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

#include <qwin_fontdatabase.h>

#include <qapplication.h>
#include <qdebug.h>
#include <qendian.h>
#include <qfont.h>
#include <qmath.h>
#include <qtextcodec.h>
#include <qthreadstorage.h>
#include <qwin_additional.h>
#include <qwin_context.h>
#include <qwin_fontdatabase_ft.h>
#include <qwin_fontengine.h>

#include <qhighdpiscaling_p.h>
#include <qsystemlibrary_p.h>

#include <wchar.h>

static constexpr const int PlatformId_Unicode   = 0;
static constexpr const int PlatformId_Apple     = 1;
static constexpr const int PlatformId_Microsoft = 3;

static constexpr const int NameRecordSize       = 12;
static constexpr const int FamilyId             = 1;
static constexpr const int MS_LangIdEnglish     = 0x009;

// Helper classes for creating font engines directly from font data
namespace {

#pragma pack(1)

// Common structure for all formats of the "name" table
struct NameTable {
   quint16 format;
   quint16 count;
   quint16 stringOffset;
};

struct NameRecord {
   quint16 platformID;
   quint16 encodingID;
   quint16 languageID;
   quint16 nameID;
   quint16 length;
   quint16 offset;
};

struct OffsetSubTable {
   quint32 scalerType;
   quint16 numTables;
   quint16 searchRange;
   quint16 entrySelector;
   quint16 rangeShift;
};

struct TableDirectory {
   quint32 identifier;
   quint32 checkSum;
   quint32 offset;
   quint32 length;
};

struct OS2Table {
   quint16 version;
   qint16  avgCharWidth;
   quint16 weightClass;
   quint16 widthClass;
   quint16 type;
   qint16  subscriptXSize;
   qint16  subscriptYSize;
   qint16  subscriptXOffset;
   qint16  subscriptYOffset;
   qint16  superscriptXSize;
   qint16  superscriptYSize;
   qint16  superscriptXOffset;
   qint16  superscriptYOffset;
   qint16  strikeOutSize;
   qint16  strikeOutPosition;
   qint16  familyClass;
   quint8  panose[10];
   quint32 unicodeRanges[4];
   quint8  vendorID[4];
   quint16 selection;
   quint16 firstCharIndex;
   quint16 lastCharIndex;
   qint16  typoAscender;
   qint16  typoDescender;
   qint16  typoLineGap;
   quint16 winAscent;
   quint16 winDescent;
   quint32 codepageRanges[2];
   qint16  height;
   qint16  capHeight;
   quint16 defaultChar;
   quint16 breakChar;
   quint16 maxContext;
};

#   pragma pack()



class EmbeddedFont
{
 public:
   EmbeddedFont(const QByteArray &fontData) : m_fontData(fontData) {}

   QString changeFamilyName(const QString &newFamilyName);
   QByteArray data() const {
      return m_fontData;
   }

   TableDirectory *tableDirectoryEntry(const QByteArray &tagName);
   QString familyName(TableDirectory *nameTableDirectory = nullptr);

 private:
   QByteArray m_fontData;
};

TableDirectory *EmbeddedFont::tableDirectoryEntry(const QByteArray &tagName)
{
   Q_ASSERT(tagName.size() == 4);

   quint32 tagId = *(reinterpret_cast<const quint32 *>(tagName.constData()));
   const size_t fontDataSize = m_fontData.size();

   if (fontDataSize < sizeof(OffsetSubTable)) {
      return nullptr;
   }

   OffsetSubTable *offsetSubTable = reinterpret_cast<OffsetSubTable *>(m_fontData.data());
   TableDirectory *tableDirectory = reinterpret_cast<TableDirectory *>(offsetSubTable + 1);

   const size_t tableCount = qFromBigEndian<quint16>(offsetSubTable->numTables);
   if (fontDataSize < sizeof(OffsetSubTable) + sizeof(TableDirectory) * tableCount) {
      return nullptr;
   }

   TableDirectory *tableDirectoryEnd = tableDirectory + tableCount;

   for (TableDirectory *entry = tableDirectory; entry < tableDirectoryEnd; ++entry) {
      if (entry->identifier == tagId) {
         return entry;
      }
   }

   return nullptr;
}

QString EmbeddedFont::familyName(TableDirectory *nameTableDirectoryEntry)
{
   QString retval;

   if (nameTableDirectoryEntry == nullptr) {
      nameTableDirectoryEntry = tableDirectoryEntry("name");
   }

   if (nameTableDirectoryEntry != nullptr) {
      quint32 offset = qFromBigEndian<quint32>(nameTableDirectoryEntry->offset);

      if (quint32(m_fontData.size()) < offset + sizeof(NameTable)) {
         return retval;
      }

      NameTable *nameTable   = reinterpret_cast<NameTable *>(m_fontData.data() + offset);
      NameRecord *nameRecord = reinterpret_cast<NameRecord *>(nameTable + 1);

      quint16 nameTableCount = qFromBigEndian<quint16>(nameTable->count);
      if (quint32(m_fontData.size()) < offset + sizeof(NameRecord) * nameTableCount) {
         return retval;
      }

      QTextCodec *codec = QTextCodec::codecForName("UTF-16BE");

      for (int i = 0; i < nameTableCount; ++i, ++nameRecord) {
         if (qFromBigEndian<quint16>(nameRecord->nameID) == 1 && qFromBigEndian<quint16>(nameRecord->platformID) == 3
            && qFromBigEndian<quint16>(nameRecord->languageID) == 0x0409) {

            quint16 stringOffset = qFromBigEndian<quint16>(nameTable->stringOffset);
            quint16 nameOffset   = qFromBigEndian<quint16>(nameRecord->offset);
            quint16 nameLength   = qFromBigEndian<quint16>(nameRecord->length);

            if (quint32(m_fontData.size()) < offset + stringOffset + nameOffset + nameLength) {
               return retval;
            }

            const char *ptr  = reinterpret_cast<const char *>(nameTable) + stringOffset + nameOffset;
            retval = codec->toUnicode(ptr, nameLength);

            break;
         }
      }
   }

   return retval;
}

QString EmbeddedFont::changeFamilyName(const QString &newFamilyName)
{
   TableDirectory *nameTableDirectoryEntry = tableDirectoryEntry("name");

   if (nameTableDirectoryEntry == nullptr) {
      return QString();
   }

   QString oldFamilyName = familyName(nameTableDirectoryEntry);

   // reserve size for name table header, five required name records and string
   const int requiredRecordCount = 5;
   quint16 nameIds[requiredRecordCount] = { 1, 2, 3, 4, 6 };

   int sizeOfHeader = sizeof(NameTable) + sizeof(NameRecord) * requiredRecordCount;
   int newFamilyNameSize = newFamilyName.size() * int(sizeof(quint16));

   const QString regularString = QString::fromLatin1("Regular");
   int regularStringSize = regularString.size() * int(sizeof(quint16));

   // align table size of table to 32 bits (pad with 0)
   int fullSize = ((sizeOfHeader + newFamilyNameSize + regularStringSize) & ~3) + 4;
   QByteArray newNameTable(fullSize, char(0));

   QTextCodec *codec = QTextCodec::codecForName("UTF-16BE");

   {
      NameTable *nameTable = reinterpret_cast<NameTable *>(newNameTable.data());

      nameTable->count        = qbswap<quint16>(requiredRecordCount);
      nameTable->stringOffset = qbswap<quint16>(sizeOfHeader);

      NameRecord *nameRecord = reinterpret_cast<NameRecord *>(nameTable + 1);

      for (int i = 0; i < requiredRecordCount; ++i, nameRecord++) {
         nameRecord->nameID     = qbswap<quint16>(nameIds[i]);
         nameRecord->encodingID = qbswap<quint16>(1);
         nameRecord->languageID = qbswap<quint16>(0x0409);
         nameRecord->platformID = qbswap<quint16>(3);
         nameRecord->length     = qbswap<quint16>(newFamilyNameSize);

         // special case for sub-family
         if (nameIds[i] == 4) {
            nameRecord->offset = qbswap<quint16>(newFamilyNameSize);
            nameRecord->length = qbswap<quint16>(regularStringSize);
         }
      }

      // nameRecord now points to string data
      char *stringStorage = reinterpret_cast<char *>(nameRecord);
      QByteArray tmp     = codec->fromUnicode(newFamilyName);

      std::memcpy(stringStorage, tmp.constData(), tmp.size());

      stringStorage += tmp.size();
      tmp = codec->fromUnicode(regularString);

      std::memcpy(stringStorage, tmp.constData(), tmp.size());
   }

   quint32 *p = reinterpret_cast<quint32 *>(newNameTable.data());
   quint32 *tableEnd = reinterpret_cast<quint32 *>(newNameTable.data() + fullSize);

   quint32 checkSum = 0;

   while (p < tableEnd) {
      checkSum += qFromBigEndian<quint32>(*(p++));
   }

   nameTableDirectoryEntry->checkSum = qbswap<quint32>(checkSum);
   nameTableDirectoryEntry->offset = qbswap<quint32>(m_fontData.size());
   nameTableDirectoryEntry->length = qbswap<quint32>(fullSize);

   m_fontData.append(newNameTable);

   return oldFamilyName;
}

}   // end namespace

QWindowsFontEngineData::QWindowsFontEngineData()
   : clearTypeEnabled(false), fontSmoothingGamma(QWindowsFontDatabase::fontSmoothingGamma())
{
   // from qapplication_win.cpp
   UINT result = 0;
   if (SystemParametersInfo(SPI_GETFONTSMOOTHINGTYPE, 0, &result, 0)) {
      clearTypeEnabled = (result == FE_FONTSMOOTHINGCLEARTYPE);
   }

   const qreal gray_gamma = 2.31;
   for (int i = 0; i < 256; ++i) {
      pow_gamma[i] = uint(qRound(qPow(i / qreal(255.), gray_gamma) * 2047));
   }

   HDC displayDC = GetDC(nullptr);
   hdc = CreateCompatibleDC(displayDC);
   ReleaseDC(nullptr, displayDC);
}

QWindowsFontEngineData::~QWindowsFontEngineData()
{
   if (hdc) {
      DeleteDC(hdc);
   }
}

qreal QWindowsFontDatabase::fontSmoothingGamma()
{
   int winSmooth;
   qreal result = 1;

   if (SystemParametersInfo(0x200C /* SPI_GETFONTSMOOTHINGCONTRAST */, 0, &winSmooth, 0)) {
      result = qreal(winSmooth) / qreal(1000.0);
   }

   // Safeguard ourselves against corrupt registry values...
   if (result > 5 || result < 1) {
      result = qreal(1.4);
   }
   return result;
}

QDebug operator<<(QDebug debug, const QFontDef &def)
{
   QDebugStateSaver saver(debug);
   debug.nospace();
   debug.noquote();

   debug << "QFontDef(Family=\"" << def.family << '"';

   if (! def.styleName.isEmpty()) {
      debug << ", stylename=" << def.styleName;
   }

   debug << ", pointsize=" << def.pointSize << ", pixelsize=" << def.pixelSize
     << ", styleHint=" << def.styleHint << ", weight=" << def.weight
     << ", stretch=" << def.stretch << ", hintingPreference="
     << def.hintingPreference << ')';

   return debug;
}

QDebug operator<<(QDebug debug, const LOGFONT &lf)
{
   QDebugStateSaver saver(debug);
   debug.nospace();
   debug.noquote();

   debug << "LOGFONT(\"" << QString::fromStdWString(std::wstring(lf.lfFaceName))
      << "\", lfWidth=" << lf.lfWidth << ", lfHeight=" << lf.lfHeight << ')';

   return debug;
}

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

#ifdef MAKE_TAG
#undef MAKE_TAG
#endif

// GetFontData expects the tags in little endian ;(
#define MAKE_TAG(ch1, ch2, ch3, ch4) (\
    (((quint32)(ch4)) << 24) | \
    (((quint32)(ch3)) << 16) | \
    (((quint32)(ch2)) << 8) |  \
    ((quint32)(ch1)) \
    )

bool localizedName(const QString &name)
{
   for (auto ch : name) {
      if (ch.unicode() >= 0x100) {
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

   // get the name table
   quint16 count;
   quint16 string_offset;
   const unsigned char *names;

   int microsoft_id = -1;
   int apple_id = -1;
   int unicode_id = -1;

   if (getUShort(table) != 0) {
      return i18n_name;
   }

   count = getUShort(table + 2);
   string_offset = getUShort(table + 4);
   names = table + 6;

   if (string_offset >= bytes || 6 + count * NameRecordSize > string_offset) {
     return i18n_name;
   }

   for (int i = 0; i < count; ++i) {
      // search for the correct name entry

      quint16 platform_id = getUShort(names + i * NameRecordSize);
      quint16 encoding_id = getUShort(names + 2 + i * NameRecordSize);
      quint16 language_id = getUShort(names + 4 + i * NameRecordSize);
      quint16 name_id     = getUShort(names + 6 + i * NameRecordSize);

      if (name_id != FamilyId) {
         continue;
      }

      quint16 length = getUShort(names + 8  + i * NameRecordSize);
      quint16 offset = getUShort(names + 10 + i * NameRecordSize);

      if (DWORD(string_offset + offset + length) >= bytes) {
         continue;
      }

      if ((platform_id == PlatformId_Microsoft && (encoding_id == 0 || encoding_id == 1))
               && (language_id & 0x3ff) == MS_LangIdEnglish && microsoft_id == -1) {
         microsoft_id = i;

      } else if (platform_id == PlatformId_Unicode && encoding_id < 4 && unicode_id == -1) {
         // not sure if encoding id 4 for Unicode is utf16 or ucs4
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
         quint16 length = getUShort(names + 8  + id * NameRecordSize);
         quint16 offset = getUShort(names + 10 + id * NameRecordSize);

         if (unicode) {
            // utf16

            length /= 2;
            i18n_name.clear();

            const unsigned char *string = table + string_offset + offset;
            std::wstring tmp;

            for (int i = 0; i < length; ++i) {
               tmp.push_back(wchar_t(getUShort(string + 2 * i)));
            }

            i18n_name = QString::fromStdWString(tmp);

         } else {
            // Apple Roman
            i18n_name.clear();
            const unsigned char *string = table + string_offset + offset;

            for (int i = 0; i < length; ++i) {
               i18n_name.append(char(string[i]));
            }
         }
      }
   }

   return i18n_name;
}

QString getEnglishName(const QString &familyName)
{
   QString i18n_name;

   HDC hdc = GetDC(nullptr);
   LOGFONT lf;

   memset(&lf, 0, sizeof(LOGFONT));

   std::wstring tmp = familyName.toStdWString();
   memcpy(lf.lfFaceName, tmp.data(), sizeof(wchar_t) * qMin(static_cast<std::wstring::size_type>(LF_FACESIZE - 1), tmp.size()));

   lf.lfCharSet = DEFAULT_CHARSET;
   HFONT hfont = CreateFontIndirect(&lf);

   if (! hfont) {
      ReleaseDC(nullptr, hdc);
      return QString();
   }

   HGDIOBJ oldobj = SelectObject(hdc, hfont);

   const DWORD name_tag = MAKE_TAG('n', 'a', 'm', 'e');

   // get the name table
   unsigned char *table = nullptr;

   DWORD bytes = GetFontData(hdc, name_tag, 0, nullptr, 0);

   if (bytes == GDI_ERROR) {
      // int err = GetLastError();
      goto error;
   }

   table = new unsigned char[bytes];
   GetFontData(hdc, name_tag, 0, table, bytes);

   if (bytes == GDI_ERROR) {
      goto error;
   }

   i18n_name = getEnglishName(table, bytes);

error:
   delete [] table;
   SelectObject(hdc, oldobj);
   DeleteObject(hfont);
   ReleaseDC(nullptr, hdc);

   return i18n_name;
}

static bool addFontToDatabase(const QString &familyName, uchar charSet,
   const TEXTMETRIC *textmetric, const FONTSIGNATURE *signature, int type, bool registerAlias)
{
   // the "@family" fonts are the same as "family" so ignore them
   if (familyName.isEmpty() || familyName.at(0) == '@' || familyName.startsWith("WST_")) {
      return false;
   }

   static const int SMOOTH_SCALABLE = 0xffff;
   const QString foundryName;                   // No such concept

   const bool fixed    = !(textmetric->tmPitchAndFamily & TMPF_FIXED_PITCH);
   const bool ttf      = (textmetric->tmPitchAndFamily & TMPF_TRUETYPE);
   const bool scalable = textmetric->tmPitchAndFamily & (TMPF_VECTOR | TMPF_TRUETYPE);
   const int size      = scalable ? SMOOTH_SCALABLE : textmetric->tmHeight;

   const QFont::Style style = textmetric->tmItalic ? QFont::StyleItalic : QFont::StyleNormal;
   const bool antialias = false;

   const QFont::Weight weight   = QPlatformFontDatabase::weightFromInteger(textmetric->tmWeight);
   const QFont::Stretch stretch = QFont::Unstretched;

#if defined(CS_SHOW_DEBUG_PLATFORM)
   QString message;
   QTextStream str(&message);

   str << "addFontToDatabase() FamilyName = " << familyName << " CharSet = " << charSet << " TTF = " << ttf;

   if (type & DEVICE_FONTTYPE) {
      str << " DEVICE";
   }

   if (type & RASTER_FONTTYPE) {
      str << " RASTER";
   }

   if (type & TRUETYPE_FONTTYPE) {
      str << " TRUETYPE";
   }

   str << "\n  "
       << " Scalable = " << scalable << " Size = " << size
       << " Style = " << style << " Weight =" << weight << " Stretch = " << stretch;

   qDebug() << message;
#endif

   QString englishName;
   if (registerAlias && ttf && localizedName(familyName)) {
      englishName = getEnglishName(familyName);
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

      // Hack to work around problem with Thai text on Windows 7. Segoe UI contains the symbol for Baht,
      // and Windows thus reports that it supports the Thai script. Since it is the default UI font on
      // this platform, most widgets will be unable to display Thai text by default. As a temporary work
      // around we special case Segoe UI and remove the Thai script from its list of supported writing systems.

      if (writingSystems.supported(QFontDatabase::Thai) && familyName == "Segoe UI") {
         writingSystems.setSupported(QFontDatabase::Thai, false);
      }

   } else {
      const QFontDatabase::WritingSystem ws = writingSystemFromCharSet(charSet);
      if (ws != QFontDatabase::Any) {
         writingSystems.setSupported(ws);
      }
   }

   QPlatformFontDatabase::registerFont(familyName, QString(), foundryName, weight,
      style, stretch, antialias, scalable, size, fixed, writingSystems, nullptr);

   // add fonts windows can generate for us
   if (weight <= QFont::DemiBold)
      QPlatformFontDatabase::registerFont(familyName, QString(), foundryName, QFont::Bold,
         style, stretch, antialias, scalable, size, fixed, writingSystems, nullptr);

   if (style != QFont::StyleItalic)
      QPlatformFontDatabase::registerFont(familyName, QString(), foundryName, weight,
         QFont::StyleItalic, stretch, antialias, scalable, size, fixed, writingSystems, nullptr);

   if (weight <= QFont::DemiBold && style != QFont::StyleItalic)
      QPlatformFontDatabase::registerFont(familyName, QString(), foundryName, QFont::Bold,
         QFont::StyleItalic, stretch, antialias, scalable, size, fixed, writingSystems, nullptr);

   if (! englishName.isEmpty()) {
      QPlatformFontDatabase::registerAliasToFontFamily(familyName, englishName);
   }

   return true;
}

static bool storeFont_callback = false;

static int QT_WIN_CALLBACK storeFont(const LOGFONT *logFont, const TEXTMETRIC *textmetric,
   DWORD type, LPARAM lParam)
{
   storeFont_callback = true;

   const ENUMLOGFONTEX *f   = reinterpret_cast<const ENUMLOGFONTEX *>(logFont);

   const QString familyName = QString::fromStdWString(std::wstring(f->elfLogFont.lfFaceName));
   const uchar charSet      = f->elfLogFont.lfCharSet;
   const bool registerAlias = bool(lParam);


   // NEWTEXTMETRICEX (passed for TT fonts) is a NEWTEXTMETRIC, which according
   // to the documentation is identical to a TEXTMETRIC except for the last four
   // members, which we do not use anyway

   const FONTSIGNATURE *signature = nullptr;
   if (type & TRUETYPE_FONTTYPE) {
      signature = &reinterpret_cast<const NEWTEXTMETRICEX *>(textmetric)->ntmFontSig;
   }

   // will call registerFont then q_registerFont ( which alters m_popluated mem var in gui\qfontdatabase.cpp )
   addFontToDatabase(familyName, charSet, textmetric, signature, type, registerAlias);

   // keep on enumerating
   return 1;
}

void QWindowsFontDatabase::populateFamily(const QString &familyName, bool registerAlias)
{
   if (familyName.size() >= LF_FACESIZE) {
      qWarning() << "QWindowsFontDatabase::populateFamily() Unable to enumerate font family =" << familyName;
      return;
   }

   HDC dummy = GetDC(nullptr);
   LOGFONT lf;
   lf.lfCharSet = DEFAULT_CHARSET;

   std::wstring tmp = familyName.toStdWString();
   memcpy(lf.lfFaceName, tmp.c_str(), tmp.size() * 2);

   lf.lfFaceName[tmp.size()] = L'\0';
   lf.lfPitchAndFamily       = 0;

   // reset
   storeFont_callback = false;

   EnumFontFamiliesExW(dummy, &lf, storeFont, LPARAM(registerAlias), 0);

   if (! storeFont_callback) {
      Q_ASSERT_X(false, "QWindowsfontdatabase() No font families were found for ", csPrintable(familyName));
   }

   ReleaseDC(nullptr, dummy);
}

void QWindowsFontDatabase::populateFamily(const QString &familyName)
{
   populateFamily(familyName, false);
}

namespace {
// Context for enumerating system fonts, records whether the default font has been encountered,
// which is normally not enumerated by EnumFontFamiliesEx()

struct PopulateFamiliesContext {
   PopulateFamiliesContext(const QString &f)
      : m_systemDefaultFont(f), m_seenSystemDefaultFont(false)
   {
   }

   QString m_systemDefaultFont;
   bool m_seenSystemDefaultFont;
};
}   // end namespace

static int QT_WIN_CALLBACK populateFontFamilies(const LOGFONT *logFont, const TEXTMETRIC *textmetric,
   DWORD, LPARAM lparam)
{
   // the "@family" fonts are just the same as "family". Ignore them.
   const ENUMLOGFONTEX *f   = reinterpret_cast<const ENUMLOGFONTEX *>(logFont);
   const wchar_t *faceNameW = f->elfLogFont.lfFaceName;

   if (faceNameW[0] && faceNameW[0] != L'@' && wcsncmp(faceNameW, L"WST_", 4)) {
      const QString faceName = QString::fromStdWString(std::wstring(faceNameW));

      QPlatformFontDatabase::registerFontFamily(faceName);
      PopulateFamiliesContext *context = reinterpret_cast<PopulateFamiliesContext *>(lparam);

      if (! context->m_seenSystemDefaultFont && faceName == context->m_systemDefaultFont) {
         context->m_seenSystemDefaultFont = true;
      }

      // Register current font's english name as alias
      const bool ttf = textmetric->tmPitchAndFamily & TMPF_TRUETYPE;
      if (ttf && localizedName(faceName)) {
         const QString englishName = getEnglishName(faceName);

         if (! englishName.isEmpty()) {
            QPlatformFontDatabase::registerAliasToFontFamily(faceName, englishName);
            // Check whether the system default font name is an alias of the current font family name,
            // as on Chinese Windows, where the system font "SimSun" is an alias to a font registered under a local name
            if (! context->m_seenSystemDefaultFont && englishName == context->m_systemDefaultFont) {
               context->m_seenSystemDefaultFont = true;
            }
         }
      }
   }

   return 1; // continue
}

void QWindowsFontDatabase::populateFontDatabase()
{
   removeApplicationFonts();

   HDC dummy = GetDC(nullptr);

   LOGFONT lf;
   lf.lfCharSet        = DEFAULT_CHARSET;
   lf.lfFaceName[0]    = 0;
   lf.lfPitchAndFamily = 0;

   PopulateFamiliesContext context(QWindowsFontDatabase::systemDefaultFont().family());
   EnumFontFamiliesEx(dummy, &lf, populateFontFamilies, reinterpret_cast<LPARAM>(&context), 0);
   ReleaseDC(nullptr, dummy);

   // Work around EnumFontFamiliesEx() not listing the system font.
   if (! context.m_seenSystemDefaultFont) {
      QPlatformFontDatabase::registerFontFamily(context.m_systemDefaultFont);
   }
}

using QWindowsFontEngineDataPtr = QSharedPointer<QWindowsFontEngineData>;
using FontEngineThreadLocalData = QThreadStorage<QWindowsFontEngineDataPtr>;

static FontEngineThreadLocalData *fontEngineThreadLocalData()
{
   static FontEngineThreadLocalData retval;
   return &retval;
}

QSharedPointer<QWindowsFontEngineData> sharedFontData()
{
   FontEngineThreadLocalData *data = fontEngineThreadLocalData();
   if (!data->hasLocalData()) {
      data->setLocalData(QSharedPointer<QWindowsFontEngineData>(new QWindowsFontEngineData));
   }

   return data->localData();
}

extern Q_GUI_EXPORT bool qt_needs_a8_gamma_correction;

QWindowsFontDatabase::QWindowsFontDatabase()
{
   const QWindowsFontEngineDataPtr data = sharedFontData();

#if defined(CS_SHOW_DEBUG_PLATFORM)
   qDebug() << "QWindowsFontDatabase() Clear type = " << data->clearTypeEnabled << " Gamma = " << data->fontSmoothingGamma;
#endif

   qt_needs_a8_gamma_correction = true;
}

QWindowsFontDatabase::~QWindowsFontDatabase()
{
   removeApplicationFonts();
}

// emerald (multi)
QFontEngineMulti *QWindowsFontDatabase::fontEngineMulti(QFontEngine *fontEngine, QChar::Script script)
{
   return new QWindowsMultiFontEngine(fontEngine, script);
}

QFontEngine *QWindowsFontDatabase::fontEngine(const QFontDef &fontDef, void *handle)
{
   (void) handle;

   QFontEngine *fe = QWindowsFontDatabase::createEngine(fontDef,
         QWindowsContext::instance()->defaultDPI(), sharedFontData());

   return fe;
}

QFontEngine *QWindowsFontDatabase::fontEngine(const QByteArray &fontData, qreal pixelSize,
      QFont::HintingPreference hintingPreference)
{
   EmbeddedFont font(fontData);
   QFontEngine *fontEngine = nullptr;

   GUID guid;
   CoCreateGuid(&guid);

   QString uniqueFamilyName = 'f' + QString::number(guid.Data1, 36) + '-'
      + QString::number(guid.Data2, 36) + '-' + QString::number(guid.Data3, 36) + '-'
      + QString::number(*reinterpret_cast<quint64 *>(guid.Data4), 36);

   QString actualFontName = font.changeFamilyName(uniqueFamilyName);

   if (actualFontName.isEmpty()) {
      qWarning("QWindowsFontDatabase::fontEngine() Unable to change family name of font");
      return nullptr;
   }

   DWORD count = 0;

   QByteArray newFontData = font.data();
   HANDLE fontHandle = AddFontMemResourceEx(const_cast<char *>(newFontData.constData()),
         DWORD(newFontData.size()), nullptr, &count);

   if (count == 0 && fontHandle != nullptr) {
      RemoveFontMemResourceEx(fontHandle);
      fontHandle = nullptr;
   }

   if (fontHandle == nullptr) {
      qWarning("QWindowsFontDatabase::fontEngine() AddFontMemResourceEx failed");

   } else {
      QFontDef request;
      request.family = uniqueFamilyName;
      request.pixelSize = pixelSize;
      request.styleStrategy = QFont::PreferMatch;
      request.hintingPreference = hintingPreference;

      fontEngine = QWindowsFontDatabase::createEngine(request, QWindowsContext::instance()->defaultDPI(), sharedFontData());

      if (fontEngine) {
         if (request.family != fontEngine->fontDef.family) {
            qWarning("QWindowsFontDatabase::fontEngine() Failed to load font, using fallback instead: %s ",
               csPrintable(fontEngine->fontDef.family));

            if (fontEngine->m_refCount.load() == 0) {
               delete fontEngine;
            }

            fontEngine = nullptr;

         } else {
            Q_ASSERT(fontEngine->m_refCount.load() == 0);

            // Override the generated font name
            static_cast<QWindowsFontEngine *>(fontEngine)->setUniqueFamilyName(uniqueFamilyName);
            fontEngine->fontDef.family = actualFontName;
            UniqueFontData uniqueData;
            uniqueData.handle = fontHandle;
            uniqueData.refCount.ref();
            m_uniqueFontData[uniqueFamilyName] = uniqueData;
         }

      } else {
         RemoveFontMemResourceEx(fontHandle);
      }
   }


   // Get style and weight info
   if (fontEngine != nullptr) {
      TableDirectory *os2TableEntry = font.tableDirectoryEntry("OS/2");

      if (os2TableEntry != nullptr) {
         const OS2Table *os2Table = reinterpret_cast<const OS2Table *>(fontData.constData()
               + qFromBigEndian<quint32>(os2TableEntry->offset));

         bool italic  = qFromBigEndian<quint16>(os2Table->selection) & 1;
         bool oblique = qFromBigEndian<quint16>(os2Table->selection) & 128;

         if (italic) {
            fontEngine->fontDef.style = QFont::StyleItalic;
         } else if (oblique) {
            fontEngine->fontDef.style = QFont::StyleOblique;
         } else {
            fontEngine->fontDef.style = QFont::StyleNormal;
         }

         fontEngine->fontDef.weight = QPlatformFontDatabase::weightFromInteger(qFromBigEndian<quint16>(os2Table->weightClass));
      }
   }

#if defined(CS_SHOW_DEBUG_PLATFORM)
   qDebug() << "QWindowsFontDatabase::fontEngine() Fontdata = "
         << fontData << pixelSize << hintingPreference << fontEngine;
#endif

   return fontEngine;
}

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

   *table  = nullptr;
   *length = 0;

   return;
}

static void getFamiliesAndSignatures(const QByteArray &fontData,
   QStringList *families, QVector<FONTSIGNATURE> *signatures)
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

      families->append(name);

      if (signatures) {
         FONTSIGNATURE signature;
         getFontTable(data, font, MAKE_TAG('O', 'S', '/', '2'), &table, &length);
         if (table && length >= 86) {
            // Offsets taken from OS/2 table in the TrueType spec
            signature.fsUsb[0] = qFromBigEndian<quint32>(table + 42);
            signature.fsUsb[1] = qFromBigEndian<quint32>(table + 46);
            signature.fsUsb[2] = qFromBigEndian<quint32>(table + 50);
            signature.fsUsb[3] = qFromBigEndian<quint32>(table + 54);

            signature.fsCsb[0] = qFromBigEndian<quint32>(table + 78);
            signature.fsCsb[1] = qFromBigEndian<quint32>(table + 82);

         } else {
            memset(&signature, 0, sizeof(signature));
         }
         signatures->append(signature);
      }
   }
}

QStringList QWindowsFontDatabase::addApplicationFont(const QByteArray &fontData, const QString &fileName)
{
   WinApplicationFont font;
   font.fileName = fileName;

   QVector<FONTSIGNATURE> signatures;
   QStringList families;

   if (! fontData.isEmpty()) {
      getFamiliesAndSignatures(fontData, &families, &signatures);

      if (families.isEmpty()) {
         return families;
      }

      DWORD dummy = 0;
      font.handle = AddFontMemResourceEx(const_cast<char *>(fontData.constData()), DWORD(fontData.size()), nullptr, &dummy);

      if (font.handle == nullptr) {
         return QStringList();
      }

      // Memory fonts won't show up in enumeration, so do add them the hard way.
      for (int j = 0; j < families.count(); ++j) {
         const QString familyName = families.at(j);
         HDC hdc = GetDC(nullptr);
         LOGFONT lf;

         memset(&lf, 0, sizeof(LOGFONT));

         std::wstring tmp = familyName.toStdWString();
         memcpy(lf.lfFaceName, tmp.data(), sizeof(wchar_t) * qMin(static_cast<std::wstring::size_type>(LF_FACESIZE) - 1, tmp.size()));

         lf.lfCharSet   = DEFAULT_CHARSET;
         HFONT hfont    = CreateFontIndirect(&lf);
         HGDIOBJ oldobj = SelectObject(hdc, hfont);

         TEXTMETRIC textMetrics;
         GetTextMetrics(hdc, &textMetrics);

         addFontToDatabase(familyName, lf.lfCharSet, &textMetrics, &signatures.at(j), TRUETYPE_FONTTYPE, true);

         SelectObject(hdc, oldobj);
         DeleteObject(hfont);
         ReleaseDC(nullptr, hdc);
      }

   } else {
      QFile f(fileName);
      if (! f.open(QIODevice::ReadOnly)) {
         return families;
      }

      QByteArray data = f.readAll();
      f.close();

      getFamiliesAndSignatures(data, &families, nullptr);
      if (families.isEmpty()) {
         return families;
      }

      if (AddFontResourceExW(fileName.toStdWString().c_str(), FR_PRIVATE, nullptr) == 0) {
         return QStringList();
      }

      font.handle = nullptr;

      // Fonts based on files are added via populate, as they will show up in font enumeration.
      for (int j = 0; j < families.count(); ++j) {
         populateFamily(families.at(j), true);
      }
   }

   m_applicationFonts << font;

   return families;
}

void QWindowsFontDatabase::removeApplicationFonts()
{
   for (const WinApplicationFont &font : m_applicationFonts) {
      if (font.handle) {
         RemoveFontMemResourceEx(font.handle);
      } else {
         RemoveFontResourceExW(font.fileName.toStdWString().c_str(), FR_PRIVATE, nullptr);
      }
   }

   m_applicationFonts.clear();
}

void QWindowsFontDatabase::releaseHandle(void *handle)
{
   (void) handle;
}

QString QWindowsFontDatabase::fontDir() const
{
   const QString result = QPlatformFontDatabase::fontDir();
   return result;
}

bool QWindowsFontDatabase::fontsAlwaysScalable() const
{
   return true;
}

void QWindowsFontDatabase::derefUniqueFont(const QString &uniqueFont)
{
   if (m_uniqueFontData.contains(uniqueFont)) {
      if (!m_uniqueFontData[uniqueFont].refCount.deref()) {
         RemoveFontMemResourceEx(m_uniqueFontData[uniqueFont].handle);
         m_uniqueFontData.remove(uniqueFont);
      }
   }
}

void QWindowsFontDatabase::refUniqueFont(const QString &uniqueFont)
{
   if (m_uniqueFontData.contains(uniqueFont)) {
      m_uniqueFontData[uniqueFont].refCount.ref();
   }
}

HFONT QWindowsFontDatabase::systemFont()
{
   static const HFONT stock_sysfont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
   return stock_sysfont;
}

// Creation functions

static const char *other_tryFonts[] = {
   "Arial",
   "MS UI Gothic",
   "Gulim",
   "SimSun",
   "PMingLiU",
   "Arial Unicode MS",
   nullptr
};

static const char *jp_tryFonts [] = {
   "MS UI Gothic",
   "Arial",
   "Gulim",
   "SimSun",
   "PMingLiU",
   "Arial Unicode MS",
   nullptr
};

static const char *ch_CN_tryFonts [] = {
   "SimSun",
   "Arial",
   "PMingLiU",
   "Gulim",
   "MS UI Gothic",
   "Arial Unicode MS",
   nullptr
};

static const char *ch_TW_tryFonts [] = {
   "PMingLiU",
   "Arial",
   "SimSun",
   "Gulim",
   "MS UI Gothic",
   "Arial Unicode MS",
   nullptr
};

static const char *kr_tryFonts[] = {
   "Gulim",
   "Arial",
   "PMingLiU",
   "SimSun",
   "MS UI Gothic",
   "Arial Unicode MS",
   nullptr
};

static const char **tryFonts = nullptr;

LOGFONT QWindowsFontDatabase::fontDefToLOGFONT(const QFontDef &request)
{
   LOGFONT lf;
   memset(&lf, 0, sizeof(LOGFONT));

   lf.lfHeight       = -qRound(request.pixelSize);
   lf.lfWidth        = 0;
   lf.lfEscapement   = 0;
   lf.lfOrientation  = 0;

   if (request.weight == 50) {
      lf.lfWeight = FW_DONTCARE;
   } else {
      lf.lfWeight = (request.weight * 900) / 99;
   }

   lf.lfItalic  = request.style != QFont::StyleNormal;
   lf.lfCharSet = DEFAULT_CHARSET;

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

   } else if (request.styleStrategy & QFont::PreferQuality) {
      qual = PROOF_QUALITY;
   }

   if (request.styleStrategy & QFont::PreferAntialias) {
      if (QSysInfo::WindowsVersion >= QSysInfo::WV_XP && !(request.styleStrategy & QFont::NoSubpixelAntialias)) {
         qual = CLEARTYPE_QUALITY;
      } else {
         qual = ANTIALIASED_QUALITY;
      }

   } else if (request.styleStrategy & QFont::NoAntialias) {
      qual = NONANTIALIASED_QUALITY;

   } else if ((request.styleStrategy & QFont::NoSubpixelAntialias) && sharedFontData()->clearTypeEnabled) {
      qual = ANTIALIASED_QUALITY;
   }

   lf.lfQuality        = qual;
   lf.lfClipPrecision  = CLIP_DEFAULT_PRECIS;

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

   lf.lfPitchAndFamily = DEFAULT_PITCH | hint;

   QString fam = request.family;

   if (fam.size() >= LF_FACESIZE) {
      qCritical("%s: Family name '%s' is too long.", __FUNCTION__, csPrintable(fam));
      fam.truncate(LF_FACESIZE - 1);
   }

   if (fam.isEmpty()) {
      fam = "MS Sans Serif";
   }

   if (fam == QLatin1String("MS Sans Serif") && (request.style == QFont::StyleItalic || (-lf.lfHeight > 18 && -lf.lfHeight != 24))) {
      // MS Sans Serif has bearing problems in italic, and does not scale
      fam = "Arial";
   }

   if (fam == "Courier" && !(request.styleStrategy & QFont::PreferBitmap)) {
      fam = "Courier New";
   }

   std::wstring tmp = fam.toStdWString();
   memcpy(lf.lfFaceName, tmp.c_str(), tmp.size() * sizeof(wchar_t));

   return lf;
}

QStringList QWindowsFontDatabase::extraTryFontsForFamily(const QString &family)
{
   QStringList result;
   QFontDatabase db;

   if (! db.writingSystems(family).contains(QFontDatabase::Symbol)) {
      if (! tryFonts) {
         LANGID lid = GetUserDefaultLangID();

         switch (lid & 0xff) {
            case LANG_CHINESE:
               if ( lid == 0x0804 || lid == 0x1004) {
                  // China mainland and Singapore
                  tryFonts = ch_CN_tryFonts;
               } else {
                  tryFonts = ch_TW_tryFonts;
                  // Taiwan, Hong Kong and Macau
               }
               break;

            case LANG_JAPANESE:
               tryFonts = jp_tryFonts;
               break;

            case LANG_KOREAN:
               tryFonts = kr_tryFonts;
               break;

            default:
               tryFonts = other_tryFonts;
               break;
         }
      }

      QFontDatabase db;
      const QStringList families = db.families();
      const char **tf = tryFonts;

      while (tf && *tf) {
         // QTBUG-31689, family might be an English alias for a localized font name.
         const QString family = QString::fromLatin1(*tf);
         if (families.contains(family) || db.hasFamily(family)) {
            result << family;
         }
         ++tf;
      }
   }

   result.append(QString("Segoe UI Emoji"));
   result.append(QString("Segoe UI Symbol"));

   return result;
}

QString QWindowsFontDatabase::familyForStyleHint(QFont::StyleHint styleHint)
{
   switch (styleHint) {
      case QFont::Times:
         return QString("Times New Roman");

      case QFont::Courier:
         return QString("Courier New");

      case QFont::Monospace:
         return QString("Courier New");

      case QFont::Cursive:
         return QString("Comic Sans MS");

      case QFont::Fantasy:
         return QString("Impact");

      case QFont::Decorative:
         return QString("Old English");

      case QFont::Helvetica:
         return QString("Arial");

      case QFont::System:
      default:
         break;
   }

   return QString("MS Shell Dlg 2");
}

QStringList QWindowsFontDatabase::fallbacksForFamily(const QString &family, QFont::Style style, QFont::StyleHint styleHint,
   QChar::Script script) const
{
   QStringList result;
   result.append(QWindowsFontDatabase::familyForStyleHint(styleHint));
   result.append(QWindowsFontDatabase::extraTryFontsForFamily(family));
   result.append(QPlatformFontDatabase::fallbacksForFamily(family, style, styleHint, script));

   return result;
}


QFontEngine *QWindowsFontDatabase::createEngine(const QFontDef &request, int dpi,
   const QSharedPointer<QWindowsFontEngineData> &data)
{
   QFontEngine *fe = nullptr;

   LOGFONT lf = fontDefToLOGFONT(request);
   const bool preferClearTypeAA = lf.lfQuality == CLEARTYPE_QUALITY;

   if (request.stretch != 100) {
      HFONT hfont = CreateFontIndirect(&lf);
      if (! hfont) {
         qErrnoWarning("%s: CreateFontIndirect failed", __FUNCTION__);
         hfont = QWindowsFontDatabase::systemFont();
      }

      HGDIOBJ oldObj = SelectObject(data->hdc, hfont);
      TEXTMETRIC tm;
      if (! GetTextMetrics(data->hdc, &tm)) {
         qErrnoWarning("%s: GetTextMetrics failed", __FUNCTION__);
      } else {
         lf.lfWidth = tm.tmAveCharWidth * request.stretch / 100;
      }
      SelectObject(data->hdc, oldObj);

      DeleteObject(hfont);
   }

   if (! fe) {
      QWindowsFontEngine *few = new QWindowsFontEngine(request.family, lf, data);
      if (preferClearTypeAA) {
         few->glyphFormat = QFontEngine::Format_A32;
      }
      few->initFontInfo(request, dpi);
      fe = few;
   }

   return fe;
}

static inline int verticalDPI()
{
   return GetDeviceCaps(QWindowsContext::instance()->displayContext(), LOGPIXELSY);
}

QFont QWindowsFontDatabase::systemDefaultFont()
{
   LOGFONT lf;
   GetObject(QWindowsFontDatabase::systemFont(), sizeof(lf), &lf);
   QFont sysFont = QWindowsFontDatabase::LOGFONT_to_QFont(lf);

   // "MS Shell Dlg 2" is the correct system font >= Win2k
   if (sysFont.family() == "MS Shell Dlg") {
      sysFont.setFamily("MS Shell Dlg 2");
   }

#if defined(CS_SHOW_DEBUG_PLATFORM)
   qDebug() << "QWindowsFontDatabase::systemDefaultFont() System font = " << sysFont;
#endif

   return sysFont;
}

QFont QWindowsFontDatabase::LOGFONT_to_QFont(const LOGFONT &logFont, int verticalDPI_In)
{
   if (verticalDPI_In <= 0) {
      verticalDPI_In = verticalDPI();
   }

   QFont qFont(QString::fromStdWString(std::wstring(logFont.lfFaceName)));
   qFont.setItalic(logFont.lfItalic);

   if (logFont.lfWeight != FW_DONTCARE) {
      qFont.setWeight(QPlatformFontDatabase::weightFromInteger(logFont.lfWeight));
   }

   const qreal logFontHeight = qAbs(logFont.lfHeight);
   qFont.setPointSizeF(logFontHeight * 72.0 / qreal(verticalDPI_In));
   qFont.setUnderline(logFont.lfUnderline);
   qFont.setOverline(false);
   qFont.setStrikeOut(logFont.lfStrikeOut);

   return qFont;
}
