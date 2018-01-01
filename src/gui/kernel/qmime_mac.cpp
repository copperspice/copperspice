/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#include <qmime.h>

//#define USE_INTERNET_CONFIG

#ifndef USE_INTERNET_CONFIG
# include <qfile.h>
# include <qfileinfo.h>
# include <qtextstream.h>
# include <qdir.h>
# include <unistd.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <sys/fcntl.h>
#endif

#include <qdebug.h>
#include <qpixmap.h>
#include <qimagewriter.h>
#include <qimagereader.h>
#include <qdatastream.h>
#include <qbuffer.h>
#include <qdatetime.h>
#include <qapplication_p.h>
#include <qtextcodec.h>
#include <qregexp.h>
#include <qurl.h>
#include <qmap.h>
#include <qt_mac_p.h>

QT_BEGIN_NAMESPACE


// #define DEBUG_MIME_MAPS

extern CGImageRef qt_mac_createCGImageFromQImage(const QImage &img,
      const QImage **imagePtr = 0); // qpaintengine_mac.cpp

typedef QList<QMacPasteboardMime *> MimeList;
Q_GLOBAL_STATIC(MimeList, globalMimeList)

static void cleanup_mimes()
{
   MimeList *mimes = globalMimeList();
   while (!mimes->isEmpty()) {
      delete mimes->takeFirst();
   }
}

Q_GLOBAL_STATIC(QStringList, globalDraggedTypesList)

Q_GUI_EXPORT void qRegisterDraggedTypes(const QStringList &types)
{
   (*globalDraggedTypesList()) += types;
}

const QStringList &qEnabledDraggedTypes()
{
   return (*globalDraggedTypesList());
}

/*!
  Constructs a new conversion object of type \a t, adding it to the
  globally accessed list of available convertors.
*/
QMacPasteboardMime::QMacPasteboardMime(char t) : type(t)
{
   globalMimeList()->append(this);
}

/*!
  Destroys a conversion object, removing it from the global
  list of available convertors.
*/
QMacPasteboardMime::~QMacPasteboardMime()
{
   if (!QApplication::closingDown()) {
      globalMimeList()->removeAll(this);
   }
}

class QMacPasteboardMimeAny : public QMacPasteboardMime
{

 public:
   QMacPasteboardMimeAny() : QMacPasteboardMime(MIME_QT_CONVERTOR | MIME_ALL) { }

   ~QMacPasteboardMimeAny() { }

   QString convertorName();

   QString flavorFor(const QString &mime);
   QString mimeFor(QString flav);
   bool canConvert(const QString &mime, QString flav);
   QVariant convertToMime(const QString &mime, QList<QByteArray> data, QString flav);
   QList<QByteArray> convertFromMime(const QString &mime, QVariant data, QString flav);
};

QString QMacPasteboardMimeAny::convertorName()
{
   return QLatin1String("Any-Mime");
}

QString QMacPasteboardMimeAny::flavorFor(const QString &mime)
{
   // do not handle the mime type name in the drag pasteboard
   if (mime == QLatin1String("application/x-qt-mime-type-name")) {
      return QString();
   }

   QString ret = QLatin1String("com.copperspice.anymime.") + mime;
   return ret.replace(QLatin1Char('/'), QLatin1String("--"));
}

QString QMacPasteboardMimeAny::mimeFor(QString flav)
{
   const QString any_prefix = QLatin1String("com.copperspice.anymime.");

   if (flav.size() > any_prefix.length() && flav.startsWith(any_prefix)) {
      return flav.mid(any_prefix.length()).replace(QLatin1String("--"), QLatin1String("/"));
   }

   return QString();
}

bool QMacPasteboardMimeAny::canConvert(const QString &mime, QString flav)
{
   return mimeFor(flav) == mime;
}

QVariant QMacPasteboardMimeAny::convertToMime(const QString &mime, QList<QByteArray> data, QString)
{
   if (data.count() > 1) {
      qWarning("QMacPasteboardMimeAny() Can not handle multiple member data");
   }

   QVariant ret;
   if (mime == QLatin1String("text/plain")) {
      ret = QString::fromUtf8(data.first());
   } else {
      ret = data.first();
   }
   return ret;
}

