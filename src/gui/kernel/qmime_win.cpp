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

#include <qmime.h>

#include <qimagereader.h>
#include <qimagewriter.h>
#include <qdatastream.h>
#include <qbuffer.h>
#include <qt_windows.h>
#include <qapplication_p.h>
#include <qtextcodec.h>
#include <qregularexpression.h>
#include <qalgorithms.h>
#include <qmap.h>
#include <qdnd_p.h>
#include <shlobj.h>
#include <qurl.h>
#include <qvariant.h>
#include <qtextdocument.h>
#include <qdir.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_IMAGEFORMAT_BMP
#ifndef CF_DIBV5
#define CF_DIBV5 17
#endif

/* The MSVC compilers allows multi-byte characters, that has the behavior of
 * that each character gets shifted into position. 0x73524742 below is for MSVC
 * equivalent to doing 'sRGB', but this does of course not work
 * on conformant C++ compilers. */

#define BMP_LCS_sRGB        0x73524742
#define BMP_LCS_GM_IMAGES   0x00000004L

struct _CIEXYZ {
   long ciexyzX, ciexyzY, ciexyzZ;
};

struct _CIEXYZTRIPLE {
   _CIEXYZ  ciexyzRed, ciexyzGreen, ciexyzBlue;
};

struct BMP_BITMAPV5HEADER {
   DWORD  bV5Size;
   LONG   bV5Width;
   LONG   bV5Height;
   WORD   bV5Planes;
   WORD   bV5BitCount;
   DWORD  bV5Compression;
   DWORD  bV5SizeImage;
   LONG   bV5XPelsPerMeter;
   LONG   bV5YPelsPerMeter;
   DWORD  bV5ClrUsed;
   DWORD  bV5ClrImportant;
   DWORD  bV5RedMask;
   DWORD  bV5GreenMask;
   DWORD  bV5BlueMask;
   DWORD  bV5AlphaMask;
   DWORD  bV5CSType;
   _CIEXYZTRIPLE bV5Endpoints;
   DWORD  bV5GammaRed;
   DWORD  bV5GammaGreen;
   DWORD  bV5GammaBlue;
   DWORD  bV5Intent;
   DWORD  bV5ProfileData;
   DWORD  bV5ProfileSize;
   DWORD  bV5Reserved;
};
static const int BMP_BITFIELDS = 3;

extern bool qt_read_dib(QDataStream &, QImage &); // qimage.cpp
extern bool qt_write_dib(QDataStream &, QImage);  // qimage.cpp
static bool qt_write_dibv5(QDataStream &s, QImage image);
static bool qt_read_dibv5(QDataStream &s, QImage &image);
#endif

//#define QMIME_DEBUG


// helpers for using global memory

static int getCf(const FORMATETC &formatetc)
{
   return formatetc.cfFormat;
}

static FORMATETC setCf(int cf)
{
   FORMATETC formatetc;
   formatetc.cfFormat = cf;
   formatetc.dwAspect = DVASPECT_CONTENT;
   formatetc.lindex = -1;
   formatetc.ptd = NULL;
   formatetc.tymed = TYMED_HGLOBAL;
   return formatetc;
}

static bool setData(const QByteArray &data, STGMEDIUM *pmedium)
{
   HGLOBAL hData = GlobalAlloc(0, data.size());
   if (!hData) {
      return false;
   }

   void *out = GlobalLock(hData);
   memcpy(out, data.data(), data.size());
   GlobalUnlock(hData);
   pmedium->tymed = TYMED_HGLOBAL;
   pmedium->hGlobal = hData;
   pmedium->pUnkForRelease = 0;
   return true;
}

static QByteArray getData(int cf, IDataObject *pDataObj)
{
   QByteArray data;
   FORMATETC formatetc = setCf(cf);
   STGMEDIUM s;
   if (pDataObj->GetData(&formatetc, &s) == S_OK) {
      DWORD *val = (DWORD *)GlobalLock(s.hGlobal);
      data = QByteArray::fromRawData((char *)val, GlobalSize(s.hGlobal));
      data.detach();
      GlobalUnlock(s.hGlobal);
      ReleaseStgMedium(&s);
   } else  {
      //Try reading IStream data
      formatetc.tymed = TYMED_ISTREAM;
      if (pDataObj->GetData(&formatetc, &s) == S_OK) {
         char szBuffer[4096];
         ULONG actualRead = 0;
         LARGE_INTEGER pos = {{0, 0}};
         //Move to front (can fail depending on the data model implemented)
         HRESULT hr = s.pstm->Seek(pos, STREAM_SEEK_SET, NULL);
         while (SUCCEEDED(hr)) {
            hr = s.pstm->Read(szBuffer, sizeof(szBuffer), &actualRead);
            if (SUCCEEDED(hr) && actualRead > 0) {
               data += QByteArray::fromRawData(szBuffer, actualRead);
            }
            if (actualRead != sizeof(szBuffer)) {
               break;
            }
         }
         data.detach();
         ReleaseStgMedium(&s);
      }
   }
   return data;
}

static bool canGetData(int cf, IDataObject *pDataObj)
{
   FORMATETC formatetc = setCf(cf);
   if (pDataObj->QueryGetData(&formatetc) != S_OK) {
      formatetc.tymed = TYMED_ISTREAM;
      return pDataObj->QueryGetData(&formatetc) == S_OK;
   }
   return true;
}

class QWindowsMimeList
{
 public:
   QWindowsMimeList();
   ~QWindowsMimeList();
   void addWindowsMime(QWindowsMime *mime);
   void removeWindowsMime(QWindowsMime *mime);
   QList<QWindowsMime *> windowsMimes();

 private:
   void init();
   bool initialized;
   QList<QWindowsMime *> mimes;
};

Q_GLOBAL_STATIC(QWindowsMimeList, theMimeList);


/*!
    \class QWindowsMime
    \brief The QWindowsMime class maps open-standard MIME to Window Clipboard formats.
    \ingroup draganddrop

    Qt's drag-and-drop and clipboard facilities use the MIME standard.
    On X11, this maps trivially to the Xdnd protocol, but on Windows
    although some applications use MIME types to describe clipboard
    formats, others use arbitrary non-standardized naming conventions,
    or unnamed built-in formats of Windows.

    By instantiating subclasses of QWindowsMime that provide conversions
    between Windows Clipboard and MIME formats, you can convert
    proprietary clipboard formats to MIME formats.

    Qt has predefined support for the following Windows Clipboard formats:

    \table
    \header \o Windows Format \o Equivalent MIME type
    \row \o \c CF_UNICODETEXT \o \c text/plain
    \row \o \c CF_TEXT        \o \c text/plain
    \row \o \c CF_DIB         \o \c{image/xyz}, where \c xyz is
                                 a \l{QImageWriter::supportedImageFormats()}{Qt image format}
    \row \o \c CF_HDROP       \o \c text/uri-list
    \row \o \c CF_INETURL     \o \c text/uri-list
    \row \o \c CF_HTML        \o \c text/html
    \endtable

    An example use of this class would be to map the Windows Metafile
    clipboard format (\c CF_METAFILEPICT) to and from the MIME type
    \c{image/x-wmf}. This conversion might simply be adding or removing
    a header, or even just passing on the data. See \l{Drag and Drop}
    for more information on choosing and definition MIME types.

    You can check if a MIME type is convertible using canConvertFromMime() and
    can perform conversions with convertToMime() and convertFromMime().
*/

