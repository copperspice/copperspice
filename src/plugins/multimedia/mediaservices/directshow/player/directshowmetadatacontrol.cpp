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

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

#include <qcoreapplication.h>
#include <qsize.h>
#include <qdatetime.h>
#include <qimage.h>
#include <qmediametadata.h>
#include <directshowmetadatacontrol.h>
#include <directshowplayerservice.h>

#include <qsystemlibrary_p.h>

#include <dshow.h>
#include <initguid.h>
#include <qnetwork.h>

#if defined(QT_USE_WMSDK)
#include <wmsdk.h>
#endif

#ifndef QT_NO_SHELLITEM
#include <ShlObj.h>
#include <propkeydef.h>

DEFINE_PROPERTYKEY(PKEY_Author, 0xF29F85E0, 0x4FF9, 0x1068, 0xAB, 0x91, 0x08, 0x00, 0x2B, 0x27, 0xB3, 0xD9, 4);
DEFINE_PROPERTYKEY(PKEY_Title, 0xF29F85E0, 0x4FF9, 0x1068, 0xAB, 0x91, 0x08, 0x00, 0x2B, 0x27, 0xB3, 0xD9, 2);
DEFINE_PROPERTYKEY(PKEY_Media_SubTitle, 0x56A3372E, 0xCE9C, 0x11D2, 0x9F, 0x0E, 0x00, 0x60, 0x97, 0xC6, 0x86, 0xF6, 38);
DEFINE_PROPERTYKEY(PKEY_ParentalRating, 0x64440492, 0x4C8B, 0x11D1, 0x8B, 0x70, 0x08, 0x00, 0x36, 0xB1, 0x1A, 0x03, 21);
DEFINE_PROPERTYKEY(PKEY_Comment, 0xF29F85E0, 0x4FF9, 0x1068, 0xAB, 0x91, 0x08, 0x00, 0x2B, 0x27, 0xB3, 0xD9, 6);
DEFINE_PROPERTYKEY(PKEY_Copyright, 0x64440492, 0x4C8B, 0x11D1, 0x8B, 0x70, 0x08, 0x00, 0x36, 0xB1, 0x1A, 0x03, 11);
DEFINE_PROPERTYKEY(PKEY_Media_ProviderStyle, 0x64440492, 0x4C8B, 0x11D1, 0x8B, 0x70, 0x08, 0x00, 0x36, 0xB1, 0x1A, 0x03, 40);
DEFINE_PROPERTYKEY(PKEY_Media_Year, 0x56A3372E, 0xCE9C, 0x11D2, 0x9F, 0x0E, 0x00, 0x60, 0x97, 0xC6, 0x86, 0xF6, 5);
DEFINE_PROPERTYKEY(PKEY_Media_DateEncoded, 0x2E4B640D, 0x5019, 0x46D8, 0x88, 0x81, 0x55, 0x41, 0x4C, 0xC5, 0xCA, 0xA0, 100);
DEFINE_PROPERTYKEY(PKEY_Rating, 0x64440492, 0x4C8B, 0x11D1, 0x8B, 0x70, 0x08, 0x00, 0x36, 0xB1, 0x1A, 0x03, 9);
DEFINE_PROPERTYKEY(PKEY_Keywords, 0xF29F85E0, 0x4FF9, 0x1068, 0xAB, 0x91, 0x08, 0x00, 0x2B, 0x27, 0xB3, 0xD9, 5);
DEFINE_PROPERTYKEY(PKEY_Language, 0xD5CDD502, 0x2E9C, 0x101B, 0x93, 0x97, 0x08, 0x00, 0x2B, 0x2C, 0xF9, 0xAE, 28);
DEFINE_PROPERTYKEY(PKEY_Media_Publisher, 0x64440492, 0x4C8B, 0x11D1, 0x8B, 0x70, 0x08, 0x00, 0x36, 0xB1, 0x1A, 0x03, 30);
DEFINE_PROPERTYKEY(PKEY_Media_Duration, 0x64440490, 0x4C8B, 0x11D1, 0x8B, 0x70, 0x08, 0x00, 0x36, 0xB1, 0x1A, 0x03, 3);
DEFINE_PROPERTYKEY(PKEY_Audio_EncodingBitrate, 0x64440490, 0x4C8B, 0x11D1, 0x8B, 0x70, 0x08, 0x00, 0x36, 0xB1, 0x1A, 0x03, 4);
DEFINE_PROPERTYKEY(PKEY_Media_AverageLevel, 0x09EDD5B6, 0xB301, 0x43C5, 0x99, 0x90, 0xD0, 0x03, 0x02, 0xEF, 0xFD, 0x46, 100);
DEFINE_PROPERTYKEY(PKEY_Audio_ChannelCount, 0x64440490, 0x4C8B, 0x11D1, 0x8B, 0x70, 0x08, 0x00, 0x36, 0xB1, 0x1A, 0x03, 7);
DEFINE_PROPERTYKEY(PKEY_Audio_PeakValue, 0x2579E5D0, 0x1116, 0x4084, 0xBD, 0x9A, 0x9B, 0x4F, 0x7C, 0xB4, 0xDF, 0x5E, 100);
DEFINE_PROPERTYKEY(PKEY_Audio_SampleRate, 0x64440490, 0x4C8B, 0x11D1, 0x8B, 0x70, 0x08, 0x00, 0x36, 0xB1, 0x1A, 0x03, 5);
DEFINE_PROPERTYKEY(PKEY_Music_AlbumTitle, 0x56A3372E, 0xCE9C, 0x11D2, 0x9F, 0x0E, 0x00, 0x60, 0x97, 0xC6, 0x86, 0xF6, 4);
DEFINE_PROPERTYKEY(PKEY_Music_AlbumArtist, 0x56A3372E, 0xCE9C, 0x11D2, 0x9F, 0x0E, 0x00, 0x60, 0x97, 0xC6, 0x86, 0xF6, 13);
DEFINE_PROPERTYKEY(PKEY_Music_Artist, 0x56A3372E, 0xCE9C, 0x11D2, 0x9F, 0x0E, 0x00, 0x60, 0x97, 0xC6, 0x86, 0xF6, 2);
DEFINE_PROPERTYKEY(PKEY_Music_Composer, 0x64440492, 0x4C8B, 0x11D1, 0x8B, 0x70, 0x08, 0x00, 0x36, 0xB1, 0x1A, 0x03, 19);
DEFINE_PROPERTYKEY(PKEY_Music_Conductor, 0x56A3372E, 0xCE9C, 0x11D2, 0x9F, 0x0E, 0x00, 0x60, 0x97, 0xC6, 0x86, 0xF6, 36);
DEFINE_PROPERTYKEY(PKEY_Music_Lyrics, 0x56A3372E, 0xCE9C, 0x11D2, 0x9F, 0x0E, 0x00, 0x60, 0x97, 0xC6, 0x86, 0xF6, 12);
DEFINE_PROPERTYKEY(PKEY_Music_Mood, 0x56A3372E, 0xCE9C, 0x11D2, 0x9F, 0x0E, 0x00, 0x60, 0x97, 0xC6, 0x86, 0xF6, 39);
DEFINE_PROPERTYKEY(PKEY_Music_TrackNumber, 0x56A3372E, 0xCE9C, 0x11D2, 0x9F, 0x0E, 0x00, 0x60, 0x97, 0xC6, 0x86, 0xF6, 7);
DEFINE_PROPERTYKEY(PKEY_Music_Genre, 0x56A3372E, 0xCE9C, 0x11D2, 0x9F, 0x0E, 0x00, 0x60, 0x97, 0xC6, 0x86, 0xF6, 11);
DEFINE_PROPERTYKEY(PKEY_ThumbnailStream, 0xF29F85E0, 0x4FF9, 0x1068, 0xAB, 0x91, 0x08, 0x00, 0x2B, 0x27, 0xB3, 0xD9, 27);
DEFINE_PROPERTYKEY(PKEY_Video_FrameHeight, 0x64440491, 0x4C8B, 0x11D1, 0x8B, 0x70, 0x08, 0x00, 0x36, 0xB1, 0x1A, 0x03, 4);
DEFINE_PROPERTYKEY(PKEY_Video_FrameWidth, 0x64440491, 0x4C8B, 0x11D1, 0x8B, 0x70, 0x08, 0x00, 0x36, 0xB1, 0x1A, 0x03, 3);
DEFINE_PROPERTYKEY(PKEY_Video_HorizontalAspectRatio, 0x64440491, 0x4C8B, 0x11D1, 0x8B, 0x70, 0x08, 0x00, 0x36, 0xB1, 0x1A, 0x03, 42);
DEFINE_PROPERTYKEY(PKEY_Video_VerticalAspectRatio, 0x64440491, 0x4C8B, 0x11D1, 0x8B, 0x70, 0x08, 0x00, 0x36, 0xB1, 0x1A, 0x03, 45);
DEFINE_PROPERTYKEY(PKEY_Video_FrameRate, 0x64440491, 0x4C8B, 0x11D1, 0x8B, 0x70, 0x08, 0x00, 0x36, 0xB1, 0x1A, 0x03, 6);
DEFINE_PROPERTYKEY(PKEY_Video_EncodingBitrate, 0x64440491, 0x4C8B, 0x11D1, 0x8B, 0x70, 0x08, 0x00, 0x36, 0xB1, 0x1A, 0x03, 8);
DEFINE_PROPERTYKEY(PKEY_Video_Director, 0x64440492, 0x4C8B, 0x11D1, 0x8B, 0x70, 0x08, 0x00, 0x36, 0xB1, 0x1A, 0x03, 20);
DEFINE_PROPERTYKEY(PKEY_Media_Writer, 0x64440492, 0x4C8B, 0x11D1, 0x8B, 0x70, 0x08, 0x00, 0x36, 0xB1, 0x1A, 0x03, 23);

