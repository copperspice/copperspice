/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#include "qmacclipboard.h"
#include "qclipboard.h"
#include "qapplication.h"
#include "qbitmap.h"
#include "qdatetime.h"
#include "qdebug.h"
#include "qapplication.h"
#include "qevent.h"
#include "qurl.h"
#include "qcocoahelpers.h"

#include <stdlib.h>
#include <string.h>

//#define DEBUG_PASTEBOARD

class QMacMimeData : public QMimeData
{
 public:
   QVariant variantData(const QString &mime) {
      return retrieveData(mime, QVariant::Invalid);
   }

 private:
   QMacMimeData();
};

QMacPasteboard::Promise::Promise(int itemId, QMacInternalPasteboardMime *c, QString m, QMacMimeData *md, int o, DataRequestType drt)
   : itemId(itemId), offset(o), convertor(c), mime(m), dataRequestType(drt)
{
   // Request the data from the application immediately for eager requests.
   if (dataRequestType == QMacPasteboard::EagerRequest) {
      variantData = md->variantData(m);
      mimeData = 0;
   } else {
      mimeData = md;
   }
}

QMacPasteboard::QMacPasteboard(PasteboardRef p, uchar mt)
{
   mac_mime_source = false;
   mime_type = mt ? mt : uchar(QMacInternalPasteboardMime::MIME_ALL);
   paste = p;
   CFRetain(paste);
   resolvingBeforeDestruction = false;
}

QMacPasteboard::QMacPasteboard(uchar mt)
{
   mac_mime_source = false;
   mime_type = mt ? mt : uchar(QMacInternalPasteboardMime::MIME_ALL);
   paste = 0;
   OSStatus err = PasteboardCreate(0, &paste);
   if (err == noErr) {
      PasteboardSetPromiseKeeper(paste, promiseKeeper, this);
   } else {
      qDebug("PasteBoard: Error creating pasteboard: [%d]", (int)err);
   }
   resolvingBeforeDestruction = false;
}

QMacPasteboard::QMacPasteboard(CFStringRef name, uchar mt)
{
   mac_mime_source = false;
   mime_type = mt ? mt : uchar(QMacInternalPasteboardMime::MIME_ALL);
   paste = 0;
   OSStatus err = PasteboardCreate(name, &paste);
   if (err == noErr) {
      PasteboardSetPromiseKeeper(paste, promiseKeeper, this);
   } else {
      qDebug("PasteBoard: Error creating pasteboard: %s [%d]", QCFString::toQString(name).toLatin1().constData(), (int)err);
   }
   resolvingBeforeDestruction = false;
}

QMacPasteboard::~QMacPasteboard()
{
   // commit all promises for paste after exit close
   resolvingBeforeDestruction = true;
   PasteboardResolvePromises(paste);
   if (paste) {
      CFRelease(paste);
   }
}

PasteboardRef QMacPasteboard::pasteBoard() const
{
   return paste;
}