QList<QByteArray> QMacPasteboardMimeAny::convertFromMime(const QString &mime, QVariant data, QString)
{
   QList<QByteArray> ret;
   if (mime == QLatin1String("text/plain")) {
      ret.append(data.toString().toUtf8());
   } else {
      ret.append(data.toByteArray());
   }
   return ret;
}

class QMacPasteboardMimeTypeName : public QMacPasteboardMime
{
 private:

 public:
   QMacPasteboardMimeTypeName() : QMacPasteboardMime(MIME_QT_CONVERTOR | MIME_ALL) {}
   ~QMacPasteboardMimeTypeName() { }

   QString convertorName();

   QString flavorFor(const QString &mime);
   QString mimeFor(QString flav);
   bool canConvert(const QString &mime, QString flav);
   QVariant convertToMime(const QString &mime, QList<QByteArray> data, QString flav);
   QList<QByteArray> convertFromMime(const QString &mime, QVariant data, QString flav);
};

QString QMacPasteboardMimeTypeName::convertorName()
{
   return QLatin1String("Qt-Mime-Type");
}

QString QMacPasteboardMimeTypeName::flavorFor(const QString &mime)
{
   if (mime == QLatin1String("application/x-qt-mime-type-name")) {
      return QLatin1String("com.copperspice.MimeTypeName");
   }

   return QString();
}

QString QMacPasteboardMimeTypeName::mimeFor(QString)
{
   return QString();
}

bool QMacPasteboardMimeTypeName::canConvert(const QString &, QString)
{
   return false;
}

QVariant QMacPasteboardMimeTypeName::convertToMime(const QString &, QList<QByteArray>, QString)
{
   QVariant ret;
   return ret;
}

QList<QByteArray> QMacPasteboardMimeTypeName::convertFromMime(const QString &, QVariant, QString)
{
   QList<QByteArray> ret;
   ret.append(QString("x-qt-mime-type-name").toUtf8());
   return ret;
}

class QMacPasteboardMimePlainText : public QMacPasteboardMime
{
 public:
   QMacPasteboardMimePlainText() : QMacPasteboardMime(MIME_ALL) { }
   QString convertorName();

   QString flavorFor(const QString &mime);
   QString mimeFor(QString flav);
   bool canConvert(const QString &mime, QString flav);
   QVariant convertToMime(const QString &mime, QList<QByteArray> data, QString flav);
   QList<QByteArray> convertFromMime(const QString &mime, QVariant data, QString flav);
};

QString QMacPasteboardMimePlainText::convertorName()
{
   return QLatin1String("PlainText");
}

QString QMacPasteboardMimePlainText::flavorFor(const QString &mime)
{
   if (mime == QLatin1String("text/plain")) {
      return QLatin1String("com.apple.traditional-mac-plain-text");
   }
   return QString();
}

QString QMacPasteboardMimePlainText::mimeFor(QString flav)
{
   if (flav == QLatin1String("com.apple.traditional-mac-plain-text")) {
      return QLatin1String("text/plain");
   }
   return QString();
}

bool QMacPasteboardMimePlainText::canConvert(const QString &mime, QString flav)
{
   return flavorFor(mime) == flav;
}

QVariant QMacPasteboardMimePlainText::convertToMime(const QString &mimetype, QList<QByteArray> data, QString flavor)
{
   if (data.count() > 1) {
      qWarning("QMacPasteboardMimePlainText: Cannot handle multiple member data");
   }
   const QByteArray &firstData = data.first();
   QVariant ret;
   if (flavor == QCFString(QLatin1String("com.apple.traditional-mac-plain-text"))) {
      QCFString str(CFStringCreateWithBytes(kCFAllocatorDefault,
                                            reinterpret_cast<const UInt8 *>(firstData.constData()),
                                            firstData.size(), CFStringGetSystemEncoding(), false));
      ret = QString(str);
   } else {
      qWarning("QMime::convertToMime: unhandled mimetype: %s", qPrintable(mimetype));
   }
   return ret;
}

QList<QByteArray> QMacPasteboardMimePlainText::convertFromMime(const QString &, QVariant data, QString flavor)
{
   QList<QByteArray> ret;
   QString string = data.toString();
   if (flavor == QCFString(QLatin1String("com.apple.traditional-mac-plain-text"))) {
      ret.append(string.toLatin1());
   }
   return ret;
}

class QMacPasteboardMimeUnicodeText : public QMacPasteboardMime
{
 public:
   QMacPasteboardMimeUnicodeText() : QMacPasteboardMime(MIME_ALL) { }
   QString convertorName();