typedef HRESULT (WINAPI *q_SHCreateItemFromParsingName)(PCWSTR, IBindCtx *, const GUID &, void **);
static q_SHCreateItemFromParsingName sHCreateItemFromParsingName = nullptr;
#endif

#if defined(QT_USE_WMSDK)

namespace {

struct QWMMetaDataKey {
   QString qtName;
   const wchar_t *wmName;

   QWMMetaDataKey(const QString &qtn, const wchar_t *wmn) : qtName(qtn), wmName(wmn) { }
};
}

using QWMMetaDataKeys = QList<QWMMetaDataKey>;
static QWMMetaDataKeys metadataKeys;

static const QWMMetaDataKeys *qt_wmMetaDataKeys()
{
   if (metadataKeys.isEmpty()) {

      metadataKeys.append(QWMMetaDataKey(QMediaMetaData::Title, L"Title"));
      metadataKeys.append(QWMMetaDataKey(QMediaMetaData::SubTitle, L"WM/SubTitle"));
      metadataKeys.append(QWMMetaDataKey(QMediaMetaData::Author, L"Author"));
      metadataKeys.append(QWMMetaDataKey(QMediaMetaData::Comment, L"Comment"));
      metadataKeys.append(QWMMetaDataKey(QMediaMetaData::Description, L"Description"));
      metadataKeys.append(QWMMetaDataKey(QMediaMetaData::Category, L"WM/Category"));
      metadataKeys.append(QWMMetaDataKey(QMediaMetaData::Genre, L"WM/Genre"));
      //metadataKeys.append(QWMMetaDataKey(QMediaMetaData::Date, 0));
      metadataKeys.append(QWMMetaDataKey(QMediaMetaData::Year, L"WM/Year"));
      metadataKeys.append(QWMMetaDataKey(QMediaMetaData::UserRating, L"Rating"));
      //metadataKeys.append(QWMMetaDataKey(QMediaMetaData::MetaDatawords, 0));
      metadataKeys.append(QWMMetaDataKey(QMediaMetaData::Language, L"WM/Language"));
      metadataKeys.append(QWMMetaDataKey(QMediaMetaData::Publisher, L"WM/Publisher"));
      metadataKeys.append(QWMMetaDataKey(QMediaMetaData::Copyright, L"Copyright"));
      metadataKeys.append(QWMMetaDataKey(QMediaMetaData::ParentalRating, L"WM/ParentalRating"));
      //metadataKeys.append(QWMMetaDataKey(QMediaMetaData::RatingOrganisation, L"RatingOrganisation"));

      // Media
      metadataKeys.append(QWMMetaDataKey(QMediaMetaData::Size, L"FileSize"));
      metadataKeys.append(QWMMetaDataKey(QMediaMetaData::MediaType, L"MediaType"));
      metadataKeys.append(QWMMetaDataKey(QMediaMetaData::Duration, L"Duration"));

      // Audio
      metadataKeys.append(QWMMetaDataKey(QMediaMetaData::AudioBitRate, L"AudioBitRate"));
      metadataKeys.append(QWMMetaDataKey(QMediaMetaData::AudioCodec, L"AudioCodec"));
      metadataKeys.append(QWMMetaDataKey(QMediaMetaData::ChannelCount, L"ChannelCount"));
      metadataKeys.append(QWMMetaDataKey(QMediaMetaData::SampleRate, L"Frequency"));

      // Music
      metadataKeys.append(QWMMetaDataKey(QMediaMetaData::AlbumTitle, L"WM/AlbumTitle"));
      metadataKeys.append(QWMMetaDataKey(QMediaMetaData::AlbumArtist, L"WM/AlbumArtist"));
      metadataKeys.append(QWMMetaDataKey(QMediaMetaData::ContributingArtist, L"Author"));
      metadataKeys.append(QWMMetaDataKey(QMediaMetaData::Composer, L"WM/Composer"));
      metadataKeys.append(QWMMetaDataKey(QMediaMetaData::Conductor, L"WM/Conductor"));
      metadataKeys.append(QWMMetaDataKey(QMediaMetaData::Lyrics, L"WM/Lyrics"));
      metadataKeys.append(QWMMetaDataKey(QMediaMetaData::Mood, L"WM/Mood"));
      metadataKeys.append(QWMMetaDataKey(QMediaMetaData::TrackNumber, L"WM/TrackNumber"));
      //metadataKeys.append(QWMMetaDataKey(QMediaMetaData::TrackCount, 0));
      //metadataKeys.append(QWMMetaDataKey(QMediaMetaData::CoverArtUriSmall, 0));
      //metadataKeys.append(QWMMetaDataKey(QMediaMetaData::CoverArtUriLarge, 0));

      // Image/Video
      metadataKeys.append(QWMMetaDataKey(QMediaMetaData::Resolution, L"WM/VideoHeight"));
      metadataKeys.append(QWMMetaDataKey(QMediaMetaData::PixelAspectRatio, L"AspectRatioX"));

      // Video
      metadataKeys.append(QWMMetaDataKey(QMediaMetaData::VideoFrameRate, L"WM/VideoFrameRate"));
      metadataKeys.append(QWMMetaDataKey(QMediaMetaData::VideoBitRate, L"VideoBitRate"));
      metadataKeys.append(QWMMetaDataKey(QMediaMetaData::VideoCodec, L"VideoCodec"));

      //metadataKeys.append(QWMMetaDataKey(QMediaMetaData::PosterUri, 0));

      // Movie
      metadataKeys.append(QWMMetaDataKey(QMediaMetaData::ChapterNumber, L"ChapterNumber"));
      metadataKeys.append(QWMMetaDataKey(QMediaMetaData::Director, L"WM/Director"));
      metadataKeys.append(QWMMetaDataKey(QMediaMetaData::LeadPerformer, L"LeadPerformer"));
      metadataKeys.append(QWMMetaDataKey(QMediaMetaData::Writer, L"WM/Writer"));
   }

   return metadataKeys;
}