OSStatus QMacPasteboard::promiseKeeper(PasteboardRef paste, PasteboardItemID id, CFStringRef flavor, void *_qpaste)
{
   QMacPasteboard *qpaste = (QMacPasteboard *)_qpaste;
   const long promise_id = (long)id;

   // Find the kept promise
   const QString flavorAsQString = QCFString::toQString(flavor);
   QMacPasteboard::Promise promise;
   for (int i = 0; i < qpaste->promises.size(); i++) {
      QMacPasteboard::Promise tmp = qpaste->promises[i];
      if (tmp.itemId == promise_id && tmp.convertor->canConvert(tmp.mime, flavorAsQString)) {
         promise = tmp;
         break;
      }
   }

   if (!promise.itemId && flavorAsQString == QLatin1String("com.trolltech.qt.MimeTypeName")) {
      // we have promised this data, but won't be able to convert, so return null data.
      // This helps in making the application/x-qt-mime-type-name hidden from normal use.
      QByteArray ba;
      QCFType<CFDataRef> data = CFDataCreate(0, (UInt8 *)ba.constData(), ba.size());
      PasteboardPutItemFlavor(paste, id, flavor, data, kPasteboardFlavorNoFlags);
      return noErr;
   }

   if (!promise.itemId) {
      // There was no promise that could deliver data for the
      // given id and flavor. This should not happend.
      qDebug("Pasteboard: %d: Request for %ld, %s, but no promise found!", __LINE__, promise_id, qPrintable(flavorAsQString));
      return cantGetFlavorErr;
   }

#ifdef DEBUG_PASTEBOARD
   qDebug("PasteBoard: Calling in promise for %s[%ld] [%s] (%s) [%d]", qPrintable(promise.mime), promise_id,
      qPrintable(flavorAsQString), qPrintable(promise.convertor->convertorName()), promise.offset);
#endif

   // Get the promise data. If this is a "lazy" promise call variantData()
   // to request the data from the application.
   QVariant promiseData;
   if (promise.dataRequestType == LazyRequest) {
      if (!qpaste->resolvingBeforeDestruction && !promise.mimeData.isNull()) {
         promiseData = promise.mimeData->variantData(promise.mime);
      }
   } else {
      promiseData = promise.variantData;
   }

   QList<QByteArray> md = promise.convertor->convertFromMime(promise.mime, promiseData, flavorAsQString);
   if (md.size() <= promise.offset) {
      return cantGetFlavorErr;
   }
   const QByteArray &ba = md[promise.offset];
   QCFType<CFDataRef> data = CFDataCreate(0, (UInt8 *)ba.constData(), ba.size());
   PasteboardPutItemFlavor(paste, id, flavor, data, kPasteboardFlavorNoFlags);
   return noErr;
}

bool QMacPasteboard::hasOSType(int c_flavor) const
{
   if (!paste) {
      return false;
   }

   sync();

   ItemCount cnt = 0;
   if (PasteboardGetItemCount(paste, &cnt) || !cnt) {
      return false;
   }

#ifdef DEBUG_PASTEBOARD
   qDebug("PasteBoard: hasOSType [%c%c%c%c]", (c_flavor >> 24) & 0xFF, (c_flavor >> 16) & 0xFF,
      (c_flavor >> 8) & 0xFF, (c_flavor >> 0) & 0xFF);
#endif
   for (uint index = 1; index <= cnt; ++index) {

      PasteboardItemID id;
      if (PasteboardGetItemIdentifier(paste, index, &id) != noErr) {
         return false;
      }

      QCFType<CFArrayRef> types;
      if (PasteboardCopyItemFlavors(paste, id, &types ) != noErr) {
         return false;
      }

      const int type_count = CFArrayGetCount(types);
      for (int i = 0; i < type_count; ++i) {
         CFStringRef flavor = (CFStringRef)CFArrayGetValueAtIndex(types, i);
         CFStringRef preferredTag = UTTypeCopyPreferredTagWithClass(flavor, kUTTagClassOSType);
         const int os_flavor = UTGetOSTypeFromString(preferredTag);
         if (preferredTag) {
            CFRelease(preferredTag);
         }
         if (os_flavor == c_flavor) {
#ifdef DEBUG_PASTEBOARD
            qDebug("  - Found!");
#endif
            return true;
         }
      }
   }
#ifdef DEBUG_PASTEBOARD
   qDebug("  - NotFound!");
#endif
   return false;
}

bool QMacPasteboard::hasFlavor(QString c_flavor) const
{
   if (!paste) {
      return false;
   }

   sync();

   ItemCount cnt = 0;
   if (PasteboardGetItemCount(paste, &cnt) || !cnt) {
      return false;
   }

#ifdef DEBUG_PASTEBOARD
   qDebug("PasteBoard: hasFlavor [%s]", qPrintable(c_flavor));
#endif
   for (uint index = 1; index <= cnt; ++index) {

      PasteboardItemID id;
      if (PasteboardGetItemIdentifier(paste, index, &id) != noErr) {
         return false;
      }

      PasteboardFlavorFlags flags;
      if (PasteboardGetItemFlavorFlags(paste, id, QCFString(c_flavor), &flags) == noErr) {
#ifdef DEBUG_PASTEBOARD
         qDebug("  - Found!");
#endif
         return true;
      }
   }
#ifdef DEBUG_PASTEBOARD
   qDebug("  - NotFound!");
#endif
   return false;
}