QWindowsMime::QWindowsMime()
{
   theMimeList()->addWindowsMime(this);
}

QWindowsMime::~QWindowsMime()
{
   theMimeList()->removeWindowsMime(this);
}

int QWindowsMime::registerMimeType(const QString &mime)
{
   int f = RegisterClipboardFormat(mime.toStdWString().c_str());

   if (! f) {
      qErrnoWarning("QWindowsMime::registerMimeType: Failed to register clipboard format");
   }

   return f;
}

QWindowsMime *QWindowsMime::converterFromMime(const FORMATETC &formatetc, const QMimeData *mimeData)
{
   QList<QWindowsMime *> mimes = theMimeList()->windowsMimes();

   for (int i = mimes.size() - 1; i >= 0; --i) {
      if (mimes.at(i)->canConvertFromMime(formatetc, mimeData)) {
         return mimes.at(i);
      }
   }

   return 0;
}

QWindowsMime *QWindowsMime::converterToMime(const QString &mimeType, IDataObject *pDataObj)
{
   QList<QWindowsMime *> mimes = theMimeList()->windowsMimes();

   for (int i = mimes.size() - 1; i >= 0; --i) {
      if (mimes.at(i)->canConvertToMime(mimeType, pDataObj)) {
         return mimes.at(i);
      }
   }
   return 0;
}

QVector<FORMATETC> QWindowsMime::allFormatsForMime(const QMimeData *mimeData)
{
   QList<QWindowsMime *> mimes = theMimeList()->windowsMimes();
   QVector<FORMATETC> formatics;

   formatics.reserve(20);

#ifndef QT_NO_DRAGANDDROP
   QStringList formats = QInternalMimeData::formatsHelper(mimeData);

   for (int f = 0; f < formats.size(); ++f) {
      for (int i = mimes.size() - 1; i >= 0; --i) {
         formatics += mimes.at(i)->formatsForMime(formats.at(f), mimeData);
      }
   }
#endif

   return formatics;
}

QStringList QWindowsMime::allMimesForFormats(IDataObject *pDataObj)
{
   QList<QWindowsMime *> mimes = theMimeList()->windowsMimes();
   QStringList formats;

   LPENUMFORMATETC FAR fmtenum;
   HRESULT hr = pDataObj->EnumFormatEtc(DATADIR_GET, &fmtenum);

   if (hr == NOERROR) {
      FORMATETC fmtetc;
      while (S_OK == fmtenum->Next(1, &fmtetc, 0)) {
#if defined(QMIME_DEBUG)
         qDebug("QWindowsMime::allMimesForFormats()");
         wchar_t buf[256] = {0};
         GetClipboardFormatName(fmtetc.cfFormat, buf, 255);
         qDebug("CF = %d : %s", fmtetc.cfFormat, QString::fromWCharArray(buf));
#endif
         for (int i = mimes.size() - 1; i >= 0; --i) {
            QString format = mimes.at(i)->mimeForFormat(fmtetc);
            if (!format.isEmpty() && !formats.contains(format)) {
               formats += format;
            }
         }
         // as documented in MSDN to avoid possible memleak
         if (fmtetc.ptd) {
            CoTaskMemFree(fmtetc.ptd);
         }
      }
      fmtenum->Release();
   }

   return formats;
}


class QWindowsMimeText : public QWindowsMime
{
 public:
   bool canConvertToMime(const QString &mimeType, IDataObject *pDataObj) const override;
   QVariant convertToMime(const QString &mime, LPDATAOBJECT pDataObj, QVariant::Type preferredType) const override;
   QString mimeForFormat(const FORMATETC &formatetc) const override;
   bool canConvertFromMime(const FORMATETC &formatetc, const QMimeData *mimeData) const override;
   bool convertFromMime(const FORMATETC &formatetc, const QMimeData *mimeData, STGMEDIUM *pmedium) const override;
   QVector<FORMATETC> formatsForMime(const QString &mimeType, const QMimeData *mimeData) const override;
};

bool QWindowsMimeText::canConvertFromMime(const FORMATETC &formatetc, const QMimeData *mimeData) const
{
   int cf = getCf(formatetc);
   return (cf == CF_UNICODETEXT || cf == CF_TEXT) && mimeData->hasText();
}

/*
text/plain is defined as using CRLF, but so many programs don't,
and programmers just look for '\n' in strings.
Windows really needs CRLF, so we ensure it here.
*/
bool QWindowsMimeText::convertFromMime(const FORMATETC &formatetc, const QMimeData *mimeData, STGMEDIUM *pmedium) const
{
   if (canConvertFromMime(formatetc, mimeData)) {
      QByteArray data;
      int cf = getCf(formatetc);

      if (cf == CF_TEXT) {
         data = mimeData->text().toLatin1();

         // Anticipate required space for CRLFs at 1/40
         int maxsize = data.size() + data.size() / 40 + 3;
         QByteArray r(maxsize, '\0');
         char *o = r.data();

         const char *d = data.data();
         const int s   = data.size();
         bool cr = false;
         int j   = 0;

         for (int i = 0; i < s; i++) {
            char c = d[i];

            if (c == '\r') {
               cr = true;

            } else {
               if (c == '\n') {
                  if (! cr) {
                     o[j++] = '\r';
                  }
               }

               cr = false;
            }

            o[j++] = c;

            if (j + 3 >= maxsize) {
               maxsize += maxsize / 4;
               r.resize(maxsize);
               o = r.data();
            }
         }

         o[j] = 0;
         return setData(r, pmedium);

      } else if (cf == CF_UNICODETEXT) {
         QString str = mimeData->text();

         QString16 tmp;
         bool cr = false;

         for (QChar c : str) {
            if (c == '\r') {
               cr = true;

            } else {
               if (c == '\n' && ! cr) {
                  tmp.append('\r');
               }

               cr = false;
            }

            tmp.append(c);
         }

         const int byteLength = tmp.size_storage() * sizeof(char16_t);

         QByteArray retval(byteLength + 2, '\0');
         memcpy(retval.data(), tmp.constData(), byteLength);

         retval[byteLength]     = 0;
         retval[byteLength + 1] = 0;

         return setData(retval, pmedium);
      }
   }

   return false;
}