   QString flavorFor(const QString &mime);
   QString mimeFor(QString flav);
   bool canConvert(const QString &mime, QString flav);
   QVariant convertToMime(const QString &mime, QList<QByteArray> data, QString flav);
   QList<QByteArray> convertFromMime(const QString &mime, QVariant data, QString flav);
};

QString QMacPasteboardMimeUnicodeText::convertorName()
{
   return QLatin1String("UnicodeText");
}

QString QMacPasteboardMimeUnicodeText::flavorFor(const QString &mime)
{
   if (mime == QLatin1String("text/plain")) {
      return QLatin1String("public.utf16-plain-text");
   }
   int i = mime.indexOf(QLatin1String("charset="));
   if (i >= 0) {
      QString cs(mime.mid(i + 8).toLower());
      i = cs.indexOf(QLatin1Char(';'));
      if (i >= 0) {
         cs = cs.left(i);
      }
      if (cs == QLatin1String("system")) {
         return QLatin1String("public.utf8-plain-text");
      } else if (cs == QLatin1String("iso-10646-ucs-2")
                 || cs == QLatin1String("utf16")) {
         return QLatin1String("public.utf16-plain-text");
      }
   }
   return QString();
}

QString QMacPasteboardMimeUnicodeText::mimeFor(QString flav)
{
   if (flav == QLatin1String("public.utf16-plain-text") || flav == QLatin1String("public.utf8-plain-text")) {
      return QLatin1String("text/plain");
   }
   return QString();
}

bool QMacPasteboardMimeUnicodeText::canConvert(const QString &mime, QString flav)
{
   return flavorFor(mime) == flav;
}

QVariant QMacPasteboardMimeUnicodeText::convertToMime(const QString &mimetype, QList<QByteArray> data, QString flavor)
{
   if (data.count() > 1) {
      qWarning("QMacPasteboardMimeUnicodeText: Cannot handle multiple member data");
   }
   const QByteArray &firstData = data.first();
   // I can only handle two types (system and unicode) so deal with them that way
   QVariant ret;
   if (flavor == QLatin1String("public.utf8-plain-text")) {
      QCFString str(CFStringCreateWithBytes(kCFAllocatorDefault,
                                            reinterpret_cast<const UInt8 *>(firstData.constData()),
                                            firstData.size(), CFStringGetSystemEncoding(), false));
      ret = QString(str);
   } else if (flavor == QLatin1String("public.utf16-plain-text")) {
      ret = QString(reinterpret_cast<const QChar *>(firstData.constData()),
                    firstData.size() / sizeof(QChar));
   } else {
      qWarning("QMime::convertToMime: unhandled mimetype: %s", qPrintable(mimetype));
   }
   return ret;
}

QList<QByteArray> QMacPasteboardMimeUnicodeText::convertFromMime(const QString &, QVariant data, QString flavor)
{
   QList<QByteArray> ret;
   QString string = data.toString();
   if (flavor == QLatin1String("public.utf8-plain-text")) {
      ret.append(string.toUtf8());
   } else if (flavor == QLatin1String("public.utf16-plain-text")) {
      ret.append(QByteArray((char *)string.utf16(), string.length() * 2));
   }
   return ret;
}

class QMacPasteboardMimeHTMLText : public QMacPasteboardMime
{
 public:
   QMacPasteboardMimeHTMLText() : QMacPasteboardMime(MIME_ALL) { }
   QString convertorName();

   QString flavorFor(const QString &mime);
   QString mimeFor(QString flav);
   bool canConvert(const QString &mime, QString flav);
   QVariant convertToMime(const QString &mime, QList<QByteArray> data, QString flav);
   QList<QByteArray> convertFromMime(const QString &mime, QVariant data, QString flav);
};

QString QMacPasteboardMimeHTMLText::convertorName()
{
   return QLatin1String("HTML");
}

QString QMacPasteboardMimeHTMLText::flavorFor(const QString &mime)
{
   if (mime == QLatin1String("text/html")) {
      return QLatin1String("public.html");
   }
   return QString();
}

QString QMacPasteboardMimeHTMLText::mimeFor(QString flav)
{
   if (flav == QLatin1String("public.html")) {
      return QLatin1String("text/html");
   }
   return QString();
}