class QMacPasteboardMimeSource : public QMimeData
{
   const QMacPasteboard *paste;
 public:
   QMacPasteboardMimeSource(const QMacPasteboard *p) : QMimeData(), paste(p) { }
   ~QMacPasteboardMimeSource() { }
   virtual QStringList formats() const {
      return paste->formats();
   }
   virtual QVariant retrieveData(const QString &format, QVariant::Type type) const {
      return paste->retrieveData(format, type);
   }
};

QMimeData
*QMacPasteboard::mimeData() const
{
   if (!mime) {
      mac_mime_source = true;
      mime = new QMacPasteboardMimeSource(this);

   }
   return mime;
}

void QMacPasteboard::setMimeData(QMimeData *mime_src, DataRequestType dataRequestType)
{
   if (!paste) {
      return;
   }

   if (mime == mime_src || (!mime_src && mime && mac_mime_source)) {
      return;
   }
   mac_mime_source = false;
   delete mime;
   mime = mime_src;

   QList<QMacInternalPasteboardMime *> availableConverters = QMacInternalPasteboardMime::all(mime_type);
   if (mime != 0) {
      clear_helper();
      QStringList formats = mime_src->formats();

      // QMimeData sub classes reimplementing the formats() might not expose the
      // temporary "application/x-qt-mime-type-name" mimetype. So check the existence
      // of this mime type while doing drag and drop.
      QString dummyMimeType(QLatin1String("application/x-qt-mime-type-name"));
      if (!formats.contains(dummyMimeType)) {
         QByteArray dummyType = mime_src->data(dummyMimeType);
         if (!dummyType.isEmpty()) {
            formats.append(dummyMimeType);
         }
      }
      for (int f = 0; f < formats.size(); ++f) {
         QString mimeType = formats.at(f);

         for (auto it = availableConverters.begin(); it != availableConverters.end(); ++it) {

            QMacInternalPasteboardMime *c = (*it);
            // Hack: The Rtf handler converts incoming Rtf to Html. We do
            // not want to convert outgoing Html to Rtf but instead keep
            // posting it as Html. Skip the Rtf handler here.
            if (c->convertorName() == QLatin1String("Rtf")) {
               continue;
            }
            QString flavor(c->flavorFor(mimeType));
            if (!flavor.isEmpty()) {
               QMacMimeData *mimeData = static_cast<QMacMimeData *>(mime_src);

               int numItems = c->count(mime_src);
               for (int item = 0; item < numItems; ++item) {
                  const NSInteger itemID = item + 1; //id starts at 1
                  //QMacPasteboard::Promise promise = (dataRequestType == QMacPasteboard::EagerRequest) ?
                  //    QMacPasteboard::Promise::eagerPromise(itemID, c, mimeType, mimeData, item) :
                  //    QMacPasteboard::Promise::lazyPromise(itemID, c, mimeType, mimeData, item);

                  QMacPasteboard::Promise promise(itemID, c, mimeType, mimeData, item, dataRequestType);
                  promises.append(promise);
                  PasteboardPutItemFlavor(paste, reinterpret_cast<PasteboardItemID>(itemID), QCFString(flavor), 0, kPasteboardFlavorNoFlags);
#ifdef DEBUG_PASTEBOARD
                  qDebug(" -  adding %d %s [%s] <%s> [%d]",
                     itemID, qPrintable(mimeType), qPrintable(flavor), qPrintable(c->convertorName()), item);
#endif
               }
            }
         }
      }
   }
}