bool QWindowsMimeText::canConvertToMime(const QString &mimeType, IDataObject *pDataObj) const
{
   return mimeType.startsWith(QLatin1String("text/plain"))
          && (canGetData(CF_UNICODETEXT, pDataObj)
              || canGetData(CF_TEXT, pDataObj));
}

QString QWindowsMimeText::mimeForFormat(const FORMATETC &formatetc) const
{
   int cf = getCf(formatetc);
   if (cf == CF_UNICODETEXT || cf == CF_TEXT) {
      return QLatin1String("text/plain");
   }
   return QString();
}


QVector<FORMATETC> QWindowsMimeText::formatsForMime(const QString &mimeType, const QMimeData *mimeData) const
{
   QVector<FORMATETC> formatics;

   if (mimeType.startsWith("text/plain") && mimeData->hasText()) {
      formatics += setCf(CF_UNICODETEXT);
      formatics += setCf(CF_TEXT);
   }

   return formatics;
}

QVariant QWindowsMimeText::convertToMime(const QString &mime, LPDATAOBJECT pDataObj, QVariant::Type preferredType) const
{
   QVariant ret;

   if (canConvertToMime(mime, pDataObj)) {
      QString str;
      QByteArray data = getData(CF_UNICODETEXT, pDataObj);

      if (! data.isEmpty()) {
         std::wstring tmp((const wchar_t *)data.constData());

         str = QString::fromStdWString(tmp);
         str.replace("\r\n", "\n");

      } else {
         data = getData(CF_TEXT, pDataObj);

         if (!data.isEmpty()) {
            const char *d = data.data();
            const int s   = qstrlen(d);

            QByteArray r(data.size() + 1, '\0');
            char *o = r.data();
            int j = 0;

            for (int i = 0; i < s; i++) {
               char c = d[i];
               if (c != '\r') {
                  o[j++] = c;
               }
            }

            o[j] = 0;
            str = QString::fromLatin1(r);
         }
      }

      if (preferredType == QVariant::String) {
         ret = str;
      } else {
         ret = str.toUtf8();
      }
   }

   return ret;
}

class QWindowsMimeURI : public QWindowsMime
{
 public:
   QWindowsMimeURI();
   bool canConvertToMime(const QString &mimeType, IDataObject *pDataObj) const override;
   QVariant convertToMime(const QString &mime, LPDATAOBJECT pDataObj, QVariant::Type preferredType) const override;
   QString mimeForFormat(const FORMATETC &formatetc) const override;
   bool canConvertFromMime(const FORMATETC &formatetc, const QMimeData *mimeData) const override;
   bool convertFromMime(const FORMATETC &formatetc, const QMimeData *mimeData, STGMEDIUM *pmedium) const override;
   QVector<FORMATETC> formatsForMime(const QString &mimeType, const QMimeData *mimeData) const override;

 private:
   int CF_INETURL_W; // wide char version
   int CF_INETURL;
};

QWindowsMimeURI::QWindowsMimeURI()
{
   CF_INETURL_W = QWindowsMime::registerMimeType(QLatin1String("UniformResourceLocatorW"));
   CF_INETURL = QWindowsMime::registerMimeType(QLatin1String("UniformResourceLocator"));
}

bool QWindowsMimeURI::canConvertFromMime(const FORMATETC &formatetc, const QMimeData *mimeData) const
{
   if (getCf(formatetc) == CF_HDROP) {
      QList<QUrl> urls = mimeData->urls();
      for (int i = 0; i < urls.size(); i++) {
         if (!urls.at(i).toLocalFile().isEmpty()) {
            return true;
         }
      }
   }
   return (getCf(formatetc) == CF_INETURL_W || getCf(formatetc) == CF_INETURL) &&
          mimeData->hasFormat(QLatin1String("text/uri-list"));
}

bool QWindowsMimeURI::convertFromMime(const FORMATETC &formatetc, const QMimeData *mimeData, STGMEDIUM *pmedium) const
{
   if (canConvertFromMime(formatetc, mimeData)) {
      if (getCf(formatetc) == CF_HDROP) {
         QList<QUrl> urls = mimeData->urls();
         QStringList fileNames;
         int size = sizeof(DROPFILES) + 2;

         for (int i = 0; i < urls.size(); i++) {
            QString fn = QDir::toNativeSeparators(urls.at(i).toLocalFile());

            if (!fn.isEmpty()) {
               size += sizeof(ushort) * (fn.length() + 1);
               fileNames.append(fn);
            }
         }

         QByteArray result(size, '\0');
         DROPFILES *d = (DROPFILES *)result.data();
         d->pFiles = sizeof(DROPFILES);

         GetCursorPos(&d->pt); // try

         d->fNC = true;
         char *files = ((char *)d) + d->pFiles;
         d->fWide = true;

         wchar_t *f = (wchar_t *)files;

         for (int i = 0; i < fileNames.size(); i++) {
            QString16 tmp = fileNames.at(i).toUtf16();
            memcpy(f, tmp.constData(), tmp.size_storage() * sizeof(ushort));

            f += tmp.size_storage();
            *f++ = '\0';
         }

         *f = '\0';

         return setData(result, pmedium);

      } else if (getCf(formatetc) == CF_INETURL_W) {
         QList<QUrl> urls = mimeData->urls();
         QByteArray result;

         if (! urls.isEmpty()) {
            QString url = urls.at(0).toString();

            QString16 tmp = url.toUtf16();
            result = QByteArray((const char *)tmp.constData(), tmp.size_storage() * sizeof(ushort));
         }

         result.append('\0');
         result.append('\0');
         return setData(result, pmedium);

      } else if (getCf(formatetc) == CF_INETURL) {
         QList<QUrl> urls = mimeData->urls();
         QByteArray result;

         if (! urls.isEmpty()) {
            result = urls.at(0).toString().toLatin1();
         }

         return setData(result, pmedium);
      }
   }

   return false;
}

bool QWindowsMimeURI::canConvertToMime(const QString &mimeType, IDataObject *pDataObj) const
{
   return mimeType == "text/uri-list"
          && (canGetData(CF_HDROP, pDataObj) || canGetData(CF_INETURL_W, pDataObj) || canGetData(CF_INETURL, pDataObj));
}

