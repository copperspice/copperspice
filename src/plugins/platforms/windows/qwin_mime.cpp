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

#include <qwin_mime.h>

#include <qalgorithms.h>
#include <qbuffer.h>
#include <qdebug.h>
#include <qdir.h>
#include <qimagereader.h>
#include <qimagewriter.h>
#include <qmap.h>
#include <qtextcodec.h>
#include <qurl.h>
#include <qwin_context.h>

#include <qdnd_p.h>

#include <shlobj.h>

/* The MSVC compilers allows multi-byte characters, that has the behavior of
 * that each character gets shifted into position. 0x73524742 below is for MSVC
 * equivalent to doing 'sRGB', but this does of course not work
 * on conformant C++ compilers. */

#define BMP_LCS_sRGB  0x73524742
#define BMP_LCS_GM_IMAGES  0x00000004L

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

static const char dibFormatC[] = "dib";

static inline QString msgConversionError(const QString &format)
{
   QString retval;

   retval += "Unable to convert DIB image. The image converter plugin for '";
   retval += format;
   retval += "' is not available. Available formats: ";

   for (const QString &item : QImageReader::supportedImageFormats()) {
      retval += item;
      retval += ' ';
   }

   return retval;
}

static inline QImage readDib(QByteArray data)
{
   QBuffer buffer(&data);
   buffer.open(QIODevice::ReadOnly);
   QImageReader reader(&buffer, dibFormatC);

   if (! reader.canRead()) {
      qWarning("readDib() %s", csPrintable(msgConversionError(dibFormatC)));
      return QImage();
   }

   return reader.read();
}

static QByteArray writeDib(const QImage &img)
{
   QByteArray ba;

   QBuffer buffer(&ba);
   buffer.open(QIODevice::ReadWrite);
   QImageWriter writer(&buffer, dibFormatC);

   if (! writer.canWrite()) {
      qWarning("writeDib() %s", csPrintable(msgConversionError(dibFormatC)));
      return ba;
   }

   if (! writer.write(img)) {
      ba.clear();
   }

   return ba;
}