bool QMacPasteboardMimeHTMLText::canConvert(const QString &mime, QString flav)
{
   return flavorFor(mime) == flav;
}

QVariant QMacPasteboardMimeHTMLText::convertToMime(const QString &mimeType, QList<QByteArray> data, QString flavor)
{
   if (!canConvert(mimeType, flavor)) {
      return QVariant();
   }
   if (data.count() > 1) {
      qWarning("QMacPasteboardMimeHTMLText: Cannot handle multiple member data");
   }
   return data.first();
}

QList<QByteArray> QMacPasteboardMimeHTMLText::convertFromMime(const QString &mime, QVariant data, QString flavor)
{
   QList<QByteArray> ret;
   if (!canConvert(mime, flavor)) {
      return ret;
   }
   ret.append(data.toByteArray());
   return ret;
}

class QMacPasteboardMimeTiff : public QMacPasteboardMime
{
 public:
   QMacPasteboardMimeTiff() : QMacPasteboardMime(MIME_ALL) { }
   QString convertorName();

   QString flavorFor(const QString &mime);
   QString mimeFor(QString flav);
   bool canConvert(const QString &mime, QString flav);
   QVariant convertToMime(const QString &mime, QList<QByteArray> data, QString flav);
   QList<QByteArray> convertFromMime(const QString &mime, QVariant data, QString flav);
};

QString QMacPasteboardMimeTiff::convertorName()
{
   return QLatin1String("Tiff");
}

QString QMacPasteboardMimeTiff::flavorFor(const QString &mime)
{
   if (mime.startsWith(QLatin1String("application/x-qt-image"))) {
      return QLatin1String("public.tiff");
   }
   return QString();
}

QString QMacPasteboardMimeTiff::mimeFor(QString flav)
{
   if (flav == QLatin1String("public.tiff")) {
      return QLatin1String("application/x-qt-image");
   }
   return QString();
}

bool QMacPasteboardMimeTiff::canConvert(const QString &mime, QString flav)
{
   return flav == QLatin1String("public.tiff") && mime == QLatin1String("application/x-qt-image");
}

QVariant QMacPasteboardMimeTiff::convertToMime(const QString &mime, QList<QByteArray> data, QString flav)
{
   if (data.count() > 1) {
      qWarning("QMacPasteboardMimeTiff: Cannot handle multiple member data");
   }
   QVariant ret;
   if (!canConvert(mime, flav)) {
      return ret;
   }
   const QByteArray &a = data.first();
   QCFType<CGImageRef> image;
   QCFType<CFDataRef> tiffData = CFDataCreateWithBytesNoCopy(0,
                                 reinterpret_cast<const UInt8 *>(a.constData()),
                                 a.size(), kCFAllocatorNull);
   QCFType<CGImageSourceRef> imageSource = CGImageSourceCreateWithData(tiffData, 0);
   image = CGImageSourceCreateImageAtIndex(imageSource, 0, 0);

   if (image != 0) {
      ret = QVariant(QPixmap::fromMacCGImageRef(image).toImage());
   }
   return ret;
}

QList<QByteArray> QMacPasteboardMimeTiff::convertFromMime(const QString &mime, QVariant variant, QString flav)
{
   QList<QByteArray> ret;
   if (!canConvert(mime, flav)) {
      return ret;
   }

   QImage img = qvariant_cast<QImage>(variant);
   QCFType<CGImageRef> cgimage = qt_mac_createCGImageFromQImage(img);

   if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_4) {
      QCFType<CFMutableDataRef> data = CFDataCreateMutable(0, 0);
      QCFType<CGImageDestinationRef> imageDestination = CGImageDestinationCreateWithData(data, kUTTypeTIFF, 1, 0);

      if (imageDestination != 0) {
         CFTypeRef keys[2];
         QCFType<CFTypeRef> values[2];
         QCFType<CFDictionaryRef> options;
         keys[0] = kCGImagePropertyPixelWidth;
         keys[1] = kCGImagePropertyPixelHeight;
         int width = img.width();
         int height = img.height();
         values[0] = CFNumberCreate(0, kCFNumberIntType, &width);
         values[1] = CFNumberCreate(0, kCFNumberIntType, &height);

         options = CFDictionaryCreate(0, reinterpret_cast<const void **>(keys),
                                      reinterpret_cast<const void **>(values), 2,
                                      &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

         CGImageDestinationAddImage(imageDestination, cgimage, options);
         CGImageDestinationFinalize(imageDestination);
      }

      QByteArray ar(CFDataGetLength(data), 0);
      CFDataGetBytes(data, CFRangeMake(0, ar.size()), reinterpret_cast<UInt8 *>(ar.data()));
      ret.append(ar);
   }

   return ret;
}