QStringList QMacPasteboard::formats() const
{
   if (!paste) {
      return QStringList();
   }

   sync();

   QStringList ret;
   ItemCount cnt = 0;
   if (PasteboardGetItemCount(paste, &cnt) || !cnt) {
      return ret;
   }

#ifdef DEBUG_PASTEBOARD
   qDebug("PasteBoard: Formats [%d]", (int)cnt);
#endif
   for (uint index = 1; index <= cnt; ++index) {

      PasteboardItemID id;
      if (PasteboardGetItemIdentifier(paste, index, &id) != noErr) {
         continue;
      }

      QCFType<CFArrayRef> types;
      if (PasteboardCopyItemFlavors(paste, id, &types ) != noErr) {
         continue;
      }

      const int type_count = CFArrayGetCount(types);
      for (int i = 0; i < type_count; ++i) {
         const QString flavor = QCFString::toQString((CFStringRef)CFArrayGetValueAtIndex(types, i));
#ifdef DEBUG_PASTEBOARD
         qDebug(" -%s", qPrintable(QString(flavor)));
#endif
         QString mimeType = QMacInternalPasteboardMime::flavorToMime(mime_type, flavor);
         if (!mimeType.isEmpty() && !ret.contains(mimeType)) {
#ifdef DEBUG_PASTEBOARD
            qDebug("   -<%d> %s [%s]", ret.size(), qPrintable(mimeType), qPrintable(QString(flavor)));
#endif
            ret << mimeType;
         }
      }
   }
   return ret;
}

bool QMacPasteboard::hasFormat(const QString &format) const
{
   if (!paste) {
      return false;
   }

   sync();

   ItemCount cnt = 0;
   if (PasteboardGetItemCount(paste, &cnt) || !cnt) {
      return false;
   }

#ifdef DEBUG_PASTEBOARD
   qDebug("PasteBoard: hasFormat [%s]", qPrintable(format));
#endif
   for (uint index = 1; index <= cnt; ++index) {

      PasteboardItemID id;
      if (PasteboardGetItemIdentifier(paste, index, &id) != noErr) {
         continue;
      }

      QCFType<CFArrayRef> types;
      if (PasteboardCopyItemFlavors(paste, id, &types ) != noErr) {
         continue;
      }

      const int type_count = CFArrayGetCount(types);
      for (int i = 0; i < type_count; ++i) {
         const QString flavor = QCFString::toQString((CFStringRef)CFArrayGetValueAtIndex(types, i));
#ifdef DEBUG_PASTEBOARD
         qDebug(" -%s [0x%x]", qPrintable(QString(flavor)), mime_type);
#endif
         QString mimeType = QMacInternalPasteboardMime::flavorToMime(mime_type, flavor);
#ifdef DEBUG_PASTEBOARD
         if (!mimeType.isEmpty()) {
            qDebug("   - %s", qPrintable(mimeType));
         }
#endif
         if (mimeType == format) {
            return true;
         }
      }
   }
   return false;
}