static bool qt_write_dibv5(QDataStream &s, QImage image)
{
   QIODevice *d = s.device();
   if (!d->isWritable()) {
      return false;
   }

   //depth will be always 32
   int bpl_bmp = image.width() * 4;

   BMP_BITMAPV5HEADER bi;
   ZeroMemory(&bi, sizeof(bi));
   bi.bV5Size          = sizeof(BMP_BITMAPV5HEADER);
   bi.bV5Width         = image.width();
   bi.bV5Height        = image.height();
   bi.bV5Planes        = 1;
   bi.bV5BitCount      = 32;
   bi.bV5Compression   = BI_BITFIELDS;
   bi.bV5SizeImage     = DWORD(bpl_bmp * image.height());
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

   if (image.format() != QImage::Format_ARGB32) {
      image = image.convertToFormat(QImage::Format_ARGB32);
   }

   uchar *buf = new uchar[bpl_bmp];

   memset(buf, 0, size_t(bpl_bmp));
   for (int y = image.height() - 1; y >= 0; y--) {
      // write the image bits
      const QRgb *p = reinterpret_cast<const QRgb *>(image.constScanLine(y));
      const QRgb *end = p + image.width();
      uchar *b = buf;
      while (p < end) {
         int alpha = qAlpha(*p);
         if (alpha) {
            *b++ = uchar(qBlue(*p));
            *b++ = uchar(qGreen(*p));
            *b++ = uchar(qRed(*p));
         } else {
            //white for fully transparent pixels.
            *b++ = 0xff;
            *b++ = 0xff;
            *b++ = 0xff;
         }
         *b++ = uchar(alpha);
         p++;
      }
      d->write(reinterpret_cast<const char *>(buf), bpl_bmp);
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

   d->read(reinterpret_cast<char *>(&bi), sizeof(bi));   // read BITMAPV5HEADER header
   if (s.status() != QDataStream::Ok) {
      return false;
   }

   const int nbits = bi.bV5BitCount;
   if (nbits != 32 || bi.bV5Planes != 1 || bi.bV5Compression != BMP_BITFIELDS) {
      return false;   //Unsupported DIBV5 format
   }

   const int w = bi.bV5Width;
   int h = bi.bV5Height;
   const int red_mask = int(bi.bV5RedMask);
   const int green_mask = int(bi.bV5GreenMask);
   const int blue_mask = int(bi.bV5BlueMask);
   const int alpha_mask = int(bi.bV5AlphaMask);

   const QImage::Format format = QImage::Format_ARGB32;

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

   const int red_shift = calc_shift(red_mask);
   const int green_shift = calc_shift(green_mask);
   const int blue_shift = calc_shift(blue_mask);
   const int alpha_shift =  alpha_mask ? calc_shift(alpha_mask) : 0u;

   const int  bpl = image.bytesPerLine();
   uchar *data = image.bits();

   uchar *buf24 = new uchar[bpl];
   const int bpl24 = ((w * nbits + 31) / 32) * 4;

   while (--h >= 0) {
      QRgb *p = reinterpret_cast<QRgb *>(data + h * bpl);
      QRgb *end = p + w;
      if (d->read(reinterpret_cast<char *>(buf24), bpl24) != bpl24) {
         break;
      }
      const uchar *b = buf24;
      while (p < end) {
         const int c = *b | (*(b + 1)) << 8 | (*(b + 2)) << 16 | (*(b + 3)) << 24;
         *p++ = qRgba(((c & red_mask) >> red_shift),
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
         memcpy(buf, data + y * bpl, size_t(bpl));
         memcpy(data + y * bpl, data + (h - y - 1) * bpl, size_t(bpl));
         memcpy(data + (h - y - 1 ) * bpl, buf, size_t(bpl));
      }
      delete [] buf;
   }

   return true;
}

// helpers for using global memory

static int getCf(const FORMATETC &formatetc)
{
   return formatetc.cfFormat;
}

static FORMATETC setCf(int cf)
{
   FORMATETC formatetc;
   formatetc.cfFormat = CLIPFORMAT(cf);
   formatetc.dwAspect = DVASPECT_CONTENT;
   formatetc.lindex = -1;
   formatetc.ptd = nullptr;
   formatetc.tymed = TYMED_HGLOBAL;
   return formatetc;
}

static bool setData(const QByteArray &data, STGMEDIUM *pmedium)
{
   HGLOBAL hData = GlobalAlloc(0, SIZE_T(data.size()));
   if (!hData) {
      return false;
   }

   void *out = GlobalLock(hData);
   memcpy(out, data.data(), size_t(data.size()));
   GlobalUnlock(hData);
   pmedium->tymed = TYMED_HGLOBAL;
   pmedium->hGlobal = hData;
   pmedium->pUnkForRelease = nullptr;

   return true;
}

static QByteArray getData(int cf, IDataObject *pDataObj, int lindex = -1)
{
   QByteArray data;
   FORMATETC formatetc = setCf(cf);
   formatetc.lindex = lindex;
   STGMEDIUM s;
   if (pDataObj->GetData(&formatetc, &s) == S_OK) {
      const void *val = GlobalLock(s.hGlobal);
      data = QByteArray::fromRawData(reinterpret_cast<const char *>(val), int(GlobalSize(s.hGlobal)));
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
         HRESULT hr = s.pstm->Seek(pos, STREAM_SEEK_SET, nullptr);
         while (SUCCEEDED(hr)) {
            hr = s.pstm->Read(szBuffer, sizeof(szBuffer), &actualRead);
            if (SUCCEEDED(hr) && actualRead > 0) {
               data += QByteArray::fromRawData(szBuffer, int(actualRead));
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

QDebug operator<<(QDebug debug, const FORMATETC &tc)
{
   QDebugStateSaver saver(debug);
   debug.nospace();
   debug << "FORMATETC(cfFormat =" << tc.cfFormat << ' ';

   switch (tc.cfFormat) {
      case CF_TEXT:
         debug << "CF_TEXT";
         break;

      case CF_BITMAP:
         debug << "CF_BITMAP";
         break;

      case CF_TIFF:
         debug << "CF_TIFF";
         break;

      case CF_OEMTEXT:
         debug << "CF_OEMTEXT";
         break;

      case CF_DIB:
         debug << "CF_DIB";
         break;

      case CF_DIBV5:
         debug << "CF_DIBV5";
         break;

      case CF_UNICODETEXT:
         debug << "CF_UNICODETEXT";
         break;

      case CF_ENHMETAFILE:
         debug << "CF_ENHMETAFILE";
         break;

      default:
         debug << QWindowsMimeConverter::clipboardFormatName(tc.cfFormat);
         break;
   }

   debug << ", dwAspect =" << tc.dwAspect << ", lindex =" << tc.lindex
     << ", tymed =" << tc.tymed << ", ptd =" << tc.ptd << ')';

   return debug;
}

QDebug operator<<(QDebug debug, IDataObject *dataObj)
{
   QDebugStateSaver saver(debug);
   debug.nospace();
   debug.noquote();
   debug << "IDataObject(";

   if (dataObj) {
      // Output formats contained in IDataObject.
      IEnumFORMATETC *enumFormatEtc;

      if (SUCCEEDED(dataObj->EnumFormatEtc(DATADIR_GET, &enumFormatEtc)) && enumFormatEtc) {
         FORMATETC formatEtc[1];
         ULONG fetched;

         if (SUCCEEDED(enumFormatEtc->Reset())) {
            while (SUCCEEDED(enumFormatEtc->Next(1, formatEtc, &fetched)) && fetched) {
               debug << formatEtc[0] << ',';
            }
            enumFormatEtc->Release();
         }
      }

   } else {
      debug << '0';
   }

   debug << ')';

   return debug;
}

QWindowsMime::QWindowsMime()
{
}

QWindowsMime::~QWindowsMime()
{
}

int QWindowsMime::registerMimeType(const QString &mime)
{
   std::wstring tmp = mime.toStdWString();
   const UINT f = RegisterClipboardFormat(tmp.data());

   if (! f) {
      qErrnoWarning("QWindowsMime::registerMimeType: Failed to register clipboard format");
   }

   return int(f);
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
         data = mimeData->text().toUtf8();

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
                  if (!cr) {
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

         // same code as code below
         QString16 res;
         bool cr = false;

         for (auto ch : str) {
            if (ch == '\r') {
               cr = true;

            } else {
               if (ch == '\n' && ! cr) {
                  res.append('\r');
               }

               cr = false;
            }

            res.append(ch);
         }

         const int byteLength = res.size_storage() * int(sizeof(char16_t));

         QByteArray r(byteLength + 2, '\0');
         memcpy(r.data(), res.constData(), size_t(byteLength));

         return setData(r, pmedium);
      }
   }

   return false;
}

bool QWindowsMimeText::canConvertToMime(const QString &mimeType, IDataObject *pDataObj) const
{
   return mimeType.startsWith("text/plain")
         && (canGetData(CF_UNICODETEXT, pDataObj) || canGetData(CF_TEXT, pDataObj));
}

QString QWindowsMimeText::mimeForFormat(const FORMATETC &formatetc) const
{
   int cf = getCf(formatetc);

   if (cf == CF_UNICODETEXT || cf == CF_TEXT) {
      return QString("text/plain");
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
   QVariant retval;

   if (canConvertToMime(mime, pDataObj)) {
      QString str;

      QByteArray data = getData(CF_UNICODETEXT, pDataObj);

      if (data.isEmpty()) {

         data = getData(CF_TEXT, pDataObj);

         if (! data.isEmpty()) {
            const char *d    = data.data();
            const unsigned s = qstrlen(d);

            QByteArray r(data.size() + 1, '\0');
            char *o = r.data();
            int j = 0;

            for (unsigned i = 0; i < s; ++i) {
               char c = d[i];
               if (c != '\r') {
                  o[j++] = c;
               }
            }

            o[j] = 0;
            str = QString::fromUtf8(r);
         }


      } else {
         QTextCodec *codec = QTextCodec::codecForName("UTF-16");

         str = codec->toUnicode(data);

         while (str.endsWith(QChar('\0'))) {
            str.chop(1);
         }

         str.replace("\r\n", "\n");
      }

      if (preferredType == QVariant::String) {
         retval = str;                  // retval is a QVariant containing a QString
      } else {
         retval = str.toUtf8();         // retval is a QVariant containing a QByteArray
      }
   }

#if defined(CS_SHOW_DEBUG_PLATFORM)
   qDebug() << "QWindowsMimeText::convertToMime() " << retval;
#endif

   return retval;
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
   CF_INETURL_W = QWindowsMime::registerMimeType(QString("UniformResourceLocatorW"));
   CF_INETURL = QWindowsMime::registerMimeType(QString("UniformResourceLocator"));
}

bool QWindowsMimeURI::canConvertFromMime(const FORMATETC &formatetc, const QMimeData *mimeData) const
{
   if (mimeData->hasUrls() && getCf(formatetc) == CF_HDROP) {
      QList<QUrl> urls = mimeData->urls();
      for (int i = 0; i < urls.size(); i++) {
         if (!urls.at(i).toLocalFile().isEmpty()) {
            return true;
         }
      }
   }
   return (getCf(formatetc) == CF_INETURL_W || getCf(formatetc) == CF_INETURL) && mimeData->hasUrls();
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

            if (! fn.isEmpty()) {
               size += sizeof(ushort) * size_t(fn.length() + 1);
               fileNames.append(fn);
            }
         }

         QByteArray result(size, '\0');
         DROPFILES *d = reinterpret_cast<DROPFILES *>(result.data());
         d->pFiles = sizeof(DROPFILES);

         GetCursorPos(&d->pt); // try
         d->fNC = true;

         char *files = (reinterpret_cast<char *>(d)) + d->pFiles;
         d->fWide = true;

         wchar_t *f = reinterpret_cast<wchar_t *>(files);

         for (int i = 0; i < fileNames.size(); i++) {

            std::wstring tmp    = fileNames.at(i).toStdWString();
            const size_t length = tmp.size();

            memcpy(f, tmp.data(), length * sizeof(wchar_t));

            f += length;

            *f = 0;
            ++f;
         }

         *f = 0;

         return setData(result, pmedium);

      } else if (getCf(formatetc) == CF_INETURL_W) {
         QList<QUrl> urls = mimeData->urls();
         QByteArray result;

         if (! urls.isEmpty()) {
            QString url = urls.at(0).toString();

            QTextCodec *codec = QTextCodec::codecForName("UTF-16");
            result = codec->fromUnicode(url);
         }

         result.append('\0');
         result.append('\0');

         return setData(result, pmedium);

      } else if (getCf(formatetc) == CF_INETURL) {
         QList<QUrl> urls = mimeData->urls();
         QByteArray result;

         if (! urls.isEmpty()) {
            result = urls.at(0).toString().toUtf8();
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
      format = "text/uri-list";
   }

   return format;
}

QVector<FORMATETC> QWindowsMimeURI::formatsForMime(const QString &mimeType, const QMimeData *mimeData) const
{
   QVector<FORMATETC> formatics;

   if (mimeType == "text/uri-list") {
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
   if (mimeType == "text/uri-list") {
      if (canGetData(CF_HDROP, pDataObj)) {
         QList<QVariant> urls;

         QByteArray data = getData(CF_HDROP, pDataObj);
         if (data.isEmpty()) {
            return QVariant();
         }

         const DROPFILES *hdrop = reinterpret_cast<const DROPFILES *>(data.constData());
         if (hdrop->fWide) {
            const wchar_t *filesw = reinterpret_cast<const wchar_t *>(data.constData() + hdrop->pFiles);
            int i = 0;

            while (filesw[i]) {
               QString fileurl = QString::fromStdWString(std::wstring(filesw + i));
               urls += QUrl::fromLocalFile(fileurl);
               i += fileurl.length() + 1;
            }

         } else {
            const char *files = reinterpret_cast<const char *>(data.constData() + hdrop->pFiles);
            int i = 0;

            while (files[i]) {
               urls += QUrl::fromLocalFile(QString::fromUtf8(files + i));
               i += int(strlen(files + i)) + 1;
            }
         }

         if (preferredType == QVariant::Url && urls.size() == 1) {
            return urls.at(0);
         } else if (! urls.isEmpty()) {
            return urls;
         }

      } else if (canGetData(CF_INETURL_W, pDataObj)) {
         QByteArray data = getData(CF_INETURL_W, pDataObj);

         if (data.isEmpty()) {
            return QVariant();
         }

         return QUrl(QString::fromStdWString(std::wstring(reinterpret_cast<const wchar_t *>(data.constData()))));

      } else if (canGetData(CF_INETURL, pDataObj)) {
         QByteArray data = getData(CF_INETURL, pDataObj);

         if (data.isEmpty()) {
            return QVariant();
         }

         return QUrl(QString::fromUtf8(data.constData()));
      }
   }

   return QVariant();
}

class QWindowsMimeHtml : public QWindowsMime
{
 public:
   QWindowsMimeHtml();

   // for converting from CS
   bool canConvertFromMime(const FORMATETC &formatetc, const QMimeData *mimeData) const override;
   bool convertFromMime(const FORMATETC &formatetc, const QMimeData *mimeData, STGMEDIUM *pmedium) const override;

   QVector<FORMATETC> formatsForMime(const QString &mimeType, const QMimeData *mimeData) const override;

   // for converting to CS
   bool canConvertToMime(const QString &mimeType, IDataObject *pDataObj) const override;
   QVariant convertToMime(const QString &mime, IDataObject *pDataObj, QVariant::Type preferredType) const override;
   QString mimeForFormat(const FORMATETC &formatetc) const override;

 private:
   int CF_HTML;
};

QWindowsMimeHtml::QWindowsMimeHtml()
{
   CF_HTML = QWindowsMime::registerMimeType(QString("HTML Format"));
}

QVector<FORMATETC> QWindowsMimeHtml::formatsForMime(const QString &mimeType, const QMimeData *mimeData) const
{
   QVector<FORMATETC> formatetcs;

   if (mimeType == QString("text/html") && (! mimeData->html().isEmpty())) {
      formatetcs += setCf(CF_HTML);
   }

   return formatetcs;
}

QString QWindowsMimeHtml::mimeForFormat(const FORMATETC &formatetc) const
{
   if (getCf(formatetc) == CF_HTML) {
      return QString("text/html");
   }

   return QString();
}

bool QWindowsMimeHtml::canConvertToMime(const QString &mimeType, IDataObject *pDataObj) const
{
   return mimeType == QString("text/html") && canGetData(CF_HTML, pDataObj);
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
   (void) preferredType;

   QVariant result;

   if (canConvertToMime(mime, pDataObj)) {
      QByteArray html = getData(CF_HTML, pDataObj);

      int start = html.indexOf("StartHTML:");
      int end   = html.indexOf("EndHTML:");

      if (start != -1) {
         int startOffset = start + 10;
         int i = startOffset;
         while (html.at(i) != '\r' && html.at(i) != '\n') {
            ++i;
         }
         QByteArray bytecount = html.mid(startOffset, i - startOffset);
         start = bytecount.toInt();
      }

      if (end != -1) {
         int endOffset = end + 8;
         int i = endOffset ;
         while (html.at(i) != '\r' && html.at(i) != '\n') {
            ++i;
         }
         QByteArray bytecount = html.mid(endOffset, i - endOffset);
         end = bytecount.toInt();
      }

      if (end > start && start > 0) {
         html = html.mid(start, end - start);
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
         "StartHTML:0000000107\r\n"           // 13-35
         "EndHTML:0000000000\r\n"             // 36-55
         "StartFragment:0000000000\r\n"       // 56-81
         "EndFragment:0000000000\r\n\r\n";    // 82-107

      if (data.indexOf("<!--StartFragment-->") == -1) {
         result += "<!--StartFragment-->";
      }
      result += data;
      if (data.indexOf("<!--EndFragment-->") == -1) {
         result += "<!--EndFragment-->";
      }

      // set the correct number for EndHTML
      QByteArray pos = QByteArray::number(result.size());
      memcpy(reinterpret_cast<char *>(result.data() + 53 - pos.length()), pos.constData(), size_t(pos.length()));

      // set correct numbers for StartFragment and EndFragment
      pos = QByteArray::number(result.indexOf("<!--StartFragment-->") + 20);
      memcpy(reinterpret_cast<char *>(result.data() + 79 - pos.length()), pos.constData(), size_t(pos.length()));
      pos = QByteArray::number(result.indexOf("<!--EndFragment-->"));
      memcpy(reinterpret_cast<char *>(result.data() + 103 - pos.length()), pos.constData(), size_t(pos.length()));

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

   // for converting to CS
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

   if (mimeData->hasImage() && mimeType == "application/x-qt-image") {
      //add DIBV5 if image has alpha channel. Do not add CF_PNG here as it will confuse MS Office (QTBUG47656).
      QImage image = (mimeData->imageData()).value<QImage>();

      if (!image.isNull() && image.hasAlphaChannel()) {
         formatetcs += setCf(CF_DIBV5);
      }

      formatetcs += setCf(CF_DIB);
   }

#if defined(CS_SHOW_DEBUG_PLATFORM)
   if (! formatetcs.isEmpty()) {
      qDebug() << "QWindowsMimeImage::formatsForMime() Mime type = " << mimeType << "\n" << formatetcs;
   }
#endif

   return formatetcs;
}

QString QWindowsMimeImage::mimeForFormat(const FORMATETC &formatetc) const
{
   int cf = getCf(formatetc);

   if (cf == CF_DIB || cf == CF_DIBV5 || cf == int(CF_PNG)) {
      return QString("application/x-qt-image");
   }

   return QString();
}

bool QWindowsMimeImage::canConvertToMime(const QString &mimeType, IDataObject *pDataObj) const
{
   if (mimeType == "application/x-qt-image" && (canGetData(CF_DIB, pDataObj) || canGetData(CF_PNG, pDataObj))) {
      return true;
   }

   return false;
}

bool QWindowsMimeImage::canConvertFromMime(const FORMATETC &formatetc, const QMimeData *mimeData) const
{
   int cf = getCf(formatetc);

   if (!mimeData->hasImage()) {
      return false;
   }

   const QImage image = (mimeData->imageData()).value<QImage>();
   if (image.isNull()) {
      return false;
   }

   return cf == CF_DIBV5 || (cf == CF_DIB) || cf == int(CF_PNG);
}

bool QWindowsMimeImage::convertFromMime(const FORMATETC &formatetc, const QMimeData *mimeData, STGMEDIUM *pmedium) const
{
   int cf = getCf(formatetc);
   if ((cf == CF_DIB || cf == CF_DIBV5 || cf == int(CF_PNG)) && mimeData->hasImage()) {
      QImage img = (mimeData->imageData()).value<QImage>();

      if (img.isNull()) {
         return false;
      }
      QByteArray ba;
      if (cf == CF_DIB) {
         if (img.format() > QImage::Format_ARGB32) {
            img = img.convertToFormat(QImage::Format_RGB32);
         }
         const QByteArray ba = writeDib(img);
         if (!ba.isEmpty()) {
            return setData(ba, pmedium);
         }
      } else if (cf == int(CF_PNG)) {
         QBuffer buffer(&ba);
         const bool written = buffer.open(QIODevice::WriteOnly) && img.save(&buffer, "PNG");
         buffer.close();
         if (written) {
            return setData(ba, pmedium);
         }
      } else {
         QDataStream s(&ba, QIODevice::WriteOnly);
         s.setByteOrder(QDataStream::LittleEndian);// Intel byte order ####
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
   IEnumFORMATETC *pEnum = nullptr;
   HRESULT res = pDataObj->EnumFormatEtc(1, &pEnum);

   if (res == S_OK && pEnum) {
      FORMATETC fc;

      while ((res = pEnum->Next(1, &fc, nullptr)) == S_OK) {
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

QVariant QWindowsMimeImage::convertToMime(const QString &mimeType, IDataObject *pDataObj, QVariant::Type preferredType) const
{
   (void) preferredType;

   QVariant result;

   if (mimeType != "application/x-qt-image") {
      return result;
   }

   //Try to convert from a format which has more data
   //DIBV5, use only if its is not synthesized
   if (canGetData(CF_DIBV5, pDataObj) && hasOriginalDIBV5(pDataObj)) {
      QImage img;
      QByteArray data = getData(CF_DIBV5, pDataObj);
      QDataStream s(&data, QIODevice::ReadOnly);

      s.setByteOrder(QDataStream::LittleEndian);
      if (qt_read_dibv5(s, img)) {
         // #### supports only 32bit DIBV5
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
      const QImage img = readDib(getData(CF_DIB, pDataObj));
      if (!img.isNull()) {
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

   // for converting from cs
   bool canConvertFromMime(const FORMATETC &formatetc, const QMimeData *mimeData) const override;
   bool convertFromMime(const FORMATETC &formatetc, const QMimeData *mimeData, STGMEDIUM *pmedium) const override;
   QVector<FORMATETC> formatsForMime(const QString &mimeType, const QMimeData *mimeData) const override;

   // for converting to cs
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
   outFormats.insert(QWindowsMime::registerMimeType(QString("application/x-color")), QString("application/x-color"));
   inFormats.insert(QWindowsMime::registerMimeType(QString("application/x-color")), QString("application/x-color"));
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

         // same code as in the text converter up above
         QString16 res;
         bool cr = false;

         for (auto ch : html) {
            if (ch == '\r') {
               cr = true;

            } else {
               if (ch == '\n' && ! cr) {
                  res.append('\r');
               }

               cr = false;
            }

            res.append(ch);
         }

         const int byteLength = res.size_storage() * int(sizeof(char16_t));

         QByteArray r(byteLength + 2, '\0');
         memcpy(r.data(), res.constData(), size_t(byteLength));
         data = r;

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
   return (!inFormats.keys(mimeType).isEmpty())
      && canGetData(inFormats.key(mimeType), pDataObj);
}

QVariant QBuiltInMimes::convertToMime(const QString &mimeType, IDataObject *pDataObj, QVariant::Type preferredType) const
{
   QVariant val;
   if (canConvertToMime(mimeType, pDataObj)) {
      QByteArray data = getData(inFormats.key(mimeType), pDataObj);

      if (! data.isEmpty()) {

         if (mimeType == "text/html" && preferredType == QVariant::String) {
            // text/html is in wide chars on windows (compatible with Mozilla)

            std::wstring tmp = reinterpret_cast<const wchar_t *>(data.constData());
            val = QString::fromStdWString(tmp);

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
      ianaTypes.append(QString("application/"));
      ianaTypes.append(QString("audio/"));
      ianaTypes.append(QString("example/"));
      ianaTypes.append(QString("image/"));
      ianaTypes.append(QString("message/"));
      ianaTypes.append(QString("model/"));
      ianaTypes.append(QString("multipart/"));
      ianaTypes.append(QString("text/"));
      ianaTypes.append(QString("video/"));
   }
   //Types handled by other classes
   if (!excludeList.size()) {
      excludeList.append(QString("HTML Format"));
      excludeList.append(QString("UniformResourceLocator"));
      excludeList.append(QString("text/html"));
      excludeList.append(QString("text/plain"));
      excludeList.append(QString("text/uri-list"));
      excludeList.append(QString("application/x-qt-image"));
      excludeList.append(QString("application/x-color"));
   }
}

bool QLastResortMimes::canConvertFromMime(const FORMATETC &formatetc, const QMimeData *mimeData) const
{
// really check
#ifndef QT_NO_DRAGANDDROP
   return formatetc.tymed & TYMED_HGLOBAL && (formats.contains(formatetc.cfFormat)
         && QInternalMimeData::hasFormatHelper(formats.value(formatetc.cfFormat), mimeData));

#else
   return formatetc.tymed & TYMED_HGLOBAL && formats.contains(formatetc.cfFormat);

#endif
}

bool QLastResortMimes::convertFromMime(const FORMATETC &formatetc, const QMimeData *mimeData, STGMEDIUM *pmedium) const
{
#ifndef QT_NO_DRAGANDDROP
   return canConvertFromMime(formatetc, mimeData)
      && setData(QInternalMimeData::renderDataHelper(formats.value(getCf(formatetc)), mimeData), pmedium);
#else
   return false;
#endif
}

QVector<FORMATETC> QLastResortMimes::formatsForMime(const QString &mimeType, const QMimeData *) const
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

#if defined(CS_SHOW_DEBUG_PLATFORM)
   if (! formatetcs.isEmpty()) {
      qDebug() << "QLastResortMimes::formatsForMime() Mime type = " << mimeType << formatetcs;
   }
#endif

   return formatetcs;
}

static const char x_qt_windows_mime[] = "application/x-qt-windows-mime;value=\"";

static bool isCustomMimeType(const QString &mimeType)
{
   return mimeType.startsWith(QString(x_qt_windows_mime), Qt::CaseInsensitive);
}

static QString customMimeType(const QString &mimeType, int *lindex = nullptr)
{
   int len = sizeof(x_qt_windows_mime) - 1;
   int n = mimeType.lastIndexOf('\"') - len;

   QString ret = mimeType.mid(len, n);

   const int beginPos = mimeType.indexOf(";index=");

   if (beginPos > -1) {
      const int endPos = mimeType.indexOf(';', beginPos + 1);
      const int indexStartPos = beginPos + 7;

      if (lindex) {
         *lindex = QStringParser::toInteger<int>(mimeType.midView(indexStartPos, endPos == -1 ? endPos : endPos - indexStartPos));
      }

   } else {
      if (lindex) {
         *lindex = -1;
      }
   }

   return ret;
}

bool QLastResortMimes::canConvertToMime(const QString &mimeType, IDataObject *pDataObj) const
{
   if (isCustomMimeType(mimeType)) {
      // MSDN documentation for QueryGetData says only -1 is supported, so ignore lindex here.
      QString clipFormat = customMimeType(mimeType);

      std::wstring tmp = clipFormat.toStdWString();
      const UINT cf = RegisterClipboardFormat(tmp.data());
      return canGetData(int(cf), pDataObj);

   } else if (formats.keys(mimeType).isEmpty()) {
      // if it is not in there then register it and see if we can get it
      int cf = QWindowsMime::registerMimeType(mimeType);
      return canGetData(cf, pDataObj);

   } else {
      return canGetData(formats.key(mimeType), pDataObj);
   }

   return false;
}

QVariant QLastResortMimes::convertToMime(const QString &mimeType, IDataObject *pDataObj, QVariant::Type preferredType) const
{
   (void) preferredType;

   QVariant retval;

   if (canConvertToMime(mimeType, pDataObj)) {
      QByteArray data;

      if (isCustomMimeType(mimeType)) {
         int lindex;

         QString clipFormat = customMimeType(mimeType, &lindex);

         const UINT cf = RegisterClipboardFormat(clipFormat.toStdWString().data());
         data = getData(int(cf), pDataObj, lindex);

      } else if (formats.keys(mimeType).isEmpty()) {
         int cf = QWindowsMime::registerMimeType(mimeType);
         data = getData(cf, pDataObj);

      } else {
         data = getData(formats.key(mimeType), pDataObj);
      }

      if (! data.isEmpty()) {
         retval = data;   // it should be enough to return the data and let QMimeData do the rest.
      }
   }

   return retval;
}

QString QLastResortMimes::mimeForFormat(const FORMATETC &formatetc) const
{
   QString format = formats.value(getCf(formatetc));
   if (! format.isEmpty()) {
      return format;
   }

   const QString clipFormat = QWindowsMimeConverter::clipboardFormatName(getCf(formatetc));

   if (! clipFormat.isEmpty()) {

#ifndef QT_NO_DRAGANDDROP
      if (QInternalMimeData::canReadData(clipFormat)) {
         format = clipFormat;

      } else if ((formatetc.cfFormat >= 0xC000)) {
         //create the mime as custom. not registered.

         if (!excludeList.contains(clipFormat, Qt::CaseInsensitive)) {
            //check if this is a mime type
            bool ianaType = false;
            int sz = ianaTypes.size();

            for (int i = 0; i < sz; i++) {
               if (clipFormat.startsWith(ianaTypes[i], Qt::CaseInsensitive)) {
                  ianaType =  true;
                  break;
               }
            }

            if (! ianaType) {
               format = x_qt_windows_mime + clipFormat + '\"';
            } else {
               format = clipFormat;
            }
         }
      }
#endif

   }

   return format;
}

QWindowsMimeConverter::QWindowsMimeConverter() : m_internalMimeCount(0)
{
}

QWindowsMimeConverter::~QWindowsMimeConverter()
{
   qDeleteAll(m_mimes.begin(), m_mimes.begin() + m_internalMimeCount);
}

QWindowsMime *QWindowsMimeConverter::converterToMime(const QString &mimeType, IDataObject *pDataObj) const
{
   ensureInitialized();

   for (int i = m_mimes.size() - 1; i >= 0; --i) {
      if (m_mimes.at(i)->canConvertToMime(mimeType, pDataObj)) {
         return m_mimes.at(i);
      }
   }

   return nullptr;
}

QStringList QWindowsMimeConverter::allMimesForFormats(IDataObject *pDataObj) const
{
   ensureInitialized();

   QStringList formats;
   LPENUMFORMATETC FAR fmtenum;
   HRESULT hr = pDataObj->EnumFormatEtc(DATADIR_GET, &fmtenum);

   if (hr == NOERROR) {
      FORMATETC fmtetc;

      while (S_OK == fmtenum->Next(1, &fmtetc, nullptr)) {

         for (int i = m_mimes.size() - 1; i >= 0; --i) {
            QString format = m_mimes.at(i)->mimeForFormat(fmtetc);

            if (! format.isEmpty() && !formats.contains(format)) {
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

QWindowsMime *QWindowsMimeConverter::converterFromMime(const FORMATETC &formatetc, const QMimeData *mimeData) const
{
   ensureInitialized();

   for (int i = m_mimes.size() - 1; i >= 0; --i) {
      if (m_mimes.at(i)->canConvertFromMime(formatetc, mimeData)) {
         return m_mimes.at(i);
      }
   }

   return nullptr;
}

QVector<FORMATETC> QWindowsMimeConverter::allFormatsForMime(const QMimeData *mimeData) const
{
   ensureInitialized();
   QVector<FORMATETC> formatics;

#ifndef QT_NO_DRAGANDDROP
   formatics.reserve(20);

   const QStringList formats = QInternalMimeData::formatsHelper(mimeData);

   for (int f = 0; f < formats.size(); ++f) {
      for (int i = m_mimes.size() - 1; i >= 0; --i) {
         formatics += m_mimes.at(i)->formatsForMime(formats.at(f), mimeData);
      }
   }
#endif

   return formatics;
}

void QWindowsMimeConverter::ensureInitialized() const
{
   if (m_mimes.isEmpty()) {

#ifndef QT_NO_IMAGEFORMAT_BMP
      m_mimes << new QWindowsMimeImage;
#endif

      m_mimes << new QLastResortMimes
              << new QWindowsMimeText << new QWindowsMimeURI
              << new QWindowsMimeHtml << new QBuiltInMimes;

      m_internalMimeCount = m_mimes.size();
   }
}

QString QWindowsMimeConverter::clipboardFormatName(int cf)
{
   QString retval;

   wchar_t buffer[256] = {0};
   int length = GetClipboardFormatName(UINT(cf), buffer, 255);

   if (length != 0) {
      retval = QString::fromStdWString(std::wstring(buffer));
   }

   return retval;
}

QVariant QWindowsMimeConverter::convertToMime(const QStringList &mimeTypes, IDataObject *pDataObj,
   QVariant::Type preferredType, QString *formatIn ) const
{
   for (const QString &format : mimeTypes) {
      if (const QWindowsMime *converter = converterToMime(format, pDataObj)) {

         if (converter->canConvertToMime(format, pDataObj)) {
            const QVariant dataV = converter->convertToMime(format, pDataObj, preferredType);

            if (dataV.isValid()) {
               if (formatIn) {
                  *formatIn = format;
               }

               return dataV;
            }
         }
      }
   }

   return QVariant();
}

void QWindowsMimeConverter::registerMime(QWindowsMime *mime)
{
   ensureInitialized();
   m_mimes.append(mime);
}

