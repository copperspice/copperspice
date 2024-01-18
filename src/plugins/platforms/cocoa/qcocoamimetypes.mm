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

#include <qcocoamimetypes.h>
#include <qmacmime_p.h>
#include <qcocoahelpers.h>

class QMacPasteboardMimeTraditionalMacPlainText : public QMacInternalPasteboardMime
{
 public:
   QMacPasteboardMimeTraditionalMacPlainText() : QMacInternalPasteboardMime(MIME_ALL) { }
   QString convertorName();

   QString flavorFor(const QString &mime);
   QString mimeFor(QString flav);
   bool canConvert(const QString &mime, QString flav);
   QVariant convertToMime(const QString &mime, QList<QByteArray> data, QString flav);
   QList<QByteArray> convertFromMime(const QString &mime, QVariant data, QString flav);
};

QString QMacPasteboardMimeTraditionalMacPlainText::convertorName()
{
   return QLatin1String("PlainText (traditional-mac-plain-text)");
}

QString QMacPasteboardMimeTraditionalMacPlainText::flavorFor(const QString &mime)
{
   if (mime == QLatin1String("text/plain")) {
      return QLatin1String("com.apple.traditional-mac-plain-text");
   }
   return QString();
}

QString QMacPasteboardMimeTraditionalMacPlainText::mimeFor(QString flav)
{
   if (flav == QLatin1String("com.apple.traditional-mac-plain-text")) {
      return QLatin1String("text/plain");
   }
   return QString();
}

bool QMacPasteboardMimeTraditionalMacPlainText::canConvert(const QString &mime, QString flav)
{
   return flavorFor(mime) == flav;
}

QVariant QMacPasteboardMimeTraditionalMacPlainText::convertToMime(const QString &mimetype, QList<QByteArray> data, QString flavor)
{
   if (data.count() > 1) {
      qWarning("QMacPasteboardMimeTraditionalMacPlainText: Cannot handle multiple member data");
   }

   const QByteArray &firstData = data.first();
   QVariant ret;

   if (flavor == "com.apple.traditional-mac-plain-text") {

      QCFString tmp = CFStringCreateWithBytes(kCFAllocatorDefault,
                        reinterpret_cast<const UInt8 *>(firstData.constData()),
                        firstData.size(), CFStringGetSystemEncoding(), false);

      return tmp.toQString();

   } else {
      qWarning("QMime::convertToMime: unhandled mimetype: %s", csPrintable(mimetype));
   }

   return ret;
}

QList<QByteArray> QMacPasteboardMimeTraditionalMacPlainText::convertFromMime(const QString &, QVariant data, QString flavor)
{
   QList<QByteArray> ret;
   QString string = data.toString();

   if (flavor == "com.apple.traditional-mac-plain-text") {
      ret.append(string.toUtf8());
   }

   return ret;
}

class QMacPasteboardMimeTiff : public QMacInternalPasteboardMime
{
 public:
   QMacPasteboardMimeTiff() : QMacInternalPasteboardMime(MIME_ALL) { }
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
      qWarning("QMacPasteboardMimeTiff: Unable to handle multiple member data");
   }

   QVariant ret;
   if (! canConvert(mime, flav)) {
      return ret;
   }

   const QByteArray &a = data.first();
   QCFType<CGImageRef> image;
   QCFType<CFDataRef> tiffData = CFDataCreateWithBytesNoCopy(nullptr,
         reinterpret_cast<const UInt8 *>(a.constData()), a.size(), kCFAllocatorNull);

   QCFType<CGImageSourceRef> imageSource = CGImageSourceCreateWithData(tiffData, nullptr);
   image = CGImageSourceCreateImageAtIndex(imageSource, 0, nullptr);

   if (image != nullptr) {
      ret = QVariant(qt_mac_toQImage(image));
   }

   return ret;
}

QList<QByteArray> QMacPasteboardMimeTiff::convertFromMime(const QString &mime, QVariant variant, QString flav)
{
   QList<QByteArray> ret;

   if (! canConvert(mime, flav)) {
      return ret;
   }

   QImage img = variant.value<QImage>();
   QCFType<CGImageRef> cgimage = qt_mac_toCGImage(img);

   QCFType<CFMutableDataRef> data = CFDataCreateMutable(nullptr, 0);
   QCFType<CGImageDestinationRef> imageDestination = CGImageDestinationCreateWithData(data, kUTTypeTIFF, 1, nullptr);

   if (imageDestination != nullptr) {
      CFTypeRef keys[2];
      QCFType<CFTypeRef> values[2];
      QCFType<CFDictionaryRef> options;

      keys[0] = kCGImagePropertyPixelWidth;
      keys[1] = kCGImagePropertyPixelHeight;
      int width  = img.width();
      int height = img.height();

      values[0]  = CFNumberCreate(nullptr, kCFNumberIntType, &width);
      values[1]  = CFNumberCreate(nullptr, kCFNumberIntType, &height);

      options = CFDictionaryCreate(nullptr, reinterpret_cast<const void **>(keys),
            reinterpret_cast<const void **>(values), 2, &kCFTypeDictionaryKeyCallBacks,
            &kCFTypeDictionaryValueCallBacks);

      CGImageDestinationAddImage(imageDestination, cgimage, options);
      CGImageDestinationFinalize(imageDestination);
   }

   QByteArray ar(CFDataGetLength(data), 0);

   CFDataGetBytes(data,
      CFRangeMake(0, ar.size()),
      reinterpret_cast<UInt8 *>(ar.data()));
   ret.append(ar);
   return ret;
}

void QCocoaMimeTypes::initializeMimeTypes()
{
   new QMacPasteboardMimeTraditionalMacPlainText;
   new QMacPasteboardMimeTiff;
}

