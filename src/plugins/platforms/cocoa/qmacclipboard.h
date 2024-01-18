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

#ifndef QMACCLIPBOARD_H
#define QMACCLIPBOARD_H

#include <qmacmime_p.h>

#undef slots
#import <Cocoa/Cocoa.h>

class QMacMimeData;

class QMacPasteboard
{
 public:
   enum DataRequestType { EagerRequest, LazyRequest };

 private:
   struct Promise {
      Promise()
         : itemId(0), convertor(nullptr)
      {
      }

      static Promise eagerPromise(int itemId, QMacInternalPasteboardMime *c, QString m, QMacMimeData *d, int o = 0);
      static Promise lazyPromise(int itemId, QMacInternalPasteboardMime *c, QString m, QMacMimeData *d, int o = 0);
      Promise(int itemId, QMacInternalPasteboardMime *c, QString m, QMacMimeData *md, int o, DataRequestType drt);

      int itemId, offset;
      QMacInternalPasteboardMime *convertor;
      QString mime;
      QPointer<QMacMimeData> mimeData;
      QVariant variantData;
      DataRequestType dataRequestType;
   };

   QList<Promise> promises;

   PasteboardRef paste;
   uchar mime_type;
   mutable QPointer<QMimeData> mime;
   mutable bool mac_mime_source;
   bool resolvingBeforeDestruction;
   static OSStatus promiseKeeper(PasteboardRef, PasteboardItemID, CFStringRef, void *);
   void clear_helper();

 public:
   QMacPasteboard(PasteboardRef p, uchar mime_type = 0);
   QMacPasteboard(uchar mime_type);
   QMacPasteboard(CFStringRef name = nullptr, uchar mime_type = 0);

   ~QMacPasteboard();

   bool hasFlavor(QString flavor) const;
   bool hasOSType(int c_flavor) const;

   PasteboardRef pasteBoard() const;
   QMimeData *mimeData() const;

   void setMimeData(QMimeData *mime, DataRequestType dataRequestType = EagerRequest);

   QStringList formats() const;
   bool hasFormat(const QString &format) const;
   QVariant retrieveData(const QString &format, QVariant::Type) const;

   void clear();
   bool sync() const;
};

QString qt_mac_get_pasteboardString(PasteboardRef paste);

#endif
