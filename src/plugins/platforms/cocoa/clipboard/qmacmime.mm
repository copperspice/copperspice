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

#include <qglobal.h>

#if defined(Q_OS_IOS)
#import <UIKit/UIKit.h>

#elif defined(Q_OS_DARWIN)
#import <Cocoa/Cocoa.h>

#endif

#include <qapplication.h>
#include <qlist.h>
#include <qmacmime_p.h>
#include <qstringlist.h>
#include <qtextcodec.h>

#include <qcore_mac_p.h>

using MimeList = QList<QMacInternalPasteboardMime *>;

MimeList &globalMimeList()
{
   static MimeList retval;
   return retval;
}

QStringList &globalDraggedTypesList()
{
   static QStringList retval;
   return retval;
}

void qt_mac_addToGlobalMimeList(QMacInternalPasteboardMime *macMime)
{
   // globalMimeList is in decreasing priority order. Recently added converters
   // take prioity over previously added converters: prepend to the list

   globalMimeList().prepend(macMime);
}

void qt_mac_removeFromGlobalMimeList(QMacInternalPasteboardMime *macMime)
{
   if (! QApplication::closingDown()) {
      globalMimeList().removeAll(macMime);
   }
}

void qt_mac_registerDraggedTypes(const QStringList &types)
{
   globalDraggedTypesList().append(types);
}

const QStringList &qt_mac_enabledDraggedTypes()
{
   return globalDraggedTypesList();
}

QMacInternalPasteboardMime::QMacInternalPasteboardMime(char t) : type(t)
{
   qt_mac_addToGlobalMimeList(this);
}

QMacInternalPasteboardMime::~QMacInternalPasteboardMime()
{
   qt_mac_removeFromGlobalMimeList(this);
}

int QMacInternalPasteboardMime::count(QMimeData *mimeData)
{
   return 1;
}

class QMacPasteboardMimeAny : public QMacInternalPasteboardMime
{

 public:
   QMacPasteboardMimeAny() : QMacInternalPasteboardMime(MIME_QT_CONVERTOR | MIME_ALL)
   { }

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
   if (mime == "application/x-qt-mime-type-name") {
      return QString();
   }

   QString retval = "com.copperspice.cs.anymime." + mime;

   return retval.replace('/', "--");
}