class QMacPasteboardMimeFileUri : public QMacPasteboardMime
{
 public:
   QMacPasteboardMimeFileUri() : QMacPasteboardMime(MIME_ALL) { }
   QString convertorName();

   QString flavorFor(const QString &mime);
   QString mimeFor(QString flav);
   bool canConvert(const QString &mime, QString flav);
   QVariant convertToMime(const QString &mime, QList<QByteArray> data, QString flav);
   QList<QByteArray> convertFromMime(const QString &mime, QVariant data, QString flav);
};

QString QMacPasteboardMimeFileUri::convertorName()
{
   return QLatin1String("FileURL");
}

QString QMacPasteboardMimeFileUri::flavorFor(const QString &mime)
{
   if (mime == QLatin1String("text/uri-list")) {
      return QCFString(UTTypeCreatePreferredIdentifierForTag(kUTTagClassOSType, CFSTR("furl"), 0));
   }
   return QString();
}

QString QMacPasteboardMimeFileUri::mimeFor(QString flav)
{
   if (flav == QCFString(UTTypeCreatePreferredIdentifierForTag(kUTTagClassOSType, CFSTR("furl"), 0))) {
      return QLatin1String("text/uri-list");
   }
   return QString();
}

bool QMacPasteboardMimeFileUri::canConvert(const QString &mime, QString flav)
{
   return mime == QLatin1String("text/uri-list")
          && flav == QCFString(UTTypeCreatePreferredIdentifierForTag(kUTTagClassOSType, CFSTR("furl"), 0));
}

QVariant QMacPasteboardMimeFileUri::convertToMime(const QString &mime, QList<QByteArray> data, QString flav)
{
   if (!canConvert(mime, flav)) {
      return QVariant();
   }
   QList<QVariant> ret;
   for (int i = 0; i < data.size(); ++i) {
      QUrl url = QUrl::fromEncoded(data.at(i));
      if (url.host().toLower() == QLatin1String("localhost")) {
         url.setHost(QString());
      }
      url.setPath(url.path().normalized(QString::NormalizationForm_C));
      ret.append(url);
   }
   return QVariant(ret);
}

QList<QByteArray> QMacPasteboardMimeFileUri::convertFromMime(const QString &mime, QVariant data, QString flav)
{
   QList<QByteArray> ret;
   if (!canConvert(mime, flav)) {
      return ret;
   }
   QList<QVariant> urls = data.toList();
   for (int i = 0; i < urls.size(); ++i) {
      QUrl url = urls.at(i).toUrl();
      if (url.scheme().isEmpty()) {
         url.setScheme(QLatin1String("file"));
      }
      if (url.scheme().toLower() == QLatin1String("file")) {
         if (url.host().isEmpty()) {
            url.setHost(QLatin1String("localhost"));
         }
         url.setPath(url.path().normalized(QString::NormalizationForm_D));
      }
      ret.append(url.toEncoded());
   }
   return ret;
}

class QMacPasteboardMimeUrl : public QMacPasteboardMime
{
 public:
   QMacPasteboardMimeUrl() : QMacPasteboardMime(MIME_ALL) { }
   QString convertorName();

   QString flavorFor(const QString &mime);
   QString mimeFor(QString flav);
   bool canConvert(const QString &mime, QString flav);
   QVariant convertToMime(const QString &mime, QList<QByteArray> data, QString flav);
   QList<QByteArray> convertFromMime(const QString &mime, QVariant data, QString flav);
};

QString QMacPasteboardMimeUrl::convertorName()
{
   return QLatin1String("URL");
}

QString QMacPasteboardMimeUrl::flavorFor(const QString &mime)
{
   if (mime.startsWith(QLatin1String("text/uri-list"))) {
      return QLatin1String("public.url");
   }
   return QString();
}

QString QMacPasteboardMimeUrl::mimeFor(QString flav)
{
   if (flav == QLatin1String("public.url")) {
      return QLatin1String("text/uri-list");
   }
   return QString();
}