QString QWindowsMimeURI::mimeForFormat(const FORMATETC &formatetc) const
{
   QString format;
   if (getCf(formatetc) == CF_HDROP || getCf(formatetc) == CF_INETURL_W || getCf(formatetc) == CF_INETURL) {
      format = QLatin1String("text/uri-list");
   }
   return format;
}

QVector<FORMATETC> QWindowsMimeURI::formatsForMime(const QString &mimeType, const QMimeData *mimeData) const
{
   QVector<FORMATETC> formatics;
   if (mimeType == QLatin1String("text/uri-list")) {
      if (canConvertFromMime(setCf(CF_HDROP), mimeData)) {
         formatics += setCf(CF_HDROP);
      }
      if (canConvertFromMime(setCf(CF_INETURL_W), mimeData)) {
         formatics += setCf(CF_INETURL_W);
      }
      if (canConvertFromMime(setCf(CF_INETURL), mimeData)) {
         formatics += setCf(CF_INETURL);
      }
   }
   return formatics;
}

QVariant QWindowsMimeURI::convertToMime(const QString &mimeType, LPDATAOBJECT pDataObj, QVariant::Type preferredType) const
{
   if (mimeType == QLatin1String("text/uri-list")) {
      if (canGetData(CF_HDROP, pDataObj)) {
         QByteArray texturi;
         QList<QVariant> urls;

         QByteArray data = getData(CF_HDROP, pDataObj);
         if (data.isEmpty()) {
            return QVariant();
         }

         LPDROPFILES hdrop = (LPDROPFILES)data.data();
         if (hdrop->fWide) {
            const wchar_t *filesw = (const wchar_t *)(data.data() + hdrop->pFiles);
            int i = 0;

            std::wstring tmp;

            while (filesw[i]) {

               tmp = filesw + i;
               QString fileurl = QString::fromStdWString(tmp);

               urls += QUrl::fromLocalFile(fileurl);
               i += fileurl.length() + 1;
            }

         } else {
            const char *files = (const char *)data.data() + hdrop->pFiles;
            int i = 0;

            while (files[i]) {
               urls += QUrl::fromLocalFile(QString::fromLatin1(files + i));
               i += int(strlen(files + i)) + 1;
            }
         }

         if (preferredType == QVariant::Url && urls.size() == 1) {
            return urls.at(0);
         } else if (!urls.isEmpty()) {
            return urls;
         }

      } else if (canGetData(CF_INETURL_W, pDataObj)) {
         QByteArray data = getData(CF_INETURL_W, pDataObj);
         if (data.isEmpty()) {
            return QVariant();
         }

         std::wstring tmp((const wchar_t *)data.constData());
         return QUrl(QString::fromStdWString(tmp));

      } else if (canGetData(CF_INETURL, pDataObj)) {
         QByteArray data = getData(CF_INETURL, pDataObj);

         if (data.isEmpty()) {
            return QVariant();
         }
         return QUrl(QString::fromLatin1(data.constData()));
      }
   }

   return QVariant();
}

class QWindowsMimeHtml : public QWindowsMime
{
 public:
   QWindowsMimeHtml();

   // for converting from Qt
   bool canConvertFromMime(const FORMATETC &formatetc, const QMimeData *mimeData) const override;
   bool convertFromMime(const FORMATETC &formatetc, const QMimeData *mimeData, STGMEDIUM *pmedium) const override;
   QVector<FORMATETC> formatsForMime(const QString &mimeType, const QMimeData *mimeData) const override;

   // for converting to Qt
   bool canConvertToMime(const QString &mimeType, IDataObject *pDataObj) const override;
   QVariant convertToMime(const QString &mime, IDataObject *pDataObj, QVariant::Type preferredType) const override;
   QString mimeForFormat(const FORMATETC &formatetc) const override;

 private:
   int CF_HTML;
};

QWindowsMimeHtml::QWindowsMimeHtml()
{
   CF_HTML = QWindowsMime::registerMimeType(QLatin1String("HTML Format"));
}

QVector<FORMATETC> QWindowsMimeHtml::formatsForMime(const QString &mimeType, const QMimeData *mimeData) const
{
   QVector<FORMATETC> formatetcs;
   if (mimeType == QLatin1String("text/html") && (!mimeData->html().isEmpty())) {
      formatetcs += setCf(CF_HTML);
   }
   return formatetcs;
}

QString QWindowsMimeHtml::mimeForFormat(const FORMATETC &formatetc) const
{
   if (getCf(formatetc) == CF_HTML) {
      return QLatin1String("text/html");
   }
   return QString();
}

bool QWindowsMimeHtml::canConvertToMime(const QString &mimeType, IDataObject *pDataObj) const
{
   return mimeType == QLatin1String("text/html") && canGetData(CF_HTML, pDataObj);
}


bool QWindowsMimeHtml::canConvertFromMime(const FORMATETC &formatetc, const QMimeData *mimeData) const
{
   return getCf(formatetc) == CF_HTML && (!mimeData->html().isEmpty());
}

/*
The windows HTML clipboard format is as follows (xxxxxxxxxx is a 10 integer number giving the positions
in bytes). Charset used is mostly utf8, but can be different, ie. we have to look for the <meta> charset tag

  Version: 1.0
  StartHTML:xxxxxxxxxx
  EndHTML:xxxxxxxxxx
  StartFragment:xxxxxxxxxx
  EndFragment:xxxxxxxxxx
  ...html...

*/
QVariant QWindowsMimeHtml::convertToMime(const QString &mime, IDataObject *pDataObj, QVariant::Type preferredType) const
{
   Q_UNUSED(preferredType);
   QVariant result;
   if (canConvertToMime(mime, pDataObj)) {
      QByteArray html = getData(CF_HTML, pDataObj);
#ifdef QMIME_DEBUG
      qDebug("QWindowsMimeHtml::convertToMime");
      qDebug("raw :");
      qDebug(html);
#endif
      int start = html.indexOf("StartFragment:");
      int end = html.indexOf("EndFragment:");

      if (start != -1) {
         int startOffset = start + 14;
         int i = startOffset;
         while (html.at(i) != '\r' && html.at(i) != '\n') {
            ++i;
         }
         QByteArray bytecount = html.mid(startOffset, i - startOffset);
         start = bytecount.toInt();
      }

      if (end != -1) {
         int endOffset = end + 12;
         int i = endOffset ;
         while (html.at(i) != '\r' && html.at(i) != '\n') {
            ++i;
         }
         QByteArray bytecount = html.mid(endOffset , i - endOffset);
         end = bytecount.toInt();
      }

      if (end > start && start > 0) {
         html = "<!--StartFragment-->" + html.mid(start, end - start);
         html += "<!--EndFragment-->";
         html.replace('\r', "");
         result = QString::fromUtf8(html);
      }
   }
   return result;
}