QVariant QMacPasteboard::retrieveData(const QString &format, QVariant::Type) const
{
   if (!paste) {
      return QVariant();
   }

   sync();

   ItemCount cnt = 0;
   if (PasteboardGetItemCount(paste, &cnt) || !cnt) {
      return QByteArray();
   }

#ifdef DEBUG_PASTEBOARD
   qDebug("Pasteboard: retrieveData [%s]", qPrintable(format));
#endif
   const QList<QMacInternalPasteboardMime *> mimes = QMacInternalPasteboardMime::all(mime_type);
   for (int mime = 0; mime < mimes.size(); ++mime) {
      QMacInternalPasteboardMime *c = mimes.at(mime);
      QString c_flavor = c->flavorFor(format);
      if (!c_flavor.isEmpty()) {
         // Handle text/plain a little differently. Try handling Unicode first.
         bool checkForUtf16 = (c_flavor == QLatin1String("com.apple.traditional-mac-plain-text")
               || c_flavor == QLatin1String("public.utf8-plain-text"));
         if (checkForUtf16 || c_flavor == QLatin1String("public.utf16-plain-text")) {
            // Try to get the NSStringPboardType from NSPasteboard, newlines are mapped
            // correctly (as '\n') in this data. The 'public.utf16-plain-text' type
            // usually maps newlines to '\r' instead.
            QString str = qt_mac_get_pasteboardString(paste);
            if (!str.isEmpty()) {
               return str;
            }
         }
         if (checkForUtf16 && hasFlavor(QLatin1String("public.utf16-plain-text"))) {
            c_flavor = QLatin1String("public.utf16-plain-text");
         }

         QVariant ret;
         QList<QByteArray> retList;
         for (uint index = 1; index <= cnt; ++index) {
            PasteboardItemID id;
            if (PasteboardGetItemIdentifier(paste, index, &id) != noErr) {
               continue;
            }

            QCFType<CFArrayRef> types;
            if (PasteboardCopyItemFlavors(paste, id, &types ) != noErr) {
               continue;
            }

            const int type_count = CFArrayGetCount(types);
            for (int i = 0; i < type_count; ++i) {
               CFStringRef flavor = static_cast<CFStringRef>(CFArrayGetValueAtIndex(types, i));
               if (c_flavor == QCFString::toQString(flavor)) {
                  QCFType<CFDataRef> macBuffer;
                  if (PasteboardCopyItemFlavorData(paste, id, flavor, &macBuffer) == noErr) {
                     QByteArray buffer((const char *)CFDataGetBytePtr(macBuffer), CFDataGetLength(macBuffer));
                     if (!buffer.isEmpty()) {
#ifdef DEBUG_PASTEBOARD
                        qDebug("  - %s [%s] (%s)", qPrintable(format), qPrintable(QCFString::toQString(flavor)), qPrintable(c->convertorName()));
#endif
                        buffer.detach(); //detach since we release the macBuffer
                        retList.append(buffer);
                        break; //skip to next element
                     }
                  }
               } else {
#ifdef DEBUG_PASTEBOARD
                  qDebug("  - NoMatch %s [%s] (%s)", qPrintable(c_flavor), qPrintable(QCFString::toQString(flavor)), qPrintable(c->convertorName()));
#endif
               }
            }
         }

         if (!retList.isEmpty()) {
            ret = c->convertToMime(format, retList, c_flavor);
            return ret;
         }
      }
   }
   return QVariant();
}

void QMacPasteboard::clear_helper()
{
   if (paste) {
      PasteboardClear(paste);
   }
   promises.clear();
}

void QMacPasteboard::clear()
{
#ifdef DEBUG_PASTEBOARD
   qDebug("PasteBoard: clear!");
#endif
   clear_helper();
}

bool QMacPasteboard::sync() const
{
   if (!paste) {
      return false;
   }
   const bool fromGlobal = PasteboardSynchronize(paste) & kPasteboardModified;

   if (fromGlobal) {
      const_cast<QMacPasteboard *>(this)->setMimeData(0);
   }

#ifdef DEBUG_PASTEBOARD
   if (fromGlobal) {
      qDebug("Pasteboard: Synchronize!");
   }
#endif
   return fromGlobal;
}


QString qt_mac_get_pasteboardString(PasteboardRef paste)
{
   QMacAutoReleasePool pool;
   NSPasteboard *pb = nil;
   CFStringRef pbname;
   if (PasteboardCopyName(paste, &pbname) == noErr) {
      pb = [NSPasteboard pasteboardWithName: const_cast<NSString *>(reinterpret_cast<const NSString *>(pbname))];
      CFRelease(pbname);
   } else {
      pb = [NSPasteboard generalPasteboard];
   }
   if (pb) {
      NSString *text = [pb stringForType: NSStringPboardType];
      if (text) {
         return QCFString::toQString(text);
      }
   }
   return QString();
}