QString QMacPasteboardMimeAny::mimeFor(QString flav)
{
   const QString any_prefix = "com.copperspice.cs.anymime.";

   if (flav.size() > any_prefix.length() && flav.startsWith(any_prefix)) {
      return flav.mid(any_prefix.length()).replace("--", "/");
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
      qWarning("QMacPasteboardMimeAny: Cannot handle multiple member data");
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

class QMacPasteboardMimeTypeName : public QMacInternalPasteboardMime
{

 public:
   QMacPasteboardMimeTypeName() : QMacInternalPasteboardMime(MIME_QT_CONVERTOR | MIME_ALL) {
   }
   ~QMacPasteboardMimeTypeName() {
   }
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
   if (mime == "application/x-qt-mime-type-name") {
      return "com.copperspice.cs.MimeTypeName";
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
   ret.append(QString(QLatin1String("x-qt-mime-type-name")).toUtf8());
   return ret;
}

class QMacPasteboardMimePlainTextFallback : public QMacInternalPasteboardMime
{
 public:
   QMacPasteboardMimePlainTextFallback() : QMacInternalPasteboardMime(MIME_ALL) { }
   QString convertorName();

   QString flavorFor(const QString &mime);
   QString mimeFor(QString flav);
   bool canConvert(const QString &mime, QString flav);
   QVariant convertToMime(const QString &mime, QList<QByteArray> data, QString flav);
   QList<QByteArray> convertFromMime(const QString &mime, QVariant data, QString flav);
};

QString QMacPasteboardMimePlainTextFallback::convertorName()
{
   return QLatin1String("PlainText (public.text)");
}

QString QMacPasteboardMimePlainTextFallback::flavorFor(const QString &mime)
{
   if (mime == "text/plain") {
      return QString("public.text");
   }

   return QString();
}

QString QMacPasteboardMimePlainTextFallback::mimeFor(QString flav)
{
   if (flav == "public.text") {
      return QString("text/plain");
   }

   return QString();
}

bool QMacPasteboardMimePlainTextFallback::canConvert(const QString &mime, QString flav)
{
   return mime == mimeFor(flav);
}

QVariant QMacPasteboardMimePlainTextFallback::convertToMime(const QString &mimetype, QList<QByteArray> data, QString flavor)
{
   if (data.count() > 1) {
      qWarning("QMacPasteboardMimePlainTextFallback: Unable to handle multiple member data elements");
   }

   if (flavor == "public.text") {
      // Note that public.text is documented by Apple to have an undefined encoding. From
      // testing it seems that utf8 is normally used, at least by Safari on iOS.
      const QByteArray &firstData = data.first();

      QCFString tmp = CFStringCreateWithBytes(kCFAllocatorDefault,
               reinterpret_cast<const UInt8 *>(firstData.constData()), firstData.size(), kCFStringEncodingUTF8, false);

      return tmp.toQString();

   } else {
      qWarning("QMime::convertToMime: unhandled mimetype: %s", csPrintable(mimetype));
   }

   return QVariant();
}

QList<QByteArray> QMacPasteboardMimePlainTextFallback::convertFromMime(const QString &, QVariant data, QString flavor)
{
   QList<QByteArray> ret;
   QString string = data.toString();

   if (flavor == "public.text") {
      ret.append(string.toUtf8());
   }

   return ret;
}

class QMacPasteboardMimeUnicodeText : public QMacInternalPasteboardMime
{
 public:
   QMacPasteboardMimeUnicodeText() : QMacInternalPasteboardMime(MIME_ALL) { }
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
   if (mime == "text/plain") {
      return QString("public.utf16-plain-text");
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
   return (mime == QLatin1String("text/plain")
         && (flav == QLatin1String("public.utf8-plain-text") || (flav == QLatin1String("public.utf16-plain-text"))));
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
      ret = QString::fromUtf8(firstData);
   } else if (flavor == QLatin1String("public.utf16-plain-text")) {
      ret = QTextCodec::codecForName("UTF-16")->toUnicode(firstData);
   } else {
      qWarning("QMime::convertToMime: unhandled mimetype: %s", csPrintable(mimetype));
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
      ret.append(QTextCodec::codecForName("UTF-16")->fromUnicode(string));
   }
   return ret;
}

class QMacPasteboardMimeHTMLText : public QMacInternalPasteboardMime
{
 public:
   QMacPasteboardMimeHTMLText() : QMacInternalPasteboardMime(MIME_ALL) { }
   QString convertorName();

   QString flavorFor(const QString &mime);
   QString mimeFor(QString flav);
   bool canConvert(const QString &mime, QString flav);
   QVariant convertToMime(const QString &mime, QList<QByteArray> data, QString flav);
   QList<QByteArray> convertFromMime(const QString &mime, QVariant data, QString flav);
};

QString QMacPasteboardMimeHTMLText::convertorName()
{
   return QString("HTML");
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

class QMacPasteboardMimeRtfText : public QMacInternalPasteboardMime
{
 public:
   QMacPasteboardMimeRtfText() : QMacInternalPasteboardMime(MIME_ALL) { }
   QString convertorName();

   QString flavorFor(const QString &mime);
   QString mimeFor(QString flav);
   bool canConvert(const QString &mime, QString flav);
   QVariant convertToMime(const QString &mime, QList<QByteArray> data, QString flav);
   QList<QByteArray> convertFromMime(const QString &mime, QVariant data, QString flav);
};

QString QMacPasteboardMimeRtfText::convertorName()
{
   return QString("Rtf");
}

QString QMacPasteboardMimeRtfText::flavorFor(const QString &mime)
{
   if (mime == "text/html") {
      return QString("public.rtf");
   }

   return QString();
}

QString QMacPasteboardMimeRtfText::mimeFor(QString flav)
{
   if (flav == "public.rtf") {
      return QString("text/html");
   }

   return QString();
}

bool QMacPasteboardMimeRtfText::canConvert(const QString &mime, QString flav)
{
#if defined(Q_OS_IOS)
   if (QSysInfo::MacintoshVersion < QSysInfo::MV_IOS_7_0) {
      return false;
   }
#endif

   return mime == mimeFor(flav);
}

QVariant QMacPasteboardMimeRtfText::convertToMime(const QString &mimeType, QList<QByteArray> data, QString flavor)
{
   if (!canConvert(mimeType, flavor)) {
      return QVariant();
   }

   if (data.count() > 1) {
      qWarning("QMacPasteboardMimeHTMLText: Can not handle multiple member data");
   }

   // Read RTF into to NSAttributedString, then convert the string to HTML
   NSAttributedString *string = [[NSAttributedString alloc] initWithData: data.at(0).toNSData()
                                                                 options: [NSDictionary dictionaryWithObject: NSRTFTextDocumentType forKey: NSDocumentTypeDocumentAttribute]
                                                      documentAttributes: nil
                                                                   error: nil];

   NSError *error;
   NSRange range = NSMakeRange(0, [string length]);
   NSDictionary *dict = [NSDictionary dictionaryWithObject: NSHTMLTextDocumentType forKey: NSDocumentTypeDocumentAttribute];
   NSData *htmlData = [string dataFromRange: range documentAttributes: dict error: &error];
   return QByteArray::fromNSData(htmlData);
}

QList<QByteArray> QMacPasteboardMimeRtfText::convertFromMime(const QString &mime, QVariant data, QString flavor)
{
   QList<QByteArray> ret;
   if (!canConvert(mime, flavor)) {
      return ret;
   }

   NSAttributedString *string = [[NSAttributedString alloc] initWithData: data.toByteArray().toNSData()
                                                                 options: [NSDictionary dictionaryWithObject: NSHTMLTextDocumentType forKey: NSDocumentTypeDocumentAttribute]
                                                      documentAttributes: nil
                                                                   error: nil];

   NSError *error;
   NSRange range = NSMakeRange(0, [string length]);
   NSDictionary *dict = [NSDictionary dictionaryWithObject: NSRTFTextDocumentType forKey: NSDocumentTypeDocumentAttribute];
   NSData *rtfData = [string dataFromRange: range documentAttributes: dict error: &error];
   ret << QByteArray::fromNSData(rtfData);

   return ret;
}

class QMacPasteboardMimeFileUri : public QMacInternalPasteboardMime
{
 public:
   QMacPasteboardMimeFileUri() : QMacInternalPasteboardMime(MIME_ALL) { }
   QString convertorName();

   QString flavorFor(const QString &mime);
   QString mimeFor(QString flav);
   bool canConvert(const QString &mime, QString flav);
   QVariant convertToMime(const QString &mime, QList<QByteArray> data, QString flav);
   QList<QByteArray> convertFromMime(const QString &mime, QVariant data, QString flav);
   int count(QMimeData *mimeData);
};

QString QMacPasteboardMimeFileUri::convertorName()
{
   return QLatin1String("FileURL");
}

QString QMacPasteboardMimeFileUri::flavorFor(const QString &mime)
{
   if (mime == "text/uri-list") {
      return QString("public.file-url");
   }

   return QString();
}

QString QMacPasteboardMimeFileUri::mimeFor(QString flav)
{
   if (flav == "public.file-url") {
      return QString("text/uri-list");
   }

   return QString();
}

bool QMacPasteboardMimeFileUri::canConvert(const QString &mime, QString flav)
{
   return mime == QLatin1String("text/uri-list") && flav == QLatin1String("public.file-url");
}

QVariant QMacPasteboardMimeFileUri::convertToMime(const QString &mime, QList<QByteArray> data, QString flav)
{
   if (!canConvert(mime, flav)) {
      return QVariant();
   }

   QList<QVariant> ret;

   for (int i = 0; i < data.size(); ++i) {
      const QByteArray &a = data.at(i);

      NSString *urlString = [[[NSString alloc] initWithBytesNoCopy: (void *)a.data() length: a.size()
                                 encoding: NSUTF8StringEncoding freeWhenDone: NO] autorelease];

      NSURL *nsurl = [NSURL URLWithString: urlString];
      QUrl url;

      // OS X 10.10 sends file references instead of file paths
      if ([nsurl isFileReferenceURL]) {
         url = QUrl::fromNSURL([nsurl filePathURL]);
      } else {
         url = QUrl::fromNSURL(nsurl);
      }

      if (url.host().toLower() == "localhost") {
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

   if (! canConvert(mime, flav)) {
      return ret;
   }

   QList<QVariant> urls = data.toList();

   for (int i = 0; i < urls.size(); ++i) {
      QUrl url = urls.at(i).toUrl();

      if (url.scheme().isEmpty()) {
         url.setScheme(QLatin1String("file"));
      }

      if (url.scheme() == "file") {

         if (url.host().isEmpty()) {
            url.setHost("localhost");
         }

         url.setPath(url.path().normalized(QString::NormalizationForm_D));
      }

      ret.append(url.toEncoded());
   }

   return ret;
}

int QMacPasteboardMimeFileUri::count(QMimeData *mimeData)
{
   return mimeData->urls().count();
}

class QMacPasteboardMimeUrl : public QMacInternalPasteboardMime
{
 public:
   QMacPasteboardMimeUrl() : QMacInternalPasteboardMime(MIME_ALL) { }
   QString convertorName();

   QString flavorFor(const QString &mime);
   QString mimeFor(QString flav);
   bool canConvert(const QString &mime, QString flav);
   QVariant convertToMime(const QString &mime, QList<QByteArray> data, QString flav);
   QList<QByteArray> convertFromMime(const QString &mime, QVariant data, QString flav);
};

QString QMacPasteboardMimeUrl::convertorName()
{
   return QString("URL");
}

QString QMacPasteboardMimeUrl::flavorFor(const QString &mime)
{
   if (mime.startsWith("text/uri-list")) {
      return QString("public.url");
   }

   return QString();
}

QString QMacPasteboardMimeUrl::mimeFor(QString flav)
{
   if (flav == "public.url") {
      return QString("text/uri-list");
   }

   return QString();
}

bool QMacPasteboardMimeUrl::canConvert(const QString &mime, QString flav)
{
   return (flav == "public.url") && (mime == "text/uri-list");
}

QVariant QMacPasteboardMimeUrl::convertToMime(const QString &mime, QList<QByteArray> data, QString flav)
{
   if (! canConvert(mime, flav)) {
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
      if (url.scheme() == QLatin1String("file")) {
         if (url.host().isEmpty()) {
            url.setHost(QLatin1String("localhost"));
         }
         url.setPath(url.path().normalized(QString::NormalizationForm_D));
      }
      ret.append(url.toEncoded());
   }
   return ret;
}

class QMacPasteboardMimeVCard : public QMacInternalPasteboardMime
{
 public:
   QMacPasteboardMimeVCard() : QMacInternalPasteboardMime(MIME_ALL) { }
   QString convertorName();

   QString flavorFor(const QString &mime);
   QString mimeFor(QString flav);
   bool canConvert(const QString &mime, QString flav);
   QVariant convertToMime(const QString &mime, QList<QByteArray> data, QString flav);
   QList<QByteArray> convertFromMime(const QString &mime, QVariant data, QString flav);
};

QString QMacPasteboardMimeVCard::convertorName()
{
   return QLatin1String("VCard");
}

bool QMacPasteboardMimeVCard::canConvert(const QString &mime, QString flav)
{
   return mimeFor(flav) == mime;
}

QString QMacPasteboardMimeVCard::flavorFor(const QString &mime)
{
   if (mime.startsWith(QLatin1String("text/vcard"))) {
      return QLatin1String("public.vcard");
   }
   return QString();
}

QString QMacPasteboardMimeVCard::mimeFor(QString flav)
{
   if (flav == "public.vcard") {
      return QString("text/vcard");
   }

   return QString();
}

QVariant QMacPasteboardMimeVCard::convertToMime(const QString &mime, QList<QByteArray> data, QString)
{
   QByteArray cards;

   if (mime == "text/vcard") {
      for (int i = 0; i < data.size(); ++i) {
         cards += data[i];
      }
   }

   return QVariant(cards);
}

QList<QByteArray> QMacPasteboardMimeVCard::convertFromMime(const QString &mime, QVariant data, QString)
{
   QList<QByteArray> retval;

   if (mime == "text/vcard") {
      retval.append(data.toString().toUtf8());
   }

   return retval;
}

/*!
  \internal

  This is an internal function.
*/
void QMacInternalPasteboardMime::initializeMimeTypes()
{
   if (globalMimeList().isEmpty()) {
      // Create QMacPasteboardMimeAny first to put it at the end of globalMimeList
      // with lowest priority. (the constructor prepends to the list)
      new QMacPasteboardMimeAny;

      // standard types that we wrap
      new QMacPasteboardMimePlainTextFallback;
      new QMacPasteboardMimeUnicodeText;
      new QMacPasteboardMimeRtfText;
      new QMacPasteboardMimeHTMLText;
      new QMacPasteboardMimeFileUri;
      new QMacPasteboardMimeUrl;
      new QMacPasteboardMimeTypeName;
      new QMacPasteboardMimeVCard;
   }
}

void QMacInternalPasteboardMime::destroyMimeTypes()
{
   for (auto item : globalMimeList() ) {
      delete item;
   }

   globalMimeList().clear();
}

QMacInternalPasteboardMime *QMacInternalPasteboardMime::convertor(uchar t, const QString &mime, QString flav)
{
   for (auto item : globalMimeList() ) {
      if ((item->type & t) && item->canConvert(mime, flav)) {
         return item;
      }
   }

   return nullptr;
}

QString QMacInternalPasteboardMime::flavorToMime(uchar t, QString flav)
{
   for (auto item : globalMimeList() ) {

      if (item->type & t) {
         QString mimeType = item->mimeFor(flav);

         if (! mimeType.isEmpty()) {
            return mimeType;
         }
      }
   }

   return QString();
}

QList<QMacInternalPasteboardMime *> QMacInternalPasteboardMime::all(uchar t)
{
   MimeList retval;

   for (auto item : globalMimeList() ) {
      if (item->type & t) {
         retval.append(item);
      }
   }

   return retval;
}