bool QWindowsMimeHtml::convertFromMime(const FORMATETC &formatetc, const QMimeData *mimeData, STGMEDIUM *pmedium) const
{
   if (canConvertFromMime(formatetc, mimeData)) {
      QByteArray data = mimeData->html().toUtf8();
      QByteArray result =
         "Version:1.0\r\n"                    // 0-12
         "StartHTML:0000000105\r\n"            // 13-35
         "EndHTML:0000000000\r\n"            // 36-55
         "StartFragment:0000000000\r\n"            // 58-86
         "EndFragment:0000000000\r\n\r\n";   // 87-105

      if (data.indexOf("<!--StartFragment-->") == -1) {
         result += "<!--StartFragment-->";
      }
      result += data;
      if (data.indexOf("<!--EndFragment-->") == -1) {
         result += "<!--EndFragment-->";
      }

      // set the correct number for EndHTML
      QByteArray pos = QString::number(result.size()).toLatin1();
      memcpy((char *)(result.data() + 53 - pos.length()), pos.constData(), pos.length());

      // set correct numbers for StartFragment and EndFragment
      pos = QString::number(result.indexOf("<!--StartFragment-->") + 20).toLatin1();
      memcpy((char *)(result.data() + 79 - pos.length()), pos.constData(), pos.length());
      pos = QString::number(result.indexOf("<!--EndFragment-->")).toLatin1();
      memcpy((char *)(result.data() + 103 - pos.length()), pos.constData(), pos.length());

      return setData(result, pmedium);
   }
   return false;
}


#ifndef QT_NO_IMAGEFORMAT_BMP
class QWindowsMimeImage : public QWindowsMime
{
 public:
   QWindowsMimeImage();
   // for converting from Qt
   bool canConvertFromMime(const FORMATETC &formatetc, const QMimeData *mimeData) const override;
   bool convertFromMime(const FORMATETC &formatetc, const QMimeData *mimeData, STGMEDIUM *pmedium) const override;
   QVector<FORMATETC> formatsForMime(const QString &mimeType, const QMimeData *mimeData) const override;

   // for converting to Qt
   bool canConvertToMime(const QString &mimeType, IDataObject *pDataObj) const override;
   QVariant convertToMime(const QString &mime, IDataObject *pDataObj, QVariant::Type preferredType) const override;
   QString mimeForFormat(const FORMATETC &formatetc) const override;

 private:
   bool hasOriginalDIBV5(IDataObject *pDataObj) const;
   UINT CF_PNG;
};

QWindowsMimeImage::QWindowsMimeImage()
{
   CF_PNG = RegisterClipboardFormat(L"PNG");
}

QVector<FORMATETC> QWindowsMimeImage::formatsForMime(const QString &mimeType, const QMimeData *mimeData) const
{
   QVector<FORMATETC> formatetcs;
   if (mimeData->hasImage() && mimeType == QLatin1String("application/x-qt-image")) {
      //add DIBV5 if image has alpha channel
      QImage image = qvariant_cast<QImage>(mimeData->imageData());
      if (!image.isNull() && image.hasAlphaChannel()) {
         formatetcs += setCf(CF_DIBV5);
      }
      formatetcs += setCf(CF_DIB);
   }
   return formatetcs;
}

QString QWindowsMimeImage::mimeForFormat(const FORMATETC &formatetc) const
{
   int cf = getCf(formatetc);
   if (cf == CF_DIB || cf == CF_DIBV5 || cf == int(CF_PNG)) {
      return QLatin1String("application/x-qt-image");
   }
   return QString();
}

bool QWindowsMimeImage::canConvertToMime(const QString &mimeType, IDataObject *pDataObj) const
{
   if ((mimeType == QLatin1String("application/x-qt-image")) &&
         (canGetData(CF_DIB, pDataObj) || canGetData(CF_PNG, pDataObj))) {
      return true;
   }
   return false;
}

bool QWindowsMimeImage::canConvertFromMime(const FORMATETC &formatetc, const QMimeData *mimeData) const
{
   int cf = getCf(formatetc);
   if (mimeData->hasImage()) {
      if (cf == CF_DIB) {
         return true;
      } else if (cf == CF_DIBV5) {
         //support DIBV5 conversion only if the image has alpha channel
         QImage image = qvariant_cast<QImage>(mimeData->imageData());
         if (!image.isNull() && image.hasAlphaChannel()) {
            return true;
         }
      }
   }
   return false;
}

bool QWindowsMimeImage::convertFromMime(const FORMATETC &formatetc, const QMimeData *mimeData, STGMEDIUM *pmedium) const
{
   int cf = getCf(formatetc);
   if ((cf == CF_DIB || cf == CF_DIBV5) && mimeData->hasImage()) {
      QImage img = qvariant_cast<QImage>(mimeData->imageData());
      if (img.isNull()) {
         return false;
      }
      QByteArray ba;
      QDataStream s(&ba, QIODevice::WriteOnly);
      s.setByteOrder(QDataStream::LittleEndian);// Intel byte order ####
      if (cf == CF_DIB) {
         if (img.format() > QImage::Format_ARGB32) {
            img = img.convertToFormat(QImage::Format_RGB32);
         }
         if (qt_write_dib(s, img)) {
            return setData(ba, pmedium);
         }
      } else {
         if (qt_write_dibv5(s, img)) {
            return setData(ba, pmedium);
         }
      }
   }
   return false;
}

bool QWindowsMimeImage::hasOriginalDIBV5(IDataObject *pDataObj) const
{
   bool isSynthesized = true;
   IEnumFORMATETC *pEnum = NULL;
   HRESULT res = pDataObj->EnumFormatEtc(1, &pEnum);
   if (res == S_OK && pEnum) {
      FORMATETC fc;
      while ((res = pEnum->Next(1, &fc, 0)) == S_OK) {
         if (fc.ptd) {
            CoTaskMemFree(fc.ptd);
         }
         if (fc.cfFormat == CF_DIB) {
            break;
         } else if (fc.cfFormat == CF_DIBV5) {
            isSynthesized  = false;
            break;
         }
      }
      pEnum->Release();
   }
   return !isSynthesized;
}