bool QMacPasteboardMimeUrl::canConvert(const QString &mime, QString flav)
{
   return flav == QLatin1String("public.url")
          && mime == QLatin1String("text/uri-list");
}

QVariant QMacPasteboardMimeUrl::convertToMime(const QString &mime, QList<QByteArray> data, QString flav)
{
   if (!canConvert(mime, flav)) {
      return QVariant();
   }

   QList<QVariant> ret;
   for (int i = 0; i < data.size(); ++i) {
      QUrl url = QUrl::fromEncoded(data.at(i));
      if (url.host().toLower() == QLatin1String("localhost")) {
         url.setHost(QString());
      }
      url.setPath(url.path().normalized(QString::NormalizationForm_C));
      ret.append(url);
   }
   return QVariant(ret);
}

QList<QByteArray> QMacPasteboardMimeUrl::convertFromMime(const QString &mime, QVariant data, QString flav)
{
   QList<QByteArray> ret;
   if (!canConvert(mime, flav)) {
      return ret;
   }

   QList<QVariant> urls = data.toList();
   for (int i = 0; i < urls.size(); ++i) {
      QUrl url = urls.at(i).toUrl();
      if (url.scheme().isEmpty()) {
         url.setScheme(QLatin1String("file"));
      }
      if (url.scheme().toLower() == QLatin1String("file")) {
         if (url.host().isEmpty()) {
            url.setHost(QLatin1String("localhost"));
         }
         url.setPath(url.path().normalized(QString::NormalizationForm_D));
      }
      ret.append(url.toEncoded());
   }
   return ret;
}

class QMacPasteboardMimeVCard : public QMacPasteboardMime
{
 public:
   QMacPasteboardMimeVCard() : QMacPasteboardMime(MIME_ALL) { }
   QString convertorName();

   QString flavorFor(const QString &mime);
   QString mimeFor(QString flav);
   bool canConvert(const QString &mime, QString flav);
   QVariant convertToMime(const QString &mime, QList<QByteArray> data, QString flav);
   QList<QByteArray> convertFromMime(const QString &mime, QVariant data, QString flav);
};

QString QMacPasteboardMimeVCard::convertorName()
{
   return QString("VCard");
}

bool QMacPasteboardMimeVCard::canConvert(const QString &mime, QString flav)
{
   return mimeFor(flav) == mime;
}

QString QMacPasteboardMimeVCard::flavorFor(const QString &mime)
{
   if (mime.startsWith(QLatin1String("text/plain"))) {
      return QLatin1String("public.vcard");
   }
   return QString();
}

QString QMacPasteboardMimeVCard::mimeFor(QString flav)
{
   if (flav == QLatin1String("public.vcard")) {
      return QLatin1String("text/plain");
   }
   return QString();
}

QVariant QMacPasteboardMimeVCard::convertToMime(const QString &mime, QList<QByteArray> data, QString)
{
   QByteArray cards;
   if (mime == QLatin1String("text/plain")) {
      for (int i = 0; i < data.size(); ++i) {
         cards += data[i];
      }
   }
   return QVariant(cards);
}

QList<QByteArray> QMacPasteboardMimeVCard::convertFromMime(const QString &mime, QVariant data, QString)
{
   QList<QByteArray> ret;
   if (mime == QLatin1String("text/plain")) {
      ret.append(data.toString().toUtf8());
   }
   return ret;
}

/*!
  \internal
*/
void QMacPasteboardMime::initialize()
{
   if (globalMimeList()->isEmpty()) {
      qAddPostRoutine(cleanup_mimes);

      //standard types that we wrap
      new QMacPasteboardMimeTiff;
      new QMacPasteboardMimeUnicodeText;
      new QMacPasteboardMimePlainText;
      new QMacPasteboardMimeHTMLText;
      new QMacPasteboardMimeFileUri;
      new QMacPasteboardMimeUrl;
      new QMacPasteboardMimeTypeName;
      new QMacPasteboardMimeVCard;

      //make sure our "non-standard" types are always last! --Sam
      new QMacPasteboardMimeAny;
   }
}

