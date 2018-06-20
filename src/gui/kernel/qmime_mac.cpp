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
#include <qregularexpression.h>
#include <qurl.h>
#include <qmap.h>
#include <qt_mac_p.h>


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
   return QString("Any-Mime");
}

QString QMacPasteboardMimeAny::flavorFor(const QString &mime)
{
   // do not handle the mime type name in the drag pasteboard
   if (mime == "application/x-qt-mime-type-name") {
      return QString();
   }

   QString ret = "com.copperspice.anymime." + mime;

   return ret.replace(QChar('/'), QString("--"));
}

QString QMacPasteboardMimeAny::mimeFor(QString flav)
{
   const QString any_prefix = "com.copperspice.anymime.";

   if (flav.size() > any_prefix.length() && flav.startsWith(any_prefix)) {
      return flav.mid(any_prefix.length()).replace(QString("--"), QString("/"));
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

   if (mime == "text/plain") {
      ret = QString::fromUtf8(data.first());
   } else {
      ret = data.first();
   }

   return ret;
}

QList<QByteArray> QMacPasteboardMimeAny::convertFromMime(const QString &mime, QVariant data, QString)
{
   QList<QByteArray> ret;
   if (mime == "text/plain") {
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
   return QString("Qt-Mime-Type");
}

QString QMacPasteboardMimeTypeName::flavorFor(const QString &mime)
{
   if (mime == "application/x-qt-mime-type-name") {
      return QString("com.copperspice.MimeTypeName");
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
   return QString("PlainText");
}

QString QMacPasteboardMimePlainText::flavorFor(const QString &mime)
{
   if (mime == "text/plain") {
      return QString("com.apple.traditional-mac-plain-text");
   }

   return QString();
}

QString QMacPasteboardMimePlainText::mimeFor(QString flav)
{
   if (flav == "com.apple.traditional-mac-plain-text") {
      return QString("text/plain");
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
   if (flavor == QCFString(QString("com.apple.traditional-mac-plain-text"))) {

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

   if (flavor == QCFString(QString("com.apple.traditional-mac-plain-text"))) {
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
   return QString("UnicodeText");
}

QString QMacPasteboardMimeUnicodeText::flavorFor(const QString &mime)
{
   if (mime == "text/plain") {
      return QString("public.utf16-plain-text");
   }

   int i = mime.indexOf("charset=");

   if (i >= 0) {

      QString cs(mime.mid(i + 8).toLower());
      i = cs.indexOf(';');

      if (i >= 0) {
         cs = cs.left(i);
      }

      if (cs == "system") {
         return QLatin1String("public.utf8-plain-text");

      } else if (cs == "iso-10646-ucs-2" || cs == "utf16") {
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

   } else if (flavor == "public.utf16-plain-text") {
      ret = QString(reinterpret_cast<const QChar *>(firstData.constData()), firstData.size() / sizeof(QChar));
   } else {
      qWarning("QMime::convertToMime: unhandled mimetype: %s", qPrintable(mimetype));
   }
   return ret;
}

QList<QByteArray> QMacPasteboardMimeUnicodeText::convertFromMime(const QString &, QVariant data, QString flavor)
{
   QList<QByteArray> ret;
   QString string = data.toString();

   if (flavor == "public.utf8-plain-text") {
      ret.append(string.toUtf8());

   } else if (flavor == "public.utf16-plain-text") {
      QString16 tmp = string.toUtf16();

      ret.append( QByteArray((char *)tmp.constData(), tmp.size_storage() * 2));
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
   if (mime == "text/html") {
      return QString("public.html");
   }

   return QString();
}

QString QMacPasteboardMimeHTMLText::mimeFor(QString flav)
{
   if (flav == "public.html") {
      return QString("text/html");
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
   if (mime.startsWith("text/plain")) {
      return QString("public.vcard");
   }

   return QString();
}

QString QMacPasteboardMimeVCard::mimeFor(QString flav)
{
   if (flav == "public.vcard") {
      return QLatin1String("text/plain");
   }

   return QString();
}

QVariant QMacPasteboardMimeVCard::convertToMime(const QString &mime, QList<QByteArray> data, QString)
{
   QByteArray cards;

   if (mime == "text/plain") {
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

QMacPasteboardMime * QMacPasteboardMime::convertor(uchar t, const QString &mime, QString flav)
{
   MimeList *mimes = globalMimeList();

   for (MimeList::const_iterator it = mimes->constBegin(); it != mimes->constEnd(); ++it) {
      if (((*it)->type & t) && (*it)->canConvert(mime, flav)) {
         return (*it);
      }
   }

   return 0;
}

QString QMacPasteboardMime::flavorToMime(uchar t, QString flav)
{
   MimeList *mimes = globalMimeList();

   for (MimeList::const_iterator it = mimes->constBegin(); it != mimes->constEnd(); ++it) {

      if ((*it)->type & t) {
         QString mimeType = (*it)->mimeFor(flav);

         if (! mimeType.isEmpty()) {
            return mimeType;
         }
      }
   }

   return QString();
}

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