QVariant QWindowsMimeImage::convertToMime(const QString &mimeType, IDataObject *pDataObj,
      QVariant::Type preferredType) const
{
   Q_UNUSED(preferredType);
   QVariant result;
   if (mimeType != QLatin1String("application/x-qt-image")) {
      return result;
   }
   //Try to convert from a format which has more data
   //DIBV5, use only if its is not synthesized
   if (canGetData(CF_DIBV5, pDataObj) && hasOriginalDIBV5(pDataObj)) {
      QImage img;
      QByteArray data = getData(CF_DIBV5, pDataObj);
      QDataStream s(&data, QIODevice::ReadOnly);
      s.setByteOrder(QDataStream::LittleEndian);
      if (qt_read_dibv5(s, img)) { // #### supports only 32bit DIBV5
         return img;
      }
   }
   //PNG, MS Office place this (undocumented)
   if (canGetData(CF_PNG, pDataObj)) {
      QImage img;
      QByteArray data = getData(CF_PNG, pDataObj);
      if (img.loadFromData(data, "PNG")) {
         return img;
      }
   }
   //Fallback to DIB
   if (canGetData(CF_DIB, pDataObj)) {
      QImage img;
      QByteArray data = getData(CF_DIB, pDataObj);
      QDataStream s(&data, QIODevice::ReadOnly);
      s.setByteOrder(QDataStream::LittleEndian);// Intel byte order ####
      if (qt_read_dib(s, img)) { // ##### encaps "-14"
         return img;
      }
   }
   // Failed
   return result;
}
#endif

class QBuiltInMimes : public QWindowsMime
{
 public:
   QBuiltInMimes();

   // for converting from Qt
   bool canConvertFromMime(const FORMATETC &formatetc, const QMimeData *mimeData) const override;
   bool convertFromMime(const FORMATETC &formatetc, const QMimeData *mimeData, STGMEDIUM *pmedium) const override;
   QVector<FORMATETC> formatsForMime(const QString &mimeType, const QMimeData *mimeData) const override;

   // for converting to Qt
   bool canConvertToMime(const QString &mimeType, IDataObject *pDataObj) const override;
   QVariant convertToMime(const QString &mime, IDataObject *pDataObj, QVariant::Type preferredType) const override;
   QString mimeForFormat(const FORMATETC &formatetc) const override;

 private:
   QMap<int, QString> outFormats;
   QMap<int, QString> inFormats;
};

QBuiltInMimes::QBuiltInMimes()
   : QWindowsMime()
{
   outFormats.insert(QWindowsMime::registerMimeType(QLatin1String("application/x-color")),
                     QLatin1String("application/x-color"));
   inFormats.insert(QWindowsMime::registerMimeType(QLatin1String("application/x-color")),
                    QLatin1String("application/x-color"));
}

bool QBuiltInMimes::canConvertFromMime(const FORMATETC &formatetc, const QMimeData *mimeData) const
{
   // really check
   return formatetc.tymed & TYMED_HGLOBAL
          && outFormats.contains(formatetc.cfFormat)
          && mimeData->formats().contains(outFormats.value(formatetc.cfFormat));
}

bool QBuiltInMimes::convertFromMime(const FORMATETC &formatetc, const QMimeData *mimeData, STGMEDIUM *pmedium) const
{
   if (canConvertFromMime(formatetc, mimeData)) {
      QByteArray data;

      if (outFormats.value(getCf(formatetc)) == "text/html") {
         // text/html is in wide chars on windows (compatible with mozillia)
         QString html = mimeData->html();

         QString16 tmp;
         bool cr = false;

         for (QChar c : html) {
            if (c == '\r') {
               cr = true;

            } else {
               if (c == '\n' && ! cr) {
                  tmp.append('\r');
               }

               cr = false;
            }

            tmp.append(c);
         }

         const int byteLength = tmp.size_storage() * sizeof(char16_t);

         QByteArray retval(byteLength + 2, '\0');
         memcpy(retval.data(), tmp.constData(), byteLength);

         retval[byteLength]     = 0;
         retval[byteLength + 1] = 0;

         data = retval;

      } else {

#ifndef QT_NO_DRAGANDDROP
         data = QInternalMimeData::renderDataHelper(outFormats.value(getCf(formatetc)), mimeData);
#endif
      }
      return setData(data, pmedium);
   }
   return false;
}

QVector<FORMATETC> QBuiltInMimes::formatsForMime(const QString &mimeType, const QMimeData *mimeData) const
{
   QVector<FORMATETC> formatetcs;

   if (!outFormats.keys(mimeType).isEmpty() && mimeData->formats().contains(mimeType)) {
      formatetcs += setCf(outFormats.key(mimeType));
   }

   return formatetcs;
}

bool QBuiltInMimes::canConvertToMime(const QString &mimeType, IDataObject *pDataObj) const
{
   return (! inFormats.keys(mimeType).isEmpty())
          && canGetData(inFormats.key(mimeType), pDataObj);
}

QVariant QBuiltInMimes::convertToMime(const QString &mimeType, IDataObject *pDataObj, QVariant::Type preferredType) const
{
   QVariant val;
   if (canConvertToMime(mimeType, pDataObj)) {
      QByteArray data = getData(inFormats.key(mimeType), pDataObj);

      if (! data.isEmpty()) {

#ifdef QMIME_DEBUG
         qDebug("QBuiltInMimes::convertToMime()");
#endif
         if (mimeType == QLatin1String("text/html") && preferredType == QVariant::String) {
            // text/html is in wide chars on windows (compatible with Mozilla)
            val = QString::fromStdWString(std::wstring((const wchar_t *)data.constData()));

         } else {
            val = data; // it should be enough to return the data and let QMimeData do the rest.
         }
      }
   }
   return val;
}

QString QBuiltInMimes::mimeForFormat(const FORMATETC &formatetc) const
{
   return inFormats.value(getCf(formatetc));
}


class QLastResortMimes : public QWindowsMime
{
 public:

   QLastResortMimes();
   // for converting from Qt
   bool canConvertFromMime(const FORMATETC &formatetc, const QMimeData *mimeData) const override;
   bool convertFromMime(const FORMATETC &formatetc, const QMimeData *mimeData, STGMEDIUM *pmedium) const override;
   QVector<FORMATETC> formatsForMime(const QString &mimeType, const QMimeData *mimeData) const override;

   // for converting to Qt
   bool canConvertToMime(const QString &mimeType, IDataObject *pDataObj) const override;
   QVariant convertToMime(const QString &mime, IDataObject *pDataObj, QVariant::Type preferredType) const override;
   QString mimeForFormat(const FORMATETC &formatetc) const override;

 private:
   QMap<int, QString> formats;
   static QStringList ianaTypes;
   static QStringList excludeList;
};

QStringList QLastResortMimes::ianaTypes;
QStringList QLastResortMimes::excludeList;