/*!
  Returns the most-recently created QMacPasteboardMime of type \a t that can convert
  between the \a mime and \a flav formats.  Returns 0 if no such convertor
  exists.
*/
QMacPasteboardMime *
QMacPasteboardMime::convertor(uchar t, const QString &mime, QString flav)
{
   MimeList *mimes = globalMimeList();
   for (MimeList::const_iterator it = mimes->constBegin(); it != mimes->constEnd(); ++it) {
#ifdef DEBUG_MIME_MAPS
      qDebug("QMacPasteboardMime::convertor: seeing if %s (%d) can convert %s to %d[%c%c%c%c] [%d]",
             (*it)->convertorName().toLatin1().constData(),
             (*it)->type & t, mime.toLatin1().constData(),
             flav, (flav >> 24) & 0xFF, (flav >> 16) & 0xFF, (flav >> 8) & 0xFF, (flav) & 0xFF,
             (*it)->canConvert(mime, flav));
      for (int i = 0; i < (*it)->countFlavors(); ++i) {
         int f = (*it)->flavor(i);
         qDebug("  %d) %d[%c%c%c%c] [%s]", i, f,
                (f >> 24) & 0xFF, (f >> 16) & 0xFF, (f >> 8) & 0xFF, (f) & 0xFF,
                (*it)->convertorName().toLatin1().constData());
      }
#endif
      if (((*it)->type & t) && (*it)->canConvert(mime, flav)) {
         return (*it);
      }
   }
   return 0;
}
/*!
  Returns a MIME type of type \a t for \a flav, or 0 if none exists.
*/
QString QMacPasteboardMime::flavorToMime(uchar t, QString flav)
{
   MimeList *mimes = globalMimeList();
   for (MimeList::const_iterator it = mimes->constBegin(); it != mimes->constEnd(); ++it) {
#ifdef DEBUG_MIME_MAPS
      qDebug("QMacMIme::flavorToMime: attempting %s (%d) for flavor %d[%c%c%c%c] [%s]",
             (*it)->convertorName().toLatin1().constData(),
             (*it)->type & t, flav, (flav >> 24) & 0xFF, (flav >> 16) & 0xFF, (flav >> 8) & 0xFF, (flav) & 0xFF,
             (*it)->mimeFor(flav).toLatin1().constData());

#endif
      if ((*it)->type & t) {
         QString mimeType = (*it)->mimeFor(flav);
         if (!mimeType.isNull()) {
            return mimeType;
         }
      }
   }
   return QString();
}

/*!
  Returns a list of all currently defined QMacPasteboardMime objects of type \a t.
*/
QList<QMacPasteboardMime *> QMacPasteboardMime::all(uchar t)
{
   MimeList ret;
   MimeList *mimes = globalMimeList();
   for (MimeList::const_iterator it = mimes->constBegin(); it != mimes->constEnd(); ++it) {
      if ((*it)->type & t) {
         ret.append((*it));
      }
   }
   return ret;
}


/*!
  \fn QString QMacPasteboardMime::convertorName()

  Returns a name for the convertor.

  All subclasses must reimplement this pure virtual function.
*/

/*!
  \fn bool QMacPasteboardMime::canConvert(const QString &mime, QString flav)

  Returns true if the convertor can convert (both ways) between
  \a mime and \a flav; otherwise returns false.

  All subclasses must reimplement this pure virtual function.
*/

/*!
  \fn QString QMacPasteboardMime::mimeFor(QString flav)

  Returns the MIME UTI used for Mac flavor \a flav, or 0 if this
  convertor does not support \a flav.

  All subclasses must reimplement this pure virtual function.
*/

/*!
  \fn QString QMacPasteboardMime::flavorFor(const QString &mime)

  Returns the Mac UTI used for MIME type \a mime, or 0 if this
  convertor does not support \a mime.

  All subclasses must reimplement this pure virtual function.
*/

/*!
    \fn QVariant QMacPasteboardMime::convertToMime(const QString &mime, QList<QByteArray> data, QString flav)

    Returns \a data converted from Mac UTI \a flav to MIME type \a
    mime.

    Note that Mac flavors must all be self-terminating. The input \a
    data may contain trailing data.

    All subclasses must reimplement this pure virtual function.
*/

/*!
  \fn QList<QByteArray> QMacPasteboardMime::convertFromMime(const QString &mime, QVariant data, QString flav)

  Returns \a data converted from MIME type \a mime
    to Mac UTI \a flav.

  Note that Mac flavors must all be self-terminating.  The return
  value may contain trailing data.

  All subclasses must reimplement this pure virtual function.
*/


QT_END_NAMESPACE