static QVariant getValue(IWMHeaderInfo *header, const wchar_t *key)
{
   WORD streamNumber = 0;
   WMT_ATTR_DATATYPE type = WMT_TYPE_DWORD;
   WORD size = 0;

   if (header->GetAttributeByName(&streamNumber, key, &type, 0, &size) == S_OK) {
      switch (type) {
         case WMT_TYPE_DWORD:
            if (size == sizeof(DWORD)) {
               DWORD word;
               if (header->GetAttributeByName(
                     &streamNumber,
                     key,
                     &type,
                     reinterpret_cast<BYTE *>(&word),
                     &size) == S_OK) {
                  return int(word);
               }
            }
            break;
         case WMT_TYPE_STRING: {
            QString string;
            string.resize(size / 2); // size is in bytes, string is in UTF16

            if (header->GetAttributeByName(
                  &streamNumber,
                  key,
                  &type,
                  reinterpret_cast<BYTE *>(const_cast<ushort *>(string.utf16())),
                  &size) == S_OK) {
               return string;
            }
         }
         break;
         case WMT_TYPE_BINARY: {
            QByteArray bytes;
            bytes.resize(size);
            if (header->GetAttributeByName(
                  &streamNumber,
                  key,
                  &type,
                  reinterpret_cast<BYTE *>(bytes.data()),
                  &size) == S_OK) {
               return bytes;
            }
         }
         break;
         case WMT_TYPE_BOOL:
            if (size == sizeof(DWORD)) {
               DWORD word;
               if (header->GetAttributeByName(
                     &streamNumber,
                     key,
                     &type,
                     reinterpret_cast<BYTE *>(&word),
                     &size) == S_OK) {
                  return bool(word);
               }
            }
            break;
         case WMT_TYPE_QWORD:
            if (size == sizeof(QWORD)) {
               QWORD word;
               if (header->GetAttributeByName(
                     &streamNumber,
                     key,
                     &type,
                     reinterpret_cast<BYTE *>(&word),
                     &size) == S_OK) {
                  return qint64(word);
               }
            }
            break;
         case WMT_TYPE_WORD:
            if (size == sizeof(WORD)) {
               WORD word;
               if (header->GetAttributeByName(
                     &streamNumber,
                     key,
                     &type,
                     reinterpret_cast<BYTE *>(&word),
                     &size) == S_OK) {
                  return short(word);
               }
            }
            break;
         case WMT_TYPE_GUID:
            if (size == 16) {
            }
            break;
         default:
            break;
      }
   }
   return QVariant();
}
#endif