QLastResortMimes::QLastResortMimes()
{
   //MIME Media-Types
   if (!ianaTypes.size()) {
      ianaTypes.append(QLatin1String("application/"));
      ianaTypes.append(QLatin1String("audio/"));
      ianaTypes.append(QLatin1String("example/"));
      ianaTypes.append(QLatin1String("image/"));
      ianaTypes.append(QLatin1String("message/"));
      ianaTypes.append(QLatin1String("model/"));
      ianaTypes.append(QLatin1String("multipart/"));
      ianaTypes.append(QLatin1String("text/"));
      ianaTypes.append(QLatin1String("video/"));
   }

   //Types handled by other classes
   if (!excludeList.size()) {
      excludeList.append(QLatin1String("HTML Format"));
      excludeList.append(QLatin1String("UniformResourceLocator"));
      excludeList.append(QLatin1String("text/html"));
      excludeList.append(QLatin1String("text/plain"));
      excludeList.append(QLatin1String("text/uri-list"));
      excludeList.append(QLatin1String("application/x-qt-image"));
      excludeList.append(QLatin1String("application/x-color"));
   }
}

bool QLastResortMimes::canConvertFromMime(const FORMATETC &formatetc, const QMimeData *mimeData) const
{
   // really check
#ifndef QT_NO_DRAGANDDROP
   return formatetc.tymed & TYMED_HGLOBAL
          && (formats.contains(formatetc.cfFormat)
              && QInternalMimeData::hasFormatHelper(formats.value(formatetc.cfFormat), mimeData));
#else
   Q_UNUSED(mimeData);
   Q_UNUSED(formatetc);
   return formatetc.tymed & TYMED_HGLOBAL
          && formats.contains(formatetc.cfFormat);
#endif //QT_NO_DRAGANDDROP
}

bool QLastResortMimes::convertFromMime(const FORMATETC &formatetc, const QMimeData *mimeData, STGMEDIUM *pmedium) const
{
#ifndef QT_NO_DRAGANDDROP
   return canConvertFromMime(formatetc, mimeData)
          && setData(QInternalMimeData::renderDataHelper(formats.value(getCf(formatetc)), mimeData), pmedium);
#else
   Q_UNUSED(mimeData);
   Q_UNUSED(formatetc);
   Q_UNUSED(pmedium);
   return false;
#endif //QT_NO_DRAGANDDROP
}

QVector<FORMATETC> QLastResortMimes::formatsForMime(const QString &mimeType, const QMimeData * /*mimeData*/) const
{
   QVector<FORMATETC> formatetcs;
   if (!formats.keys(mimeType).isEmpty()) {
      formatetcs += setCf(formats.key(mimeType));
   } else if (!excludeList.contains(mimeType, Qt::CaseInsensitive)) {
      // register any other available formats
      int cf = QWindowsMime::registerMimeType(mimeType);
      QLastResortMimes *that = const_cast<QLastResortMimes *>(this);
      that->formats.insert(cf, mimeType);
      formatetcs += setCf(cf);
   }
   return formatetcs;
}
static const char x_qt_windows_mime[] = "application/x-qt-windows-mime;value=\"";

static bool isCustomMimeType(const QString &mimeType)
{
   return mimeType.startsWith(QLatin1String(x_qt_windows_mime), Qt::CaseInsensitive);
}

static QString customMimeType(const QString &mimeType)
{
   int len = sizeof(x_qt_windows_mime) - 1;
   int n = mimeType.lastIndexOf(QLatin1Char('\"')) - len;
   return mimeType.mid(len, n);
}

bool QLastResortMimes::canConvertToMime(const QString &mimeType, IDataObject *pDataObj) const
{
   if (isCustomMimeType(mimeType)) {
      QString clipFormat = customMimeType(mimeType);
      int cf = RegisterClipboardFormat(clipFormat.toStdWString().c_str());
      return canGetData(cf, pDataObj);

   } else if (formats.keys(mimeType).isEmpty()) {
      // if it is not in there then register it an see if we can get it
      int cf = QWindowsMime::registerMimeType(mimeType);
      return canGetData(cf, pDataObj);

   } else {
      return canGetData(formats.key(mimeType), pDataObj);

   }

   return false;
}

QVariant QLastResortMimes::convertToMime(const QString &mimeType, IDataObject *pDataObj,
      QVariant::Type preferredType) const
{
   Q_UNUSED(preferredType);
   QVariant val;

   if (canConvertToMime(mimeType, pDataObj)) {
      QByteArray data;

      if (isCustomMimeType(mimeType)) {
         QString clipFormat = customMimeType(mimeType);
         int cf = RegisterClipboardFormat(clipFormat.toStdWString().c_str());
         data = getData(cf, pDataObj);

      } else if (formats.keys(mimeType).isEmpty()) {
         int cf = QWindowsMime::registerMimeType(mimeType);
         data = getData(cf, pDataObj);

      } else {
         data = getData(formats.key(mimeType), pDataObj);

      }

      if (!data.isEmpty()) {
         val = data;   // it should be enough to return the data and let QMimeData do the rest.
      }
   }
   return val;
}

QString QLastResortMimes::mimeForFormat(const FORMATETC &formatetc) const
{
   QString format = formats.value(getCf(formatetc));
   if (! format.isEmpty()) {
      return format;
   }

   std::wstring buffer(256, L'\0');
   int len = GetClipboardFormatName(getCf(formatetc), &buffer[0], 256);

   if (len) {
      QString clipFormat = QString::fromStdWString(buffer);

#ifndef QT_NO_DRAGANDDROP
      if (QInternalMimeData::canReadData(clipFormat)) {
         format = clipFormat;

      } else if ((formatetc.cfFormat >= 0xC000)) {
         //create the mime as custom. not registered.
         if (! excludeList.contains(clipFormat, Qt::CaseInsensitive)) {
            //check if this is a mime type
            bool ianaType = false;
            int sz = ianaTypes.size();

            for (int i = 0; i < sz; i++) {
               if (clipFormat.startsWith(ianaTypes[i], Qt::CaseInsensitive)) {
                  ianaType =  true;
                  break;
               }
            }

            if (!ianaType) {
               format = QLatin1String(x_qt_windows_mime) + clipFormat + '\"';
            } else {
               format = clipFormat;
            }
         }
      }
#endif

   }

   return format;
}

QWindowsMimeList::QWindowsMimeList()
   : initialized(false)
{
}

QWindowsMimeList::~QWindowsMimeList()
{
   while (mimes.size()) {
      delete mimes.first();
   }
}


void QWindowsMimeList::init()
{
   if (!initialized) {
      initialized = true;
#ifndef QT_NO_IMAGEFORMAT_BMP
      new QWindowsMimeImage;
#endif
      new QLastResortMimes;
      new QWindowsMimeText;
      new QWindowsMimeURI;

      new QWindowsMimeHtml;
      new QBuiltInMimes;
   }
}

void QWindowsMimeList::addWindowsMime(QWindowsMime *mime)
{
   init();
   mimes.append(mime);
}

void QWindowsMimeList::removeWindowsMime(QWindowsMime *mime)
{
   init();
   mimes.removeAll(mime);
}

QList<QWindowsMime *> QWindowsMimeList::windowsMimes()
{
   init();
   return mimes;
}

#ifndef QT_NO_IMAGEFORMAT_BMP
static bool qt_write_dibv5(QDataStream &s, QImage image)
{
   QIODevice *d = s.device();
   if (!d->isWritable()) {
      return false;
   }

   //depth will be always 32
   int bpl_bmp = image.width() * 4;

   BMP_BITMAPV5HEADER bi = {0};
   bi.bV5Size          = sizeof(BMP_BITMAPV5HEADER);
   bi.bV5Width         = image.width();
   bi.bV5Height        = image.height();
   bi.bV5Planes        = 1;
   bi.bV5BitCount      = 32;
   bi.bV5Compression   = BI_BITFIELDS;
   bi.bV5SizeImage     = bpl_bmp * image.height();
   bi.bV5XPelsPerMeter = 0;
   bi.bV5YPelsPerMeter = 0;
   bi.bV5ClrUsed       = 0;
   bi.bV5ClrImportant  = 0;
   bi.bV5BlueMask      = 0x000000ff;
   bi.bV5GreenMask     = 0x0000ff00;
   bi.bV5RedMask       = 0x00ff0000;
   bi.bV5AlphaMask     = 0xff000000;
   bi.bV5CSType        = BMP_LCS_sRGB;         //LCS_sRGB
   bi.bV5Intent        = BMP_LCS_GM_IMAGES;    //LCS_GM_IMAGES

   d->write(reinterpret_cast<const char *>(&bi), bi.bV5Size);
   if (s.status() != QDataStream::Ok) {
      return false;
   }

   DWORD colorSpace[3] = {0x00ff0000, 0x0000ff00, 0x000000ff};
   d->write(reinterpret_cast<const char *>(colorSpace), sizeof(colorSpace));
   if (s.status() != QDataStream::Ok) {
      return false;
   }

   if (image.format() != QImage::Format_ARGB32) {
      image = image.convertToFormat(QImage::Format_ARGB32);
   }

   uchar *buf = new uchar[bpl_bmp];
   uchar *b;

   memset(buf, 0, bpl_bmp);
   for (int y = image.height() - 1; y >= 0; y--) {
      // write the image bits
      QRgb *p = (QRgb *)image.scanLine(y);
      QRgb *end = p + image.width();
      b = buf;
      while (p < end) {
         int alpha = qAlpha(*p);
         if (alpha) {
            *b++ = qBlue(*p);
            *b++ = qGreen(*p);
            *b++ = qRed(*p);
         } else {
            //white for fully transparent pixels.
            *b++ = 0xff;
            *b++ = 0xff;
            *b++ = 0xff;
         }
         *b++ = alpha;
         p++;
      }
      d->write((char *)buf, bpl_bmp);
      if (s.status() != QDataStream::Ok) {
         delete[] buf;
         return false;
      }
   }
   delete[] buf;
   return true;
}

static int calc_shift(int mask)
{
   int result = 0;
   while (!(mask & 1)) {
      result++;
      mask >>= 1;
   }
   return result;
}

//Supports only 32 bit DIBV5
static bool qt_read_dibv5(QDataStream &s, QImage &image)
{
   BMP_BITMAPV5HEADER bi;
   QIODevice *d = s.device();
   if (d->atEnd()) {
      return false;
   }

   d->read((char *)&bi, sizeof(bi));   // read BITMAPV5HEADER header
   if (s.status() != QDataStream::Ok) {
      return false;
   }

   int nbits = bi.bV5BitCount;
   int comp = bi.bV5Compression;
   if (nbits != 32 || bi.bV5Planes != 1 || comp != BMP_BITFIELDS) {
      return false;   //Unsupported DIBV5 format
   }

   int w = bi.bV5Width, h = bi.bV5Height;
   int red_mask = bi.bV5RedMask;
   int green_mask = bi.bV5GreenMask;
   int blue_mask = bi.bV5BlueMask;
   int alpha_mask = bi.bV5AlphaMask;
   int red_shift = 0;
   int green_shift = 0;
   int blue_shift = 0;
   int alpha_shift = 0;
   QImage::Format format = QImage::Format_ARGB32;

   if (bi.bV5Height < 0) {
      h = -h;   // support images with negative height
   }
   if (image.size() != QSize(w, h) || image.format() != format) {
      image = QImage(w, h, format);
      if (image.isNull()) {   // could not create image
         return false;
      }
   }
   image.setDotsPerMeterX(bi.bV5XPelsPerMeter);
   image.setDotsPerMeterY(bi.bV5YPelsPerMeter);
   // read color table
   DWORD colorSpace[3];
   if (d->read((char *)colorSpace, sizeof(colorSpace)) != sizeof(colorSpace)) {
      return false;
   }

   red_shift = calc_shift(red_mask);
   green_shift = calc_shift(green_mask);
   blue_shift = calc_shift(blue_mask);
   if (alpha_mask) {
      alpha_shift = calc_shift(alpha_mask);
   }

   int  bpl = image.bytesPerLine();
   uchar *data = image.bits();
   QRgb *p;
   QRgb  *end;
   uchar *buf24 = new uchar[bpl];
   int    bpl24 = ((w * nbits + 31) / 32) * 4;
   uchar *b;
   unsigned int c;

   while (--h >= 0) {
      p = (QRgb *)(data + h * bpl);
      end = p + w;
      if (d->read((char *)buf24, bpl24) != bpl24) {
         break;
      }
      b = buf24;
      while (p < end) {
         c = *b | (*(b + 1)) << 8 | (*(b + 2)) << 16 | (*(b + 3)) << 24;
         *p++ = qRgba(((c & red_mask) >> red_shift) ,
                      ((c & green_mask) >> green_shift),
                      ((c & blue_mask) >> blue_shift),
                      ((c & alpha_mask) >> alpha_shift));
         b += 4;
      }
   }
   delete[] buf24;

   if (bi.bV5Height < 0) {
      // Flip the image
      uchar *buf = new uchar[bpl];
      h = -bi.bV5Height;
      for (int y = 0; y < h / 2; ++y) {
         memcpy(buf, data + y * bpl, bpl);
         memcpy(data + y * bpl, data + (h - y - 1)*bpl, bpl);
         memcpy(data + (h - y - 1)*bpl, buf, bpl);
      }
      delete [] buf;
   }

   return true;
}

#endif

QT_END_NAMESPACE