#ifndef QT_NO_SHELLITEM
static QVariant convertValue(const PROPVARIANT &var)
{
   QVariant value;
   switch (var.vt) {

      case VT_LPWSTR: {
         std::wstring tmp(var.pwszVal);
         value = QString::fromStdWString(tmp);
         break;
      }

      case VT_UI4:
         value = uint(var.ulVal);
         break;

      case VT_UI8:
         value = quint64(var.uhVal.QuadPart);
         break;

      case VT_BOOL:
         value = bool(var.boolVal);
         break;

      case VT_FILETIME:
         SYSTEMTIME sysDate;
         if (!FileTimeToSystemTime(&var.filetime, &sysDate)) {
            break;
         }
         value = QDate(sysDate.wYear, sysDate.wMonth, sysDate.wDay);
         break;

      case VT_STREAM: {
         STATSTG stat;
         if (FAILED(var.pStream->Stat(&stat, STATFLAG_NONAME))) {
            break;
         }
         void *data = malloc(stat.cbSize.QuadPart);
         ULONG read = 0;
         if (FAILED(var.pStream->Read(data, stat.cbSize.QuadPart, &read))) {
            free(data);
            break;
         }
         value = QImage::fromData(reinterpret_cast<const uchar *>(data), read);
         free(data);
      }
      break;
      case VT_VECTOR | VT_LPWSTR:
         QStringList vList;

         for (ULONG i = 0; i < var.calpwstr.cElems; ++i) {
            std::wstring tmp( var.calpwstr.pElems[i] );
            vList.append(QString::fromStdWString(tmp));
         }

         value = vList;
         break;
   }

   return value;
}
#endif

DirectShowMetaDataControl::DirectShowMetaDataControl(QObject *parent)
   : QMetaDataReaderControl(parent), m_available(false)
{
}

DirectShowMetaDataControl::~DirectShowMetaDataControl()
{
}

bool DirectShowMetaDataControl::isMetaDataAvailable() const
{
   return m_available;
}

QVariant DirectShowMetaDataControl::metaData(const QString &key) const
{
   return m_metadata.value(key);
}

QStringList DirectShowMetaDataControl::availableMetaData() const
{
   return m_metadata.keys();
}

static QString convertBSTR(BSTR *string)
{
   std::wstring tmp(*string, ::SysStringLen(*string));
   QString value = QString::fromStdWString(tmp);

   ::SysFreeString(*string);
   string = nullptr;

   return value;
}

void DirectShowMetaDataControl::reset()
{
   bool hadMetadata = !m_metadata.isEmpty();
   m_metadata.clear();

   setMetadataAvailable(false);

   if (hadMetadata) {
      emit metaDataChanged();
   }
}

void DirectShowMetaDataControl::updateMetadata(IFilterGraph2 *graph, IBaseFilter *source, const QString &fileSrc)
{
   m_metadata.clear();

#ifndef QT_NO_SHELLITEM
   if (! sHCreateItemFromParsingName) {
      QSystemLibrary lib("shell32");
      sHCreateItemFromParsingName = (q_SHCreateItemFromParsingName)(lib.resolve("SHCreateItemFromParsingName"));
   }

   if (! fileSrc.isEmpty() && sHCreateItemFromParsingName) {
      IShellItem2 *shellItem = nullptr;

      std::wstring tmp = fileSrc.toStdWString();

      if (sHCreateItemFromParsingName(tmp.data(), nullptr, IID_PPV_ARGS(&shellItem)) == S_OK) {

         IPropertyStore *pStore = nullptr;
         if (shellItem->GetPropertyStore(GPS_DEFAULT, IID_PPV_ARGS(&pStore)) == S_OK) {
            DWORD cProps;

            if (SUCCEEDED(pStore->GetCount(&cProps))) {
               for (DWORD i = 0; i < cProps; ++i) {
                  PROPERTYKEY key;
                  PROPVARIANT var;
                  PropVariantInit(&var);

                  if (FAILED(pStore->GetAt(i, &key))) {
                     continue;
                  }

                  if (FAILED(pStore->GetValue(key, &var))) {
                     continue;
                  }

                  if (IsEqualPropertyKey(key, PKEY_Author)) {
                     m_metadata.insert(QMediaMetaData::Author, convertValue(var));
                  } else if (IsEqualPropertyKey(key, PKEY_Title)) {
                     m_metadata.insert(QMediaMetaData::Title, convertValue(var));
                  } else if (IsEqualPropertyKey(key, PKEY_Media_SubTitle)) {
                     m_metadata.insert(QMediaMetaData::SubTitle, convertValue(var));
                  } else if (IsEqualPropertyKey(key, PKEY_ParentalRating)) {
                     m_metadata.insert(QMediaMetaData::ParentalRating, convertValue(var));
                  } else if (IsEqualPropertyKey(key, PKEY_Comment)) {
                     m_metadata.insert(QMediaMetaData::Description, convertValue(var));
                  } else if (IsEqualPropertyKey(key, PKEY_Copyright)) {
                     m_metadata.insert(QMediaMetaData::Copyright, convertValue(var));
                  } else if (IsEqualPropertyKey(key, PKEY_Media_ProviderStyle)) {
                     m_metadata.insert(QMediaMetaData::Genre, convertValue(var));
                  } else if (IsEqualPropertyKey(key, PKEY_Media_Year)) {
                     m_metadata.insert(QMediaMetaData::Year, convertValue(var));
                  } else if (IsEqualPropertyKey(key, PKEY_Media_DateEncoded)) {
                     m_metadata.insert(QMediaMetaData::Date, convertValue(var));
                  } else if (IsEqualPropertyKey(key, PKEY_Rating)) {
                     m_metadata.insert(QMediaMetaData::UserRating,
                        int((convertValue(var).toUInt() - 1) / qreal(98) * 100));
                  } else if (IsEqualPropertyKey(key, PKEY_Keywords)) {
                     m_metadata.insert(QMediaMetaData::Keywords, convertValue(var));
                  } else if (IsEqualPropertyKey(key, PKEY_Language)) {
                     m_metadata.insert(QMediaMetaData::Language, convertValue(var));
                  } else if (IsEqualPropertyKey(key, PKEY_Media_Publisher)) {
                     m_metadata.insert(QMediaMetaData::Publisher, convertValue(var));
                  } else if (IsEqualPropertyKey(key, PKEY_Media_Duration)) {
                     m_metadata.insert(QMediaMetaData::Duration,
                        (convertValue(var).toLongLong() + 10000) / 10000);
                  } else if (IsEqualPropertyKey(key, PKEY_Audio_EncodingBitrate)) {
                     m_metadata.insert(QMediaMetaData::AudioBitRate, convertValue(var));
                  } else if (IsEqualPropertyKey(key, PKEY_Media_AverageLevel)) {
                     m_metadata.insert(QMediaMetaData::AverageLevel, convertValue(var));
                  } else if (IsEqualPropertyKey(key, PKEY_Audio_ChannelCount)) {
                     m_metadata.insert(QMediaMetaData::ChannelCount, convertValue(var));
                  } else if (IsEqualPropertyKey(key, PKEY_Audio_PeakValue)) {
                     m_metadata.insert(QMediaMetaData::PeakValue, convertValue(var));
                  } else if (IsEqualPropertyKey(key, PKEY_Audio_SampleRate)) {
                     m_metadata.insert(QMediaMetaData::SampleRate, convertValue(var));
                  } else if (IsEqualPropertyKey(key, PKEY_Music_AlbumTitle)) {
                     m_metadata.insert(QMediaMetaData::AlbumTitle, convertValue(var));
                  } else if (IsEqualPropertyKey(key, PKEY_Music_AlbumArtist)) {
                     m_metadata.insert(QMediaMetaData::AlbumArtist, convertValue(var));
                  } else if (IsEqualPropertyKey(key, PKEY_Music_Artist)) {
                     m_metadata.insert(QMediaMetaData::ContributingArtist, convertValue(var));
                  } else if (IsEqualPropertyKey(key, PKEY_Music_Composer)) {
                     m_metadata.insert(QMediaMetaData::Composer, convertValue(var));
                  } else if (IsEqualPropertyKey(key, PKEY_Music_Conductor)) {
                     m_metadata.insert(QMediaMetaData::Conductor, convertValue(var));
                  } else if (IsEqualPropertyKey(key, PKEY_Music_Lyrics)) {
                     m_metadata.insert(QMediaMetaData::Lyrics, convertValue(var));
                  } else if (IsEqualPropertyKey(key, PKEY_Music_Mood)) {
                     m_metadata.insert(QMediaMetaData::Mood, convertValue(var));
                  } else if (IsEqualPropertyKey(key, PKEY_Music_TrackNumber)) {
                     m_metadata.insert(QMediaMetaData::TrackNumber, convertValue(var));
                  } else if (IsEqualPropertyKey(key, PKEY_Music_Genre)) {
                     m_metadata.insert(QMediaMetaData::Genre, convertValue(var));
                  } else if (IsEqualPropertyKey(key, PKEY_ThumbnailStream)) {
                     m_metadata.insert(QMediaMetaData::ThumbnailImage, convertValue(var));
                  } else if (IsEqualPropertyKey(key, PKEY_Video_FrameHeight)) {
                     QSize res;
                     res.setHeight(convertValue(var).toUInt());
                     if (SUCCEEDED(pStore->GetValue(PKEY_Video_FrameWidth, &var))) {
                        res.setWidth(convertValue(var).toUInt());
                     }
                     m_metadata.insert(QMediaMetaData::Resolution, res);
                  } else if (IsEqualPropertyKey(key, PKEY_Video_HorizontalAspectRatio)) {
                     QSize aspectRatio;
                     aspectRatio.setWidth(convertValue(var).toUInt());
                     if (SUCCEEDED(pStore->GetValue(PKEY_Video_VerticalAspectRatio, &var))) {
                        aspectRatio.setHeight(convertValue(var).toUInt());
                     }
                     m_metadata.insert(QMediaMetaData::PixelAspectRatio, aspectRatio);
                  } else if (IsEqualPropertyKey(key, PKEY_Video_FrameRate)) {
                     m_metadata.insert(QMediaMetaData::VideoFrameRate,
                        convertValue(var).toReal() / 1000);
                  } else if (IsEqualPropertyKey(key, PKEY_Video_EncodingBitrate)) {
                     m_metadata.insert(QMediaMetaData::VideoBitRate, convertValue(var));
                  } else if (IsEqualPropertyKey(key, PKEY_Video_Director)) {
                     m_metadata.insert(QMediaMetaData::Director, convertValue(var));
                  } else if (IsEqualPropertyKey(key, PKEY_Media_Writer)) {
                     m_metadata.insert(QMediaMetaData::Writer, convertValue(var));
                  }

                  PropVariantClear(&var);
               }
            }

            pStore->Release();
         }

         shellItem->Release();
      }
   }

   if (! m_metadata.isEmpty()) {
      goto send_event;
   }
#endif

#if defined(QT_USE_WMSDK)
   IWMHeaderInfo *info = com_cast<IWMHeaderInfo>(source, IID_IWMHeaderInfo);

   if (info) {

      for (const QWMMetaDataKey &key : *qt_wmMetaDataKeys()) {
         QVariant var = getValue(info, key.wmName);

         if (var.isValid()) {
            if (key.qtName == QMediaMetaData::Duration) {
               // duration is provided in 100-nanosecond units, convert to milliseconds
               var = (var.toLongLong() + 10000) / 10000;

            } else if (key.qtName == QMediaMetaData::Resolution) {
               QSize res;
               res.setHeight(var.toUInt());
               res.setWidth(getValue(info, L"WM/VideoWidth").toUInt());
               var = res;

            } else if (key.qtName == QMediaMetaData::VideoFrameRate) {
               var = var.toReal() / 1000.f;

            } else if (key.qtName == QMediaMetaData::PixelAspectRatio) {
               QSize aspectRatio;
               aspectRatio.setWidth(var.toUInt());
               aspectRatio.setHeight(getValue(info, L"AspectRatioY").toUInt());
               var = aspectRatio;

            } else if (key.qtName == QMediaMetaData::UserRating) {
               var = (var.toUInt() - 1) / qreal(98) * 100;
            }

            m_metadata.insert(key.qtName, var);
         }
      }

      info->Release();
   }

   if (! m_metadata.isEmpty()) {
      goto send_event;
   }
#endif

   {
      IAMMediaContent *content = nullptr;

      if ((! graph || graph->QueryInterface(IID_IAMMediaContent, reinterpret_cast<void **>(&content)) != S_OK)
         && (! source || source->QueryInterface(IID_IAMMediaContent, reinterpret_cast<void **>(&content)) != S_OK)) {
         content = nullptr;
      }

      if (content) {
         BSTR string = nullptr;

         if (content->get_AuthorName(&string) == S_OK) {
            m_metadata.insert(QMediaMetaData::Author, convertBSTR(&string));
         }

         if (content->get_Title(&string) == S_OK) {
            m_metadata.insert(QMediaMetaData::Title, convertBSTR(&string));
         }

         if (content->get_Description(&string) == S_OK) {
            m_metadata.insert(QMediaMetaData::Description, convertBSTR(&string));
         }

         if (content->get_Rating(&string) == S_OK) {
            m_metadata.insert(QMediaMetaData::UserRating, convertBSTR(&string));
         }

         if (content->get_Copyright(&string) == S_OK) {
            m_metadata.insert(QMediaMetaData::Copyright, convertBSTR(&string));
         }

         content->Release();
      }
   }

send_event:
   // DirectShowMediaPlayerService holds a lock at this point so defer emitting signals to a later time
   QCoreApplication::postEvent(this, new QEvent(QEvent::Type(MetaDataChanged)));
}

void DirectShowMetaDataControl::customEvent(QEvent *event)
{
   if (event->type() == QEvent::Type(MetaDataChanged)) {
      event->accept();

      setMetadataAvailable(!m_metadata.isEmpty());

      emit metaDataChanged();
   } else {
      QMetaDataReaderControl::customEvent(event);
   }
}

void DirectShowMetaDataControl::setMetadataAvailable(bool available)
{
   if (m_available == available) {
      return;
   }

   m_available = available;
   emit metaDataAvailableChanged(m_available);
}
